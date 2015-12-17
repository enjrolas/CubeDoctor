    //
//  AppDelegate.m
//  CubetubeTool
//
//  Created by luozhuang on 15-12-3.
//  Copyright (c) 2015 Cubetube. All rights reserved.
//

#import "AppDelegate.h"

enum EStep
{
    kEStepCheckDeviceInitStatus,
    kEStepFindDevice,
    kEStepMax
};

typedef enum EStep EStep;

@implementation AppDelegate
{
    NSString *tmpRbFilePath1;// install rb file
    NSString *tmpRbFilePath2;// uninstall rb file
    
    BOOL bNeedUninstall;
    BOOL bNeedInstallXRun;
    
    NSTask *unixTask;
    NSPipe *unixStandardOutputPipe;
    NSPipe *unixStandardErrorPipe;
    NSPipe *unixStandardInputPipe;
    NSFileHandle *fhOutput;
    NSFileHandle *fhError;
    NSFileHandle *fhInput;
    NSData *standardOutputData;
    NSData *standardErrorData;

    EStep curStep;
    BOOL bRunInstallHomebrewOk;
    BOOL bInstallDfuUtil;
    BOOL bCheckDeviceInitStatus;
    BOOL bFindDevice;
    
    // dwonload file path
    NSString *file1;
    NSString *file2;
    NSString *file3;
    
    NSString *deviceName;
    
    NSString *pas;
    
}

//[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    [self.indicator setHidden:YES];
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication
                    hasVisibleWindows:(BOOL)flag{
    if (!flag)
    {
        NSWindow *w = theApplication.windows[0];
        [w makeKeyAndOrderFront:self];
    }
    return YES;
}

- (IBAction)btnPressed:(id)sender
{
    NSLog(@"%@",NSTemporaryDirectory());
    bNeedInstallXRun = NO;
    bNeedUninstall = NO;
    self.button.enabled = NO;
    self.textView.string = @"";
    [self.indicator startAnimation:nil];
    [self performSelectorInBackground:@selector(allStep) withObject:nil];
}

- (IBAction)openLink1:(id)sender {
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.cubetube.org"]];
}
- (IBAction)openLink2:(id)sender {
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/enjrolas/CubeDoctor"]];
}

#pragma mark -
// show log info
- (void)showLog:(NSString *)info
{
    if(!info) return;
    NSLog(@"%@",info);
    dispatch_async(dispatch_get_main_queue(), ^{
        [[[self.textView textStorage] mutableString] appendString:info];
        [self.textView scrollToEndOfDocument:nil];
    });
}

// set progress
- (void)setProgressValue:(double)v
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progress.doubleValue = v;
    });
}

#pragma mark -
- (NSString *)runCmd:(NSString *)cmd andArgs:(NSString *)args
{
    NSString *log = [NSString stringWithFormat:@"%@ %@\n",cmd,args];
    [self showLog:log];
    @try
    {
        NSArray *arr = [args componentsSeparatedByString:@" "];
        
        NSTask *task;
        task = [[NSTask alloc ]init];
        [task setLaunchPath:cmd];
        [task setArguments:arr];
        
        NSPipe *pipe;
        pipe = [NSPipe pipe];
        [task setStandardOutput:pipe];
        
        NSFileHandle *file;
        file = [pipe fileHandleForReading];
        
        [task launch];
        
        NSData *data;
        data = [file readDataToEndOfFile];
        
        NSString *string;
        string = [[NSString alloc]initWithData:data encoding:NSUTF8StringEncoding];
        
        return string;
    }@catch (NSException * e)
    {
        NSString *log = [NSString stringWithFormat:@"Exception: %@", e];
        [self showLog:log];
        return nil;
    }
    
}

