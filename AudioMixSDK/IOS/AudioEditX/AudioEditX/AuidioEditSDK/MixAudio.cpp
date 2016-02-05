//
//  MixAudio.c
//  MixAudioDemo
//
//  Created by kaola on 16/1/13.
//  Copyright (c) 2016年 kaola. All rights reserved.
//

#include "MixAudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <memory.h>

#include "PcmResample.h"
#include "faacEncoder.h"
//#include "speexEC.h"

#define CHECK_MAX_VALUE(value) ((value > 32767) ? 32767 : value)
#define CHECK_MIN_VALUE(value) ((value < -32767) ? -32767 : value)

#define PCM_MAX_SIZE (8192*2)

static unsigned long s_ulSampleInputSize = 0;
static pthread_mutex_t s_EncoderMutex;

static void* s_pPcmEncoder = NULL;

static void* s_pMusicPcmQueue = NULL;
static void* s_pMicPcmQueue = NULL;

static float s_fMusicGain = 0.5;
static float s_fMicGain   = 2.5;

//static int s_iMusicSampleRate = 0;
//static int s_iMusicaChannelNumber = 0;
//static int s_iMicSampleRate = 0;
//static int s_iMicChannelNumber = 0;

int AudioGain(int iChannelNum, short* audioData, float fGain, short* outputData){
    if (iChannelNum <= 0) {
        return -1;
    }
    
    if (iChannelNum > 2) {
        return -2;
    }
    
    if (iChannelNum == 2) {
        float fLeftValue1 = (float)(audioData[0]);
        float fRightValue1 = (float)(audioData[1]);
        
        fLeftValue1 = fLeftValue1*fGain;
        fRightValue1 = fRightValue1*fGain;
        fLeftValue1 = CHECK_MAX_VALUE(fLeftValue1);
        fLeftValue1 = CHECK_MIN_VALUE(fLeftValue1);
        fRightValue1 = CHECK_MAX_VALUE(fRightValue1);
        fRightValue1 = CHECK_MIN_VALUE(fRightValue1);
        outputData[0] = (short)fLeftValue1;
        outputData[1] = (short)fRightValue1;
    }else{
        float fValue = 0;
        
        fValue = (float)(*(short*)(audioData));
        fValue = fValue*fGain;
        fValue = CHECK_MAX_VALUE(fValue);
        fValue = CHECK_MIN_VALUE(fValue);
        *outputData = (short)fValue;
    }
    return 1;
}

int MixAudio(int iChannelNum, short* sourceData1, short* sourceData2, float fBackgroundGain, float fWordsGain, short *outputData) {
    int const MAX = 32767;
    int const MIN = -32767;
    
    double f = 1.0;
    
    if (iChannelNum <= 0) {
        return -1;
    }
    
    if (iChannelNum > 2) {
        return -2;
    }
    
    if (iChannelNum == 2) {
        float fLeftValue1 = 0;
        float fRightValue1 = 0;
        float fLeftValue2 = 0;
        float fRightValue2 = 0;
        float fLeftValue = 0;
        float fRightValue = 0;
        int output = 0;
        int iIndex = 0;
        
        fLeftValue1 = (float)(sourceData1[0]);
        fRightValue1 = (float)(sourceData1[1]);
        fLeftValue2 = (float)(sourceData2[0]);
        fRightValue2 = (float)(sourceData2[1]);
        fLeftValue1 = fLeftValue1*fBackgroundGain;
        fRightValue1 = fRightValue1*fBackgroundGain;
        fLeftValue2 = fLeftValue2*fWordsGain;
        fRightValue2 = fRightValue2*fWordsGain;
        fLeftValue1 = CHECK_MAX_VALUE(fLeftValue1);
        fLeftValue1 = CHECK_MIN_VALUE(fLeftValue1);
        fRightValue1 = CHECK_MAX_VALUE(fRightValue1);
        fRightValue1 = CHECK_MIN_VALUE(fRightValue1);
        fLeftValue2 = CHECK_MAX_VALUE(fLeftValue2);
        fLeftValue2 = CHECK_MIN_VALUE(fLeftValue2);
        fRightValue2 = CHECK_MAX_VALUE(fRightValue2);
        fRightValue2 = CHECK_MIN_VALUE(fRightValue2);
        fLeftValue = fLeftValue1 + fLeftValue2;
        fRightValue = fRightValue1 + fRightValue2;

        for (iIndex = 0; iIndex < 2; iIndex++) {

            if (iIndex == 0) {
                output = (int)(fLeftValue*f);
            }
            else {
                output = (int)(fRightValue*f);
            }
            if (output>MAX)
            {
                f = (double)MAX / (double)(output);
                output = MAX;
            }
            if (output<MIN)
            {
                f = (double)MIN / (double)(output);
                output = MIN;
            }
            if (f<1)
            {
                f += ((double)1 - f) / (double)32;
            }
            outputData[iIndex] = (short)output;
        }
    }
    else {
        float fValue1 = 0;
        float fValue2 = 0;
        float fValue = 0;
        
        fValue1 = (float)(*(short*)(sourceData1));
        fValue2 = (float)(*(short*)(sourceData2));
        fValue1 = fValue1*fBackgroundGain;
        fValue2 = fValue2*fWordsGain;
        fValue = fValue1 + fValue2;
        
        fValue = CHECK_MAX_VALUE(fValue);
        fValue = CHECK_MIN_VALUE(fValue);
        *outputData = (short)fValue;
    }
    return 1;
}

