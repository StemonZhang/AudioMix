//
//  AudioEditManager.h
//  AudioEditX
//
//  Created by kaola on 16/2/2.
//  Copyright © 2016年 com.Alex. All rights reserved.
//
#import "RecordTimeStampManager.h"
#import <Foundation/Foundation.h>

#define AUIDEO_DEF_FILENAME @"RecordSave.m4a"
#define AUIDEO_NEW_FILENAME @"RecordNew.m4a"

@interface AudioEditManager : NSObject

@property (assign) int _iMusicFlag;
@property (assign) int _iMicFlag;
@property NSString* _RecFilename;
@property NSLock* _fileLock;
@property RecordTimeStampManager* _RecTimeStampManager;

+(AudioEditManager*)sharedInstance;

-(void) InitAudio;

-(void) OnMusicDataHandle:(char*)pMusicData
                   Length:(int)iLength
               SampleRate:(int)iSampleRate
            ChannelNumber:(int)iChannelNumber;

-(void) OnMicDataHandle:(char*)pMicData
                 Length:(int)iLength;

-(float) GetDuration;

-(void) RecordPause;
-(void) MicFlush;
-(void) MusicFlush;

-(void) SaveAudioFile:(NSString*)strFilepathname;

-(void) DataDebugAll;
-(void) DataDebugData:(float)fStartTimeStamp EndTimeStamp:(float)fEndTimeStamp;

@end