- (void)runCmd:(NSString *)cmd withArgs:(NSString *)args
{
    NSString *log = [NSString stringWithFormat:@"cmd:%@ %@\n",cmd,args];
    [self showLog:log];
    @try
    {
        //setup system pipes and filehandles to process output data
        unixStandardOutputPipe = [[NSPipe alloc] init];
        unixStandardErrorPipe = [[NSPipe alloc] init];
        unixStandardInputPipe = [[NSPipe alloc] init];        
        fhOutput = [unixStandardOutputPipe fileHandleForReading];
        fhError = [unixStandardErrorPipe fileHandleForReading];
        fhInput = [unixStandardInputPipe fileHandleForWriting];        
        //setup notification alerts
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(notifiedForStdOutput:) name:NSFileHandleReadCompletionNotification object:fhOutput];
        [nc addObserver:self selector:@selector(notifiedForStdError:) name:NSFileHandleReadCompletionNotification object:fhError];
        [nc addObserver:self selector:@selector(notifiedForComplete:) name:NSTaskDidTerminateNotification object:unixTask];

        NSArray *arr = [args componentsSeparatedByString:@" "];

        unixTask = [[NSTask alloc ]init];
        [unixTask setLaunchPath:cmd];
        [unixTask setArguments:arr];

        [unixTask setStandardOutput:unixStandardOutputPipe];
        [unixTask setStandardError:unixStandardErrorPipe];
        [unixTask setStandardInput:unixStandardInputPipe];

        //note we are calling the file handle not the pipe
        [fhOutput readInBackgroundAndNotify];
        [fhError readInBackgroundAndNotify];

        [unixTask launch];
        [unixTask waitUntilExit];
    }@catch (NSException * e)
    {
        NSString *log = [NSString stringWithFormat:@"cmd exception:%@", e];
        if([unixTask isRunning])
        {
            [unixTask terminate];
        }
        unixTask = nil;
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        
        [self showLog:log];
    }
}


- (void)runRubyWithArgs:(NSString *)args
{
    NSString *log = [NSString stringWithFormat:@"ruby:%@\n",args];
    [self showLog:log];
    @try
    {
        //setup system pipes and filehandles to process output data
        unixStandardOutputPipe = [[NSPipe alloc] init];
        unixStandardErrorPipe = [[NSPipe alloc] init];
        unixStandardInputPipe = [[NSPipe alloc] init];
        fhOutput = [unixStandardOutputPipe fileHandleForReading];
        fhError = [unixStandardErrorPipe fileHandleForReading];
        fhInput = [unixStandardInputPipe fileHandleForWriting];
        //setup notification alerts
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(notifiedForStdOutput:) name:NSFileHandleReadCompletionNotification object:fhOutput];
        [nc addObserver:self selector:@selector(notifiedForStdError:) name:NSFileHandleReadCompletionNotification object:fhError];
        [nc addObserver:self selector:@selector(notifiedForComplete:) name:NSTaskDidTerminateNotification object:unixTask];
        
        NSArray *arr = [args componentsSeparatedByString:@" "];
        
        unixTask = [[NSTask alloc ]init];
        [unixTask setLaunchPath:@"/usr/bin/ruby"];
        [unixTask setArguments:arr];
        
        [unixTask setStandardOutput:unixStandardOutputPipe];
        [unixTask setStandardError:unixStandardErrorPipe];
        [unixTask setStandardInput:unixStandardInputPipe];
        
        //note we are calling the file handle not the pipe
        [fhOutput readInBackgroundAndNotify];
        [fhError readInBackgroundAndNotify];
        
        [unixTask launch];
        [unixTask waitUntilExit];
    }@catch (NSException * e)
    {
        NSString *log = [NSString stringWithFormat:@"cmd exception:%@", e];
        [self showLog:log];
    }
}


#pragma mark -
-(void) notifiedForStdOutput: (NSNotification *)notified
{
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
    NSLog(@"standard data ready %ld bytes",data.length);
    if ([data length]){
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [self showLog:[NSString stringWithFormat:@"%@",outputString]];
        
        switch (curStep) {
            case kEStepCheckDeviceInitStatus:
            {
                if(outputString!=nil && [outputString rangeOfString:@"Internal Flash"].location != NSNotFound)
                {
                    bCheckDeviceInitStatus=YES;
                }
            }
                break;
            case kEStepFindDevice:
            {
                if(outputString!=nil && [outputString rangeOfString:@"tty.usbmodem"].location != NSNotFound)
                {
                    NSCharacterSet *cs = [NSCharacterSet whitespaceAndNewlineCharacterSet];                    
                    NSArray *arr = [outputString componentsSeparatedByCharactersInSet:cs];
                    
                    for(NSString *name in arr)
                    {
                        NSRange range = [name rangeOfString:@"tty.usbmodem"];
                        if(range.location != NSNotFound)
                        {
                            NSString *log=[NSString stringWithFormat:@"finded:%@",name];
                            [self showLog:log];
                            deviceName = [NSString stringWithFormat:@"/dev/%@",name];
                            bFindDevice = YES;
                            break;
                        }
                    }
                }
            }
                break;
                
            default:
                break;
        }
    }
    if (unixTask != nil) {
        [fhOutput readInBackgroundAndNotify];
    }
}

