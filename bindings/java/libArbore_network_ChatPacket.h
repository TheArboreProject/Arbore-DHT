/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class libArbore_network_ChatPacket */

#ifndef _Included_libArbore_network_ChatPacket
#define _Included_libArbore_network_ChatPacket
#ifdef __cplusplus
extern "C" {
#endif
#undef libArbore_network_ChatPacket_REQUESTACK
#define libArbore_network_ChatPacket_REQUESTACK 1L
#undef libArbore_network_ChatPacket_ACK
#define libArbore_network_ChatPacket_ACK 2L
#undef libArbore_network_ChatPacket_MUSTROUTE
#define libArbore_network_ChatPacket_MUSTROUTE 4L
/*
 * Class:     libArbore_network_ChatPacket
 * Method:    N_toString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libArbore_network_ChatPacket_N_1toString
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    N_SetFlags
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_libArbore_network_ChatPacket_N_1SetFlags
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    initCppSide
 * Signature: (JJLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_network_ChatPacket_initCppSide
  (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_network_ChatPacket_destroyCppSide
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
