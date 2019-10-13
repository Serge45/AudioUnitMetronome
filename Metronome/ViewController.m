//
//  ViewController.m
//  Metronome
//
//  Created by 呂宗錡 on 2019/10/12.
//  Copyright © 2019年 Positive Grid. All rights reserved.
//

#import "ViewController.h"
#import "AudioUnitMetronome.h"

@interface ViewController () {
}
@property (nonatomic, retain) AVAudioUnit *metronomeNode;
@property (nonatomic, retain) AVAudioUnitSampler *sampler;
@property (nonatomic, retain) AVAudioEngine *audioEngine;
@property (weak, nonatomic) IBOutlet UILabel *beatLabel;
@property (weak, nonatomic) IBOutlet UIPickerView *bpmPicker;
@property (weak, nonatomic) IBOutlet UISlider *gainSlider;
@property (weak, nonatomic) IBOutlet UIPickerView *timeSignaturePicker;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.beatLabel.textAlignment = NSTextAlignmentCenter;
    self.bpmPicker.dataSource = self;
    self.bpmPicker.delegate = self;
    self.timeSignaturePicker.dataSource = self;
    self.timeSignaturePicker.delegate = self;
    [self.bpmPicker selectRow:120 - 40 inComponent:0 animated:NO];
    [self.timeSignaturePicker selectRow:3 inComponent:0 animated:NO];
    [self.timeSignaturePicker selectRow:3 inComponent:1 animated:NO];
}

- (void)viewWillAppear:(BOOL)animated {
    [self initAudioEngine];
    [self initAudioNodes];
}

- (void)viewWillDisappear:(BOOL)animated {
    [_audioEngine stop];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)gainSliderValueChanged:(id)sender {
    self.sampler.masterGain = self.gainSlider.value;
}

- (void)initAudioEngine {
    _audioEngine = [[AVAudioEngine alloc] init];
}

- (void)initAudioNodes {
    AudioComponentDescription samplerDesc = {
        .componentType = kAudioUnitType_MusicDevice,
        .componentSubType = kAudioUnitSubType_Sampler,
        .componentManufacturer = kAudioUnitManufacturer_Apple,
        .componentFlags = 0,
        .componentFlagsMask = 0
    };
    self.sampler = [[AVAudioUnitSampler alloc] initWithAudioComponentDescription:samplerDesc];
    NSURL *resourceRoot = [NSBundle mainBundle].resourceURL;
    NSURL *lowSoundUrl = [[NSURL alloc] initFileURLWithPath:@"MetronomeLow.wav" relativeToURL:resourceRoot];
    NSError *err = nil;
    [self.sampler loadAudioFilesAtURLs:@[lowSoundUrl] error:&err];
    NSAssert(err == nil, @"Cannot init sampler");
    AudioComponentDescription desc = {kAudioUnitType_MusicDevice, 'metr', 'posg', 0, 0};
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [AUAudioUnit registerSubclass:[MetronomeAudioUnit class]
               asComponentDescription:desc
                                 name:@"metr"
                              version:0];
    });
    
    __block ViewController *thiz = self;
    [AVAudioUnit instantiateWithComponentDescription:desc
                                             options:kAudioComponentInstantiation_LoadOutOfProcess
                                   completionHandler:^(__kindof AVAudioUnit * _Nullable audioUnit, NSError * _Nullable error) {
        thiz.metronomeNode = audioUnit;
        ((MetronomeAudioUnit *)thiz.metronomeNode.AUAudioUnit).beatChangedDelegate = thiz;
        [thiz.audioEngine attachNode:thiz.metronomeNode];
        [thiz.audioEngine attachNode:thiz.sampler];
        [thiz.audioEngine connect: thiz.metronomeNode to: thiz.audioEngine.mainMixerNode format:nil];
        [thiz.audioEngine connect: thiz.sampler to: thiz.audioEngine.mainMixerNode format:nil];
        [thiz.audioEngine prepare];
        [thiz.audioEngine startAndReturnError: nil];
    }];
}

- (void)onBeatChanged:(NSUInteger)beat {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.beatLabel.text = [NSString stringWithFormat:@"%ld", beat];
    });
    
    const NSUInteger beatsPerBar = ((MetronomeAudioUnit *)self.metronomeNode.AUAudioUnit).beatsPerBar;
    [self.sampler startNote:!(beat % beatsPerBar) ? 66 : 60 withVelocity:128 onChannel:0];
}

- (NSInteger)numberOfComponentsInPickerView:(nonnull UIPickerView *)pickerView {
    if (pickerView == self.bpmPicker) {
        return 1;
    } else if (pickerView == self.timeSignaturePicker) {
        return 2;
    }
    
    return 0;
}

- (NSInteger)pickerView:(nonnull UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component {
    if (pickerView == self.bpmPicker) {
        return 400 - 40 + 1;
    } else if (pickerView == self.timeSignaturePicker) {
        return 32;
    }
    
    return 0;
}

- (nullable NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component {
    if (pickerView == self.bpmPicker) {
        return [NSString stringWithFormat:@"%ld", row + 40];
    } else if (pickerView == self.timeSignaturePicker) {
        return [NSString stringWithFormat:@"%ld", row + 1];
    }
    
    return nil;
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
    if (pickerView == self.bpmPicker) {
        if (self.metronomeNode) {
            ((MetronomeAudioUnit *)self.metronomeNode.AUAudioUnit).bpm = row + 40;
        }
    } else if (pickerView == self.timeSignaturePicker) {
        if (component == 0) {
            ((MetronomeAudioUnit *)self.metronomeNode.AUAudioUnit).beatsPerBar = row + 1;
        } else if (component == 1) {
            ((MetronomeAudioUnit *)self.metronomeNode.AUAudioUnit).beatUnit = row + 1;
        }
    }
}

@end