-(void) notifiedForStdError: (NSNotification *)notified
{
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
    NSLog(@"standard error ready %ld bytes",data.length);
    if ([data length])
    {
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [self showLog:[NSString stringWithFormat:@"%@",outputString]];
        switch (curStep) {
            default:
                break;
        }
    }
    if (unixTask != nil) {
        [fhError readInBackgroundAndNotify];
    }
}

-(void) notifiedForComplete:(NSNotification *)anotification
{

    if ([unixTask terminationStatus] == 0)
    {
        [self showLog:@"[cmd successed]\n"];
    }
    else
    {
        NSString *str = [NSString stringWithFormat:@"cmd failed exit code=%d\n",[unixTask terminationStatus]];
        [self showLog:str];
        
//        if(bNeedInstallXRun)
//        {
//            [self showLog:@"\nAfter Install xcrun,push update button again.\n"];
//        }
    }

    unixTask = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - allStep
- (void)allStep
{
    [self setProgressValue:2.0];
    
    // 1. 2. install homebrew and dfu-util
    if(![self checkDfuUtil])
    {
        if(![self checkHomebrew])
        {
            if(![self step1])
            {
                dispatch_async(dispatch_get_main_queue(), ^{
                    self.button.enabled = YES;
                    [self.indicator setHidden:YES];
                });
                return;
            }
            dispatch_async(dispatch_get_main_queue(), ^{
                self.progress.doubleValue = 15.0;
            });
        }
        if(![self step2])
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                self.button.enabled = YES;
                [self.indicator setHidden:YES];
            });
            return;
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            self.progress.doubleValue = 30.0;
        });
    }
    
    // 3.download
    if(![self step3])
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.button.enabled = YES;
            [self.indicator setHidden:YES];
        });
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progress.doubleValue = 45.0;
    });
    

    // 4.find and init device
    if(![self step4])
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.button.enabled = YES;
            [self.indicator setHidden:YES];
        });
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progress.doubleValue = 60.0;
    });

    // 5.write fireware
    if(![self step5])
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.button.enabled = YES;
            [self.indicator setHidden:YES];
        });
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progress.doubleValue = 100.0;
        self.button.enabled = YES;
        [self.indicator setHidden:YES];
        [self showLog:@"All done!"];
    });

}
#pragma mark -
- (BOOL)checkHomebrew
{
    return  [[NSFileManager defaultManager] fileExistsAtPath:@"/usr/local/bin/brew"];
}

- (BOOL)checkDfuUtil
{
    return [[NSFileManager defaultManager] fileExistsAtPath:@"/usr/local/bin/dfu-util"];
}


#pragma mark - 1.install homebrew
- (BOOL)step1
{
    [self showLog:@"\n============================================\n"];
    [self showLog:@"Installing homebrew.\n"];
    [self showLog:@"============================================\n"];

    
    if([self modifySudoersFile])
    {
        if([self downloadInstallHomebrewScript])
        {
            if([self runInstallHomebrewScript])
            {
                return YES;
            }
            else
            {
                if(bNeedUninstall)
                {
                    if([self downloadUninstallHomebrewScript])
                    {
                        if([self runUninstallHomebrewScript])
                        {
                            return [self runInstallHomebrewScript];
                        }
                    }
                }
                else if(bNeedInstallXRun)
                {
                    [self showLog:@"Need install xrun first.After Installed xrun,push Fix my Cube Again.\n"];
                }
            }

        }
    }
    
    return NO;
}

