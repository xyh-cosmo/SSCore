/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_southernstars_sscore_JSSTime */

#ifndef _Included_com_southernstars_sscore_JSSTime
#define _Included_com_southernstars_sscore_JSSTime
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    fromCalendarDate
 * Signature: (Lcom/southernstars/sscore/JSSDate;)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_fromCalendarDate
  (JNIEnv *, jclass, jobject);

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    fromSystem
 * Signature: ()Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_fromSystem
  (JNIEnv *, jclass);

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    toCalendarDate
 * Signature: (I)Lcom/southernstars/sscore/JSSDate;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_toCalendarDate
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getWeekday
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSTime_getWeekday
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getDeltaT
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getDeltaT
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getJulianEphemerisDate
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getJulianEphemerisDate
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getSiderealTime
 * Signature: (D)D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getSiderealTime
  (JNIEnv *, jobject, jdouble);

#ifdef __cplusplus
}
#endif
#endif
