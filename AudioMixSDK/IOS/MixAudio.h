//
//  MixAudio.h
//  MixAudioDemo
//
//  Created by kaola on 16/1/13.
//  Copyright (c) 2016年 kaola. All rights reserved.
//

#ifndef MixAudioDemo_MixAudio_h
#define MixAudioDemo_MixAudio_h
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PCM_ENCODER_SAMPLERATE_DEFAULT 32000
#define PCM_ENCODER_CHANNELNUM_DEFAULT 2

int MixAudio(int iChannelNum, short* sourceData1, short* sourceData2, float fBackgroundGain, float fWordsGain, short *outputData);

int init_PCM_resample(void **ppHandle,int output_channels,int input_channels,int output_rate,int input_rate);
int start_PCM_resample(void *pHandle,int in_len,unsigned char *in_buf,unsigned char *out_buf);
int bit_wide_transform(void *pHandle,int flag,int in_len,unsigned char* in_buf,unsigned char* out_buf);
int uninit_PCM_resample(void *pHandle);

int volume_control(short* out_buf,short* in_buf,int in_len, float vol);

//void SpeexEcInit(void** ppHandle, int iFramesize, int iFilterlength, int iSamplingrate);
//void SpeexEcDeInit(void* pHandle);
//int SpeexDoAEC(void* pHandle, short* mic, short* ref, short* out);

void FaacInit(void** ppHandle, int iSampleRate, int iChannelNumber);
int FaacEncode(void* pHandle, char* pBuffer, int iLen, char* pAacBuffer);
void FaacDeInit(void* pHandle);
unsigned long FaacGetSampleInputSize(void* pHandle);
unsigned long FaacGetSamples(void* pHandle);
void FaacSetSamples(void* pHandle, unsigned long ulSamples);
unsigned long FaacGetTotalTime(void* pHandle);

void PcmQueueInit(void** ppHandle, unsigned long ulSampleInputSize);
unsigned long PcmQueueInsert(void* pHandle, char* pData, unsigned long ulSize);
unsigned long PcmQueueRead(void* pHandle, char** ppData);
void PcmQueueClean(void* pHandle);
unsigned long PcmGetCurrentLength(void* pHandle);
void PcmQueueDeInit(void* pHandle);

int PcmMixEncoderInit();
void PcmMixEncoderDeInit();
int MusicPcmMixEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer);
int MicPcmMixEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer);
int MusicPcmEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer);
int MicPcmEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer);
int PcmMixFlush(char** ppAacBuffer);

void SetMusicGain(float fMusicGain);
void SetMicGain(float fMicGain);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
