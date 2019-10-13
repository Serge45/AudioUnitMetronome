//
//  AudioUnitMetronome.m
//  Metronome
//
//  Created by 呂宗錡 on 2019/10/13.
//  Copyright © 2019年 Positive Grid. All rights reserved.
//

#import "AudioUnitMetronome.h"

@implementation MetronomeAudioUnit
- (void)advanceClockWithNumberSamples:(NSUInteger)numSamples {
    const double beatRatio = 4. / self.beatUnit;
    const double samplesPerBeat = self.audioFormat.sampleRate * 60 * beatRatio / self.bpm;
    
    NSAssert(self.clockPhase >= -.5 && self.clockPhase < .5, @"Invalid clock phase");
    
    while (self.clockPhase < 0) {
        self.clockPhase = self.clockPhase + 1;
    }
    
    self.clockPhase = self.clockPhase + numSamples / samplesPerBeat;
    
    while (self.clockPhase >= .5) {
        self.clockPhase -= 1;
    }
    
    NSAssert(self.clockPhase >= -.5 && self.clockPhase < .5, @"Invalid clock phase");
}

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription
                                     options:(AudioComponentInstantiationOptions)options
                                       error:(NSError * _Nullable __autoreleasing * _Nullable)outError {
    if (self = [super initWithComponentDescription:componentDescription options:options error:outError]) {
        self.beatUnit = 4;
        self.beatsPerBar = 4;
        self.bpm = 120;
        self.clockPhase = 0;
        self.currentBeat = 0;
        _audioFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100. channels:2];
        _outputBus = [[AUAudioUnitBus alloc] initWithFormat:self.audioFormat error:nil];
        _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses: @[_outputBus]];
    }
    
    return self;
}

- (AUAudioUnitBusArray *)outputBusses {
    return _outputBusArray;
}

- (AUInternalRenderBlock)internalRenderBlock {
    __block MetronomeAudioUnit *thiz = self;
    return ^AUAudioUnitStatus(AudioUnitRenderActionFlags * _Nonnull actionFlags,
                              const AudioTimeStamp * _Nonnull timestamp,
                              AUAudioFrameCount frameCount,
                              NSInteger onputBusNumber,
                              AudioBufferList * _Nonnull outputData,
                              const AURenderEvent *realtimeEventListHead,
                              AURenderPullInputBlock pullInputCallback) {
        const double beatRatio = 4. / self.beatUnit;
        const double samplesPerBeat = thiz.audioFormat.sampleRate * 60 * beatRatio / self.bpm;
        
        double beat = 0;
        
        if (thiz.clockPhase > 0) {
            beat = 1 - thiz.clockPhase;
        } else {
            beat = 0 - thiz.clockPhase;
        }
        
        for (;;beat += 1.0) {
            int pos = (int)(beat * samplesPerBeat);
            
            if (pos < frameCount) {
                [self.beatChangedDelegate onBeatChanged:thiz.currentBeat];
                ++thiz.currentBeat;
                thiz.currentBeat = thiz.currentBeat % thiz.beatsPerBar;
            } else {
                break;
            }
        }
        
        [self advanceClockWithNumberSamples:frameCount];
        return noErr;
    };
}

- (BOOL)isOutputEnabled {
    return YES;
}

- (BOOL)canPerformOutput {
    return YES;
}
@end


