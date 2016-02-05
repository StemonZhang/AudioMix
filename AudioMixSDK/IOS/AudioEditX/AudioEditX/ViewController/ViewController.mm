//
//  ViewController.m
//  AudioEditX
//
//  Created by kaola on 16/2/1.
//  Copyright © 2016年 com.Alex. All rights reserved.
//

#import "ViewController.h"
#import "AudioEditManager.h"
#import "MixAudio.h"
#import "ASValueTrackingSlider.h"
#import "FileCutManagerViewController.h"
#import "RETrimControl.h"

#import <MediaPlayer/MediaPlayer.h>

#define AUIDEO_SRC_FILENAME @"sample32000.mp3"

@interface ViewController ()<MPMediaPickerControllerDelegate, ASValueTrackingSliderDataSource, ASValueTrackingSliderDelegate, RETrimControlDelegate>

@end

@implementation ViewController
{
    Boolean _bStartRecFlag;
    Boolean _bMicPlayFlag;
    Boolean _bMusicPlayFlag;
    Boolean _bRecordPlayFlag;
    Boolean _isMediaSliderBeingDragged;
    
    NSString* _MusicFilename;
    UILabel* _MusicLabel;
    UILabel* _MicLabel;
    ASValueTrackingSlider* _MusicSlider;
    ASValueTrackingSlider* _MicSlider;
    
    UIButton* _RecordPlayButton;
    UIButton* _FileEditStartButton;
    
    UIView* _PlayerBackGroudView;
    UILabel* _TimeStampLabel;
    UILabel* _DurationLabel;
    ASValueTrackingSlider* _PlayerSlider;
    
    RETrimControl* _TrimControl;
    UIButton* _FileCutAndPlayButton;
    UIButton* _FileSaveButton;
    
    UIView* _EditPlayerBackGroudView;
    UILabel* _EditTimeStampLabel;
    UILabel* _EditDurationLabel;
    ASValueTrackingSlider* _EditPlayerSlider;
    
    IJKFFMoviePlayerController* _ffRecordPlayerController;
    Boolean _isEditMediaSliderBeingDragged;
    Boolean _bEditRecordPlayFlag;
    
    float _fNewDuration;

    IJKFFMoviePlayerController* _ffEditRecordPlayerController;
    //UIButton* _MainView2FileCutButton;
}
@synthesize _MicPlayButton;
@synthesize _MusicPlayButton;
@synthesize _ffMoviePlayerController;
@synthesize _MicRecorder;
@synthesize _ffRecordPlayerController;

#pragma mark - Delegate
//选中后调用
- (void)mediaPicker:(MPMediaPickerController *)mediaPicker didPickMediaItems:(MPMediaItemCollection *)mediaItemCollection{
    NSArray *items = mediaItemCollection.items;
    MPMediaItem *item = [items objectAtIndex:0];
    NSString *name = [item valueForProperty:MPMediaItemPropertyTitle];
    //NSString *filepath = [item valueForProperty:MPMediaItemPropertyAssetURL];
    NSURL *assetURL = [item valueForProperty: MPMediaItemPropertyAssetURL];
    _MusicFilename = [[NSString alloc] initWithFormat:@"%@", assetURL.absoluteString];
    NSLog(@"music name= %@, filepath=%@",name, assetURL);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_ffMoviePlayerController != nil) {
            [_ffMoviePlayerController stop];
            _ffMoviePlayerController = nil;
        }
        _ffMoviePlayerController = [[IJKFFMoviePlayerController alloc] initWithContentURL:assetURL withOptions:nil];
        [_ffMoviePlayerController play];
    });
//    MPMediaItemArtwork *artwork = [item valueForProperty:MPMediaItemPropertyArtwork];
//    UIImage *image = [artwork imageWithSize:CGSizeMake(100, 100)];//获取图片
//    //  MPMediaItemPropertyPlaybackDuration     总时间的属性名称
//    
//    //     MPMusicPlayerController *mpc = [MPMusicPlayerController iPodMusicPlayer];    //调用ipod播放器
//    MPMusicPlayerController *mpc = [MPMusicPlayerController applicationMusicPlayer];
//    //设置播放集合
//    [mpc setQueueWithItemCollection:mediaItemCollection];
//    [mpc play];
    
    [self dismissViewControllerAnimated:YES completion:nil];
}
//点击取消时回调
- (void)mediaPickerDidCancel:(MPMediaPickerController *)mediaPicker{
    [self dismissViewControllerAnimated:YES completion:nil];
}

