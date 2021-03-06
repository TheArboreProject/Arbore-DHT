/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class libArbore_chimera_Chimera */

#ifndef _Included_libArbore_chimera_Chimera
#define _Included_libArbore_chimera_Chimera
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_getMe
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_N_1getMe
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_getNetwork
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_N_1getNetwork
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_getLeafset
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_N_1getLeafset
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_join
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_libArbore_chimera_Chimera_N_1join
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_route
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_libArbore_chimera_Chimera_N_1route
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    initCppSide
 * Signature: (IJ)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_initCppSide
  (JNIEnv *, jobject, jint, jlong);

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_chimera_Chimera_destroyCppSide
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
