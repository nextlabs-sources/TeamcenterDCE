#include "FMSRMX.h"
#include "RMXInstance.h"
#include <SDLInc.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <inttypes.h>

ISDRmUser* pDRmUser = nullptr;

JNIEXPORT jboolean JNICALL Java_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance_loadDRmUser(JNIEnv *env, jobject jobj) {
	bool result = CRMXInstance::GetInstance().Initialize();

	if (result) {
		pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
		return JNI_TRUE;
	}
	else {
		return JNI_FALSE;
	}
}

// build NXL header will be called once before decrypt partial, so init drm user at this method
JNIEXPORT jboolean JNICALL Java_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance_buildNxlHeader(JNIEnv *env, jobject jobj, jbyteArray header, jlongArray outData) {
	long header_len = env->GetArrayLength(header);
	jlong* arrLens = env->GetLongArrayElements(outData, NULL); // content length, content offset
	unsigned char* chaHeader = as_unsigned_char_array(env, header);

	unsigned int contentOffset = 0;
	int64_t contentLength = 0;

	SDWLResult res = pDRmUser->PDSetupHeader(chaHeader, header_len, &contentLength, &contentOffset);

	if (res.GetCode() == 0) {
		jbyte* arrHeader = env->GetByteArrayElements(as_byte_array(env, chaHeader, header_len), NULL);
		arrLens[0] = contentLength;
		arrLens[1] = contentOffset;

		env->ReleaseByteArrayElements(header, arrHeader, 0);
		env->ReleaseLongArrayElements(outData, arrLens, 0);

		return JNI_TRUE;
	}
	else {
		env->ReleaseLongArrayElements(outData, arrLens, 0);

		return JNI_FALSE;
	}
}

JNIEXPORT jlong JNICALL Java_com_nextlabs_teamcenter_fms_decrypt_segment_client_FMSRMXInstance_decryptPartial(JNIEnv *env, jobject jobj, jbyteArray input, jbyteArray output, jbyteArray header, jlong offset, jlong len) {
	jbyte* input_arr = env->GetByteArrayElements(input, NULL);
	jbyte* output_arr = env->GetByteArrayElements(output, NULL);
	jbyte* header_arr = env->GetByteArrayElements(header, NULL);
	long header_len = env->GetArrayLength(header);
	unsigned char* headerBuffer = as_unsigned_char_array(env, header);

	unsigned char* encryptedBuffer = as_unsigned_char_array(env, input);
	unsigned char* plaintextBuffer = as_unsigned_char_array(env, output);

	long plainReadLen = 0;

	unsigned int contentOffset = 0;
	int64_t contentLength = 0;
	void *contextLocal;

	SDWLResult resHeader = pDRmUser->PDSetupHeaderEx(headerBuffer, header_len, &contentLength, &contentOffset, contextLocal);
	if (resHeader.GetCode() == 0) {
		SDWLResult resDecrypt = pDRmUser->PDDecryptPartialEx(encryptedBuffer, len, (offset - (offset % 512)), plaintextBuffer, &plainReadLen, headerBuffer, header_len, contextLocal);
		pDRmUser->PDTearDownHeaderEx(contextLocal);

		if (resDecrypt.GetCode() == 0) {
			memcpy(output_arr, plaintextBuffer + (offset % 512), len - (offset % 512));
		}
		else {
			memcpy(input_arr, plaintextBuffer, len);
		}
	}
	else {
		memcpy(input_arr, plaintextBuffer, len);
	}

	env->ReleaseByteArrayElements(output, output_arr, 0);
	env->ReleaseByteArrayElements(input, input_arr, 0);
	env->ReleaseByteArrayElements(header, header_arr, 0);
	free(headerBuffer);
	free(encryptedBuffer);
	free(plaintextBuffer);

	return len;
}

jbyteArray as_byte_array(JNIEnv *env, unsigned char* buf, int len) {
	jbyteArray array = env->NewByteArray(len);
	env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
	return array;
}

unsigned char* as_unsigned_char_array(JNIEnv *env, jbyteArray array) {
	int len = env->GetArrayLength(array);
	unsigned char* buf = new unsigned char[len];
	env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
	return buf;
}
