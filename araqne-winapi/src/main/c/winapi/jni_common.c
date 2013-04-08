#include <windows.h>
#include "jni_common.h"

jint throwExc(JNIEnv *env, const char* fmt, ...) 
{
	char formatted_string[2048];
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf_s(formatted_string, 2048, fmt, argptr);
	va_end(argptr);
	return (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), formatted_string);
}