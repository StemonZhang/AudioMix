//
//  RecordTimeStampManager.h
//  AudioEditX
//
//  Created by kaola on 16/2/3.
//  Copyright © 2016年 com.Alex. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface RecordTimeStampManager : NSObject

@property NSMutableArray* _TimeStampArray;
@property NSMutableArray* _CuttingTimeStampArray;

-(id) init;

-(void) InsertTimeStamp:(float)fTimeStamp FilePos:(long)lFilePos;

-(long) GetFilePos:(float)fTimeStamp;

-(NSDictionary*) GetFilePosEx:(float)fTimeStamp;

-(int) CutingFile:(float)fStartTimeStamp EndTimeStamp:(float)fEndTimeStamp;

-(float) SyncDataAfterCutFile;

-(void) DataDebugAll;

-(void) DataDebugData:(float)fStartTimeStamp EndTimeStamp:(float)fEndTimeStamp;

@end