- (BOOL)modifySudoersFile
{
//    [self showLog:@"Modifing sudo config file.\n"];
    NSDictionary* errorDict;
    NSString *tmpDataFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"data"];
    [@"%wheel        ALL=(ALL) NOPASSWD: ALL\nALL ALL=(ALL) ALL\nDefaults    visiblepw\n" writeToFile:tmpDataFilePath atomically:YES encoding:NSASCIIStringEncoding error:nil];
    NSString *script1 = [NSString stringWithFormat:@"do shell script \"cat %@ >> /etc/sudoers\" with administrator privileges", tmpDataFilePath];
    NSAppleScript *appleScript1 = [[NSAppleScript alloc] initWithSource:script1];
    NSAppleEventDescriptor* des1 = [appleScript1 executeAndReturnError:&errorDict];
    if (des1)
    {
        [self showLog:@"Modified sudo config file successed.\n"];
        return YES;
    } else
    {
        [self showLog:@"Modified sudo config file failed.\n"];
        return NO;
    }
}

- (BOOL)downloadInstallHomebrewScript
{
    [self showLog:@"Downloading homebrew install script.\n"];
    tmpRbFilePath1 = [NSTemporaryDirectory() stringByAppendingPathComponent:@"install"];
//    if(![[NSFileManager defaultManager] fileExistsAtPath:tmpRbFilePath1])
    {
        NSData *data = [NSData dataWithContentsOfURL:[NSURL URLWithString:@"https://raw.githubusercontent.com/Homebrew/install/master/install"]];
        NSString *tmpStr = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
        tmpStr = [tmpStr stringByReplacingOccurrencesOfString:@"abort \"Don't run this as root!\"" withString:@"#abort \"Don't run this as root!\""];        
        data = [tmpStr dataUsingEncoding:NSASCIIStringEncoding];
        if(data.length<1)
        {
            [self showLog:@"Download homebrew install script failed."];
            return NO;
        }
        else
        {
            [data writeToFile:tmpRbFilePath1 atomically:YES];
            [self showLog:@"Download homebrew install script successed.\n"];
        }
    }
    return YES;
}

- (BOOL)runInstallHomebrewScript
{
    [self showLog:@"Running install homebrew script,please wait.\n"];
    NSString* script = [NSString stringWithFormat:@"do shell script \"ruby %@\" with administrator privileges", tmpRbFilePath1];
    
    NSAppleScript *appleScript = [[NSAppleScript alloc] initWithSource:script];
    NSDictionary *error;
    NSAppleEventDescriptor* des = [appleScript executeAndReturnError:&error];
    if (des)
    {
        [self showLog:@"Run install homebrew script successed."];
        return YES;
    }
    else {
        NSString *log=[NSString stringWithFormat:@"Run install homebrew script failed:%@\n",error];
        NSString *errorMessage = error[NSAppleScriptErrorMessage];
        if(errorMessage != nil)
        {
            if([errorMessage rangeOfString:@"It appears Homebrew is already installed."].location != NSNotFound)
            {
                bNeedUninstall = YES;
            }
            if([errorMessage rangeOfString:@"xcode-select"].location != NSNotFound)
            {
                bNeedInstallXRun = YES;
            }
            
        }
        
        [self showLog:log];
        return NO;
    }
}

- (BOOL)downloadUninstallHomebrewScript
{
    [self showLog:@"Downloading homebrew uninstall script failed."];    
    tmpRbFilePath2 = [NSTemporaryDirectory() stringByAppendingPathComponent:@"uninstall"];
    if(![[NSFileManager defaultManager] fileExistsAtPath:tmpRbFilePath2])
    {
        NSData *data = [NSData dataWithContentsOfURL:[NSURL URLWithString:@"https://raw.githubusercontent.com/Homebrew/install/master/uninstall"]];
        NSString *tmpStr = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
        data = [tmpStr dataUsingEncoding:NSASCIIStringEncoding];
        if(data.length<1)
        {
            [self showLog:@"Download homebrew uninstall script failed."];
            return NO;
        }
        else
        {
            [data writeToFile:tmpRbFilePath2 atomically:YES];
            [self showLog:@"Download homebrew uninstall script successed."];
        }
    }
    return YES;
}

- (BOOL)runUninstallHomebrewScript
{
    [self showLog:@"Run uninstall homebrew script.\n"];
    [self runRubyWithArgs:tmpRbFilePath2];
    return YES;
}

