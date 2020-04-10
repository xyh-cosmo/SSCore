/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_southernstars_sscore_JSSMatrix */

#ifndef _Included_com_southernstars_sscore_JSSMatrix
#define _Included_com_southernstars_sscore_JSSMatrix
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    inverse
 * Signature: ()Lcom/southernstars/sscore/JSSMatrix;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_inverse
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    determinant
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSMatrix_determinant
  (JNIEnv *, jobject);

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    multiply
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSVector;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_multiply__Lcom_southernstars_sscore_JSSVector_2
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    multiply
 * Signature: (Lcom/southernstars/sscore/JSSMatrix;)Lcom/southernstars/sscore/JSSMatrix;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_multiply__Lcom_southernstars_sscore_JSSMatrix_2
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    rotate
 * Signature: (ID)Lcom/southernstars/sscore/JSSMatrix;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_rotate
  (JNIEnv *, jobject, jint, jdouble);

#ifdef __cplusplus
}
#endif
#endif