int init_PCM_resample(void** ppHandle,int output_channels,int input_channels,int output_rate,int input_rate)
{
    CResample *pResample = new CResample();
    if(NULL == pResample)
    {
        printf("new CResample faile\n");
        return -1;
    }
    
    if(!pResample->audio_resample_init(output_channels,input_channels,output_rate,input_rate))
    {
        printf("resample init faile\n");
    }
    
    *ppHandle = (void*)pResample;
    return 0;
}

// ÷ÿ≤…—˘
// input : para1 ÷ÿ≤…—˘∂‘œÛ
//         para2  ‰»Î≥§∂»
//         para3  ‰»Î ˝æ›
// output: para4  ‰≥ˆ ˝æ›
// return  ‰≥ˆ≥§∂»
int start_PCM_resample(void* pHandle,int in_len,unsigned char *in_buf,unsigned char *out_buf)
{
    CResample *pResample = (CResample*)pHandle;
    
    int out_len = 0;
    int ns_sample_in =0;
    int ns_sample_out =0;
    
    ns_sample_in = in_len/(pResample->m_context.input_channels * sizeof(short));
    
    ns_sample_out = pResample->audio_resample((short *)out_buf, (short *)in_buf, ns_sample_in);
    
    out_len = ns_sample_out*pResample->m_context.output_channels * sizeof(short);
    //printf("ns_sample_in[%d]----ns_sample_out[%d]--out_len[%d]------\n",ns_sample_in,ns_sample_out,out_len);
    
    return out_len;
}

// ŒªøÌ◊™ªª
// input : para1 ÷ÿ≤…—˘∂‘œÛ
//         para2 flag : [8]--16Œª◊™8Œª ,[16]--8Œª◊™16Œª
//         para3  ‰»Î ˝æ›≥§∂»
//         para4  ‰»Î ˝æ›
// output: para5  ‰≥ˆ ˝æ›
// return  ‰≥ˆ ˝æ›≥§∂»
int bit_wide_transform(void* pHandle,int flag,int in_len,unsigned char* in_buf,unsigned char* out_buf)
{
    CResample *pResample = (CResample*)pHandle;
    
    int ns_sample = 0;
    if(8 == flag)
    {
        ns_sample = in_len/2;
        pResample->mono_16bit_to_8bit(out_buf, (short *)in_buf, ns_sample);
        return ns_sample*1;
    }
    else if(16 == flag)
    {
        ns_sample = in_len;
        pResample->mono_8bit_to_16bit((short*)out_buf, in_buf, ns_sample);
        return ns_sample*2;
    }
    return ns_sample*2;
}

// »•≥ı ºªØ
int uninit_PCM_resample(void* pHandle)
{
    CResample *pResample = (CResample*)pHandle;
    if(NULL == pResample)
    {
        delete pResample;
    }
    return 0;
}

