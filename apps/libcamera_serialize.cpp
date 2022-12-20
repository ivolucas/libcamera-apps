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
#include "core/frame_info.hpp"
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


static std::string generate_filename(SerializeOptions const *options)
{
	if(options->output == "-")
		return options->output;
	char filename[128];
	std::string folder = options->output; // sometimes "output" is used as a folder name
	if (!folder.empty() && folder.back() != '/')
		folder += "/";
	if (options->datetime)
	{
		std::time_t raw_time;
		std::time(&raw_time);
		char time_string[32];
		std::tm *time_info = std::localtime(&raw_time);
		std::strftime(time_string, sizeof(time_string), "%m%d%H%M%S", time_info);
		snprintf(filename, sizeof(filename), "%s%s.%s", folder.c_str(), time_string, options->encoding.c_str());
	}
	else if (options->timestamp)
		snprintf(filename, sizeof(filename), "%s%u.%s", folder.c_str(), (unsigned)time(NULL),
				 options->encoding.c_str());
	else
		snprintf(filename, sizeof(filename), options->output.c_str(), options->framestart);
	filename[sizeof(filename) - 1] = 0;
	return std::string(filename);
}



static void write_buffer(std::vector<libcamera::Span<uint8_t>> const &mem, StreamInfo const &info,FrameInfo const &frameInfo, std::string const &filename)
{
	if (mem.size() != 1)
		throw std::runtime_error("incorrect number of planes in YUV420 data");
	FILE *fp = filename == "-" ? stdout : fopen(filename.c_str(), "w");
	if (!fp)
		throw std::runtime_error("failed to open file " + filename);
	try{
		if(frameInfo.focus>0)
		LOG(2, "Save image width=" << info.width << "\theight=" << info.height << "\tstride="<< info.stride << "\tpixel_format="<< info.pixel_format <<"\tsize="<< mem[0].size() << "\tframeInfo.focus=" << frameInfo.focus);
		fprintf(fp, 
			"{ \"width\":%d, \"height\":%d, \"stride\":%d, \"pixel_format\":\"%s\", \"buffer_size\":%d",
			 info.width, info.height, info.stride,info.pixel_format.toString().c_str(), (int)mem[0].size());
		fprintf(fp, ", \"frame_info\":{\"frame\":%d, \"fps\":%.2f, \"exposure_time\":%.2f, \"analogue_gain\":%.2f, \"digital_gain\":%.2f, \"red_colour_gain\":%.2f, \"blue_colour_gain\":%.2f, \"focus\":%.2f, \"aelock\":%s }",
			frameInfo.sequence,frameInfo.fps,frameInfo.exposure_time,frameInfo.analogue_gain,frameInfo.digital_gain,frameInfo.colour_gains[0],frameInfo.colour_gains[1],frameInfo.focus, frameInfo.aelock ? "true" : "false");
		fprintf(fp, "}\n");
		fflush(fp);
		if (fwrite(mem[0].data(), mem[0].size(), 1, fp) != 1)
			throw std::runtime_error("failed to write output bytes");
		fflush(fp);
	}
	catch (std::exception const &e)
	{
		if (fp != stdout)
			fclose(fp);
		throw;
	}

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
	int lens_position = 400;
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

		FrameInfo frame_info(completed_request->metadata);
		frame_info.fps = completed_request->framerate;
		frame_info.sequence = completed_request->sequence;
		if (app.ViewfinderStream())
		{
			auto now = std::chrono::high_resolution_clock::now();
			if (options->timeout && now - start_time > std::chrono::milliseconds(options->timeout))
				return;

			StreamInfo info;
			libcamera::Stream *stream = app.ViewfinderStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);
			write_buffer(mem, info, frame_info, generate_filename(options));
			app.ShowPreview(completed_request, stream);
		}
		// In still capture mode, save a jpeg and go back to preview.
		else if (app.VideoStream())
		{
			StreamInfo info;
			libcamera::Stream *stream = app.VideoStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);
			write_buffer(mem, info, frame_info, generate_filename(options));
			
		}

		int key = get_key_or_signal(options, p);

		switch (key)
		{
		case 'x':
		case 'X':
			LOG(1, "Exiting");
			return;
		case 'f':
		case 'F':
		{
			LOG(1, "Focus Trigger");
			libcamera::ControlList controls;
			controls.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerStart);
			app.SetControls(controls);

			break;
		}
		case 'M':
		case 'm':
		{
			LOG(1, "Manul Focus");
			libcamera::ControlList controls;
			controls.set(libcamera::controls::AfMode, libcamera::controls::AfModeManual);
			app.SetControls(controls);
			break;
		}
		case 'A':
		case 'a':
		{
			LOG(1, "Auto Focus");
			libcamera::ControlList controls;
			controls.set(libcamera::controls::AfMode, libcamera::controls::AfModeAuto);
			controls.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerStart);
			app.SetControls(controls);
			break;
		}
		case 'C':
		case 'c':
		{
			LOG(1, "Continuous Auto Focus");
			libcamera::ControlList controls;
			controls.set(libcamera::controls::AfMode, libcamera::controls::AfModeContinuous);
			controls.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerStart);
			app.SetControls(controls);
			break;
		}

		case '+':
		{
			libcamera::ControlList controls;
			lens_position +=20;
			if(lens_position>1024)
				lens_position= 1024; 
			LOG(2, "+ LensPosition :"<< lens_position);
			controls.set(libcamera::controls::LensPosition, lens_position);
			app.SetControls(controls);
			break;
		}
		case '-':
		{
			libcamera::ControlList controls;
			lens_position -=20;
			if(lens_position<0)
				lens_position= 0; 
			LOG(1, "- LensPosition :"<< lens_position);
			controls.set(libcamera::controls::LensPosition, lens_position);
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
