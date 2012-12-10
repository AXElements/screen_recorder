//
//  MCScreenRecorder.m
//  ScreenRecorder
//
//  Created by Mark Rada on 12-04-07.
//  Copyright (c) 2012 Marketcircle Incorporated. All rights reserved.
//

#import "MCScreenRecorder.h"


@implementation MCScreenRecorder

@synthesize session;
@synthesize input;
@synthesize output;
@synthesize file;
@synthesize sema;


- (id)init
{
  if ([super init]) {
    self.session = [[AVCaptureSession alloc] init];
    self.input   = [[AVCaptureScreenInput alloc] initWithDisplayID:CGMainDisplayID()];

    self.input.capturesMouseClicks = YES;

    self.output = [[AVCaptureMovieFileOutput alloc] init];
    self.output.delegate = self;

    [self.session addInput:self.input];
    [self.session addOutput:self.output];

    self.sema = dispatch_semaphore_create(0);

    return self;
  }
  return nil;
}

- (void)dealloc
{
  dispatch_release(self.sema);

  self.file = nil;

  [self.output release];
  [self.input release];
  [self.session release];
  [super dealloc];
}


#define SEMA_WAIT_TIME dispatch_time(DISPATCH_TIME_NOW, (30LL * NSEC_PER_SEC))

static
NSURL*
default_file_name()
{
  static NSDateFormatter* formatter;
  static NSURL*           homeDir;
  static dispatch_once_t default_name_generator_token;
  dispatch_once(&default_name_generator_token, ^{
      formatter = [[NSDateFormatter alloc] init];
      [formatter setDateFormat:@"YYYYMMDDHHmmss"];

      homeDir = [NSURL fileURLWithPath:[@"~/Movies/" stringByExpandingTildeInPath]];
  });

  NSString* date = [formatter stringFromDate:[NSDate date]];
  NSURL*    path = [homeDir URLByAppendingPathComponent:[@"TestRecording-" stringByAppendingString:date]];
  return [path URLByAppendingPathExtension:@"mov"];
}

- (BOOL) start
{
  NSURL* path = default_file_name();
  BOOL result = [self start:path];
  [path release];
  return result;
}

- (BOOL) start:(NSURL*)file_name
{
  self.file = file_name;

  [self.session startRunning];
  [self.output startRecordingToOutputFileURL:self.file
                           recordingDelegate:self];

  if (dispatch_semaphore_wait(self.sema, SEMA_WAIT_TIME))
    return NO;
  else
    return YES;
}

- (BOOL)isStarted
{
  return self.output.isRecording;
}

- (double) length
{
  CMTime duration = self.output.recordedDuration;
  return (double)duration.value / (double)duration.timescale;
}

- (size_t)size
{
  return self.output.recordedFileSize;
}

- (BOOL) stop
{
  [self.session stopRunning];
  [self.output stopRecording];

  if (dispatch_semaphore_wait(self.sema, SEMA_WAIT_TIME))
    return NO;

  // need to wait for some callbacks because we want synchronousness
  switch (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 30, false))
    {
    case kCFRunLoopRunStopped:
      break;
    case kCFRunLoopRunTimedOut:
      NSLog(@"Did not get callback");
      return NO;
    default:
      NSLog(@"Unexpected result from waiting for callback");
      return NO;
    }

  if (dispatch_semaphore_wait(self.sema, SEMA_WAIT_TIME))
    return NO;
  else
    return YES;
}


#pragma mark AVCaptureFileOutputDelegate

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection*)connection
{
}

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
  didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection*)connection
{
  NSLog(@"MCScreenRecorder: dropped same data from recording.\n%@", self);
}

- (BOOL)captureOutputShouldProvideSampleAccurateRecordingStart:(AVCaptureOutput*)captureOutput
{
  return YES;
}


#pragma mark AVCaptureFileOutputRecordingDelegate

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
didFinishRecordingToOutputFileAtURL:(NSURL*)outputFileURL
      fromConnections:(NSArray*)connections
		error:(NSError*)error
{
  CFRunLoopStop(CFRunLoopGetCurrent());
  dispatch_semaphore_signal(self.sema);
}

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
didPauseRecordingToOutputFileAtURL:(NSURL*)fileURL
      fromConnections:(NSArray*)connections
{
  CFRunLoopStop(CFRunLoopGetCurrent());
  dispatch_semaphore_signal(self.sema);
}

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
didResumeRecordingToOutputFileAtURL:(NSURL*)fileURL
      fromConnections:(NSArray*)connections
{
  CFRunLoopStop(CFRunLoopGetCurrent());
  dispatch_semaphore_signal(self.sema);
}

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
didStartRecordingToOutputFileAtURL:(NSURL*)fileURL
      fromConnections:(NSArray*)connections
{
  dispatch_semaphore_signal(self.sema);
}

- (void)captureOutput:(AVCaptureFileOutput*)captureOutput
willFinishRecordingToOutputFileAtURL:(NSURL*)fileURL
      fromConnections:(NSArray*)connections
		error:(NSError*)error
{
  dispatch_semaphore_signal(self.sema);
}

@end
