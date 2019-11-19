/*
* JNI bindings for the libfuzzy shared library.
* Copyright (C) 2019 Russell Francis
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <jni.h>
#include <stdlib.h>
#include <fuzzy.h>
#include "us_ideasaur_jfuzzy_JFuzzy.h"

// Maintain a global reference to the java.lang.RuntimeException class for generating exceptions.
jclass runtimeExceptionKlass = NULL;
jclass outofmemoryErrorKlass = NULL;

/*
 * This function is called once when the native library is loaded via System.loadLibrary.  It must
 * return the version of JNI needed by the native library.
 *
 * @param vm A reference to the invoking Java Virtual Machine.
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_2) != JNI_OK) {
    return JNI_ERR;
  }

  // RuntimeException
  runtimeExceptionKlass = (*env)->FindClass(env, "java/lang/RuntimeException");
  if ((*env)->ExceptionCheck(env)) {
    return JNI_ERR;
  }
  runtimeExceptionKlass = (jclass) (*env)->NewGlobalRef(env, runtimeExceptionKlass);
  if ((*env)->ExceptionCheck(env)) {
    return JNI_ERR;
  }

  // OutOfMemoryError
  outofmemoryErrorKlass = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
  if ((*env)->ExceptionCheck(env)) {
    return JNI_ERR;
  }
  outofmemoryErrorKlass = (jclass) (*env)->NewGlobalRef(env, outofmemoryErrorKlass);
  if ((*env)->ExceptionCheck(env)) {
    return JNI_ERR;
  }

  return JNI_VERSION_1_1;
}

/*
 * Invoked when the JVM is shutting down, this releases any allocated resources and global
 * references.
 *
 * @param vm A reference to the invoking Java Virtual Machine.
 */
void JNI_OnUnload(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_2) != JNI_OK) {
    return;
  }

  if (runtimeExceptionKlass != NULL) {
    (*env)->DeleteGlobalRef(env, runtimeExceptionKlass);
    runtimeExceptionKlass = NULL;
  }

  if (outofmemoryErrorKlass != NULL) {
    (*env)->DeleteGlobalRef(env, outofmemoryErrorKlass);
    outofmemoryErrorKlass = NULL;
  }
}

/*
 * Allocate memory for maintaining the state of the fuzzy hash computation.  This is necessary
 * if we are going to generate the hash using the update and digest methods.
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @return A pointer to the fuzzy_state structure, casted to a long.
 * @throws An OutOfMemoryError may be thrown if we are unable to allocate sufficient memory to
 * store the fuzzy_state data.
 */
JNIEXPORT jlong JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1new(JNIEnv *env, jobject jfuzzy) {
  struct fuzzy_state *state = fuzzy_new();
  if (state == NULL) {
    (*env)->ThrowNew(env, outofmemoryErrorKlass, "Unable to allocate memory for a new fuzzy_state structure.");
  }
  return state;
}

/*
 * Release memory previously acquired by an invocation of fuzzy_new().
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @param fuzzy_state_ptr A pointer to the fuzzy_state structure, casted to a long.
 */
JNIEXPORT void JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1free(JNIEnv *env, jobject jfuzzy, jlong fuzzy_state_ptr) {
  fuzzy_free((struct fuzzy_state *)fuzzy_state_ptr);
}

/*
 * Returns a value from 0 to 100 indicating the quality of the match between two provided signatures.
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @param sig1 A string representing the ssdeep hash of the first signature.
 * @param sig2 A string representing the ssdeep hash of the second signature.
 * @return A value from 0 to 100 indicating the match score of the two provided signatures.  A match
 * of zero indicates the signatures did not match.  A score of 100 indicates a perfect match.
 * @throws RuntimeException if there is an error comparing the signatures.
 */
JNIEXPORT int JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1compare(JNIEnv *env, jobject jfuzzy, jstring sig1, jstring sig2) {
  const char *cstrSig1 = (*env)->GetStringUTFChars(env, sig1, NULL);
  if ((*env)->ExceptionCheck(env)) {
    return -1;
  }
  const char *cstrSig2 = (*env)->GetStringUTFChars(env, sig2, NULL);
  if ((*env)->ExceptionCheck(env)) {
    (*env)->ReleaseStringUTFChars(env, sig1, cstrSig1);
    return -1;
  }

  int result = fuzzy_compare(cstrSig1, cstrSig2);
  if (result < 0) {
    (*env)->ThrowNew(env, runtimeExceptionKlass, "An exception was encountered comparing these signatures.");
  }

  (*env)->ReleaseStringUTFChars(env, sig2, cstrSig2);
  (*env)->ReleaseStringUTFChars(env, sig1, cstrSig1);

  return result;
}

/*
 * Update the fuzzy_state hash with the additional data provided in the byteArray.
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @param fuzzy_state_ptr A pointer to the fuzzy_state structure, casted to a long.
 * @param byteArray A java byte[] containing data we wish to include in the fuzzy hash.
 * @param length The length from index 0 of bytes in the array we wish to include in the hash.
 */
JNIEXPORT void JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1update(JNIEnv *env, jobject jfuzzy, jlong fuzzy_state_ptr, jbyteArray byteArray, jint length) {
  uint8_t* buf = (uint8_t*)(*env)->GetByteArrayElements(env, byteArray, NULL);
  if ((*env)->ExceptionCheck(env)) {
    return;
  }

  int retval = fuzzy_update((struct fuzzy_state *)fuzzy_state_ptr, buf, (size_t) length);
  if (retval != 0) {
    (*env)->ThrowNew(env, runtimeExceptionKlass, "An exception was encountered updating the fuzzy_state structure.");
  }

  (*env)->ReleaseByteArrayElements(env, byteArray, (jbyte*)buf, 0);
}

