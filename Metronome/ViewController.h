//
//  ViewController.h
//  Metronome
//
//  Created by 呂宗錡 on 2019/10/12.
//  Copyright © 2019年 Positive Grid. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MetronomeBeatChangedPerceivable.h"

@interface ViewController : UIViewController<MetronomeBeatChangedPerceivable, UIPickerViewDataSource, UIPickerViewDelegate>


@end

