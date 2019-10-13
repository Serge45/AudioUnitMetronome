//
//  AudioUnitMetronome.h
//  Metronome
//
//  Created by 呂宗錡 on 2019/10/13.
//  Copyright © 2019年 Positive Grid. All rights reserved.
//

#ifndef AudioUnitMetronome_h
#define AudioUnitMetronome_h
#import <AVFoundation/AVFoundation.h>
#import "MetronomeBeatChangedPerceivable.h"

@interface MetronomeAudioUnit : AUAudioUnit

@property (nonatomic, readonly) AUAudioUnitBus *outputBus;
@property (nonatomic, readonly) AUAudioUnitBusArray *outputBusArray;
@property NSUInteger bpm;
@property NSUInteger beatUnit;
@property NSUInteger beatsPerBar;
@property float clockPhase;
@property NSUInteger currentBeat;
@property (nonatomic, readonly) AVAudioFormat *audioFormat;
@property (weak, nonatomic) id<MetronomeBeatChangedPerceivable> beatChangedDelegate;
@end

#endif /* AudioUnitMetronome_h */
