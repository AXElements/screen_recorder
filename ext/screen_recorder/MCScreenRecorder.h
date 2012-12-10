//
//  MCScreenRecorder.h
//  ScreenRecorder
//
//  Created by Mark Rada on 12-04-07.
//  Copyright (c) 2012 Marketcircle Incorporated. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@interface MCScreenRecorder : NSObject <AVCaptureFileOutputDelegate, AVCaptureFileOutputRecordingDelegate> {
}

@property (assign) AVCaptureSession* session;
@property (assign) AVCaptureScreenInput* input;
@property (assign) AVCaptureMovieFileOutput* output;

// Used for pseudo run-loop synchronization
@property (assign) dispatch_semaphore_t sema;

// File that the recording will be saved to
@property (retain) NSURL* file;

- (BOOL)   start;
- (BOOL)   start:(NSURL*)file_name;
- (BOOL)   isStarted;
- (double) length;
- (size_t) size;
- (BOOL)   stop;

#pragma mark AVCaptureFileOutputDelegate
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection;

#pragma mark AVCaptureFileOutputRecordingDelegate
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error;
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didPauseRecordingToOutputFileAtURL:(NSURL *)fileURL fromConnections:(NSArray *)connections;
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didResumeRecordingToOutputFileAtURL:(NSURL *)fileURL fromConnections:(NSArray *)connections;
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didStartRecordingToOutputFileAtURL:(NSURL *)fileURL fromConnections:(NSArray *)connections;
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput willFinishRecordingToOutputFileAtURL:(NSURL *)fileURL fromConnections:(NSArray *)connections error:(NSError *)error;

@end
