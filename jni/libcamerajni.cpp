
#include <chrono>

#include "core/frame_info.hpp"
#include "core/libcamera_app.hpp"
#include "core/options.hpp"
#include "output/output.hpp"
#include "image/image.hpp"

#include "org_libcamera_apps_LibcameraApps.h"

using namespace std::placeholders;

static LibcameraApp app;

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    setOptions
 * Signature: ([Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_setOptions(JNIEnv * env, jclass jc, jobjectArray elements)
{
	try
	{
		jint nOptions = env->GetArrayLength(elements);
		char **argOptions = new char*[nOptions];
		jboolean result;
		int i;
		for(i=0;i<nOptions;++i){
			jstring s = (jstring)env->GetObjectArrayElement( elements,i);
			argOptions[i]=(char*)env->GetStringUTFChars(s,NULL);
		}		
		Options *options = app.GetOptions();
		result = options->Parse(nOptions, argOptions);
		//release
		for(i=0;i<nOptions;++i){
			jstring s =  (jstring)env->GetObjectArrayElement(elements,i);
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
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_openCamera(JNIEnv *, jclass)
{
	try
	{
		app.OpenCamera();
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
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_closeCamera(JNIEnv *, jclass)
{
	try
	{
		app.CloseCamera();
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
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureViewfinder(JNIEnv *, jclass)
{
	try
	{
		app.ConfigureViewfinder();
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
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureStill(JNIEnv *, jclass)
{
	try
	{
		app.ConfigureStill();
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
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_configureVideo(JNIEnv *, jclass)
{
	try
	{
		app.ConfigureVideo();
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
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_teardown(JNIEnv *, jclass)
{
	try
	{
		app.Teardown();
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}

static void write_buffer(std::vector<libcamera::Span<uint8_t>> const &mem, StreamInfo const &info,FrameInfo const &frameInfo,JNIEnv * env, jobject jObjectFrame)
{
	jclass jclassFrame = env->FindClass("org/libcamera/apps/Frame");
	//class jclassFrameInfo = env->FindClass("org/libcamera/apps/FrameInfo");

	jfieldID jfieldFrame_width = env->GetFieldID(jclassFrame,"width","I");
	jfieldID jfieldFrame_height = env->GetFieldID(jclassFrame,"height","I");
	jfieldID jfieldFrame_stride = env->GetFieldID(jclassFrame,"stride","I");
	//jfieldID jfieldFrame_pixelFormat = env->GetFieldID(jclassFrame,"pixelFormat","Ljava/lang/String;");
	jfieldID jfieldFrame_bufferSize = env->GetFieldID(jclassFrame,"bufferSize","I");
	jfieldID jfieldFrame_buffer = env->GetFieldID(jclassFrame,"buffer","[B");
	//jfieldID jfieldFrame_frameInfo = env->GetFieldID(jclassFrame,"frameInfo","Lorg/libcamera/apps/FrameInfo;");
	
	if (mem.size() != 1)
		throw std::runtime_error("incorrect number of planes in YUV420 data");

	
	LOG(2, "Save image width=" << info.width << "\theight=" << info.height << "\tstride="<< info.stride << "\tpixel_format="<< info.pixel_format <<"\tsize="<< mem[0].size() << "\tframeInfo.focus=" << frameInfo.focus);
	jint size = mem[0].size();
	env->SetIntField(jObjectFrame,jfieldFrame_width,info.width);
	env->SetIntField(jObjectFrame,jfieldFrame_height,info.height);
	env->SetIntField(jObjectFrame,jfieldFrame_stride,info.stride);
	env->SetIntField(jObjectFrame,jfieldFrame_bufferSize,mem[0].size());
	jbyteArray jFrame_buffer = (jbyteArray) env->GetObjectField(jObjectFrame,jfieldFrame_buffer);
	if(jFrame_buffer == NULL)
		jFrame_buffer = env->NewByteArray(size);
	else if (env->GetArrayLength(jFrame_buffer)< size){
		jFrame_buffer = env->NewByteArray(size);
	}
	env->SetByteArrayRegion(jFrame_buffer,0,size,(const jbyte*)mem[0].data());
	env->SetObjectField(jObjectFrame,jfieldFrame_buffer,jFrame_buffer);

}

/*
 * Class:     org_libcamera_apps_LibcameraApps
 * Method:    retrive
 * Signature: (Lorg/libcamera/apps/Frame;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_libcamera_apps_LibcameraApps_retrive(JNIEnv * env, jclass, jobject jObjectFrame)
{
	try
	{
		LibcameraApp::Msg msg = app.Wait();
		if (msg.type == LibcameraApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			app.StopCamera();
			app.StartCamera();
			return JNI_FALSE;
		}
		if (msg.type == LibcameraApp::MsgType::Quit)
			return JNI_FALSE;
		else if (msg.type != LibcameraApp::MsgType::RequestComplete)
			throw std::runtime_error("unrecognised message!");

// In viewfinder mode, simply run until the timeout, but do a capture if the object
		// we're looking for is detected.
		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);

		FrameInfo frame_info(completed_request->metadata);
		frame_info.fps = completed_request->framerate;
		frame_info.sequence = completed_request->sequence;
		if (app.ViewfinderStream())
		{
			StreamInfo info;
			libcamera::Stream *stream = app.ViewfinderStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);
			write_buffer(mem, info, frame_info, env, jObjectFrame);
			app.ShowPreview(completed_request, stream);
		}
		// In still capture mode, save a jpeg and go back to preview.
		else if (app.VideoStream())
		{
			StreamInfo info;
			libcamera::Stream *stream = app.VideoStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);
			write_buffer(mem, info, frame_info, env, jObjectFrame);
			
		}else if (app.StillStream())
		{
			StreamInfo info;
			libcamera::Stream *stream = app.StillStream(&info);
			const std::vector<libcamera::Span<uint8_t>> mem = app.Mmap(completed_request->buffers[stream]);
			write_buffer(mem, info, frame_info,env, jObjectFrame);
			
		}
		
		return JNI_TRUE;
	}
	catch (std::exception const &e)
	{
		LOG_ERROR("ERROR: *** " << e.what() << " ***");
		return JNI_FALSE;
	}
}