#pragma mark - 2.install dfu-util
- (BOOL)step2
{
    if([[NSFileManager defaultManager] fileExistsAtPath:@"/usr/local/bin/dfu-util"]) return YES;
    
    [self showLog:@"\n============================================\n"];
    [self showLog:@"Install dfu-util\n"];
    [self showLog:@"============================================\n"];
    
    [self showLog:@"Installing dfu-util\n"];
    @try
    {
        NSString* script = [NSString stringWithFormat:@"do shell script \"/usr/local/bin/brew install dfu-util\" with administrator privileges"];
        
        NSDictionary* errorDict;
        NSAppleScript *appleScript = [[NSAppleScript alloc] initWithSource:script];
        NSAppleEventDescriptor *des = [appleScript executeAndReturnError:&errorDict];
        if (des) {
            [self showLog:@"Install dfu-util successed.\n"];
            return YES;
        } else {
            NSString *log=[NSString stringWithFormat:@"Install dfu-util successed failed.%@\n",errorDict];
            [self showLog:log];
            return NO;
        }
    }
    @catch (NSException * e)
    {
        NSString *log=[NSString stringWithFormat:@"Exception: %@", e];
        [self showLog:log];
        return NO;
    }
}

#pragma mark - 3.Download firmware
- (BOOL)step3
{
    [self showLog:@"\n============================================\n"];
    [self showLog:@"Download firmware.\n"];
    [self showLog:@"============================================\n"];
    
    //    http://cubetube.org/static/recovery/demo.bin
    //    http://cubetube.org/static/recovery/firmware-part1.bin
    //    http://cubetube.org/static/recovery/firmware-part2.bin
    
    NSURL *url = [NSURL URLWithString:@"http://cubetube.org/static/recovery/demo.bin"];
    NSString *filePath = [NSTemporaryDirectory() stringByAppendingPathComponent:url.lastPathComponent];
    [self showLog:filePath];
    [self showLog:@"\n"];
    NSData *data = [NSData dataWithContentsOfURL:url];
    if(data.length<1)
    {
        [self showLog:@"Download failed."];
        return NO;
    }
    [data writeToFile:filePath  atomically:YES];
    file1=filePath;
    
    url = [NSURL URLWithString:@"http://cubetube.org/static/recovery/firmware-part1.bin"];
    data = [NSData dataWithContentsOfURL:url];
    filePath = [NSTemporaryDirectory() stringByAppendingPathComponent:url.lastPathComponent];
    [self showLog:filePath];
    [self showLog:@"\n"];    
    if(data.length<1)
    {
        [self showLog:@"Download failed."];
        return NO;
    }
    [data writeToFile:filePath  atomically:YES];
    file2=filePath;
    
    url = [NSURL URLWithString:@"http://cubetube.org/static/recovery/firmware-part2.bin"];
    data = [NSData dataWithContentsOfURL:url];
    filePath = [NSTemporaryDirectory() stringByAppendingPathComponent:url.lastPathComponent];
    [self showLog:filePath];
    [self showLog:@"\n"];    
    if(data.length<1)
    {
        [self showLog:@"Download failed."];
        return NO;
    }
    [data writeToFile:filePath  atomically:YES];
    
    file3=filePath;
    
    return YES;
}

#pragma mark - 4.Find And Init Device
- (BOOL)step4
{
    [self showLog:@"\n============================================\n"];
    [self showLog:@"Find and init device.\n"];
    [self showLog:@"============================================\n"];
    if([self checkInitDeviceStatus])// find inited device
    {
        [self showLog:@"Find inited device.\n"];
        return YES; // found
    }
    
    if(![self findDevice]) // find device
    {
        //[self showLog:@"Can't find any device.\n"];
        return NO;// no device
    }
    
    // init device
    return [self initDevice];
}

