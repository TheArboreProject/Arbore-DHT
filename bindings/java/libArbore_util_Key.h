/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class libArbore_util_Key */

#ifndef _Included_libArbore_util_Key
#define _Included_libArbore_util_Key
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     libArbore_util_Key
 * Method:    N_toString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libArbore_util_Key_N_1toString
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_util_Key
 * Method:    N_GetRandomKey
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_util_Key_N_1GetRandomKey
  (JNIEnv *, jclass);

/*
 * Class:     libArbore_util_Key
 * Method:    initCppSide
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_util_Key_initCppSide
  (JNIEnv *, jobject);

/*
 * Class:     libArbore_util_Key
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Key_destroyCppSide
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_util_Key
 * Method:    InitRandomNumberGenerator
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Key_InitRandomNumberGenerator
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
