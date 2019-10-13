//
//  MetronomeBeatChangedPerceivable.h
//  Metronome
//
//  Created by 呂宗錡 on 2019/10/13.
//  Copyright © 2019年 Positive Grid. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol MetronomeBeatChangedPerceivable <NSObject>
- (void) onBeatChanged:(NSUInteger)beat;
@end