// “Ù¡øøÿ÷∆
// output: para1  ‰≥ˆ ˝æ›
// input : para2  ‰»Î ˝æ›
//         para3  ‰»Î≥§∂»
//         para4 “Ù¡øøÿ÷∆≤Œ ˝,”––ßøÿ÷∆∑∂Œß[0,100]
int volume_control(short* out_buf,short* in_buf,int in_len, float in_vol)
{
    int i,tmp;
    
    // in_vol[0,100]
    float vol = in_vol - 98;
    
    if(-98 < vol  &&  vol <0 )
    {
        vol = 1/(vol*(-1));
    }
    else if(0 <= vol && vol <= 1)
    {
        vol = 1;
    }
    /*else if(1 < vol && vol <= 2)
     {
     vol = vol;
     }*/
    else if(vol <= -98)
    {
        vol = 0;
    }
    else if(2 <= vol)
    {
        vol = 2;
    }	
    
    for(i=0; i<in_len/2; i++)
    {
        tmp = in_buf[i]*vol;
        if(tmp > 32767)
        {
            tmp = 32767;
        }
        else if( tmp < -32768)
        {
            tmp = -32768;
        }
        out_buf[i] = tmp;
    }
    
    return 0;
}
void FaacInit(void** ppHandle, int iSampleRate, int iChannelNumber){
    *ppHandle = new faacEncoder(iSampleRate, iChannelNumber);
}

int FaacEncode(void* pHandle, char* pBuffer, int iLen, char* pAacBuffer){
    int iRetLen = 0;
    
    if (pHandle == NULL) {
        return -1;
    }
    if (pBuffer == NULL) {
        return -2;
    }
    if (iLen <= 0) {
        return -3;
    }
    if (pAacBuffer == NULL) {
        return -4;
    }
    faacEncoder* pEncoder = (faacEncoder*)pHandle;
    iRetLen = pEncoder->AudioEncode(pBuffer, iLen, pAacBuffer);
    
    return iRetLen;
}

void FaacDeInit(void* pHandle){
    if (pHandle) {
        faacEncoder* pEncoder = (faacEncoder*)pHandle;
        delete pEncoder;
    }
}

unsigned long FaacGetSampleInputSize(void* pHandle){
    if (pHandle) {
        faacEncoder* pEncoder = (faacEncoder*)pHandle;
        return pEncoder->getSampleInputSize();
    }
    return 0;
}

unsigned long FaacGetSamples(void* pHandle){
    if (pHandle) {
        faacEncoder* pEncoder = (faacEncoder*)pHandle;
        return pEncoder->getSamples();
    }
    return 0;
}

void FaacSetSamples(void* pHandle, unsigned long ulSamples){
    if (pHandle) {
        faacEncoder* pEncoder = (faacEncoder*)pHandle;
        pEncoder->setSamples(ulSamples);
    }
    return;
}

unsigned long FaacGetTotalTime(void* pHandle){
    if (pHandle) {
        faacEncoder* pEncoder = (faacEncoder*)pHandle;
        return pEncoder->getTotalTime();
    }
    return 0;
}

void PcmQueueInit(void** ppHandle, unsigned long ulSampleInputSize){
    PcmQueue* pHandle = new PcmQueue(ulSampleInputSize);
    *ppHandle = (void*)pHandle;
}

unsigned long PcmQueueInsert(void* pHandle, char* pData, unsigned long ulSize){
    PcmQueue* pPcmQueueHandle = (PcmQueue*)pHandle;
    if (pPcmQueueHandle) {
        return pPcmQueueHandle->InsertData(pData, ulSize);
    }
    return 0;
}

unsigned long PcmQueueRead(void* pHandle, char** ppData){
    PcmQueue* pPcmQueueHandle = (PcmQueue*)pHandle;
    if (pPcmQueueHandle) {
        return pPcmQueueHandle->GetData(ppData);
    }
    return 0;
}

void PcmQueueClean(void* pHandle){
    PcmQueue* pPcmQueueHandle = (PcmQueue*)pHandle;
    if (pPcmQueueHandle) {
        pPcmQueueHandle->CleanQueue();
    }
    return;
}

