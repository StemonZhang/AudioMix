#include <jni.h>
#include <stdlib.h>
#include <stdio.h>

#include "android/log.h"
#include "com_kaolafm_record_AudioMixerNative.h"
#include "src/MixAudio.h"

#define LOG_TAG "RECSDKAPI"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);

 char* js2c(JNIEnv* env, jstring jstr)
 {
    char* rtn = 0;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, 0);
    if (alen > 0)
    {
      rtn = (char*)malloc(alen + 1);
      memcpy(rtn, ba, alen);
      rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
 }

#define FIX_SIZE 800

int FindStartPosition(char* pStart, unsigned long ulStartPosition,
		unsigned long ulEndPosition,
		unsigned long ulTotalSize)
{
	 ulStartPosition = ((ulStartPosition-FIX_SIZE)>0) ? (ulStartPosition-FIX_SIZE) : 0;
	 ulEndPosition = ((ulEndPosition+FIX_SIZE)<ulTotalSize) ? (ulEndPosition+FIX_SIZE) : ulTotalSize;
     unsigned long ulCurrentPostion = ulStartPosition;
     unsigned char* start = (unsigned char*)pStart+ulStartPosition;
     int iFoundFlag = 0;

     do{
         if(start[0]==0xff&&(start[1]&0xf0)==0xf0&&
            (start[5]&0x1f)==0x1f&&(start[6]&0xfc)==0xfc) {
             iFoundFlag = 1;
             break;
         }
         ulCurrentPostion++;
         start++;
     }while (ulCurrentPostion < ulEndPosition);

     if(iFoundFlag == 0){
         return -1;
     }
     return ulCurrentPostion;
 }

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    PcmMixEncoderInit
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_kaolafm_record_AudioMixerNative_PcmMixEncoderInit
  (JNIEnv *, jobject){
	jint iRet = (jint)PcmMixEncoderInit();
	LOGI("PcmMixEncoderInit return %d", iRet);
    return iRet;
}

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    PcmMixEncoderDeInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_kaolafm_record_AudioMixerNative_PcmMixEncoderDeInit
  (JNIEnv *, jobject){
	PcmMixEncoderDeInit();
	LOGI("PcmMixEncoderDeInit....");
}

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    MusicPcmMixEncode
 * Signature: (II[BI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_kaolafm_record_AudioMixerNative_MusicPcmMixEncode
  (JNIEnv *env, jobject obj, jint iSampleRate, jint iChannelNumber, jbyteArray PcmArray, jint iDataSize){
    jbyte * arrayBody = 0;
    jsize theArrayLengthJ = 0;

    arrayBody       = env->GetByteArrayElements(PcmArray,0);
    theArrayLengthJ = env->GetArrayLength(PcmArray);

    LOGI("MusicPcmMixEncode PCM length=%d", theArrayLengthJ);
    char* pData = (char*)arrayBody;
    int iLen = (int)theArrayLengthJ;
    char* pAacBuffer = (char*)0;
    int iAACLen = MusicPcmMixEncode(iSampleRate, iChannelNumber, pData, iLen, &pAacBuffer);
    if((iAACLen <= 0) || (pAacBuffer == (char*)0)){
    	env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);
        return 0;
    }
    env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);

    LOGI("MusicPcmMixEncode AAC length=%d", iAACLen);
    jbyteArray retAACArray = env->NewByteArray(iAACLen);
    env->SetByteArrayRegion(retAACArray, 0, iAACLen, (jbyte*)pAacBuffer);

    if(pAacBuffer) free(pAacBuffer);

    return retAACArray;
}

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    MicPcmMixEncode
 * Signature: (II[BI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_kaolafm_record_AudioMixerNative_MicPcmMixEncode
  (JNIEnv *env, jobject obj, jint iSampleRate, jint iChannelNumber, jbyteArray PcmArray, jint iDataSize){
    jbyte * arrayBody = 0;
    jsize theArrayLengthJ = 0;

    arrayBody       = env->GetByteArrayElements(PcmArray,0);
    theArrayLengthJ = env->GetArrayLength(PcmArray);
    LOGI("MicPcmMixEncode PCM length=%d", theArrayLengthJ);

    char* pData = (char*)arrayBody;
    int iLen = (int)theArrayLengthJ;
    char* pAacBuffer = (char*)0;
    int iAACLen = MicPcmMixEncode(iSampleRate, iChannelNumber, pData, iLen, &pAacBuffer);
    if((iAACLen <= 0) || (pAacBuffer == (char*)0)){
    	env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);
        return 0;
    }
    LOGI("MicPcmMixEncode AAC length=%d", iAACLen);
    env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);

    //LOGI("AACDec pcm length=%d", iPCMLen);
    jbyteArray retAACArray = env->NewByteArray(iAACLen);
    env->SetByteArrayRegion(retAACArray, 0, iAACLen, (jbyte*)pAacBuffer);

    if(pAacBuffer) free(pAacBuffer);

    return retAACArray;
}

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    MusicPcmEncode
 * Signature: (II[BI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_kaolafm_record_AudioMixerNative_MusicPcmEncode
(JNIEnv *env, jobject obj, jint iSampleRate, jint iChannelNumber, jbyteArray PcmArray, jint iDataSize){
  jbyte * arrayBody = 0;
  jsize theArrayLengthJ = 0;

  arrayBody       = env->GetByteArrayElements(PcmArray,0);
  theArrayLengthJ = env->GetArrayLength(PcmArray);
  LOGI("MusicPcmEncode PCM length=%d", theArrayLengthJ);

  char* pData = (char*)arrayBody;
  int iLen = (int)theArrayLengthJ;
  char* pAacBuffer = (char*)0;
  int iAACLen = MusicPcmEncode(iSampleRate, iChannelNumber, pData, iLen, &pAacBuffer);
  if((iAACLen <= 0) || (pAacBuffer == (char*)0)){
	  env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);
      return 0;
  }
  LOGI("MusicPcmEncode AAC length=%d", iAACLen);
  env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);
  //LOGI("AACDec pcm length=%d", iPCMLen);
  jbyteArray retAACArray =env->NewByteArray(iAACLen);
  env->SetByteArrayRegion(retAACArray, 0, iAACLen, (jbyte*)pAacBuffer);

  if(pAacBuffer) free(pAacBuffer);

  return retAACArray;
}

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    MicPcmEncode
 * Signature: (II[BI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_kaolafm_record_AudioMixerNative_MicPcmEncode
(JNIEnv *env, jobject obj, jint iSampleRate, jint iChannelNumber, jbyteArray PcmArray, jint iDataSize){
  jbyte * arrayBody = 0;
  jsize theArrayLengthJ = 0;

  arrayBody       = env->GetByteArrayElements(PcmArray,0);
  theArrayLengthJ = env->GetArrayLength(PcmArray);

  char* pData = (char*)arrayBody;
  int iLen = (int)theArrayLengthJ;
  char* pAacBuffer = (char*)0;

  LOGI("MicPcmEncode PCM length=%d, pData=0x%08x", theArrayLengthJ, (unsigned long)pData);
  int iAACLen = MicPcmEncode(iSampleRate, iChannelNumber, pData, iLen, &pAacBuffer);
  if((iAACLen <= 0) || (pAacBuffer == (char*)0)){
	  LOGI("MicPcmEncode return length=%d, pData=0x%08x", iAACLen, (unsigned long)pAacBuffer);
	  env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);
      return 0;
  }
  LOGI("MicPcmEncode AAC length=%d", iAACLen);
  env->ReleaseByteArrayElements(PcmArray, arrayBody, 0);

  //LOGI("AACDec pcm length=%d", iPCMLen);
  jbyteArray retAACArray =env->NewByteArray(iAACLen);
  env->SetByteArrayRegion(retAACArray, 0, iAACLen, (jbyte*)pAacBuffer);

  if(pAacBuffer) free(pAacBuffer);

  return retAACArray;
}

/*
 * Class:     com_example_com_kaolafm_recordsdk_RecordSDKAPI
 * Method:    PcmMixFlush
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_kaolafm_record_AudioMixerNative_PcmMixFlush
  (JNIEnv *env, jobject obj){
	LOGI("PcmMixFlush is called...");
	char* pAacBuffer = (char*)0;
	int iAACLen = PcmMixFlush(&pAacBuffer);
	if((iAACLen <= 0) || (pAacBuffer == (char*)0)){
	    return 0;
	}
	LOGI("PcmMixFlush AAC length=%d", iAACLen);

	jbyteArray retAACArray =env->NewByteArray(iAACLen);
	env->SetByteArrayRegion(retAACArray, 0, iAACLen, (jbyte*)pAacBuffer);

	if(pAacBuffer) free(pAacBuffer);

	return retAACArray;
}

JNIEXPORT void JNICALL Java_com_kaolafm_record_AudioMixerNative_MusicGain
  (JNIEnv *env, jobject obj, jfloat fMusicGain){
	SetMusicGain(fMusicGain);
}

/*
 * Class:     com_kaolafm_record_AudioMixerNative
 * Method:    MicGain
 * Signature: (F)V
 */
JNIEXPORT void JNICALL Java_com_kaolafm_record_AudioMixerNative_MicGain
  (JNIEnv *env, jobject obj, jfloat fMicGain){
	SetMicGain(fMicGain);
}

jint JNICALL Java_com_kaolafm_record_AudioMixerNative_audioFileCut
  (JNIEnv *env, jobject obj, jstring InputFilePathname, jstring OutputFilePathname,
		  jfloat fStartTime, jfloat fEndTime, jfloat fBitRate){
	 char* szInputFilePathname = NULL;
	 char* szOutputFilePathname = NULL;

	 szInputFilePathname = js2c(env,  InputFilePathname);
	 szOutputFilePathname = js2c(env,  OutputFilePathname);

	 LOGI("%f:%f %f", fStartTime, fEndTime, fBitRate);
	 LOGI("audioFileCut %s:%s", szInputFilePathname, szOutputFilePathname);

	 int iRet = 0;
	 float fCutDuration = 0;
	 float fAllDuration = 0;
	 unsigned long ulStartPosition = 0;
	 unsigned long ulEndPosition = 0;

	 if (fStartTime < 0) {
		 if (szInputFilePathname)
			 free(szInputFilePathname);
		 if (szOutputFilePathname)
			 free(szOutputFilePathname);
	     return -1;
	 }

	 if ((fEndTime <= fStartTime) || (fEndTime < 0)) {
		 if (szInputFilePathname)
			 free(szInputFilePathname);
		 if (szOutputFilePathname)
			 free(szOutputFilePathname);
	     return -2;
	 }

	 fCutDuration = fEndTime - fStartTime;
	 FILE* pReadFile = fopen(szInputFilePathname, "rb");
	 if(pReadFile == (FILE*)0){
		 if (szInputFilePathname)
			 free(szInputFilePathname);
		 if (szOutputFilePathname)
			 free(szOutputFilePathname);
	     return -3;
	 }


	 FILE* pWriteFile = fopen(szOutputFilePathname, "ab+");
	 if(pWriteFile == (FILE*)0){
		 if (szInputFilePathname)
			 free(szInputFilePathname);
		 if (szOutputFilePathname)
			 free(szOutputFilePathname);
		 fclose(pReadFile);
	     return -4;
	 }

	 fseek(pReadFile, 0L, SEEK_END);
	 unsigned long ulFileSize = ftell(pReadFile);
	 fAllDuration = ulFileSize*8/fBitRate;
	 fseek(pReadFile, 0L, SEEK_SET);

	 LOGI("%d:%f", ulFileSize, fAllDuration);

	 ulStartPosition = (fStartTime/fAllDuration)*(float)ulFileSize;
	 ulEndPosition   = (fEndTime/fAllDuration)*(float)ulFileSize;

	 LOGI("Audio file %s\r\n length=%d, Duration=%.3f, iStartPosition=%d, iEndPosition=%d",
			 szInputFilePathname, ulFileSize, fAllDuration, ulStartPosition, ulEndPosition);

	 char* pData = (char*)malloc(ulFileSize);
	 if(pData == (char*)0){
		 if (szInputFilePathname)
			 free(szInputFilePathname);
		 if (szOutputFilePathname)
			 free(szOutputFilePathname);
		 fclose(pReadFile);
		 fclose(pWriteFile);
	     return -5;
	 }
	 LOGI("indata size=%d", ulFileSize);

	 int iReadLen = fread(pData, 1, ulFileSize, pReadFile);
	 if(iReadLen != ulFileSize){
		 if (szInputFilePathname)
			 free(szInputFilePathname);
		 if (szOutputFilePathname)
			 free(szOutputFilePathname);
		 fclose(pReadFile);
		 fclose(pWriteFile);
		 free(pData);
	     return -6;
	 }
	 LOGI("read file %s size=%d, %d:%d", szInputFilePathname, iReadLen,
			 ulStartPosition, ulEndPosition);

	 int iRealStartPosition = FindStartPosition(pData, ulStartPosition, ulEndPosition, ulFileSize);
	 int iRealEndPosition = FindStartPosition(pData, ulEndPosition, ulFileSize, ulFileSize);
	 LOGI("FindStartPosition return %d:%d", iRealStartPosition, iRealEndPosition);

	 if(iRealStartPosition < 0){
		 return -7;
	 }
	 if(iRealEndPosition < 0){
		 return -8;
	 }

	 char* pStart = pData + iRealStartPosition;
	 int iWriteLen = iRealEndPosition - iRealStartPosition;

	 if(iWriteLen > 0){
        iRet = fwrite(pStart, 1, iWriteLen, pWriteFile);
	 }
	 LOGI("fwrite %d:%d", iWriteLen, iRet);

	 if(iRet != iWriteLen){
		iRet = -9;
	 }
	 fclose(pReadFile);
	 fclose(pWriteFile);

	 if (szInputFilePathname)
		 free(szInputFilePathname);
	 if (szOutputFilePathname)
		 free(szOutputFilePathname);
	 if (pData)
	 	 free(pData);
	 return iRet;
}
