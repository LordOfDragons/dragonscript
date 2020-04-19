#ifdef ANDROID

#if 0

#include <string.h>
#include <jni.h>

#include "../libdscript.h"
#include "../scriptengine/dsEngine.h"

extern "C" {

JNIEXPORT jstring JNICALL
Java_ch_rptd_DScriptJni_test( JNIEnv *env, jobject myself ){
	return env->NewStringUTF( "DScript called from JNI." );
}

JNIEXPORT jlong JNICALL
Java_ch_rptd_DScriptJni_dsEngineNew( JNIEnv *env, jobject myself ){
	dsEngine * const engine = new dsEngine;
	engine->AddDefaultPackageSources();
	return (long)engine;
}

JNIEXPORT void JNICALL
Java_ch_rptd_DScriptJni_dsEngineDelete( JNIEnv *env, jobject myself, jlong nativeData ){
	delete ( dsEngine* )nativeData;
}

JNIEXPORT jlong JNICALL
Java_ch_rptd_DScriptJni_dsEngineGetPackageSourceCount( JNIEnv *env, jobject myself, jlong nativeData ){
	const dsEngine &engine = *( ( dsEngine* )nativeData );
	return engine.GetPackageSourceCount();
}

}

#endif

#endif
