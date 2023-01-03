
#include "jni.h"
#include "libcamera/control_ids.h"
#include <chrono>

#include "core/frame_info.hpp"
#include "core/libcamera_app.hpp"
#include "core/options.hpp"
#include "image/image.hpp"
#include "output/output.hpp"

#include "org_libcamera_apps_LibcameraApps.h"
#include <string>

using namespace std::placeholders;
/*
class LibcameraJni : public LibcameraApp
{
public:	
	void ConfigureCustomVideo(int width, int height, int bufferCount){
		LOG(2, "Configuring custom video...");
		GetOptions()->nopreview = true;
		GetOptions()->width=width;
		GetOptions()->height=height;
		GetOptions()->buffer_count=bufferCount;
		ConfigureVideo();
	}

		void SetRoi(int width, int height, int bufferCount){
		LOG(2, "Configuring custom video...");
		GetOptions()->nopreview = true;
		GetOptions()->width=width;
		GetOptions()->height=height;
		GetOptions()->buffer_count=bufferCount;
		ConfigureVideo();
	}

};
*/

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_init(JNIEnv *env, jobject libcameraApp)
{
	jclass jclassLibcameraApps = env->FindClass("org/libcamera/apps/LibcameraApps");
	jfieldID jfield_pointer = env->GetFieldID(jclassLibcameraApps, "pointer", "J");
	LibcameraApp *app = new LibcameraApp();
	//force no preview
	app->GetOptions()->nopreview = true;
	env->SetLongField(libcameraApp, jfield_pointer, (long)app);
}