unsigned long PcmGetCurrentLength(void* pHandle){
    PcmQueue* pPcmQueueHandle = (PcmQueue*)pHandle;
    if (pPcmQueueHandle) {
        return pPcmQueueHandle->GetCurrentQueueLength();
    }
    return 0;
}

void PcmQueueDeInit(void* pHandle){
    PcmQueue* pPcmQueueHandle = (PcmQueue*)pHandle;
    if (pPcmQueueHandle) {
        delete pPcmQueueHandle;
    }
}

int PcmMixEncoderInit(){
    FaacInit(&s_pPcmEncoder, PCM_ENCODER_SAMPLERATE_DEFAULT, PCM_ENCODER_CHANNELNUM_DEFAULT);
    if (s_pPcmEncoder == NULL) {
        return -1;
    }
    s_ulSampleInputSize = FaacGetSampleInputSize(s_pPcmEncoder);
    pthread_mutex_init(&s_EncoderMutex,NULL);

    PcmQueueInit(&s_pMusicPcmQueue, s_ulSampleInputSize);
    PcmQueueInit(&s_pMicPcmQueue, s_ulSampleInputSize);
    
    return 0;
}

void PcmMixEncoderDeInit(){
    if (s_pPcmEncoder != NULL) {
        FaacDeInit(s_pPcmEncoder);
        s_pPcmEncoder = NULL;
    }
    pthread_mutex_destroy(&s_EncoderMutex);
    
    if (s_pMusicPcmQueue) {
        PcmQueueDeInit(s_pMusicPcmQueue);
    }
    
    if (s_pMicPcmQueue) {
        PcmQueueDeInit(s_pMicPcmQueue);
    }
    return;
}

int MusicPcmMixEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer){
    int iRet = 0;
    void* pPcmResample = NULL;
    unsigned char* pNewData = NULL;
    int iNewLen = 0;
    
    pthread_mutex_lock(&s_EncoderMutex);
    if ((iSampleRate != PCM_ENCODER_SAMPLERATE_DEFAULT) || (iChannelNumber != PCM_ENCODER_CHANNELNUM_DEFAULT)) {
        pNewData = (unsigned char*)malloc(PCM_MAX_SIZE);
        init_PCM_resample(&pPcmResample, PCM_ENCODER_CHANNELNUM_DEFAULT, iChannelNumber,
                          PCM_ENCODER_SAMPLERATE_DEFAULT, iSampleRate);

        iNewLen = start_PCM_resample(pPcmResample, iLen, (unsigned char*)pData, pNewData);
        uninit_PCM_resample(pPcmResample);
    }else{
        pNewData = (unsigned char*)pData;
        iNewLen = iLen;
    }

    unsigned long ulMusicaQueueLen = PcmQueueInsert(s_pMusicPcmQueue, (char*)pNewData, iNewLen);
    unsigned long ulMicQueueLen = PcmGetCurrentLength(s_pMicPcmQueue);
    if ((ulMusicaQueueLen >= s_ulSampleInputSize*sizeof(short)) && (ulMicQueueLen >= s_ulSampleInputSize*sizeof(short))) {
        char* pMusicData = NULL;
        char* pMicData   = NULL;
        unsigned long ulMusicLen = PcmQueueRead(s_pMusicPcmQueue, &pMusicData);
        unsigned long ulMicLen   = PcmQueueRead(s_pMicPcmQueue, &pMicData);
        
        if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))){
            short* pMusicUnit = (short*)pMusicData;
            short* pMicUnit   = (short*)pMicData;
            short* pOutputPcm        = (short*)malloc(s_ulSampleInputSize*sizeof(short));
            
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                MixAudio((int)PCM_ENCODER_CHANNELNUM_DEFAULT, &pMusicUnit[iIndex], &pMicUnit[iIndex], s_fMusicGain, s_fMicGain, &pOutputPcm[iIndex]);
            }
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pOutputPcm, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen > 0) {
                *ppAacBuffer = pAacBuffer;
                iRet = iAacLen;
            }else{
                free(pAacBuffer);
                iRet = 0;
                *ppAacBuffer = NULL;
            }
            free(pOutputPcm);
        }
        
    }
    if ((iSampleRate != PCM_ENCODER_SAMPLERATE_DEFAULT) || (iChannelNumber != PCM_ENCODER_CHANNELNUM_DEFAULT)) {
        free(pNewData);
    }
    pthread_mutex_unlock(&s_EncoderMutex);

    return iRet;
}

int MicPcmMixEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer){
    int iRet = 0;
    
    pthread_mutex_lock(&s_EncoderMutex);

    unsigned long ulMicQueueLen = PcmQueueInsert(s_pMicPcmQueue, (char*)pData, iLen);
    unsigned long ulMusicaQueueLen = PcmGetCurrentLength(s_pMusicPcmQueue);
    if ((ulMusicaQueueLen >= s_ulSampleInputSize*sizeof(short)) && (ulMicQueueLen >= s_ulSampleInputSize*sizeof(short))) {
        char* pMusicData = NULL;
        char* pMicData   = NULL;
        unsigned long ulMusicLen = PcmQueueRead(s_pMusicPcmQueue, &pMusicData);
        unsigned long ulMicLen   = PcmQueueRead(s_pMicPcmQueue, &pMicData);
        
        if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))){
            short* pMusicUnit = (short*)pMusicData;
            short* pMicUnit   = (short*)pMicData;
            short* pOutputPcm        = (short*)malloc(s_ulSampleInputSize*sizeof(short));
            
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                MixAudio((int)PCM_ENCODER_CHANNELNUM_DEFAULT, &pMusicUnit[iIndex], &pMicUnit[iIndex], s_fMusicGain, s_fMicGain, &pOutputPcm[iIndex]);
            }
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pOutputPcm, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                *ppAacBuffer = pAacBuffer;
                iRet = iAacLen;
            }else{
                free(pAacBuffer);
                iRet = 0;
                *ppAacBuffer = NULL;
            }
            free(pOutputPcm);
        }
        
    }
    pthread_mutex_unlock(&s_EncoderMutex);
    return iRet;
}

int PcmMixFlush(char** ppAacBuffer){
    int iRet = 0;
    unsigned long ulMusicLen = 0;
    unsigned long ulMicLen   = 0;
    char* pFinishAacBuffer   = NULL;
    
    pthread_mutex_lock(&s_EncoderMutex);
    
    do{
        char* pMusicData = NULL;
        char* pMicData   = NULL;
        ulMusicLen = PcmQueueRead(s_pMusicPcmQueue, &pMusicData);
        ulMicLen   = PcmQueueRead(s_pMicPcmQueue, &pMicData);
        
        if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))){
            short* pMusicUnit = (short*)pMusicData;
            short* pMicUnit   = (short*)pMicData;
            short* pOutputPcm        = (short*)malloc(s_ulSampleInputSize*sizeof(short));
            
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                MixAudio((int)PCM_ENCODER_CHANNELNUM_DEFAULT, &pMusicUnit[iIndex], &pMicUnit[iIndex], s_fMusicGain, s_fMicGain, &pOutputPcm[iIndex]);
            }
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pOutputPcm, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
            free(pOutputPcm);
        }else if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen != s_ulSampleInputSize*sizeof(short))) {
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pMusicData, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
        }else if ((ulMusicLen != s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))) {
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pMicData, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
        }
    }while((ulMusicLen > 0) || (ulMicLen > 0));
    
    PcmQueueClean(s_pMicPcmQueue);
    PcmQueueClean(s_pMusicPcmQueue);
    pthread_mutex_unlock(&s_EncoderMutex);
    return iRet;
}

