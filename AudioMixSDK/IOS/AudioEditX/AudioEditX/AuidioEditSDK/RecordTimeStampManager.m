//
//  RecordTimeStampManager.m
//  AudioEditX
//
//  Created by kaola on 16/2/3.
//  Copyright © 2016年 com.Alex. All rights reserved.
//

#import "RecordTimeStampManager.h"

@implementation RecordTimeStampManager
{
    NSLock* _ArrayLock;
}

@synthesize _TimeStampArray;
@synthesize _CuttingTimeStampArray;

-(id) init{
    self = [super init];
    if (self) {
        _TimeStampArray = [[NSMutableArray alloc] init];
        _CuttingTimeStampArray = [[NSMutableArray alloc] init];
        _ArrayLock = [[NSLock alloc] init];
    }
    return self;
}

-(void) InsertTimeStamp:(float)fTimeStamp FilePos:(long)lFilePos{
    NSNumber *timestamp = [[NSNumber alloc] initWithFloat:fTimeStamp];
    NSNumber *filepos = [[NSNumber alloc] initWithLong:lFilePos];
    
    NSDictionary* timePos = [NSDictionary dictionaryWithObjectsAndKeys:filepos,timestamp, nil];;

    [_ArrayLock lock];
    [_TimeStampArray addObject:timePos];
    [_ArrayLock unlock];
}

-(long) GetFilePos:(float)fTimeStamp{
    long lPos = -1;
    
    [_ArrayLock lock];
    int iArraySize = (int)[_TimeStampArray count];
    if (iArraySize <= 0) {
        return lPos;
    }else if (iArraySize <=2){
        return 0;
    }else{
        int iFirstPos = 0;
        int iMidPos = iArraySize/2;
        int iLastPos = iArraySize-1;
        
        while ((iMidPos != iFirstPos) && (iMidPos != iLastPos)){
            NSDictionary* midleDic = _TimeStampArray[iMidPos];
            NSArray *midleKeys = [midleDic allKeys];
            NSNumber* timestamp = midleKeys[0];
            if(timestamp.floatValue > fTimeStamp){
                iLastPos = iMidPos;
            }else if(timestamp.floatValue < fTimeStamp){
                iFirstPos = iMidPos;
            }else{
                break;
            }
            int iTmp = (iLastPos-iFirstPos)/2;
            if(iTmp <= 0){
                break;
            }
            iMidPos = iFirstPos + iTmp;
        }
        NSDictionary* foundDic = _TimeStampArray[iMidPos];
        NSArray *foundKeys = [foundDic allKeys];
        NSNumber* foundTimestamp = foundKeys[0];
        NSNumber* foundFilePos = [foundDic objectForKey:foundTimestamp];
        lPos = foundFilePos.longValue;
    }
    [_ArrayLock unlock];
    
    return lPos;
}

-(NSDictionary*) GetFilePosEx:(float)fTimeStamp{
    NSDictionary* PosDic = nil;
    
    [_ArrayLock lock];
    int iArraySize = (int)[_TimeStampArray count];
    if (iArraySize <= 0) {
        return PosDic;
    }else if (iArraySize <=2){
        return 0;
    }else{
        int iFirstPos = 0;
        int iMidPos = iArraySize/2;
        int iLastPos = iArraySize-1;
        
        while ((iMidPos != iFirstPos) && (iMidPos != iLastPos)){
            NSDictionary* midleDic = _TimeStampArray[iMidPos];
            NSArray *midleKeys = [midleDic allKeys];
            NSNumber* timestamp = midleKeys[0];
            if(timestamp.floatValue > fTimeStamp){
                iLastPos = iMidPos;
            }else if(timestamp.floatValue < fTimeStamp){
                iFirstPos = iMidPos;
            }else{
                break;
            }
            int iTmp = (iLastPos-iFirstPos)/2;
            if(iTmp <= 0){
                break;
            }
            iMidPos = iFirstPos + iTmp;
        }
        PosDic = _TimeStampArray[iMidPos];
    }
    [_ArrayLock unlock];
    
    return PosDic;
}