-(void)refreshMediaControl{
    // duration
    NSTimeInterval duration = [[AudioEditManager sharedInstance] GetDuration];//_ffRecordPlayerController.duration;
    NSInteger intDuration = duration + 0.5;
    if (intDuration > 0) {
        _PlayerSlider.maximumValue = duration;
        _DurationLabel.text = [NSString stringWithFormat:@"%02d:%02d", (int)(intDuration / 60), (int)(intDuration % 60)];
    } else {
        _DurationLabel.text = @"--:--";
        _PlayerSlider.maximumValue = 1.0f;
    }
    
    // position
    NSTimeInterval position;
    if (_isMediaSliderBeingDragged) {
        position = _PlayerSlider.value;
    } else {
        position = _ffRecordPlayerController.currentPlaybackTime;
    }
    NSInteger intPosition = position + 0.5;
    if (intDuration > 0) {
        _PlayerSlider.value = position;
    } else {
        _PlayerSlider.value = 0.0f;
    }
    _TimeStampLabel.text = [NSString stringWithFormat:@"%02d:%02d", (int)(intPosition / 60), (int)(intPosition % 60)];

    // status
    BOOL isPlaying = [_ffRecordPlayerController isPlaying];

    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(refreshMediaControl) object:nil];
    if (isPlaying || _bRecordPlayFlag) {
        [self performSelector:@selector(refreshMediaControl) withObject:nil afterDelay:0.5];
    }
}

-(BOOL)IsFileExist:(NSString *)name
{
    NSFileManager *file_manager = [NSFileManager defaultManager];
    return [file_manager fileExistsAtPath:name];
}