LibcameraApp *getLibcameraApp(JNIEnv *env, jobject libcameraApp)
{
	jclass jclassLibcameraApps = env->FindClass("org/libcamera/apps/LibcameraApps");
	jfieldID jfield_pointer = env->GetFieldID(jclassLibcameraApps, "pointer", "J");
	LibcameraApp *app = (LibcameraApp *)env->GetLongField(libcameraApp, jfield_pointer);
	return app;
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    setOptions
 * Signature: ([Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_setOptions(JNIEnv *env, jobject jc,
																			jobjectArray elements)
{
	try
	{
		LibcameraApp *app = getLibcameraApp(env, jc);
		jint nOptions = env->GetArrayLength(elements);
		char **argOptions = new char *[nOptions];
		jboolean result;
		int i;
		for (i = 0; i < nOptions; ++i)
		{
			jstring s = (jstring)env->GetObjectArrayElement(elements, i);
			argOptions[i] = (char *)env->GetStringUTFChars(s, NULL);
		}
		Options *options = app->GetOptions();
		result = options->Parse(nOptions, argOptions);
		//force no preview
		options->nopreview = true;
		//release
		for (i = 0; i < nOptions; ++i)
		{
			jstring s = (jstring)env->GetObjectArrayElement(elements, i);
			env->ReleaseStringUTFChars(s, argOptions[i]);
		}
		delete[] argOptions;
		return result;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    openCamera
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_openCamera(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->OpenCamera();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    startCamera
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_startCamera(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->StartCamera();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    stopCamera
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_stopCamera(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->StopCamera();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    closeCamera
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_closeCamera(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->CloseCamera();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    configureViewfinder
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureViewfinder(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->ConfigureViewfinder();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    configureStill
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureStill__(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->ConfigureStill();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    configureVideo
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureVideo__(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->ConfigureVideo();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    configureStill
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureStill__I(JNIEnv *env, jobject jc, jint flags)
{
	try
	{
		getLibcameraApp(env, jc)->ConfigureStill(flags);
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    configureVideo
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureVideo__I(JNIEnv *env, jobject jc, jint flags)
{
	try
	{
		getLibcameraApp(env, jc)->ConfigureVideo(flags);
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    teardown
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_teardown(JNIEnv *env, jobject jc)
{
	try
	{
		getLibcameraApp(env, jc)->Teardown();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

static void write_buffer(std::vector<libcamera::Span<uint8_t>> const &mem, StreamInfo const &info,
						 FrameInfo const &frameInfo, JNIEnv *env, jobject jObjectFrame)
{
	jclass jclassFrame = env->FindClass("org/libcamera/apps/Frame");
	jclass jclassFrameInfo = env->FindClass("org/libcamera/apps/FrameInfo");
	jfieldID jfieldFrame_width = env->GetFieldID(jclassFrame, "width", "I");
	jfieldID jfieldFrame_height = env->GetFieldID(jclassFrame, "height", "I");
	jfieldID jfieldFrame_stride = env->GetFieldID(jclassFrame, "stride", "I");
	jfieldID jfieldFrame_pixelFormat = env->GetFieldID(jclassFrame, "pixelFormat", "Ljava/lang/String;");
	jfieldID jfieldFrame_bufferSize = env->GetFieldID(jclassFrame, "bufferSize", "I");
	jfieldID jfieldFrame_buffer = env->GetFieldID(jclassFrame, "buffer", "[B");
	jfieldID jfieldFrame_frameInfo = env->GetFieldID(jclassFrame, "frameInfo", "Lorg/libcamera/apps/FrameInfo;");
	jmethodID jclassFrameInfo_new = env->GetMethodID(jclassFrameInfo, "<init>", "()V");
	jfieldID jfieldFrameInfo_frame = env->GetFieldID(jclassFrameInfo, "frame", "I");
	jfieldID jfieldFrameInfo_fps = env->GetFieldID(jclassFrameInfo, "fps", "F");
	jfieldID jfieldFrameInfo_exposureTime = env->GetFieldID(jclassFrameInfo, "exposureTime", "F");
	jfieldID jfieldFrameInfo_analogueGain = env->GetFieldID(jclassFrameInfo, "analogueGain", "F");
	jfieldID jfieldFrameInfo_digitalGain = env->GetFieldID(jclassFrameInfo, "digitalGain", "F");
	jfieldID jfieldFrameInfo_redColourGain = env->GetFieldID(jclassFrameInfo, "redColourGain", "F");
	jfieldID jfieldFrameInfo_blueColourGain = env->GetFieldID(jclassFrameInfo, "blueColourGain", "F");
	jfieldID jfieldFrameInfo_focus = env->GetFieldID(jclassFrameInfo, "focus", "F");
	jfieldID jfieldFrameInfo_aelock = env->GetFieldID(jclassFrameInfo, "aelock", "Z");
	jfieldID jfieldFrameInfo_lensPosition = env->GetFieldID(jclassFrameInfo, "lensPosition", "F");
	jfieldID jfieldFrameInfo_afState = env->GetFieldID(jclassFrameInfo, "afState", "I");

	if (mem.size() != 1)
		throw std::runtime_error("incorrect number of planes in YUV420 data");

	LOG(2, "Save image width=" << info.width << "\theight=" << info.height << "\tstride=" << info.stride
							   << "\tpixel_format=" << info.pixel_format << "\tsize=" << mem[0].size()
							   << "\tframeInfo.focus=" << frameInfo.focus);
	jint size = mem[0].size();

	env->SetIntField(jObjectFrame, jfieldFrame_width, info.width);
	env->SetIntField(jObjectFrame, jfieldFrame_height, info.height);
	env->SetIntField(jObjectFrame, jfieldFrame_stride, info.stride);
	env->SetIntField(jObjectFrame, jfieldFrame_bufferSize, mem[0].size());
	env->SetObjectField(jObjectFrame, jfieldFrame_pixelFormat, env->NewStringUTF(info.pixel_format.toString().c_str()));
	jbyteArray jFrame_buffer = (jbyteArray)env->GetObjectField(jObjectFrame, jfieldFrame_buffer);
	if (jFrame_buffer == NULL)
		jFrame_buffer = env->NewByteArray(size);
	else if (env->GetArrayLength(jFrame_buffer) < size)
	{
		jFrame_buffer = env->NewByteArray(size);
	}
	env->SetByteArrayRegion(jFrame_buffer, 0, size, (const jbyte *)mem[0].data());
	env->SetObjectField(jObjectFrame, jfieldFrame_buffer, jFrame_buffer);

	jobject frameInfoJava = env->GetObjectField(jObjectFrame, jfieldFrame_frameInfo);
	if (frameInfoJava == NULL)
	{
		frameInfoJava = env->NewObject(jclassFrameInfo, jclassFrameInfo_new);
		env->SetObjectField(jObjectFrame, jfieldFrame_frameInfo, frameInfoJava);
	}
	env->SetIntField(frameInfoJava, jfieldFrameInfo_frame, frameInfo.sequence);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_fps, frameInfo.fps);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_exposureTime, frameInfo.exposure_time);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_analogueGain, frameInfo.analogue_gain);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_digitalGain, frameInfo.digital_gain);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_redColourGain, frameInfo.colour_gains[0]);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_blueColourGain, frameInfo.colour_gains[1]);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_focus, frameInfo.focus);
	env->SetBooleanField(frameInfoJava, jfieldFrameInfo_aelock, frameInfo.aelock);
	env->SetFloatField(frameInfoJava, jfieldFrameInfo_lensPosition, frameInfo.lens_position);
	env->SetIntField(frameInfoJava, jfieldFrameInfo_afState, frameInfo.af_state);
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    retrive
 * Signature: (Lorg/libcamera/apps/Frame;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_retrive(JNIEnv *env, jobject jc, jobject jObjectFrame)
{
	try
	{
		LibcameraApp *app = getLibcameraApp(env, jc);
		LibcameraApp::Msg msg = app->Wait();
		if (msg.type == LibcameraApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			app->StopCamera();
			app->StartCamera();
			return JNI_FALSE;
		}
		if (msg.type == LibcameraApp::MsgType::Quit)
			return JNI_FALSE;
		else if (msg.type != LibcameraApp::MsgType::RequestComplete)
			throw std::runtime_error("unrecognised message!");

		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);

		FrameInfo frame_info(completed_request->metadata);
		frame_info.fps = completed_request->framerate;
		frame_info.sequence = completed_request->sequence;
		libcamera::Stream *stream = app->GetMainStream();
		if (stream)
		{
			StreamInfo info = app->GetStreamInfo(stream);
			const std::vector<libcamera::Span<uint8_t>> mem = app->Mmap(completed_request->buffers[stream]);
			write_buffer(mem, info, frame_info, env, jObjectFrame);
			return JNI_TRUE;
		}
		return JNI_FALSE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    configureCustomVideo
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureCustomVideo(JNIEnv *env, jobject jc,
																					  jint width, jint height,
																					  jint bufferCount)
{
	try
	{
		LibcameraApp *app = getLibcameraApp(env, jc);
		Options *options = app->GetOptions();
		options->nopreview = true;
		options->width = width;
		options->height = height;
		options->buffer_count = bufferCount;
		app->ConfigureVideo();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    setRoi
 * Signature: (FFFF)V
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_setRoi(JNIEnv *env, jobject jc, jfloat roi_x, jfloat roi_y,
																	jfloat roi_width, jfloat roi_height)
{
	Options *options = getLibcameraApp(env, jc)->GetOptions();
	options->roi_x = roi_x;
	options->roi_y = roi_y;
	options->roi_width = roi_width;
	options->roi_height = roi_height;
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    resetRoi
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_resetRoi(JNIEnv *env, jobject jc)
{
	Options *options = getLibcameraApp(env, jc)->GetOptions();
	options->roi_x = 0.0;
	options->roi_y = 0.0;
	options->roi_width = 0.0;
	options->roi_height = 0.0;
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    focusManual
 * Signature: ()Z
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_focusManual(JNIEnv *env, jobject jc)
{
	LOG(1, "Manul Focus");
	libcamera::ControlList controls;
	controls.set(libcamera::controls::AfMode, libcamera::controls::AfModeManual);
	getLibcameraApp(env, jc)->SetControls(controls);
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    focusAuto
 * Signature: ()Z
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_focusAuto(JNIEnv *env, jobject jc)
{
	LOG(1, "Auto Focus");
	libcamera::ControlList controls;
	controls.set(libcamera::controls::AfMode, libcamera::controls::AfModeAuto);
	getLibcameraApp(env, jc)->SetControls(controls);
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    focusContinuesAuto
 * Signature: ()Z
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_focusContinuesAuto(JNIEnv *env, jobject jc)
{
	LOG(1, "Continuous Focus");
	libcamera::ControlList controls;
	controls.set(libcamera::controls::AfMode, libcamera::controls::AfModeContinuous);
	getLibcameraApp(env, jc)->SetControls(controls);
}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    triggerFocus
 * Signature: ()Z
 */
JNIEXPORT void JNICALL Java_org_libcamera_apps_LibcameraApps_triggerFocus(JNIEnv *env, jobject jc)
{
	LOG(1, "Trigger Focus");
	libcamera::ControlList controls;
	controls.set(libcamera::controls::AfTrigger, libcamera::controls::AfTriggerStart);
	getLibcameraApp(env, jc)->SetControls(controls);
}