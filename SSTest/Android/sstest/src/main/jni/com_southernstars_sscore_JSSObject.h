/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_southernstars_sscore_JSSObject */

#ifndef _Included_com_southernstars_sscore_JSSObject
#define _Included_com_southernstars_sscore_JSSObject
#ifdef __cplusplus
extern "C" {
#endif
#undef com_southernstars_sscore_JSSObject_kTypeNonexistent
#define com_southernstars_sscore_JSSObject_kTypeNonexistent 0L
#undef com_southernstars_sscore_JSSObject_kTypePlanet
#define com_southernstars_sscore_JSSObject_kTypePlanet 1L
#undef com_southernstars_sscore_JSSObject_kTypeMoon
#define com_southernstars_sscore_JSSObject_kTypeMoon 2L
#undef com_southernstars_sscore_JSSObject_kTypeAsteroid
#define com_southernstars_sscore_JSSObject_kTypeAsteroid 3L
#undef com_southernstars_sscore_JSSObject_kTypeComet
#define com_southernstars_sscore_JSSObject_kTypeComet 4L
#undef com_southernstars_sscore_JSSObject_kTypeSatellite
#define com_southernstars_sscore_JSSObject_kTypeSatellite 5L
#undef com_southernstars_sscore_JSSObject_kTypeSpacecraft
#define com_southernstars_sscore_JSSObject_kTypeSpacecraft 6L
#undef com_southernstars_sscore_JSSObject_kTypeStar
#define com_southernstars_sscore_JSSObject_kTypeStar 10L
#undef com_southernstars_sscore_JSSObject_kTypeDoubleStar
#define com_southernstars_sscore_JSSObject_kTypeDoubleStar 12L
#undef com_southernstars_sscore_JSSObject_kTypeVariableStar
#define com_southernstars_sscore_JSSObject_kTypeVariableStar 13L
#undef com_southernstars_sscore_JSSObject_kTypeDoubleVariableStar
#define com_southernstars_sscore_JSSObject_kTypeDoubleVariableStar 14L
#undef com_southernstars_sscore_JSSObject_kTypeOpenCluster
#define com_southernstars_sscore_JSSObject_kTypeOpenCluster 20L
#undef com_southernstars_sscore_JSSObject_kTypeGlobularCluster
#define com_southernstars_sscore_JSSObject_kTypeGlobularCluster 21L
#undef com_southernstars_sscore_JSSObject_kTypeBrightNebula
#define com_southernstars_sscore_JSSObject_kTypeBrightNebula 22L
#undef com_southernstars_sscore_JSSObject_kTypeDarkNebula
#define com_southernstars_sscore_JSSObject_kTypeDarkNebula 23L
#undef com_southernstars_sscore_JSSObject_kTypePlanetaryNebula
#define com_southernstars_sscore_JSSObject_kTypePlanetaryNebula 24L
#undef com_southernstars_sscore_JSSObject_kTypeGalaxy
#define com_southernstars_sscore_JSSObject_kTypeGalaxy 25L
#undef com_southernstars_sscore_JSSObject_kTypeConstellation
#define com_southernstars_sscore_JSSObject_kTypeConstellation 30L
#undef com_southernstars_sscore_JSSObject_kTypeAsterism
#define com_southernstars_sscore_JSSObject_kTypeAsterism 31L
/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    typeToCode
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSObject_typeToCode
  (JNIEnv *, jclass, jint);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    codeToType
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObject_codeToType
  (JNIEnv *, jclass, jstring);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getType
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObject_getType
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSObject_getName
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getIdentifier
 * Signature: (I)Lcom/southernstars/sscore/JSSIdentifier;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObject_getIdentifier
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getDirection
 * Signature: ()Lcom/southernstars/sscore/JSSVector;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObject_getDirection
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getDistance
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSObject_getDistance
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getMagnitude
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_com_southernstars_sscore_JSSObject_getMagnitude
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setDirection
 * Signature: (Lcom/southernstars/sscore/JSSVector;)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setDirection
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setDistance
 * Signature: (D)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setDistance
  (JNIEnv *, jobject, jdouble);

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setMagnitude
 * Signature: (F)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setMagnitude
  (JNIEnv *, jobject, jfloat);

#ifdef __cplusplus
}
#endif
#endif