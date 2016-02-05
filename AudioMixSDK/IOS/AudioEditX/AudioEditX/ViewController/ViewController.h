//
//  ViewController.h
//  AudioEditX
//
//  Created by kaola on 16/2/1.
//  Copyright © 2016年 com.Alex. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "IJKFFMoviePlayerController.h"
#import "AQRecorder.h"

@interface ViewController : UIViewController

@property IJKFFMoviePlayerController* _ffMoviePlayerController;
@property IJKFFMoviePlayerController* _ffRecordPlayerController;

@property UIButton* _MusicPlayButton;
@property UIButton* _MicPlayButton;
@property AQRecorder* _MicRecorder;

- (OSStatus) recordingCallback:(char*)pData
                        Length:(int)iLen;
@end