-(long) GetFilePosIndex:(float)fTimeStamp{
    long lPos = -1;
    
    [_ArrayLock lock];
    int iArraySize = (int)[_TimeStampArray count];
    if (iArraySize <= 0) {
        return lPos;
    }else if (iArraySize <=2){
        return 0;
    }else{
        int iFirstPos = 0;
        int iMidPos = iArraySize/2;
        int iLastPos = iArraySize-1;
        
        while ((iMidPos != iFirstPos) && (iMidPos != iLastPos)){
            NSDictionary* midleDic = _TimeStampArray[iMidPos];
            NSArray *midleKeys = [midleDic allKeys];
            NSNumber* timestamp = midleKeys[0];
            if(timestamp.floatValue > fTimeStamp){
                iLastPos = iMidPos;
            }else if(timestamp.floatValue < fTimeStamp){
                iFirstPos = iMidPos;
            }else{
                break;
            }
            int iTmp = (iLastPos-iFirstPos)/2;
            if(iTmp <= 0){
                break;
            }
            iMidPos = iFirstPos + iTmp;
        }
        lPos = iMidPos;
    }
    [_ArrayLock unlock];
    
    return lPos;
}

-(int) CutingFile:(float)fStartTimeStamp EndTimeStamp:(float)fEndTimeStamp{
    int iRet = 0;
    

    long lStartPos = [self GetFilePosIndex:fStartTimeStamp];
    long lEndPos = [self GetFilePosIndex:fEndTimeStamp];
    
    [_ArrayLock lock];
    lStartPos = (lStartPos < 0) ? 0 : lStartPos;
    lEndPos = (lEndPos < 0) ? [_TimeStampArray count] : lEndPos;
    NSMutableArray* tmpArray = [[NSMutableArray alloc] init];
    
    for (int iIndex = (int)lStartPos; iIndex < lEndPos; iIndex++) {
        [tmpArray addObject:_TimeStampArray[iIndex]];
    }
    _CuttingTimeStampArray = tmpArray;
    
    [_ArrayLock unlock];
    
    return iRet;
}

-(float) SyncDataAfterCutFile{
    float fDuration = 0;
    
    [_ArrayLock lock];
    NSDictionary* baseDic = _CuttingTimeStampArray[0];
    NSArray *baseKeys = [baseDic allKeys];
    NSNumber* baseTimestamp = baseKeys[0];
    NSNumber* baseFilepos   = [baseDic objectForKey:baseTimestamp];
    
    _TimeStampArray = [[NSMutableArray alloc] init];
    int iIndex = 0;
    for (iIndex=0; iIndex < _CuttingTimeStampArray.count; iIndex++) {
        NSDictionary* tmpDic = _CuttingTimeStampArray[iIndex];
        NSArray *tmpKeys = [tmpDic allKeys];
        NSNumber* tmpTimestamp = tmpKeys[0];
        NSNumber* tmpFilepos   = [tmpDic objectForKey:tmpTimestamp];
        
        NSNumber* iIndexTimestamp = [[NSNumber alloc] initWithFloat:(tmpTimestamp.floatValue-baseTimestamp.floatValue)];
        NSNumber* iIndexFilepos   = [[NSNumber alloc] initWithFloat:(tmpFilepos.longValue-baseFilepos.floatValue)];
        NSDictionary* itemDic = [NSDictionary dictionaryWithObjectsAndKeys:iIndexFilepos,iIndexTimestamp, nil];;
        
        [_TimeStampArray addObject:itemDic];
    }
    NSLog(@"SyncDataAfterCutFile: %@", _TimeStampArray);
    
    if (iIndex != 0) {
        NSDictionary* lastDic = _TimeStampArray[iIndex-1];
        NSArray *lastKeys = [lastDic allKeys];
        NSNumber* lastTimestamp = lastKeys[0];
        
        fDuration = lastTimestamp.floatValue;
    }

    [_ArrayLock unlock];
    
    return fDuration;
}

-(void) DataDebugAll{
    [_ArrayLock lock];
    NSLog(@"%@", _TimeStampArray);
    [_ArrayLock unlock];
}

-(void) DataDebugData:(float)fStartTimeStamp EndTimeStamp:(float)fEndTimeStamp{
    NSDictionary* firstPosDic = [self GetFilePosEx:fStartTimeStamp];
    NSDictionary* lastPosDic = [self GetFilePosEx:fEndTimeStamp];
    NSLog(@"start:%@, end:%@", firstPosDic, lastPosDic);
}
@end