int MicFlush(char** ppAacBuffer){
    int iRet = 0;
    unsigned long ulMusicLen = 0;
    unsigned long ulMicLen   = 0;
    char* pFinishAacBuffer   = NULL;
    
    pthread_mutex_lock(&s_EncoderMutex);
    
    do{
        char* pMusicData = NULL;
        char* pMicData   = NULL;
        ulMusicLen = PcmQueueRead(s_pMusicPcmQueue, &pMusicData);
        ulMicLen   = PcmQueueRead(s_pMicPcmQueue, &pMicData);
        
        if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))){
            short* pMusicUnit = (short*)pMusicData;
            short* pMicUnit   = (short*)pMicData;
            short* pOutputPcm        = (short*)malloc(s_ulSampleInputSize*sizeof(short));
            
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                MixAudio((int)PCM_ENCODER_CHANNELNUM_DEFAULT, &pMusicUnit[iIndex], &pMicUnit[iIndex], s_fMusicGain, s_fMicGain, &pOutputPcm[iIndex]);
            }
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pOutputPcm, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
            free(pOutputPcm);
        }else if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen != s_ulSampleInputSize*sizeof(short))) {
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pMusicData, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
        }
    }while((ulMusicLen > 0) || (ulMicLen > 0));
    
    PcmQueueClean(s_pMicPcmQueue);
    pthread_mutex_unlock(&s_EncoderMutex);
    return iRet;
}

int MusicFlush(char** ppAacBuffer){
    int iRet = 0;
    unsigned long ulMusicLen = 0;
    unsigned long ulMicLen   = 0;
    char* pFinishAacBuffer   = NULL;
    
    pthread_mutex_lock(&s_EncoderMutex);
    
    do{
        char* pMusicData = NULL;
        char* pMicData   = NULL;
        ulMusicLen = PcmQueueRead(s_pMusicPcmQueue, &pMusicData);
        ulMicLen   = PcmQueueRead(s_pMicPcmQueue, &pMicData);
        
        if ((ulMusicLen == s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))){
            short* pMusicUnit = (short*)pMusicData;
            short* pMicUnit   = (short*)pMicData;
            short* pOutputPcm        = (short*)malloc(s_ulSampleInputSize*sizeof(short));
            
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                MixAudio((int)PCM_ENCODER_CHANNELNUM_DEFAULT, &pMusicUnit[iIndex], &pMicUnit[iIndex], s_fMusicGain, s_fMicGain, &pOutputPcm[iIndex]);
            }
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pOutputPcm, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
            free(pOutputPcm);
        }else if ((ulMusicLen != s_ulSampleInputSize*sizeof(short)) && (ulMicLen == s_ulSampleInputSize*sizeof(short))) {
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)pMicData, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
        }
    }while((ulMusicLen > 0) || (ulMicLen > 0));
    
    PcmQueueClean(s_pMicPcmQueue);
    pthread_mutex_unlock(&s_EncoderMutex);
    return iRet;
}

int MusicPcmEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer){
    int iRet = 0;
    void* pPcmResample = NULL;
    unsigned char* pNewData = NULL;
    int iNewLen = 0;
    char* pFinishAacBuffer = NULL;
    
    pthread_mutex_lock(&s_EncoderMutex);
    if ((iSampleRate != PCM_ENCODER_SAMPLERATE_DEFAULT) || (iChannelNumber != PCM_ENCODER_CHANNELNUM_DEFAULT)) {
        pNewData = (unsigned char*)malloc(PCM_MAX_SIZE);
        init_PCM_resample(&pPcmResample, PCM_ENCODER_CHANNELNUM_DEFAULT, iChannelNumber,
                          PCM_ENCODER_SAMPLERATE_DEFAULT, iSampleRate);
        
        iNewLen = start_PCM_resample(pPcmResample, iLen, (unsigned char*)pData, pNewData);
        uninit_PCM_resample(pPcmResample);
    }else{
        pNewData = (unsigned char*)pData;
        iNewLen = iLen;
    }
    
    unsigned long ulMusicaQueueLen = PcmQueueInsert(s_pMusicPcmQueue, (char*)pNewData, iNewLen);
    while (ulMusicaQueueLen >= s_ulSampleInputSize*sizeof(short)) {
        char* pMusicData = NULL;
        ulMusicaQueueLen = PcmQueueRead(s_pMusicPcmQueue, &pMusicData);
        
        if (ulMusicaQueueLen == s_ulSampleInputSize*sizeof(short) ){
            short* outputData = (short*)malloc(ulMusicaQueueLen);
            short* pMusicUnit = (short*)pMusicData;
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                AudioGain(PCM_ENCODER_CHANNELNUM_DEFAULT, &pMusicUnit[iIndex], s_fMusicGain, &outputData[iIndex]);
            }
            free(pMusicData);
            pMusicData = NULL;
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)outputData, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen > 0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
                iRet = 0;
                *ppAacBuffer = NULL;
            }
            if (outputData != NULL) {
                free(outputData);
            }
        }
    }
    if ((iSampleRate != PCM_ENCODER_SAMPLERATE_DEFAULT) || (iChannelNumber != PCM_ENCODER_CHANNELNUM_DEFAULT)) {
        free(pNewData);
    }
    pthread_mutex_unlock(&s_EncoderMutex);
    
    return iRet;
}


