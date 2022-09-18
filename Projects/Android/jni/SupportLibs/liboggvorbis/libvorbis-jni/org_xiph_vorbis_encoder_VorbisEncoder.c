/* Reads from a jni pcm callback and  encodes it into a Vorbis bitstream */
#include "org_xiph_vorbis_encoder_VorbisEncoder.h"

/*message codes to send to the java layer*/
#define ERROR_INITIALIZING -44
#define SUCCESS 0

#define WITH_BITRATE 1
#define WITH_QUALITY 2

#define READ 1024

jfloat NO_QUALITY = -1;
jlong NO_BITRATE = -1;

//Starts the encode feed
void startEncodeFeed(JNIEnv *env, jobject *vorbisDataFeed, jmethodID* startMethodId) {
    __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Notifying encode feed to start");

    //Call header start reading method
    (*env)->CallVoidMethod(env, (*vorbisDataFeed), (*startMethodId));
}

//Stops the vorbis data feed
void stopEncodeFeed(JNIEnv *env, jobject* vorbisDataFeed, jmethodID* stopMethodId) {
    (*env)->CallVoidMethod(env, (*vorbisDataFeed), (*stopMethodId));
}

//Reads pcm data from the jni callback
long readPCMDataFromEncoderDataFeed(JNIEnv *env, jobject* encoderDataFeed, jmethodID* readPCMDataMethodId, char* buffer, int length, jbyteArray* jByteArrayBuffer) {
    long readByteCount = (*env)->CallLongMethod(env, (*encoderDataFeed), (*readPCMDataMethodId), (*jByteArrayBuffer), length);

    //Don't bother copying, just delete the reference and return 0
    if(readByteCount == 0) {
        (*env)->DeleteLocalRef(env, (*jByteArrayBuffer));
        return 0;
    }

    //Gets the bytes from the java array and copies them to the pcm buffer
    jbyte* readBytes = (*env)->GetByteArrayElements(env, (*jByteArrayBuffer), NULL);
    memcpy(buffer, readBytes, readByteCount);

    //Clean up memory and return how much data was read
    (*env)->ReleaseByteArrayElements(env, (*jByteArrayBuffer), readBytes, JNI_ABORT);

    return readByteCount;
}

//Writes the vorbis data to the Java layer
int writeVorbisDataToEncoderDataFeed(JNIEnv *env, jobject* encoderDataFeed, jmethodID* writeVorbisDataMethodId, char* buffer, int bytes, jbyteArray* jByteArrayWriteBuffer) {

    //No data to write, just exit
    if(bytes == 0) {
        return 0;
    }

    //Create and copy the contents of what we're writing to the java byte array
    jbyteArray jByteArray = (*env)->NewByteArray(env, bytes);
    (*env)->SetByteArrayRegion(env, (*jByteArrayWriteBuffer), 0, bytes, (jbyte *)buffer);

    //Call the write vorbis data method
    int amountWritten = (*env)->CallIntMethod(env, (*encoderDataFeed), (*writeVorbisDataMethodId), (*jByteArrayWriteBuffer), bytes);

    // clean up
    (*env)->DeleteLocalRef(env, jByteArray);

    //Return the amount that was actually written
    return amountWritten;
}