-(void) OnStartEditRecordClicked:(id)send{
    if ((_bMicPlayFlag != 0) || (_bMusicPlayFlag != 0)) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"录音状态下不能剪辑" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }

    if (!_bStartRecFlag) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"还未开始录音, 不能剪辑" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    if (_bEditRecordPlayFlag) {
        _TrimControl.hidden = YES;
        _FileCutAndPlayButton.hidden = YES;
        _FileSaveButton.hidden = YES;
        _EditPlayerBackGroudView.hidden = YES;
        //_EditPlayerBackGroudView.hidden = YES;
        _FileEditStartButton.backgroundColor = [UIColor yellowColor];
        [_FileEditStartButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [_FileEditStartButton setTitle:@"剪切录音" forState:UIControlStateNormal];
    }else{
        _TrimControl.length = [[AudioEditManager sharedInstance] GetDuration];
        _TrimControl.hidden = NO;
        _FileCutAndPlayButton.hidden = NO;
        _FileSaveButton.hidden = NO;
        
        _FileEditStartButton.backgroundColor = [UIColor grayColor];
        [_FileEditStartButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        [_FileEditStartButton setTitle:@"停止剪切" forState:UIControlStateNormal];
        //_EditPlayerBackGroudView.hidden = NO;
    }
    _bEditRecordPlayFlag = !_bEditRecordPlayFlag;
}

-(void) OnRecordPlayClicked:(id)send{
    if ((_bMicPlayFlag != 0) || (_bMusicPlayFlag != 0)) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"录音状态下不能播放" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
    strDir = [[NSString alloc] initWithFormat:@"%@/%@", strDir, AUIDEO_DEF_FILENAME];
    
    if (!_bStartRecFlag) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"还未开始录音, 不能播放" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    
    if (!_bRecordPlayFlag) {
        [_RecordPlayButton setTitle:@"停止播放" forState:UIControlStateNormal];
        [_RecordPlayButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        [_RecordPlayButton setBackgroundColor: [UIColor grayColor]];
        
        _ffRecordPlayerController = [[IJKFFMoviePlayerController alloc] initWithOptions:nil];
        _ffRecordPlayerController.view.frame = CGRectZero;
        
        NSLog(@"playing record file:%@", strDir);
        [_ffRecordPlayerController prepareToPlayWithUrl:strDir];
        [_ffRecordPlayerController play];
        _PlayerBackGroudView.hidden = NO;
        [self performSelector:@selector(refreshMediaControl) withObject:nil afterDelay:0.5];
    }else{
        [_RecordPlayButton setTitle:@"播放录音" forState:UIControlStateNormal];
        [_RecordPlayButton setBackgroundColor: [UIColor yellowColor]];
        [_RecordPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        
        if (_ffRecordPlayerController != nil) {
            [_ffRecordPlayerController stop];
        }
        _PlayerSlider.value = 0;
        _PlayerBackGroudView.hidden = YES;
    }
    _bRecordPlayFlag = !_bRecordPlayFlag;
    
    [[AudioEditManager sharedInstance] DataDebugAll];
    [[AudioEditManager sharedInstance] DataDebugData:5.0 EndTimeStamp:10.0];
}

-(void) OnMicPlayClicked:(id)send{
    if (_bRecordPlayFlag) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"播放中不能录音" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    if (_bEditRecordPlayFlag) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"剪切中不能录音" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    _bStartRecFlag = true;
    _bMicPlayFlag = !_bMicPlayFlag;

    if (_bMicPlayFlag) {
        [AudioEditManager sharedInstance]._iMicFlag = 1;
        if (_MicRecorder == nil) {
            _MicRecorder = new AQRecorder();
        }
        _MicRecorder->StartRecord(CFSTR("recordedFile.caf"));
        
        [_MicPlayButton setTitle:@"录音停止" forState:UIControlStateNormal];
        [_MicPlayButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        [_MicPlayButton setBackgroundColor: [UIColor grayColor]];
    }else{
        [AudioEditManager sharedInstance]._iMicFlag = 0;

        _MicRecorder->PauseRecord();
        [[AudioEditManager sharedInstance] MicFlush];
        //delete _MicRecorder;
        [_MicPlayButton setTitle:@"录音开始" forState:UIControlStateNormal];
        [_MicPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [_MicPlayButton setBackgroundColor: [UIColor yellowColor]];
        if ([AudioEditManager sharedInstance]._iMusicFlag == 0) {
            [[AudioEditManager sharedInstance] RecordPause];
        }
    }
}

-(void) OnMusicPlayClicked:(id)send{
    if (_bRecordPlayFlag) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"播放中不能录音" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    if (_bEditRecordPlayFlag) {
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"剪切中不能录音" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
        [alertView show];
        return;
    }
    _bStartRecFlag = true;
    _bMusicPlayFlag = !_bMusicPlayFlag;
    if (_bMusicPlayFlag) {
        //[self musicInit];
        [AudioEditManager sharedInstance]._iMusicFlag = 1;
        [_MusicPlayButton setTitle:@"音频停止" forState:UIControlStateNormal];
        [_MusicPlayButton setBackgroundColor: [UIColor grayColor]];
        [_MusicPlayButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        
        _MusicFilename=[[NSBundle mainBundle]pathForResource:AUIDEO_SRC_FILENAME ofType:nil];
        _ffMoviePlayerController = [[IJKFFMoviePlayerController alloc] initWithOptions:nil];
        _ffMoviePlayerController.view.frame = CGRectZero;

        [_ffMoviePlayerController prepareToPlayWithUrl:_MusicFilename];
        [_ffMoviePlayerController play];
    }else{
        [AudioEditManager sharedInstance]._iMusicFlag = 0;
        [_MusicPlayButton setTitle:@"音频播放" forState:UIControlStateNormal];
        [_MusicPlayButton setBackgroundColor: [UIColor yellowColor]];
        [_MusicPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        
        if (_ffMoviePlayerController != nil) {
            [_ffMoviePlayerController stop];
        }
        [[AudioEditManager sharedInstance] MusicFlush];
        
        if ([AudioEditManager sharedInstance]._iMicFlag == 0) {
            [[AudioEditManager sharedInstance] RecordPause];
        }
    }
}

//-(void) OnMainView2FileCutViewClicked:(id)sender{
//    if ((_bMicPlayFlag != 0) || (_bMusicPlayFlag != 0)) {
//        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"录音状态下不能编辑" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
//        [alertView show];
//        return;
//    }
//    NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
//    strDir = [[NSString alloc] initWithFormat:@"%@/%@", strDir, AUIDEO_DEF_FILENAME];
//    
//    if (!_bStartRecFlag) {
//        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"提示" message:@"还未开始录音, 不能编辑" delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
//        [alertView show];
//        return;
//    }
//    FileCutManagerViewController *fileCutViewController = [[FileCutManagerViewController alloc] init];
//    fileCutViewController._fDuration = [AudioEditManager sharedInstance]._fDuration;
//    [self presentModalViewController:fileCutViewController animated:YES];
//}

-(void)FileCutOperation{
    float fStartTimeStamp = _TrimControl.leftValue;
    float fEndTimeStamp = _TrimControl.rightValue;
    
    [[AudioEditManager sharedInstance]._RecTimeStampManager CutingFile:fStartTimeStamp EndTimeStamp:fEndTimeStamp];
    long lSize = [AudioEditManager sharedInstance]._RecTimeStampManager._CuttingTimeStampArray.count;
    NSDictionary* firstDic = [AudioEditManager sharedInstance]._RecTimeStampManager._CuttingTimeStampArray[0];
    NSDictionary* lastDic = [AudioEditManager sharedInstance]._RecTimeStampManager._CuttingTimeStampArray[lSize-1];
    NSLog(@"%@:%@", firstDic, lastDic);
    
    NSArray *firstKeys = [firstDic allKeys];
    NSNumber* firstTimestamp = firstKeys[0];
    NSNumber* firstPos = [firstDic objectForKey:firstTimestamp];
    
    NSArray *lastKeys = [lastDic allKeys];
    NSNumber* lastTimestamp = lastKeys[0];
    NSNumber* lastPos = [lastDic objectForKey:lastTimestamp];
    _fNewDuration = lastTimestamp.floatValue - firstTimestamp.floatValue;
    
    NSString* strRecFilename = AUIDEO_DEF_FILENAME;
    NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
    NSString* strRecPathFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, strRecFilename];
    NSFileHandle* readFile = [NSFileHandle fileHandleForReadingAtPath:strRecPathFilename];
    long lReadSize;
    NSData* pReadData;
    
    @try{
        [readFile seekToFileOffset:firstPos.longValue];
        lReadSize = lastPos.longValue-firstPos.longValue;
        pReadData = [readFile readDataOfLength:lReadSize];
    }@catch(NSException* e){
        NSLog(@"%@", e);
        [readFile closeFile];
        return;
    }
    
    [readFile closeFile];
    
    NSString* strNewPathFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, AUIDEO_NEW_FILENAME];
    [[NSFileManager defaultManager] createFileAtPath:strNewPathFilename contents:nil attributes:nil];
    
    NSFileHandle* writeFile = [NSFileHandle fileHandleForWritingAtPath:strNewPathFilename];
    
    [writeFile writeData:pReadData];
    [writeFile closeFile];
}

-(void)refreshRecordMediaControl{
    // duration
    NSTimeInterval duration = _fNewDuration;
    NSInteger intDuration = duration + 0.5;
    if (intDuration > 0) {
        _EditPlayerSlider.maximumValue = duration;
        _EditDurationLabel.text = [NSString stringWithFormat:@"%02d:%02d", (int)(intDuration / 60), (int)(intDuration % 60)];
    } else {
        _EditDurationLabel.text = @"--:--";
        _EditPlayerSlider.maximumValue = 1.0f;
    }
    
    // position
    NSTimeInterval position;
    if (_isMediaSliderBeingDragged) {
        position = _EditPlayerSlider.value;
    } else {
        position = _ffEditRecordPlayerController.currentPlaybackTime;
    }
    NSInteger intPosition = position + 0.5;
    if (intDuration > 0) {
        _EditPlayerSlider.value = position;
    } else {
        _EditPlayerSlider.value = 0.0f;
    }
    _EditTimeStampLabel.text = [NSString stringWithFormat:@"%02d:%02d", (int)(intPosition / 60), (int)(intPosition % 60)];
    
    // status
    BOOL isPlaying = [_ffEditRecordPlayerController isPlaying];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(refreshRecordMediaControl) object:nil];
    if (isPlaying || _bEditRecordPlayFlag) {
        [self performSelector:@selector(refreshRecordMediaControl) withObject:nil afterDelay:0.5];
    }
}

-(void)OnFileCutAndPlayClicked:(id)sender{
    if (!_bRecordPlayFlag) {
        [self FileCutOperation];
        
        [_FileCutAndPlayButton setTitle:@"停止播放" forState:UIControlStateNormal];
        [_FileCutAndPlayButton setBackgroundColor: [UIColor grayColor]];
        [_FileCutAndPlayButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        
        _ffEditRecordPlayerController = [[IJKFFMoviePlayerController alloc] initWithOptions:nil];
        _ffEditRecordPlayerController.view.frame = CGRectZero;
        
        NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
        NSString* strNewPathFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, AUIDEO_NEW_FILENAME];
        NSLog(@"playing record file:%@", strNewPathFilename);
        [_ffEditRecordPlayerController prepareToPlayWithUrl:strNewPathFilename];
        [_ffEditRecordPlayerController play];
        _EditPlayerBackGroudView.hidden = NO;
        [self performSelector:@selector(refreshRecordMediaControl) withObject:nil afterDelay:0.5];
    }else{
        [_FileCutAndPlayButton setTitle:@"剪切试听" forState:UIControlStateNormal];
        [_FileCutAndPlayButton setBackgroundColor: [UIColor yellowColor]];
        [_FileCutAndPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        if (_ffEditRecordPlayerController != nil) {
            [_ffEditRecordPlayerController stop];
        }
        _PlayerSlider.value = 0;
        _EditPlayerBackGroudView.hidden = YES;
    }
    _bRecordPlayFlag = !_bRecordPlayFlag;
    
}

-(void) OnFileSaveButtonClicked:(id)sender{
    [[AudioEditManager sharedInstance] SaveAudioFile:AUIDEO_DEF_FILENAME];
    NSString* strDscr = [[NSString alloc] initWithFormat:@"文件%@已保存", AUIDEO_DEF_FILENAME];
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"剪切保存" message:strDscr delegate:self cancelButtonTitle:@"确定" otherButtonTitles:nil];
    [alertView show];
}

-(void) UIInit {
    CGRect screenBounds = [ [ UIScreen mainScreen ] bounds ];
    float fScreenW = screenBounds.size.width;
    //float fScreenH = screenBounds.size.height;
    
    float fMicPlayButtonW = 60;
    float fMicPlayButtonH = 60;
    float fMicPlayButtonX = fScreenW/2-fMicPlayButtonW-10;
    float fMicPlayButtonY = 50;
    _MicPlayButton = [[UIButton alloc] initWithFrame:CGRectMake(fMicPlayButtonX, fMicPlayButtonY, fMicPlayButtonW, fMicPlayButtonH)];
    _MicPlayButton.titleLabel.font = [UIFont systemFontOfSize: 11];
    [_MicPlayButton setBackgroundColor: [UIColor yellowColor]];
    [_MicPlayButton setTitle:@"录音开始" forState:UIControlStateNormal];
    [_MicPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    _MicPlayButton.layer.cornerRadius = fMicPlayButtonW/2;
    _MicPlayButton.layer.masksToBounds = YES;
    [_MicPlayButton addTarget:self action:@selector(OnMicPlayClicked:)forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview:_MicPlayButton];
    
    float fMusicPlayButtonW = 60;
    float fMusicPlayButtonH = 60;
    float fMusicPlayButtonX = fScreenW/2+10;
    float fMusicPlayButtonY = 50;
    _MusicPlayButton = [[UIButton alloc] initWithFrame:CGRectMake(fMusicPlayButtonX, fMusicPlayButtonY, fMusicPlayButtonW, fMusicPlayButtonH)];
    _MusicPlayButton.titleLabel.font = [UIFont systemFontOfSize: 11];
    [_MusicPlayButton setBackgroundColor: [UIColor yellowColor]];
    [_MusicPlayButton setTitle:@"音频播放" forState:UIControlStateNormal];
    [_MusicPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    _MusicPlayButton.layer.cornerRadius = fMusicPlayButtonW/2;
    _MusicPlayButton.layer.masksToBounds = YES;
    [_MusicPlayButton addTarget:self action:@selector(OnMusicPlayClicked:)forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview:_MusicPlayButton];
    
    float fMicLabelX = 10;
    float fMicLabelY = fMusicPlayButtonY + fMusicPlayButtonH + 10;
    float fMicLabelW = 70;
    float fMicLabelH = 30;
    _MicLabel = [[UILabel alloc] initWithFrame:CGRectMake(fMicLabelX, fMicLabelY, fMicLabelW, fMicLabelH)];
    _MicLabel.font = [UIFont systemFontOfSize: 14];
    _MicLabel.text = @"录音音量:";
    [self.view addSubview:_MicLabel];
    
    float fMicSliderX = 80;
    float fMicSliderY = fMusicPlayButtonY + fMusicPlayButtonH + 10;
    float fMicSliderW = fScreenW - fMicSliderX - 10;
    float fMicSliderH = 30;
    _MicSlider = [[ASValueTrackingSlider alloc] initWithFrame:CGRectMake(fMicSliderX, fMicSliderY, fMicSliderW, fMicSliderH)];
    _MicSlider.maximumValue = 10.0;
    _MicSlider.popUpViewCornerRadius = 4;
    [_MicSlider setMaxFractionDigitsDisplayed:0];
    _MicSlider.popUpViewColor = [UIColor colorWithHue:0.55 saturation:0.8 brightness:0.9 alpha:0.7];
    _MicSlider.font = [UIFont fontWithName:@"GillSans-Bold" size:18];
    _MicSlider.textColor = [UIColor colorWithHue:0.55 saturation:1.0 brightness:0.5 alpha:1];
    _MicSlider.popUpViewWidthPaddingFactor = 1.7;
    _MicSlider.delegate = self;
    _MicSlider.dataSource = self;
    _MicSlider.value = 9;
    [self.view addSubview:_MicSlider];
    
    float fMusicLabelX = 10;
    float fMusicLabelY = fMicSliderY + fMicSliderH + 10;
    float fMusicLabelW = 70;
    float fMusicLabelH = 30;
    _MusicLabel = [[UILabel alloc] initWithFrame:CGRectMake(fMusicLabelX, fMusicLabelY, fMusicLabelW, fMusicLabelH)];
    _MusicLabel.font = [UIFont systemFontOfSize: 14];
    _MusicLabel.text = @"音乐音量:";
    [self.view addSubview:_MusicLabel];
    
    float fMusicSliderX = 80;
    float fMusicSliderY = fMicSliderY + fMicSliderH + 10;
    float fMusicSliderW = fScreenW - fMusicSliderX - 10;
    float fMusicSliderH = 30;
    _MusicSlider = [[ASValueTrackingSlider alloc] initWithFrame:CGRectMake(fMusicSliderX, fMusicSliderY, fMusicSliderW, fMusicSliderH)];
    _MusicSlider.maximumValue = 10.0;
    _MusicSlider.popUpViewCornerRadius = 4;
    [_MusicSlider setMaxFractionDigitsDisplayed:0];
    _MusicSlider.popUpViewColor = [UIColor colorWithHue:0.55 saturation:0.8 brightness:0.9 alpha:0.7];
    _MusicSlider.font = [UIFont fontWithName:@"GillSans-Bold" size:18];
    _MusicSlider.textColor = [UIColor colorWithHue:0.55 saturation:1.0 brightness:0.5 alpha:1];
    _MusicSlider.popUpViewWidthPaddingFactor = 1.7;
    _MusicSlider.delegate = self;
    _MusicSlider.dataSource = self;
    _MusicSlider.value = 2.0;
    [self.view addSubview:_MusicSlider];
    
    float fRecordPlayButtonW = 60;
    float fRecordPlayButtonH = 60;
    float fRecordPlayButtonX = fScreenW/2-fRecordPlayButtonW-10;
    float fRecordPlayButtonY = fMusicSliderY+fMusicSliderH+30;
    _RecordPlayButton = [[UIButton alloc] initWithFrame:CGRectMake(fRecordPlayButtonX, fRecordPlayButtonY, fRecordPlayButtonW, fRecordPlayButtonH)];
    _RecordPlayButton.titleLabel.font = [UIFont systemFontOfSize: 11];
    [_RecordPlayButton setBackgroundColor: [UIColor yellowColor]];
    [_RecordPlayButton setTitle:@"播放录音" forState:UIControlStateNormal];
    [_RecordPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    _RecordPlayButton.layer.cornerRadius = fRecordPlayButtonW/2;
    _RecordPlayButton.layer.masksToBounds = YES;
    [_RecordPlayButton addTarget:self action:@selector(OnRecordPlayClicked:)forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview:_RecordPlayButton];
    
    float fFileEditStartButtonW = 60;
    float fFileEditStartButtonH = 60;
    float fFileEditStartButtonX = fScreenW/2+10;
    float fFileEditStartButtonY = fMusicSliderY+fMusicSliderH+30;
    _FileEditStartButton = [[UIButton alloc] initWithFrame:CGRectMake(fFileEditStartButtonX, fFileEditStartButtonY, fFileEditStartButtonW, fFileEditStartButtonH)];
    _FileEditStartButton.titleLabel.font = [UIFont systemFontOfSize: 11];
    [_FileEditStartButton setBackgroundColor: [UIColor yellowColor]];
    [_FileEditStartButton setTitle:@"剪切录音" forState:UIControlStateNormal];
    [_FileEditStartButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    _FileEditStartButton.layer.cornerRadius = fRecordPlayButtonW/2;
    _FileEditStartButton.layer.masksToBounds = YES;
    [_FileEditStartButton addTarget:self action:@selector(OnStartEditRecordClicked:)forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview:_FileEditStartButton];
    
    float fPlayerBackGroudViewW = fScreenW-10;
    float fPlayerBackGroudViewH = 40;
    float fPlayerBackGroudViewX = 5;
    float fPlayerBackGroudViewY = fRecordPlayButtonY + fRecordPlayButtonH + 10;
    _PlayerBackGroudView = [[UIView alloc] initWithFrame:CGRectMake(fPlayerBackGroudViewX, fPlayerBackGroudViewY, fPlayerBackGroudViewW, fPlayerBackGroudViewH)];
    _PlayerBackGroudView.backgroundColor = [UIColor lightGrayColor];
    _PlayerBackGroudView.layer.cornerRadius = 10;
    _PlayerBackGroudView.layer.masksToBounds = YES;
    _PlayerBackGroudView.hidden = YES;
    [self.view addSubview:_PlayerBackGroudView];
    
    float fTimeStampLabelW = 60;
    float fTimeStampLabelH = 30;
    float fTimeStampLabelX = 5;
    float fTimeStampLabelY = 5;
    _TimeStampLabel = [[UILabel alloc] initWithFrame:CGRectMake(fTimeStampLabelX, fTimeStampLabelY, fTimeStampLabelW, fTimeStampLabelH)];
    _TimeStampLabel.font = [UIFont systemFontOfSize: 12];
    _TimeStampLabel.textColor = [UIColor whiteColor];
    _TimeStampLabel.text = @"00:00";
    [_PlayerBackGroudView addSubview:_TimeStampLabel];
    
    float fPlayerSliderX = fTimeStampLabelX + fTimeStampLabelW;
    float fPlayerSliderY = fTimeStampLabelY;
    float fPlayerSliderH = fTimeStampLabelH;
    float fPlayerSliderW = fPlayerBackGroudViewW - fTimeStampLabelW*2 - 10;
    _PlayerSlider = [[ASValueTrackingSlider alloc] initWithFrame:CGRectMake(fPlayerSliderX, fPlayerSliderY, fPlayerSliderW, fPlayerSliderH)];
    _PlayerSlider.popUpViewCornerRadius = 4;
    [_PlayerSlider setMaxFractionDigitsDisplayed:0];
    _PlayerSlider.popUpViewColor = [UIColor colorWithHue:0.55 saturation:0.8 brightness:0.9 alpha:0.7];
    _PlayerSlider.font = [UIFont fontWithName:@"GillSans-Bold" size:18];
    _PlayerSlider.textColor = [UIColor colorWithHue:0.55 saturation:1.0 brightness:0.5 alpha:1];
    _PlayerSlider.popUpViewWidthPaddingFactor = 1.7;
    _PlayerSlider.delegate = self;
    _PlayerSlider.dataSource = self;
    _PlayerSlider.value = 0.0;
    [_PlayerBackGroudView addSubview:_PlayerSlider];

    float fDurationLabelW = 60;
    float fDurationLabelH = 30;
    float fDurationLabelX = fPlayerSliderX+fPlayerSliderW+5;
    float fDurationLabelY = 5;
    _DurationLabel = [[UILabel alloc] initWithFrame:CGRectMake(fDurationLabelX, fDurationLabelY, fDurationLabelW, fDurationLabelH)];
    _DurationLabel.font = [UIFont systemFontOfSize: 12];
    _DurationLabel.textColor = [UIColor whiteColor];
    _DurationLabel.text = @"00:00";
    [_PlayerBackGroudView addSubview:_DurationLabel];
    
//    float fMainView2FileCutButtonW = 60;
//    float fMainView2FileCutButtonH = fMainView2FileCutButtonW;
//    float fMainView2FileCutButtonX = fScreenW/2-fMainView2FileCutButtonW/2;
//    float fMainView2FileCutButtonY = fPlayerBackGroudViewY + fPlayerBackGroudViewH + 100;
//    _MainView2FileCutButton = [[UIButton alloc] initWithFrame:CGRectMake(fMainView2FileCutButtonX, fMainView2FileCutButtonY, fMainView2FileCutButtonW, fMainView2FileCutButtonH)];
//    _MainView2FileCutButton.backgroundColor = [UIColor yellowColor];
//    _MainView2FileCutButton.titleLabel.textAlignment = NSTextAlignmentCenter;
//    [_MainView2FileCutButton setTitle:@"录像剪切" forState:UIControlStateNormal];
//    [_MainView2FileCutButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
//    _MainView2FileCutButton.layer.cornerRadius = fMainView2FileCutButtonW/2;
//    _MainView2FileCutButton.layer.masksToBounds = YES;
//    _MainView2FileCutButton.titleLabel.font = [UIFont systemFontOfSize: 12];
//    [_MainView2FileCutButton addTarget:self action:@selector(OnMainView2FileCutViewClicked:)forControlEvents: UIControlEventTouchUpInside];
//    [self.view addSubview:_MainView2FileCutButton];
    
    float fTrimControlW = fScreenW - 10;
    float fTrimControlH = 30;
    float fTrimControlX = 3;
    float fTrimControlY = fPlayerBackGroudViewY + fPlayerBackGroudViewH + 10;
    _TrimControl = [[RETrimControl alloc] initWithFrame:CGRectMake(fTrimControlX, fTrimControlY, fTrimControlW, fTrimControlH)];
    _TrimControl.length = 10;
    _TrimControl.delegate = self;
    [self.view addSubview:_TrimControl];
    
    float fFileCutAndPlayButtonW = 60;
    float fFileCutAndPlayButtonH = 30;
    float fFileCutAndPlayButtonX = fScreenW/2-fFileCutAndPlayButtonW-10;
    float fFileCutAndPlayButtonY = fTrimControlY + fTrimControlH +20;
    _FileCutAndPlayButton = [[UIButton alloc] initWithFrame:CGRectMake(fFileCutAndPlayButtonX, fFileCutAndPlayButtonY, fFileCutAndPlayButtonW, fFileCutAndPlayButtonH)];
    [_FileCutAndPlayButton setTitle:@"剪切试听" forState:UIControlStateNormal];
    _FileCutAndPlayButton.titleLabel.font = [UIFont systemFontOfSize: 13];
    _FileCutAndPlayButton.backgroundColor = [UIColor yellowColor];
    [_FileCutAndPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    _FileCutAndPlayButton.layer.cornerRadius = 5;
    _FileCutAndPlayButton.layer.masksToBounds = YES;
    [_FileCutAndPlayButton addTarget:self action:@selector(OnFileCutAndPlayClicked:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_FileCutAndPlayButton];
    
    float fFileSaveButtonW = 60;
    float fFileSaveButtonH = 30;
    float fFileSaveButtonX = fScreenW/2+10;
    float fFileSaveButtonY = fFileCutAndPlayButtonY;
    _FileSaveButton = [[UIButton alloc] initWithFrame:CGRectMake(fFileSaveButtonX, fFileSaveButtonY, fFileSaveButtonW, fFileSaveButtonH)];
    [_FileSaveButton setTitle:@"保存文件" forState:UIControlStateNormal];
    _FileSaveButton.titleLabel.font = [UIFont systemFontOfSize: 13];
    _FileSaveButton.backgroundColor = [UIColor yellowColor];
    [_FileSaveButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    _FileSaveButton.layer.cornerRadius = 5;
    _FileSaveButton.layer.masksToBounds = YES;
    [_FileSaveButton addTarget:self action:@selector(OnFileSaveButtonClicked:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_FileSaveButton];
    
    float fEditPlayerBackGroudViewW = fScreenW-10;
    float fEditPlayerBackGroudViewH = 40;
    float fEditPlayerBackGroudViewX = 5;
    float fEditPlayerBackGroudViewY = fFileCutAndPlayButtonY + fFileCutAndPlayButtonH + 10;
    _EditPlayerBackGroudView = [[UIView alloc] initWithFrame:CGRectMake(fEditPlayerBackGroudViewX, fEditPlayerBackGroudViewY, fEditPlayerBackGroudViewW, fEditPlayerBackGroudViewH)];
    _EditPlayerBackGroudView.backgroundColor = [UIColor lightGrayColor];
    _EditPlayerBackGroudView.layer.cornerRadius = 10;
    _EditPlayerBackGroudView.layer.masksToBounds = YES;
    _EditPlayerBackGroudView.hidden = YES;
    [self.view addSubview:_EditPlayerBackGroudView];
    
    float fEditTimeStampLabelW = 60;
    float fEditTimeStampLabelH = 30;
    float fEditTimeStampLabelX = 5;
    float fEditTimeStampLabelY = 5;
    _EditTimeStampLabel = [[UILabel alloc] initWithFrame:CGRectMake(fEditTimeStampLabelX, fEditTimeStampLabelY, fEditTimeStampLabelW, fEditTimeStampLabelH)];
    _EditTimeStampLabel.font = [UIFont systemFontOfSize: 12];
    _EditTimeStampLabel.textColor = [UIColor whiteColor];
    _EditTimeStampLabel.text = @"00:00";
    [_EditPlayerBackGroudView addSubview:_EditTimeStampLabel];
    
    float fEditPlayerSliderX = fEditTimeStampLabelX + fEditTimeStampLabelW;
    float fEditPlayerSliderY = fEditTimeStampLabelY;
    float fEditPlayerSliderH = fEditTimeStampLabelH;
    float fEditPlayerSliderW = fEditPlayerBackGroudViewW - fEditTimeStampLabelW*2 - 10;
    _EditPlayerSlider = [[ASValueTrackingSlider alloc] initWithFrame:CGRectMake(fEditPlayerSliderX, fEditPlayerSliderY, fEditPlayerSliderW, fEditPlayerSliderH)];
    _EditPlayerSlider.popUpViewCornerRadius = 4;
    [_EditPlayerSlider setMaxFractionDigitsDisplayed:0];
    _EditPlayerSlider.popUpViewColor = [UIColor colorWithHue:0.55 saturation:0.8 brightness:0.9 alpha:0.7];
    _EditPlayerSlider.font = [UIFont fontWithName:@"GillSans-Bold" size:18];
    _EditPlayerSlider.textColor = [UIColor colorWithHue:0.55 saturation:1.0 brightness:0.5 alpha:1];
    _EditPlayerSlider.popUpViewWidthPaddingFactor = 1.7;
    _EditPlayerSlider.delegate = self;
    _EditPlayerSlider.dataSource = self;
    _EditPlayerSlider.value = 0.0;
    [_EditPlayerBackGroudView addSubview:_EditPlayerSlider];
    
    float fEditDurationLabelW = 60;
    float fEditDurationLabelH = 30;
    float fEditDurationLabelX = fEditPlayerSliderX+fEditPlayerSliderW+5;
    float fEditDurationLabelY = 5;
    _EditDurationLabel = [[UILabel alloc] initWithFrame:CGRectMake(fEditDurationLabelX, fEditDurationLabelY, fEditDurationLabelW, fEditDurationLabelH)];
    _EditDurationLabel.font = [UIFont systemFontOfSize: 12];
    _EditDurationLabel.textColor = [UIColor whiteColor];
    _EditDurationLabel.text = @"00:00";
    [_EditPlayerBackGroudView addSubview:_EditDurationLabel];
    
    _TrimControl.hidden = YES;
    _FileCutAndPlayButton.hidden = YES;
    _FileSaveButton.hidden = YES;
    _EditPlayerBackGroudView.hidden = YES;
}

-(void)musicInit{
    //创建播放器控制器
    MPMediaPickerController *mpc = [[MPMediaPickerController alloc] initWithMediaTypes:MPMediaTypeAnyAudio];
    //设置代理
    mpc.delegate = self;
    [self presentViewController:mpc animated:YES completion:nil];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    _bMicPlayFlag = false;
    _bMusicPlayFlag = false;
    _MicRecorder = nil;
    _bRecordPlayFlag = false;
    _isMediaSliderBeingDragged = false;
    _bStartRecFlag = false;
    _bEditRecordPlayFlag = false;
    
    [self UIInit];
}

- (OSStatus) recordingCallback:(char*)pData
                        Length:(int)iLen{
    [[AudioEditManager sharedInstance] OnMicDataHandle:pData Length:iLen];
    
    return 0;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSString *)slider:(ASValueTrackingSlider *)slider stringForValue:(float)value{
    if (slider == _MusicSlider) {
        float fMusicGain = value/10.0;
        NSLog(@"music slider:%0.2f, %0.2f", value, fMusicGain);
        SetMusicGain(fMusicGain);
    }
    if (slider == _MicSlider) {
        float fMicGain = value/10.0*4;
        NSLog(@"mic slider:%0.2f, %0.2f", value, fMicGain);
        SetMicGain(fMicGain);
    }

    return nil;
}

- (void)sliderWillDisplayPopUpView:(ASValueTrackingSlider *)slider{
    NSLog(@"sliderWillDisplayPopUpView...");
    if (slider == _PlayerSlider) {
        _isMediaSliderBeingDragged = true;
    }
    return;
}

- (void)sliderWillHidePopUpView:(ASValueTrackingSlider *)slider{
    NSLog(@"sliderWillHidePopUpView...");
    if (slider == _PlayerSlider) {
        _ffRecordPlayerController.currentPlaybackTime = _PlayerSlider.value;
        _isMediaSliderBeingDragged = false;
    }
}
@end
