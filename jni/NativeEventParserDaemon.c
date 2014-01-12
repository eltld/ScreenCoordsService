#include <jni.h>
#include <android/input.h>
#include "native_parseevent.h"
#include "native_log.h"

enum {
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_DIMEN = 2
};

enum {
	ERR_DEV_RESOLUTION,
	ERR_DEV_DIMEN,
};

/**
 * important! the JNIEnv* is valid only in the current thread, be very very careful!!!
 * TODO: Java_notifyAll will be called back somewhere else, I know it's anti-design-pattern,
 * but too busy to improve. Maybe I'll refactor it someday.
 */
static JavaVM* gJavaVM;
static jobject sSingleton;

static JNIEnv* JNI_GetEnv(JavaVM *vm) {
	JNIEnv* env;
	if ((*gJavaVM)->GetEnv(gJavaVM, (void**) &env, JNI_VERSION_1_6) < 0) {
		if ((*gJavaVM)->AttachCurrentThread(gJavaVM, &env, NULL) < 0) {
			env = NULL;
		}
	}
	return env;
}

static void Java_sync() {
	nativeDebug("JNI::Java_sync");

	jobject thiz = sSingleton;
	if (gJavaVM == NULL || thiz == NULL) {
		return;
	}

	JNIEnv* env = JNI_GetEnv(gJavaVM);

	jclass CLASS_NativeEventParserDaemon = (*env)->GetObjectClass(env, thiz);
	if(CLASS_NativeEventParserDaemon == NULL) {
		LOGW("inner class NativeEventParserDaemon not found");
	}
	jmethodID METHOD_sync = (*env)->GetMethodID(env, CLASS_NativeEventParserDaemon, "sync", "()V");
	if (METHOD_sync == NULL) {
		LOGW("sync() not found");
	}

	nativeDebug("sync...");

	(*env)->CallVoidMethod(env, thiz, METHOD_sync);

	nativeDebug("sync ok.");

	(*env)->DeleteLocalRef(env, CLASS_NativeEventParserDaemon);
	(*env)->DeleteLocalRef(env, METHOD_sync);
	(*env)->DeleteLocalRef(env, env);

	// the thread running in java vm, don't detach; maybe it has side-effect
//	(*gJavaVM)->DetachCurrentThread(gJavaVM);

	nativeDebug("recycle ok.");
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	nativeDebug("JNI_OnLoad");

	gJavaVM = vm;
	return JNI_VERSION_1_2; // use JNI_VERSION_1_2 or later
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	nativeDebug("JNI_OnUnload");

	gJavaVM = NULL;
}

JNIEXPORT jlong JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getDownTime(
		JNIEnv* env, jobject thiz) {
	return nativeGetDownTime();
}

JNIEXPORT jlong JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getEventTime(
		JNIEnv* env, jobject thiz) {
	return nativeGetEventTime();
}

JNIEXPORT jint JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getAction(
		JNIEnv* env, jobject thiz) {
	return nativeGetAction();
}

JNIEXPORT jfloat JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getPrecision
  (JNIEnv* env, jobject thiz, jint axis) {
	return nativeGetPrecision(axis);
}

JNIEXPORT jint JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getPointerCount(
		JNIEnv* env, jobject thiz) {
	return nativeGetPointerCount();
}

JNIEXPORT jintArray JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getPointerIds(
		JNIEnv* env, jobject thiz) {
	const int cnt = nativeGetPointerCount();
	jintArray ids = (*env)->NewIntArray(env, cnt);
	jint* id;

	coords_t* coords = nativeGetPointerCoords();

	int i;
	for(i = 0; i != cnt; ++i) {
		id = (*env)->GetIntArrayElements(env, ids, NULL);
		*id = coords[i].id;
		(*env)->ReleaseIntArrayElements(env, ids, id, NULL);
	}

	return ids;
}

JNIEXPORT jobjectArray JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_getPointerCoords(
		JNIEnv* env, jobject thiz) {
	const int cnt = nativeGetPointerCount();
	coords_t* coords = nativeGetPointerCoords();
	int i;

	jclass CLASS_PointerCoords = (*env)->FindClass(env, "android/view/MotionEvent$PointerCoords");
	if(CLASS_PointerCoords == NULL) {
		LOGW("inner class MotionEvent.PointerCoords not found");
		return NULL;
	}

	jobjectArray arrPointerCoords = (*env)->NewObjectArray(env, cnt, CLASS_PointerCoords, NULL);
	if(arrPointerCoords == NULL) {
		LOGW("cannot new MotionEvent.PointerCoords[]");
		return NULL;
	}

	nativeDebug("new MotionEvent.PointerCoords[%d]", cnt);

	jmethodID METHOD_constructor = (*env)->GetMethodID(env, CLASS_PointerCoords, "<init>", "()V");
	if(METHOD_constructor == NULL) {
		LOGW("default constructor not found");
		return NULL;
	}

	jmethodID METHOD_setAxisValue = (*env)->GetMethodID(env, CLASS_PointerCoords, "setAxisValue", "(IF)V");
	if(METHOD_setAxisValue == NULL) {
		LOGW("setAxisValue(int, float) not found");
		return NULL;
	}

	jobject objPointerCoords;
	for(i = 0; i != cnt; ++i) {
		objPointerCoords = (*env)->NewObject(env, CLASS_PointerCoords, METHOD_constructor);
		if(objPointerCoords == NULL) {
			LOGW("cannot new MotionEvent.PointerCoords()");
			break;
		}

		(*env)->CallVoidMethod(env, objPointerCoords, METHOD_setAxisValue, AXIS_X, coords[i].x);
		(*env)->CallVoidMethod(env, objPointerCoords, METHOD_setAxisValue, AXIS_Y, coords[i].y);

		(*env)->SetObjectArrayElement(env, arrPointerCoords, i, objPointerCoords);

		nativeDebug("processing %d / %d...", (i + 1), cnt);
	}

	nativeDebug("done.");

	return arrPointerCoords;
}

JNIEXPORT void JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_recycle
  (JNIEnv* env, jobject thiz) {
	if(sSingleton != NULL) {
		(*env)->DeleteGlobalRef(env, sSingleton);
		sSingleton = NULL;
	}
}

JNIEXPORT jint JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_init(
		JNIEnv* env, jobject thiz, jintArray resolution) {
	if(resolution == NULL) {
		LOGE("bad device resolution [null]!");
		return ERR_DEV_RESOLUTION;
	}
	if((*env)->GetArrayLength(env, resolution) != AXIS_DIMEN) {
		// screen should be 2-d
		LOGE("bad device resolution");
		return ERR_DEV_DIMEN;
	}

	jint *axsi = (*env)->GetIntArrayElements(env, resolution, NULL);
	nativeSetResolution(axsi[AXIS_X], axsi[AXIS_Y]);
	(*env)->ReleaseIntArrayElements(env, resolution, axsi, NULL);

	nativeDebug("device resolution %dx%d...", axsi[AXIS_Y], axsi[AXIS_X]);

	sSingleton = (*env)->NewGlobalRef(env, thiz);
	if(sSingleton == NULL) {
		LOGW("cannot reference global jobject thiz");
	}

	METHOD_sync = Java_sync;

	nativeDebug("[%p(*env) %p(thiz)] halfway ok.", (*env), sSingleton);

	return native_init();
}

JNIEXPORT jint JNICALL Java_me_wtao_service_ScreenCoordsService_00024NativeEventParserDaemon_doInBackground
  (JNIEnv* env, jobject thiz) {
	return native_getevent();
}
