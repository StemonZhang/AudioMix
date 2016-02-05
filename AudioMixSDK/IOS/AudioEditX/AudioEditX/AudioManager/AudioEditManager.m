//
//  AudioEditManager.m
//  AudioEditX
//
//  Created by kaola on 16/2/2.
//  Copyright © 2016年 com.Alex. All rights reserved.
//

#import "AudioEditManager.h"
#import "MixAudio.h"

#define AUIDIO_EDIT_MANAGER_QUEUE "AudioEditManager"


@implementation AudioEditManager
{
    dispatch_queue_t _AudioEditQueue;
    double _fStartStamp;
    double _fEndStamp;
    double _fRecordStamp;
    double _fDuration;
    NSMutableArray* _DurationArray;
}
@synthesize _iMicFlag;
@synthesize _iMusicFlag;
@synthesize _RecFilename;
@synthesize _fileLock;
@synthesize _RecTimeStampManager;

+(AudioEditManager*)sharedInstance
{
    static dispatch_once_t onceToken;
    static AudioEditManager * manger= nil;
    dispatch_once(&onceToken, ^{
        manger = [[AudioEditManager alloc] init];
        [manger InitAudio];
    });
    return manger;
}

-(void) InitAudio{
    _AudioEditQueue = dispatch_queue_create(AUIDIO_EDIT_MANAGER_QUEUE, DISPATCH_QUEUE_SERIAL);
    _iMicFlag = 0;
    _iMusicFlag = 0;
    PcmMixEncoderInit();
    _fileLock = [[NSLock alloc] init];
    
    _RecFilename = AUIDEO_DEF_FILENAME;
    NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
    _RecFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, _RecFilename];
    [[NSFileManager defaultManager] createFileAtPath:_RecFilename contents:nil attributes:nil];
    _RecTimeStampManager = [[RecordTimeStampManager alloc] init];
    
    _fDuration = 0;
    _fStartStamp = 0;
    _fEndStamp = 0;
    _fRecordStamp = 0;
    _DurationArray = [[NSMutableArray alloc] init];
}

-(void)dealloc{
    PcmMixEncoderDeInit();
}

-(void)RecordPause{
    _fStartStamp = 0;
    _fEndStamp = 0;
    NSNumber* durationNum = [[NSNumber alloc] initWithFloat:_fDuration];
    [_DurationArray addObject:durationNum];
}

-(float) GetDuration{
    float _fRetDuration = 0;
    
    for (int iIndex=0; iIndex<_DurationArray.count; iIndex++) {
        NSNumber* tmp = _DurationArray[iIndex];
        _fRetDuration += tmp.floatValue;
    }
    return _fRetDuration;
}

-(void) OnMusicDataHandle:(char*)pMusicData
                   Length:(int)iLength
               SampleRate:(int)iSampleRate
            ChannelNumber:(int)iChannelNumber{
    //NSLog(@"Music PCM length:%d, SampleRate=%d, ChannelNUmber=%d", iLength, iSampleRate, iChannelNumber);
    char* pAacBuffer = nil;
    int iAacLen = 0;
    if ((_iMusicFlag != 0) && (_iMicFlag != 0)) {
        iAacLen = MusicPcmMixEncode(iSampleRate, iChannelNumber, pMusicData, iLength, &pAacBuffer);
        if (iAacLen>0) {
            NSLog(@"MusicMix aac length:%d, %d:%d pcmlen=%d", iAacLen, iSampleRate, iChannelNumber, iLength);
        }
    }else if ((_iMusicFlag != 0) && (_iMicFlag == 0)){
        iAacLen = MusicPcmEncode(iSampleRate, iChannelNumber, pMusicData, iLength, &pAacBuffer);
        if (iAacLen>0) {
            NSLog(@"Music aac length:%d, %d:%d pcmlen=%d", iAacLen, iSampleRate, iChannelNumber, iLength);
        }
    }
    if ((pAacBuffer != nil)&&(iAacLen > 0)) {
        [self._fileLock lock];
        if (0 == _fStartStamp) {
            _fStartStamp = [[NSDate date] timeIntervalSince1970];
            _fRecordStamp = _fStartStamp;
        }

        _fEndStamp = [[NSDate date] timeIntervalSince1970];
        _fDuration = _fEndStamp - _fStartStamp;
        NSFileHandle* outFile = [NSFileHandle fileHandleForWritingAtPath:self._RecFilename];
        [outFile seekToEndOfFile];
        
        if ((_fEndStamp-_fRecordStamp) > 0.5) {
            _fRecordStamp = _fEndStamp;
            long lPos = outFile.offsetInFile;
            float fAudioTimeStamp = _fRecordStamp - _fStartStamp + [self GetDuration];
            [_RecTimeStampManager InsertTimeStamp:fAudioTimeStamp FilePos:lPos];
        }
        NSData *data = [NSData dataWithBytes:pAacBuffer length:iAacLen];
        [outFile writeData:data];
        
        [outFile closeFile];
        free(pAacBuffer);
        [self._fileLock unlock];
    }
}

