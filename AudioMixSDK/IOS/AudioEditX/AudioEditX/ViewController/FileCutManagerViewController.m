//
//  FileCutManagerViewController.m
//  AudioEditX
//
//  Created by kaola on 16/2/4.
//  Copyright © 2016年 com.Alex. All rights reserved.
//

#import "FileCutManagerViewController.h"
#import "RETrimControl.h"
#import "ASValueTrackingSlider.h"
#import "AudioEditManager.h"
#import "IJKFFMoviePlayerController.h"

@interface FileCutManagerViewController ()<RETrimControlDelegate, ASValueTrackingSliderDataSource, ASValueTrackingSliderDelegate>

@end

@implementation FileCutManagerViewController
{
    UIView* _TitleBackgroudView;
    UILabel* _TitleLabel;
    UIButton* _GobackButton;
    RETrimControl* _TrimControl;
    UIButton* _FileCutAndPlayButton;
    UIButton* _FileSaveButton;
    
    UIView* _PlayerBackGroudView;
    UILabel* _TimeStampLabel;
    UILabel* _DurationLabel;
    ASValueTrackingSlider* _PlayerSlider;
    
    IJKFFMoviePlayerController* _ffRecordPlayerController;
    Boolean _isMediaSliderBeingDragged;
    Boolean _bRecordPlayFlag;
    
    float _fNewDuration;
}

@synthesize _fDuration;

-(void) UIInit{
    self.view.backgroundColor = [UIColor whiteColor];
    
    CGRect screenBounds = [ [ UIScreen mainScreen ] bounds ];
    float fScreenW = screenBounds.size.width;
    _TitleBackgroudView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, fScreenW, 50)];
    _TitleBackgroudView.backgroundColor = [UIColor yellowColor];
    [self.view addSubview:_TitleBackgroudView];
    
    float fGobackButtonW = 60;
    float fGobackButtonH = 30;
    float fGobackButtonX = 5;
    float fGobackButtonY = 16;
    _GobackButton = [[UIButton alloc] initWithFrame:CGRectMake(fGobackButtonX, fGobackButtonY, fGobackButtonW, fGobackButtonH)];
    [_GobackButton setTitle:@"返回" forState:UIControlStateNormal];
    [_GobackButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_GobackButton addTarget:self action:@selector(OnGobackClicked:)forControlEvents: UIControlEventTouchUpInside];
    [_TitleBackgroudView addSubview:_GobackButton];
    
    float fTitleLabelW = 70;
    float fTitleLabelH = 30;
    float fTitleLabelX = fScreenW/2-fTitleLabelW/2;
    float fTitleLabelY = 16;
    _TitleLabel = [[UILabel alloc] initWithFrame:CGRectMake(fTitleLabelX, fTitleLabelY, fTitleLabelW, fTitleLabelH)];
    _TitleLabel.textAlignment = NSTextAlignmentCenter;
    _TitleLabel.font = [UIFont systemFontOfSize: 14];
    _TitleLabel.text = @"录音剪辑";
    [_TitleBackgroudView addSubview:_TitleLabel];
    
    float fTrimControlW = fScreenW - 10;
    float fTrimControlH = 30;
    float fTrimControlX = 3;
    float fTrimControlY = 50+20;
    _TrimControl = [[RETrimControl alloc] initWithFrame:CGRectMake(fTrimControlX, fTrimControlY, fTrimControlW, fTrimControlH)];
    _TrimControl.length = _fDuration;
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
    
    float fPlayerBackGroudViewW = fScreenW-10;
    float fPlayerBackGroudViewH = 40;
    float fPlayerBackGroudViewX = 5;
    float fPlayerBackGroudViewY = fFileCutAndPlayButtonY + fFileCutAndPlayButtonH + 10;
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
}

-(void)refreshMediaControl{
    // duration
    NSTimeInterval duration = _fNewDuration;
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

-(void)OnFileSaveButtonClicked:(id)sender{
    
}

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
-(void)OnFileCutAndPlayClicked:(id)sender{
    if (!_bRecordPlayFlag) {
        [self FileCutOperation];
        
        [_FileCutAndPlayButton setTitle:@"停止播放" forState:UIControlStateNormal];
        [_FileCutAndPlayButton setBackgroundColor: [UIColor grayColor]];
        [_FileCutAndPlayButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        
        _ffRecordPlayerController = [[IJKFFMoviePlayerController alloc] initWithOptions:nil];
        _ffRecordPlayerController.view.frame = CGRectZero;
        
        NSString* strDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents//"];
        NSString* strNewPathFilename = [[NSString alloc] initWithFormat:@"%@/%@", strDir, AUIDEO_NEW_FILENAME];
        NSLog(@"playing record file:%@", strNewPathFilename);
        [_ffRecordPlayerController prepareToPlayWithUrl:strNewPathFilename];
        [_ffRecordPlayerController play];
        _PlayerBackGroudView.hidden = NO;
        [self performSelector:@selector(refreshMediaControl) withObject:nil afterDelay:0.5];
    }else{
        [_FileCutAndPlayButton setTitle:@"剪切试听" forState:UIControlStateNormal];
        [_FileCutAndPlayButton setBackgroundColor: [UIColor yellowColor]];
        [_FileCutAndPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        if (_ffRecordPlayerController != nil) {
            [_ffRecordPlayerController stop];
        }
        _PlayerSlider.value = 0;
        _PlayerBackGroudView.hidden = YES;
    }
    _bRecordPlayFlag = !_bRecordPlayFlag;

}

-(void)OnGobackClicked:(id)sender{
    [self dismissModalViewControllerAnimated:YES];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    _isMediaSliderBeingDragged = false;
    _bRecordPlayFlag = false;
    
    [self UIInit];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)trimControl:(RETrimControl *)trimControl didChangeLeftValue:(CGFloat)leftValue rightValue:(CGFloat)rightValue{
    NSLog(@"trimControl: %0.2f:%0.2f", leftValue, rightValue);
}

- (NSString *)slider:(ASValueTrackingSlider *)slider stringForValue:(float)value{
    NSLog(@"play slider:%0.2f", value);

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
