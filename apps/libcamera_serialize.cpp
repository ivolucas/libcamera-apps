/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2021, Raspberry Pi (Trading) Ltd.
 *
 * libcamera_detect.cpp - take pictures when objects are detected
 */

// Example: libcamera-detect --post-process-file object_detect_tf.json --lores-width 400 --lores-height 300 -t 0 --object cat -o cat%03d.jpg

#include <chrono>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/stat.h>

#include "core/libcamera_app.hpp"
#include "core/still_options.hpp"

#include "output/output.hpp"

#include "image/image.hpp"

struct SerializeOptions : public StillOptions
{
	SerializeOptions() : StillOptions()
	{
		using namespace boost::program_options;
		options_.add_options()

			;
	}

	virtual void Print() const override { StillOptions::Print(); }
};

class LibcameraSerializeApp : public LibcameraApp
{
public:
	LibcameraSerializeApp() : LibcameraApp(std::make_unique<SerializeOptions>()) {}
	SerializeOptions *GetOptions() const { return static_cast<SerializeOptions *>(options_.get()); }
};

// Some keypress/signal handling.

static int signal_received;
static void default_signal_handler(int signal_number)
{
	signal_received = signal_number;
	LOG(1, "Received signal " << signal_number);
}

static int get_key_or_signal(StillOptions const *options, pollfd p[1])
{
	int key = 0;
	if (options->keypress)
	{
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
		{
			char *user_string = nullptr;
			size_t len;
			[[maybe_unused]] size_t r = getline(&user_string, &len, stdin);
			key = user_string[0];
		}
	}
	if (options->signal)
	{
		if (signal_received == SIGUSR1)
			key = '\n'; //take full res
		else if (signal_received == SIGUSR2)
			key = 'x'; // exit
		signal_received = 0;
	}
	return key;
}

// The main even loop for the application.

static void event_loop(LibcameraSerializeApp &app)
{
	SerializeOptions *options = app.GetOptions();
	app.OpenCamera();

	// Monitoring for keypresses and signals.
	signal(SIGUSR1, default_signal_handler);
	signal(SIGUSR2, default_signal_handler);
	pollfd p[1] = { { STDIN_FILENO, POLLIN, 0 } };

	app.ConfigureViewfinder();
	app.StartCamera();
	auto start_time = std::chrono::high_resolution_clock::now();

	for (unsigned int count = 0;; count++)
	{
		LibcameraApp::Msg msg = app.Wait();
		if (msg.type == LibcameraApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			app.StopCamera();
			app.StartCamera();
			continue;
		}
		if (msg.type == LibcameraApp::MsgType::Quit)
			return;

		// In viewfinder mode, simply run until the timeout, but do a capture if the object
		// we're looking for is detected.
		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
		if (app.ViewfinderStream())
		{
			auto now = std::chrono::high_resolution_clock::now();
			if (options->timeout && now - start_time > std::chrono::milliseconds(options->timeout))
				return;

			StreamInfo info;
			libcamera::Stream *stream = app.ViewfinderStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);
			fprintf(stdout, "%i,%i,%i\n", info.width, info.height, info.stride);
			fflush(stdout);
			LOG(2, "Save full image width=" << info.width << "height=" << info.height << "stride="<< info.stride);
			yuv_save(mem, info, std::string("/dev/null"), options);
		}
		// In still capture mode, save a jpeg and go back to preview.
		else if (app.VideoStream())
		{
			StreamInfo info;
			libcamera::Stream *stream = app.VideoStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);

			fprintf(stdout, "%i,%i,%i\n", info.width, info.height, info.stride);
			fflush(stdout);
			LOG(2, "Save full image width=" << info.width << "height=" << info.height << "stride="<< info.stride);
			yuv_save(mem, info, std::string("/dev/null"), options);

		}

		int key = get_key_or_signal(options, p);

		switch (key)
		{
		case 'x':
		case 'X':
			LOG(2, "Exiting");
			return;
		case 'f':
		case 'F':
		{
			LOG(2, "Focus");
			libcamera::ControlList controls;
			controls.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerStart);
			app.SetControls(controls);

			break;
		}
		case '1':
		{
			LOG(1, "Change for ConfigureViewfinder");
			app.StopCamera();
			app.Teardown();
			app.ConfigureViewfinder();
			app.StartCamera();
			break;
		}
		case '2':
		{
			LOG(1, "Change for ConfigureVideo");
			app.StopCamera();
			app.Teardown();
			app.ConfigureVideo();
			app.StartCamera();
			break;
		}
		}
	}
}

int main(int argc, char *argv[])
{
	try
	{
		LibcameraSerializeApp app;
		SerializeOptions *options = app.GetOptions();
		if (options->Parse(argc, argv))
		{
			if (options->verbose >= 2)
				options->Print();
			if (options->output.empty())
				throw std::runtime_error("output file name required");

			event_loop(app);
		}
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return -1;
	}
	return 0;
}