int MicPcmEncode(int iSampleRate, int iChannelNumber, char* pData, int iLen, char** ppAacBuffer){
    int iRet = 0;
    char* pFinishAacBuffer = NULL;
    
    pthread_mutex_lock(&s_EncoderMutex);
    
    *ppAacBuffer = NULL;
    unsigned long ulMicQueueLen = PcmQueueInsert(s_pMicPcmQueue, (char*)pData, iLen);
    
    while (ulMicQueueLen >= s_ulSampleInputSize*sizeof(short)) {
        char* pMicData   = NULL;
        ulMicQueueLen   = PcmQueueRead(s_pMicPcmQueue, &pMicData);
        
        if (ulMicQueueLen == s_ulSampleInputSize*sizeof(short)){
            short* outputData = (short*)malloc(ulMicQueueLen);
            short* pMicUnit = (short*)pMicData;
            for (int iIndex = 0; iIndex < s_ulSampleInputSize; iIndex=iIndex+PCM_ENCODER_CHANNELNUM_DEFAULT) {
                AudioGain(PCM_ENCODER_CHANNELNUM_DEFAULT, &pMicUnit[iIndex], s_fMicGain, &outputData[iIndex]);
            }
            free(pMicData);
            pMicData = NULL;
            
            char* pAacBuffer = (char*)malloc(s_ulSampleInputSize*sizeof(short));
            int iAacLen = FaacEncode(s_pPcmEncoder, (char*)outputData, (int)s_ulSampleInputSize*sizeof(short), pAacBuffer);
            if (iAacLen>0) {
                if (pFinishAacBuffer == NULL) {
                    pFinishAacBuffer = pAacBuffer;
                }else{
                    char* pTmp = pFinishAacBuffer;
                    pFinishAacBuffer = (char*)malloc(iRet+iAacLen);
                    memcpy((void*)pFinishAacBuffer, (void*)pTmp, iRet);
                    memcpy((void*)(pFinishAacBuffer+iRet), (void*)pAacBuffer, iAacLen);
                    free(pTmp);
                }
                iRet += iAacLen;
                *ppAacBuffer = pFinishAacBuffer;
            }else{
                free(pAacBuffer);
            }
            if (outputData != NULL) {
                free(outputData);
                outputData = NULL;
            }
        }
    }
    pthread_mutex_unlock(&s_EncoderMutex);
    return iRet;
}

void SetMusicGain(float fMusicGain){
    s_fMusicGain = fMusicGain;
}

void SetMicGain(float fMicGain){
    s_fMicGain = fMicGain;
}

//void SpeexEcInit(void** ppHandle, int iFramesize, int iFilterlength, int iSamplingrate){
//    CSpeexEC* pEc = new CSpeexEC();
//    if (pEc == NULL) {
//        *ppHandle = NULL;
//        return;
//    }
//    
//    *ppHandle = pEc;
//}
//
//void SpeexEcDeInit(void* pHandle){
//    if (pHandle == NULL) {
//        return;
//    }
//    CSpeexEC* pEc = (CSpeexEC*)pHandle;
//    delete pEc;
//}
//
//int SpeexDoAEC(void* pHandle, short* mic, short* ref, short* out){
//    CSpeexEC* pEc = (CSpeexEC*)pHandle;
//    
//    if (pHandle == NULL) {
//        return -1;
//    }
//    if (mic == NULL) {
//        return -2;
//    }
//    if (ref == NULL) {
//        return -3;
//    }
//    if (out == NULL) {
//        return -4;
//    }
//
//    pEc->DoAEC(mic, ref, out);
//    return 0;
//}