-(void) OnMicDataHandle:(char*)pMicData
                 Length:(int)iLength{
    //NSLog(@"Mic PCM length:%d", iLength);
    char* pAacBuffer = nil;
    int iAacLen = 0;
    if ((_iMusicFlag != 0) && (_iMicFlag != 0)) {
        iAacLen = MicPcmMixEncode(PCM_ENCODER_SAMPLERATE_DEFAULT, PCM_ENCODER_CHANNELNUM_DEFAULT, pMicData, iLength, &pAacBuffer);
        if (iAacLen>0) {
            NSLog(@"MicMix aac length:%d", iAacLen);
        }
    }else if ((_iMusicFlag == 0) && (_iMicFlag != 0)){
        iAacLen = MicPcmEncode(PCM_ENCODER_SAMPLERATE_DEFAULT, PCM_ENCODER_CHANNELNUM_DEFAULT, pMicData, iLength, &pAacBuffer);
        if (iAacLen>0) {
            NSLog(@"Mic aac length:%d", iAacLen);
        }
    }
    if ((pAacBuffer != nil)&&(iAacLen > 0)) {
        [self._fileLock lock];
        if (0 == _fStartStamp) {
            _fStartStamp = [[NSDate date] timeIntervalSince1970];
            _fRecordStamp = _fStartStamp;
        }
        _fEndStamp = [[NSDate date] timeIntervalSince1970];
        _fDuration = _fEndStamp - _fStartStamp;
        NSFileHandle* outFile = [NSFileHandle fileHandleForWritingAtPath:self._RecFilename];
        [outFile seekToEndOfFile];
        
        if ((_fEndStamp-_fRecordStamp) > 0.5) {
            _fRecordStamp = _fEndStamp;
            long lPos = outFile.offsetInFile;
            float fAudioTimeStamp = _fRecordStamp - _fStartStamp + [self GetDuration];
            [_RecTimeStampManager InsertTimeStamp:fAudioTimeStamp FilePos:lPos];
        }
        
        NSData *data = [NSData dataWithBytes:pAacBuffer length:iAacLen];
        [outFile writeData:data];
        
        [outFile closeFile];
        free(pAacBuffer);
        [self._fileLock unlock];
    }
}

-(void) MicFlush{
    char* pAacBuffer = nil;
    int iAacLen = 0;
    
    iAacLen = MicFlush(&pAacBuffer);
    if ((pAacBuffer != nil)&&(iAacLen > 0)) {
        [self._fileLock lock];
        _fEndStamp = [[NSDate date] timeIntervalSince1970];
        _fDuration = _fEndStamp - _fStartStamp;
        NSFileHandle* outFile = [NSFileHandle fileHandleForWritingAtPath:self._RecFilename];
        [outFile seekToEndOfFile];
        
        if ((_fEndStamp-_fRecordStamp) > 0.5) {
            _fRecordStamp = _fEndStamp;
            long lPos = outFile.offsetInFile;
            float fAudioTimeStamp = _fRecordStamp - _fStartStamp;
            [_RecTimeStampManager InsertTimeStamp:fAudioTimeStamp FilePos:lPos];
        }
        
        NSData *data = [NSData dataWithBytes:pAacBuffer length:iAacLen];
        [outFile writeData:data];
        
        [outFile closeFile];
        free(pAacBuffer);
        [self._fileLock unlock];
    }
}

-(void) MusicFlush{
    char* pAacBuffer = nil;
    int iAacLen = 0;
    
    iAacLen = MusicFlush(&pAacBuffer);
    if ((pAacBuffer != nil)&&(iAacLen > 0)) {
        [self._fileLock lock];
        _fEndStamp = [[NSDate date] timeIntervalSince1970];
        _fDuration = _fEndStamp - _fStartStamp;
        NSFileHandle* outFile = [NSFileHandle fileHandleForWritingAtPath:self._RecFilename];
        [outFile seekToEndOfFile];
        
        if ((_fEndStamp-_fRecordStamp) > 0.5) {
            _fRecordStamp = _fEndStamp;
            long lPos = outFile.offsetInFile;
            float fAudioTimeStamp = _fRecordStamp - _fStartStamp;
            [_RecTimeStampManager InsertTimeStamp:fAudioTimeStamp FilePos:lPos];
        }
        
        NSData *data = [NSData dataWithBytes:pAacBuffer length:iAacLen];
        [outFile writeData:data];
        
        [outFile closeFile];
        free(pAacBuffer);
        [self._fileLock unlock];
    }
}

-(void) SaveAudioFile:(NSString*)strFilepathname{
    NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
    NSString* strPathFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, strFilepathname];
    NSError *err;
    if([[NSFileManager defaultManager] fileExistsAtPath:strPathFilename]){//File exist
        [[NSFileManager defaultManager] removeItemAtPath:strPathFilename error:&err];
        NSLog(@"remove error=%@", err);
    }
    NSString* strNewPathFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, AUIDEO_NEW_FILENAME];
    
    [[NSFileManager defaultManager] copyItemAtPath:strNewPathFilename toPath:strPathFilename error:&err];
    NSLog(@"SaveAudioFile OK, %@", strFilepathname);
    
    float fDuration = [[AudioEditManager sharedInstance]._RecTimeStampManager SyncDataAfterCutFile];
    NSNumber* _duration = [[NSNumber alloc] initWithFloat:fDuration];
    
    _DurationArray = [[NSMutableArray alloc] init];
    [_DurationArray addObject:_duration];
    NSLog(@"SaveAudioFile duration=%@", _DurationArray);
}

-(void) DataDebugAll{
    [_RecTimeStampManager DataDebugAll];
}

-(void) DataDebugData:(float)fStartTimeStamp EndTimeStamp:(float)fEndTimeStamp{
    [_RecTimeStampManager DataDebugData:fStartTimeStamp EndTimeStamp:fEndTimeStamp];
}
@end