/*
 * Generate a string representation of the ssdeep hash for the provided input.
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @param fuzzy_state_ptr A pointer to the fuzzy_state structure, casted to a long.
 * @param flag_eliminate_sequences true if you wish to eliminate sequences of more than 3 identical
 * characters from the generated hash.
 * @param flag_no_truncate true indicates that we should not truncate the second portion of the hash
 * to SPAMSUM_LENGTH/2 characters.
 * @return A Java string representing the ssdeep hash.
 * @throws OutOfMemoryError if there is an issue allocating memory to store the digest.
 * @throws RuntimeException if there are other issues computing the digest.
 */
JNIEXPORT jstring JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1digest(JNIEnv *env, jobject jfuzzy, jlong fuzzy_state_ptr, jboolean flag_eliminate_sequences, jboolean flag_no_truncate) {
  unsigned int flags = 0;
  flags |= flag_eliminate_sequences == JNI_TRUE ? FUZZY_FLAG_ELIMSEQ : 0;
  flags |= flag_no_truncate == JNI_TRUE ? FUZZY_FLAG_NOTRUNC : 0;

  char *result = malloc(FUZZY_MAX_RESULT);
  if (result == NULL) {
    (*env)->ThrowNew(env, outofmemoryErrorKlass, "Unable to allocate memory for the digest.");
    return NULL;
  }

  int retval = fuzzy_digest((struct fuzzy_state *)fuzzy_state_ptr, result, flags);
  if (retval != 0) {
    free(result);
    (*env)->ThrowNew(env, runtimeExceptionKlass, "Unable to generate a digest for the provided input.");
    return NULL;
  }
  jstring hash = (*env)->NewStringUTF(env, result);
  if ((*env)->ExceptionCheck(env)) {
    free(result);
    return NULL;
  }
  free(result);
  return hash;
}

/*
 * Generate an ssdeep hash for the entire content of the provided byte[].
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @param byteArray a reference to a Java byte[] containing the data we wish to generate an ssdeep
 * hash of.
 * @return a Java string representing the hash.
 * @throws OutOfMemoryError if there is an issue allocating memory to store the digest.
 * @throws RuntimeException if there are other issues computing the digest.
 */
JNIEXPORT jstring JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1hash_1buf(JNIEnv *env, jobject jfuzzy, jbyteArray byteArray) {
  jstring hash;

  // Get the length of the byte[]
  jsize length = (*env)->GetArrayLength(env, byteArray);
  if ((*env)->ExceptionCheck(env)) {
    return NULL;
  }

  // Retrieve the bytes of the byte[].
  uint8_t* buf = (uint8_t*)(*env)->GetByteArrayElements(env, byteArray, NULL);
  if ((*env)->ExceptionCheck(env)) {
    return NULL;
  }

  // allocate memory for the resulting hash.
  char *result = malloc(FUZZY_MAX_RESULT);
  if (result == NULL) {
    (*env)->ReleaseByteArrayElements(env, byteArray, (jbyte*)buf, 0);
    (*env)->ThrowNew(env, outofmemoryErrorKlass, "Unable to allocate memory for the digest result.");
    return NULL;
  }

  // generate the hash.
  int retval = fuzzy_hash_buf(buf, length, result);
  if (retval != 0) {
    free(result);
    (*env)->ReleaseByteArrayElements(env, byteArray, (jbyte*)buf, 0);
    (*env)->ThrowNew(env, runtimeExceptionKlass, "Unable to generate a hash for the provided byte[].");
    return NULL;
  }

  // convert the char* into a java string, free intermediate results and return.
  hash = (*env)->NewStringUTF(env, result);
  free(result);
  (*env)->ReleaseByteArrayElements(env, byteArray, (jbyte*)buf, 0);
  return hash;
}

/*
 * Generate an ssdeep hash of the given file.
 *
 * @param env A reference to the JNIEnvironment.
 * @param jfuzzy A reference to the JFuzzy class instance which invoked this method.
 * @param filename A string representing the name of a file to hash.
 * @return a Java string representing the hash.
 * @throws OutOfMemoryError if there is an issue allocating memory to store the digest.
 * @throws RuntimeException if there are other issues computing the digest.
 */
JNIEXPORT jstring JNICALL Java_us_ideasaur_jfuzzy_JFuzzy_fuzzy_1hash_1file(JNIEnv *env, jobject jfuzzy, jstring filename) {
  const char *cstrFilename = (*env)->GetStringUTFChars(env, filename, NULL);
  if ((*env)->ExceptionCheck(env)) {
    return NULL;
  }

  // allocate space for the hash.
  char *result = malloc(FUZZY_MAX_RESULT);
  if (result == NULL) {
    (*env)->ReleaseStringUTFChars(env, filename, cstrFilename);
    (*env)->ThrowNew(env, outofmemoryErrorKlass, "Unable to allocate memory for the digest result.");
    return NULL;
  }

  // generate the hash.
  int retval = fuzzy_hash_filename(cstrFilename, result);
  if (retval != 0) {
    free(result);
    (*env)->ReleaseStringUTFChars(env, filename, cstrFilename);
    (*env)->ThrowNew(env, runtimeExceptionKlass, "An exception was encountered generating the digest.");
    return NULL;
  }

  // convert the hash from a char* into a jstring, free intermediate resources and return the result.
  jstring hash = (*env)->NewStringUTF(env, result);
  free(result);
  (*env)->ReleaseStringUTFChars(env, filename, cstrFilename);
  return hash;
}