//Method to start encoding
int startEncoding(JNIEnv *env, jclass *cls_ptr, jlong *sampleRate_ptr, jlong *channels_ptr, jfloat *quality_ptr, jlong *bitrate_ptr, jobject *encoderDataFeed_ptr, int type) {
    //Dereference our variables
    jclass cls = (*cls_ptr);
    jlong sampleRate = (*sampleRate_ptr);
    jlong channels = (*channels_ptr);
    jfloat quality = (*quality_ptr);
    jlong bitrate = (*bitrate_ptr);
    jobject encoderDataFeed = (*encoderDataFeed_ptr);

    //Create our PCM data buffer
    signed char readbuffer[READ*4+44];

    //Create a new java byte array to pass to the data feed method
    jbyteArray jByteArrayBuffer = (*env)->NewByteArray(env, READ*4);

    //Create a new java byte buffer to write to
    jbyteArray jByteArrayWriteBuffer = (*env)->NewByteArray(env, READ*8);

    //Find our java classes we'll be calling
    jclass encoderDataFeedClass = (*env)->FindClass(env, "org/xiph/vorbis/encoder/EncodeFeed");

    //Find our java method id's we'll be calling
    jmethodID writeVorbisDataMethodId = (*env)->GetMethodID(env, encoderDataFeedClass, "writeVorbisData", "([BI)I");
    jmethodID readPCMDataMethodId = (*env)->GetMethodID(env, encoderDataFeedClass, "readPCMData", "([BI)J");
    jmethodID startMethodId = (*env)->GetMethodID(env, encoderDataFeedClass, "start", "()V");
    jmethodID stopMethodId = (*env)->GetMethodID(env, encoderDataFeedClass, "stop", "()V");

    ogg_stream_state os; /* take physical pages, weld into a logical
                            stream of packets */
    ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */

    vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                            settings */
    vorbis_comment   vc; /* struct that stores all the user comments */

    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    int eos=0,ret;
    int i, founddata;

    /********** Encode setup ************/
    __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Setting up encoding");
    vorbis_info_init(&vi);

    /* choose an encoding mode.  A few possibilities commented out, one
       actually used: */

    /*********************************************************************
     Encoding using a VBR quality mode.  The usable range is -.1
     (lowest quality, smallest file) to 1. (highest quality, largest file).
     Example quality mode .4: 44kHz stereo coupled, roughly 128kbps VBR

     ret = vorbis_encode_init_vbr(&vi,2,44100,.4);

     ---------------------------------------------------------------------

     Encoding using an average bitrate mode (ABR).
     example: 44kHz stereo coupled, average 128kbps VBR

     ret = vorbis_encode_init(&vi,2,44100,-1,128000,-1);

     ---------------------------------------------------------------------

     Encode using a quality mode, but select that quality mode by asking for
     an approximate bitrate.  This is not ABR, it is true VBR, but selected
     using the bitrate interface, and then turning bitrate management off:

     ret = ( vorbis_encode_setup_managed(&vi,2,44100,-1,128000,-1) ||
             vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE2_SET,NULL) ||
             vorbis_encode_setup_init(&vi));

     *********************************************************************/
     switch(type) {
        case WITH_BITRATE:
            __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Initializing with %lld channels %lldHz sample rate and %lld bitrate", channels, sampleRate, bitrate);
            ret=vorbis_encode_init(&vi, (long)channels, (long)sampleRate, (long)-1, (long)bitrate, (long)-1);
            break;
        case WITH_QUALITY:
            __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Initializing with %lld channels %lldHz sample rate and %f quality", channels, sampleRate, quality);
            ret=vorbis_encode_init_vbr(&vi, (long)channels, (long)sampleRate, (float)quality);
            break;
        default:
            __android_log_print(ANDROID_LOG_ERROR, "VorbisEncoder", "Failed to initialize");
            stopEncodeFeed(env, &encoderDataFeed, &stopMethodId);
            return ERROR_INITIALIZING;
     }


    /* do not continue if setup failed; this can happen if we ask for a
       mode that libVorbis does not support (eg, too low a bitrate, etc,
       will return 'OV_EIMPL') */

    if(ret) {
      __android_log_print(ANDROID_LOG_ERROR, "VorbisEncoder", "Failed to initialize");
      stopEncodeFeed(env, &encoderDataFeed, &stopMethodId);
      return ERROR_INITIALIZING;
    }

    startEncodeFeed(env, &encoderDataFeed, &startMethodId);

    /* add a comment */
    __android_log_print(ANDROID_LOG_DEBUG, "VorbisEncoder", "Adding comments");
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc,"ENCODER","JNIVorbisEncoder");

    /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd,&vi);
    vorbis_block_init(&vd,&vb);

    /* set up our packet->stream encoder */
    /* pick a random serial number; that way we can more likely build
       chained streams just by concatenation */
    srand(time(NULL));
    ogg_stream_init(&os,rand());

    /* Vorbis streams begin with three headers; the initial header (with
       most of the codec setup parameters) which is mandated by the Ogg
       bitstream spec.  The second header holds any comment fields.  The
       third header holds the bitstream codebook.  We merely need to
       make the headers, then pass them to libvorbis one at a time;
       libvorbis handles the additional Ogg bitstream constraints */

    {
      ogg_packet header;
      ogg_packet header_comm;
      ogg_packet header_code;

      vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
      ogg_stream_packetin(&os,&header); /* automatically placed in its own
                                           page */
      ogg_stream_packetin(&os,&header_comm);
      ogg_stream_packetin(&os,&header_code);

      /* This ensures the actual
       * audio data will start on a new page, as per spec
       */
      __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Writting header");
      while(!eos){
        int result=ogg_stream_flush(&os,&og);
        if(result==0)break;
        writeVorbisDataToEncoderDataFeed(env, &encoderDataFeed, &writeVorbisDataMethodId, og.header, og.header_len, &jByteArrayWriteBuffer);
        writeVorbisDataToEncoderDataFeed(env, &encoderDataFeed, &writeVorbisDataMethodId, og.body, og.body_len, &jByteArrayWriteBuffer);
      }

    }

    __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Starting to read from pcm callback");
    while(!eos){
      long i;
      long bytes = readPCMDataFromEncoderDataFeed(env, &encoderDataFeed, &readPCMDataMethodId, readbuffer, READ*4, &jByteArrayBuffer);

      if(bytes==0){
        /* end of file.  this can be done implicitly in the mainline,
           but it's easier to see here in non-clever fashion.
           Tell the library we're at end of stream so that it can handle
           the last frame and mark end of stream in the output properly */
        __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "End of file");
        vorbis_analysis_wrote(&vd,0);

      }else{
        /* data to encode */

        /* expose the buffer to submit data */
        float **buffer=vorbis_analysis_buffer(&vd,bytes/(2*channels));

        /* uninterleave samples */
        int channel;
        for(i=0;i<bytes/(2*channels);i++) {
            for(channel = 0; channel < channels; channel++) {
                buffer[channel][i]=((readbuffer[i*(2*channels)+(channel*2+1)]<<8)|
                              (0x00ff&(int)readbuffer[i*(2*channels)+(channel*2)]))/32768.f;
            }
        }

        /* tell the library how much we actually submitted */
        vorbis_analysis_wrote(&vd,i);
      }

      /* vorbis does some data preanalysis, then divvies up blocks for
         more involved (potentially parallel) processing.  Get a single
         block for encoding now */
      while(vorbis_analysis_blockout(&vd,&vb)==1){

        /* analysis, assume we want to use bitrate management */
        vorbis_analysis(&vb,NULL);
        vorbis_bitrate_addblock(&vb);

        while(vorbis_bitrate_flushpacket(&vd,&op)){

          /* weld the packet into the bitstream */
          ogg_stream_packetin(&os,&op);

          /* write out pages (if any) */
          while(!eos){
            int result=ogg_stream_pageout(&os,&og);
            if(result==0)break;
            writeVorbisDataToEncoderDataFeed(env, &encoderDataFeed, &writeVorbisDataMethodId, og.header, og.header_len, &jByteArrayWriteBuffer);
            writeVorbisDataToEncoderDataFeed(env, &encoderDataFeed, &writeVorbisDataMethodId, og.body, og.body_len, &jByteArrayWriteBuffer);

            /* this could be set above, but for illustrative purposes, I do
               it here (to show that vorbis does know where the stream ends) */

            if(ogg_page_eos(&og))eos=1;
          }
        }
      }
    }

    /* clean up and exit.  vorbis_info_clear() must be called last */
    __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Cleaning up encoder");
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    /* ogg_page and ogg_packet structs always point to storage in
       libvorbis.  They're never freed or manipulated directly */
    __android_log_print(ANDROID_LOG_INFO, "VorbisEncoder", "Completed encoding.");
    stopEncodeFeed(env, &encoderDataFeed, &stopMethodId);

    //Clean up encode buffers
    (*env)->DeleteLocalRef(env, jByteArrayBuffer);
    (*env)->DeleteLocalRef(env, jByteArrayWriteBuffer);

    return SUCCESS;
}

//jni method for encoding with quality
JNIEXPORT int JNICALL Java_org_xiph_vorbis_encoder_VorbisEncoder_startEncodingWithQuality
(JNIEnv *env, jclass cls, jlong sampleRate, jlong channels, jfloat quality, jobject encoderDataFeed) {
    startEncoding(env, &cls, &sampleRate, &channels, &quality, &NO_BITRATE, &encoderDataFeed, WITH_QUALITY);
}

//jni method for encoding with bitrate
JNIEXPORT int JNICALL Java_org_xiph_vorbis_encoder_VorbisEncoder_startEncodingWithBitrate
(JNIEnv *env, jclass cls, jlong sampleRate, jlong channels, jlong bitrate, jobject encoderDataFeed) {
    startEncoding(env, &cls, &sampleRate, &channels, &NO_QUALITY, &bitrate, &encoderDataFeed, WITH_BITRATE);
}
