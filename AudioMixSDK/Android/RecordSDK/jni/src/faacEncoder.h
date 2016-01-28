//
//  faacEncocder.h
//  MixAudioDemo
//
//  Created by kaola on 16/1/18.
//  Copyright © 2016年 kaola. All rights reserved.
//

#ifndef faacEncocder_h
#define faacEncocder_h

#include <stdio.h>

class faacEncoder{
public:
    faacEncoder(int iSampleRate=32000, int iChannel=2);
    ~faacEncoder();
    
    int AudioEncode(char* pBuffer, int iLen, char* pAacBuffer);
    
    void AudioClose();
    
    unsigned long getSampleInputSize();
    unsigned long getSamples();
    void setSamples(unsigned long ulSamples);
    
    unsigned long getTotalTime();
private:
    faacEncoder();
    
private:
    void* _pHandle;
    void* _pFaacConf;
    int _iSampleRate;
    int _iChannelNumber;
    unsigned long _ulSamplesInputSize;
    unsigned long _ulMaxBytesOutput;
    unsigned long _ulSamples;
    short* _pPcmBuffer;
    short* _pFaacFlushBuffer;
    
    unsigned long _ulFaacFlushBufferSize;
};
#endif /* faacEncocder_h */