- (BOOL)initDevice
{
    [self showLog:@"Initial device.\n"];
    NSString *args= [NSString stringWithFormat:@"-f %@ 14400",deviceName];
    NSString *result = [self runCmd:@"/bin/stty" andArgs:args];
    NSString *log=[NSString stringWithFormat:@"%@\n",result];
    [self showLog:log];
    
    [NSThread sleepForTimeInterval:3];
    int i=0;
    while (i++<3)
    {
        if([self checkInitDeviceStatus])
        {
            [self showLog:@"Init device successed.\n"];
            return YES;
        }
        [NSThread sleepForTimeInterval:3];
    }
    
    [self showLog:@"Init device failed.\n"];
    return NO;
}

- (BOOL)checkInitDeviceStatus
{
    NSString *result = [self runCmd:@"/usr/local/bin/dfu-util" andArgs:@"-l"];
    if(result!=nil && [result rangeOfString:@"Internal Flash"].location != NSNotFound)
    {
        return YES;
    }
    
    return NO;
}


- (BOOL)findDevice
{
    [self showLog:@"Find device.\n"];
    NSString *string = [self runCmd:@"/bin/ls" andArgs:@"/dev/"];
    
    NSCharacterSet *cs = [NSCharacterSet whitespaceAndNewlineCharacterSet];
    NSArray *arr = [string componentsSeparatedByCharactersInSet:cs];
    
    for(NSString *name in arr)
    {
        NSRange range = [name rangeOfString:@"tty.usbmodem"];
        if(range.location != NSNotFound)
        {
            NSString *log=[NSString stringWithFormat:@"Finded:%@\n",name];
            [self showLog:log];
            deviceName = [NSString stringWithFormat:@"/dev/%@",name];
            
            return YES;
        }
    }
    [self showLog:@"Can't find Cubetube device.\n"];
    return NO;
}

#pragma mark - 5.Write firmware to device
- (BOOL)step5
{
    [self showLog:@"\n============================================\n"];
    [self showLog:@"Write firmware to device.\n"];
    [self showLog:@"============================================\n"];
    //    /usr/local/bin/dfu-util
    //    dfu-util -d 2b04:d006 -a 0 -s 0x8020000 -D /Users/qiang/firmware-part1.bin
    //    dfu-util -d 2b04:d006 -a 0 -s 0x8060000 -D /Users/qiang/firmware-part2.bin
    //    dfu-util -d 0x2b04:d006 -a 0 -s 0x80a0000:leave -D /Users/qiang/demo.bin
    
    [NSThread sleepForTimeInterval:2];
    NSString *str = [NSString stringWithFormat:@"-d 2b04:d006 -a 0 -s 0x8020000 -D %@",file2];
    NSString *result = [self runCmd:@"/usr/local/bin/dfu-util" andArgs:str];
    
    NSString *log = [NSString stringWithFormat:@"result = %@\n",result];
    [self showLog:log];
    if(result!=nil && [result rangeOfString:@"File downloaded successfully"].location != NSNotFound)
    {
        [self showLog:@"Write 1 File(/3 Files) successed\n"];
    }
    else
    {
        [self showLog:@"Write 1 File(/3 Files) failed\n"];
    }
    [NSThread sleepForTimeInterval:2];
    
    str = [NSString stringWithFormat:@"-d 2b04:d006 -a 0 -s 0x8060000 -D %@",file3];
    result = [self runCmd:@"/usr/local/bin/dfu-util" andArgs:str];
    log = [NSString stringWithFormat:@"result = %@\n",result];
    [self showLog:log];
    if(result!=nil && [result rangeOfString:@"File downloaded successfully"].location != NSNotFound)
    {
        [self showLog:@"Write 2 Files(/3 Files) successed\n"];
    }
    else
    {
        [self showLog:@"Write 2 Files(/3 Files) failed\n"];
    }
    [NSThread sleepForTimeInterval:2];
    
    str = [NSString stringWithFormat:@"-d 0x2b04:d006 -a 0 -s 0x80a0000:leave -D %@",file1];
    result = [self runCmd:@"/usr/local/bin/dfu-util" andArgs:str];
    log = [NSString stringWithFormat:@"result = %@\n",result];
    [self showLog:log];
    if(result!=nil && [result rangeOfString:@"File downloaded successfully"].location != NSNotFound)
    {
        [self showLog:@"write 3 Files(/3 Files) successed\n"];
    }
    else
    {
        [self showLog:@"Write 3 Files(/3 Files) failed\n"];
    }
    
    return YES;
}

@end
