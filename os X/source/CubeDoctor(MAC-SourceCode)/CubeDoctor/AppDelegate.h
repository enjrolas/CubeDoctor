//
//  AppDelegate.h
//  CubetubeTool
//
//  Created by luozhuang on 15-12-3.
//  Copyright (c) 2015 Cubetube. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;

@property (weak) IBOutlet NSButton *button;
- (IBAction)btnPressed:(id)sender;
- (IBAction)openLink1:(id)sender;
- (IBAction)openLink2:(id)sender;

@property (weak) IBOutlet NSProgressIndicator *indicator;
@property (weak) IBOutlet NSProgressIndicator *progress;

@property (unsafe_unretained) IBOutlet NSTextView *textView;

@end
