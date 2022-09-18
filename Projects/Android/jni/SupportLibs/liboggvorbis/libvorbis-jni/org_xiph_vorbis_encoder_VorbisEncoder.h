#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vorbis/vorbisenc.h>
#include <android/log.h>

#ifndef _Included_org_xiph_vorbis_encoder_VorbisEncoder
#define _Included_org_xiph_vorbis_encoder_VorbisEncoder
#ifdef __cplusplus
extern "C" {
#endif

//Starts the encode feed
void startEncodeFeed(JNIEnv *env, jobject *vorbisDataFeed, jmethodID* startMethodId);

//Stops the vorbis data feed
void stopEncodeFeed(JNIEnv *env, jobject* vorbisDataFeed, jmethodID* stopMethodId);

//Reads pcm data from the jni callback
long readPCMDataFromEncoderDataFeed(JNIEnv *env, jobject* encoderDataFeed, jmethodID* readPCMDataMethodId, char* buffer, int length, jbyteArray* jByteArrayBuffer);

//Writes the vorbis data to the Java layer
int writeVorbisDataToEncoderDataFeed(JNIEnv *env, jobject* encoderDataFeed, jmethodID* writeVorbisDataMethodId, char* buffer, int bytes, jbyteArray* jByteArrayWriteBuffer);

//Method to start encoding
int startEncoding(JNIEnv *env, jclass *cls_ptr, jlong *sampleRate_ptr, jlong *channels_ptr, jfloat *quality_ptr, jlong *bitrate_ptr, jobject *encoderDataFeed_ptr, int type);

//jni method for encoding with quality
JNIEXPORT int JNICALL Java_org_xiph_vorbis_encoder_VorbisEncoder_startEncodingWithQuality
(JNIEnv *env, jclass cls, jlong sampleRate, jlong channels, jfloat quality, jobject encoderDataFeed);

//jni method for encoding with bitrate
JNIEXPORT int JNICALL Java_org_xiph_vorbis_encoder_VorbisEncoder_startEncodingWithBitrate
(JNIEnv *env, jclass cls, jlong sampleRate, jlong channels, jlong bitrate, jobject encoderDataFeed);
#ifdef __cplusplus
}
#endif
#endif