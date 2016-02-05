//
//  faacEncocder.c
//  MixAudioDemo
//
//  Created by kaola on 16/1/18.
//  Copyright © 2016年 kaola. All rights reserved.
//

#include "faacEncoder.h"
#include "faac.h"

#include <memory.h>
#include <stdlib.h>

#define BIT_RATE_DEFAULT (64*1000)

faacEncoder::faacEncoder(int iSampleRate, int iChannel){
    _iSampleRate = iSampleRate;
    _iChannelNumber = iChannel;
    
    _pHandle = faacEncOpen(iSampleRate, iChannel, &_ulSamplesInputSize, &_ulMaxBytesOutput);
    
    faacEncConfigurationPtr faacConf = faacEncGetCurrentConfiguration(_pHandle);
    faacConf->mpegVersion   = MPEG4; //
    faacConf->bitRate = BIT_RATE_DEFAULT/iChannel;
    faacConf->aacObjectType = LOW;
    faacConf->quantqual = 100;
    faacConf->outputFormat = 1;
    faacConf->inputFormat = FAAC_INPUT_16BIT;
    faacConf->bandWidth = 0;//BIT_RATE_DEFAULT/iChannel/8;
    faacConf->allowMidside = 0;
    faacConf->useTns = 1;
    faacConf->shortctl = SHORTCTL_NORMAL;
    
    faacEncSetConfiguration(_pHandle, faacConf);
    
    _pPcmBuffer = (short*)malloc(_ulSamplesInputSize*sizeof(short)*2);
    _pFaacFlushBuffer = (short*)malloc(_ulSamplesInputSize*sizeof(short)*2);
    _ulFaacFlushBufferSize = 0;
    _ulSamples = 0;
}

faacEncoder::~faacEncoder(){
    AudioClose();
}

int faacEncoder::AudioEncode(char* pBuffer, int iLen, char* pAacBuffer){
    int iRetLen = 0;
    short * pcm = (short *)pBuffer;
    unsigned int length = (unsigned int)iLen / 2;
    unsigned long ulOutputLength = ((length / _ulSamplesInputSize) + 1) * _ulMaxBytesOutput;
    
    unsigned char * output = (unsigned char*)malloc(ulOutputLength);
    
    unsigned long ulPcmBufPos = _ulFaacFlushBufferSize;
    
    unsigned int uiEncSize = (unsigned int)_ulMaxBytesOutput;
    int iPcmPos = 0;
    
    do{
        memcpy(_pPcmBuffer + ulPcmBufPos, pcm + iPcmPos, sizeof(short) * (_ulSamplesInputSize - ulPcmBufPos));
        iPcmPos += _ulSamplesInputSize - ulPcmBufPos;
        
        int enc_result = faacEncEncode(_pHandle, (int32_t *) _pPcmBuffer, (unsigned int)_ulSamplesInputSize, output + iRetLen, uiEncSize);
        if (enc_result >= 0) {
            iRetLen += enc_result;
        } else {
            iRetLen = enc_result;
        }
        
        ulPcmBufPos = 0;
    }while (iRetLen >= 0 && (length - iPcmPos >= _ulSamplesInputSize));
    _ulSamples += iPcmPos;
    
    if (iRetLen > 0) {
        memcpy((void*)pAacBuffer, (void*)output, iRetLen);
        _ulFaacFlushBufferSize = length - iPcmPos;
        if (_ulFaacFlushBufferSize > 0) {
            memcpy(_pFaacFlushBuffer, pcm + iPcmPos, sizeof(short) * _ulFaacFlushBufferSize);
        }
    }
    
    free(output);
    
    return iRetLen;
}

void faacEncoder::AudioClose(){
    if (_pHandle) {
        faacEncClose(_pHandle);
        _pHandle = NULL;
    }
    if(_pPcmBuffer){
        free(_pPcmBuffer);
        _pPcmBuffer = NULL;
    }
    if(_pFaacFlushBuffer){
        free(_pFaacFlushBuffer);
        _pFaacFlushBuffer = NULL;
    }
}

unsigned long faacEncoder::getSampleInputSize(){
    return _ulSamplesInputSize;
}

unsigned long faacEncoder::getSamples(){
    return _ulSamples;
}

void faacEncoder::setSamples(unsigned long ulSamples){
    _ulSamples = ulSamples;
}


unsigned long faacEncoder::getTotalTime(){
    return _ulSamples*1000/_iSampleRate/_iChannelNumber;
}