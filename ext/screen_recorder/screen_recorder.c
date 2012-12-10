#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

#include "ruby.h"
#include "MCScreenRecorder.h"

static VALUE rb_cRecorder;
static ID sel_new;

static
void
rb_recorder_finalizer(id recorder)
{
#if NOT_MACRUBY
  [recorder release];
#else
#endif
}

static
VALUE
rb_objc_wrap(id obj)
{
  return Data_Wrap_Struct(rb_cRecorder, 0, rb_recorder_finalizer, obj);
}
#define OBJC_WRAP(x) (rb_objc_wrap(x))

static
MCScreenRecorder*
rb_objc_unwrap(VALUE obj)
{
  MCScreenRecorder* recorder;
  Data_Get_Struct(obj, MCScreenRecorder, recorder);
  return recorder;
}
#define OBJC_UNWRAP(x) (rb_objc_unwrap(x))

static
VALUE
rb_recorder_alloc(VALUE klass)
{
  MCScreenRecorder* recorder = [[MCScreenRecorder alloc] init];
  return OBJC_WRAP(recorder);
}

/*
 * Synchrnously start recording
 *
 * You can optionally specify a file name for the recording; if you do
 * not then a default name will be provided in the form
 * `~/Movies/TestRecording-20121017123230.mov`
 * (the timestamp will be different for you).
 *
 * @param file_name [String] (__optional__)
 * @return [Boolean]
 */
static
VALUE
rb_recorder_start(int argc, VALUE* argv, VALUE self)
{
  BOOL     result = NO;
  NSString*  path = nil;
  NSURL* path_url = nil;

  switch (argc)
    {
    case 0:
      result = ([OBJC_UNWRAP(self) start]);
      break;
    case 1:
    default:
      path = [NSString stringWithCString:StringValueCStr(argv[0])
	                        encoding:NSUTF8StringEncoding];
      path_url = [NSURL fileURLWithPath:path isDirectory:NO];
      result = [OBJC_UNWRAP(self) start:path_url];
      [path release];
      [path_url release];
    }

  return (result ? Qtrue : Qfalse);
}

/*
 * Whether or not the recording has begun
 *
 * This will be `true`  after calling {#start} until {#stop} is called.
 */
static
VALUE
rb_recorder_started(VALUE self)
{
  return ([OBJC_UNWRAP(self) isStarted] ? Qtrue : Qfalse);
}

/*
 * Duration of the recording, in seconds
 *
 * @return [Float]
 */
static
VALUE
rb_recorder_length(VALUE self)
{
  return DBL2NUM([OBJC_UNWRAP(self) length]);
}

/*
 * Size of the recording on disk, in bytes
 *
 * @return [Fixnum]
 */
static
VALUE
rb_recorder_size(VALUE self)
{
  return SIZET2NUM([OBJC_UNWRAP(self) size]);
}

/*
 * Synchronously stop recording and finish up commiting any data to disk
 *
 * A recording cannot be {#start}ed again after it has been stopped. You
 * will need to start a new recording.
 *
 * @return [Boolean]
 */
static
VALUE
rb_recorder_stop(VALUE self)
{
  return ([OBJC_UNWRAP(self) stop] ? Qtrue : Qfalse);
}

/*
 * Path to the screen recording on disk
 *
 * This is `nil` until the screen recording begins.
 *
 * @return [String,nil]
 */
static
VALUE
rb_recorder_file(VALUE self)
{
  NSString* name = [OBJC_UNWRAP(self).file path];
  VALUE     path;

  if (name) {
    path = rb_str_new_cstr([name cStringUsingEncoding:NSUTF8StringEncoding]);
    [name release];
  }
  else {
    path = Qnil;
  }

  return path;
}

static
VALUE
rb_recorder_yielder(VALUE self)
{
  rb_yield(self);
  return rb_recorder_file(self);
}

/*
 * Record the screen while executing the given block
 *
 * You may optionally specify the path to save the recording to, just
 * as when calling {#start}.
 *
 * The path to the recording will be returned. The recorder object is yielded.
 *
 * @yield
 * @yieldparam recorder [ScreenRecorder]
 * @param file_name [String] (__optional__)
 * @return [String]
 */
static
VALUE
rb_recorder_record(int argc, VALUE* argv, VALUE self)
{
  VALUE recorder = rb_funcall(rb_cRecorder, sel_new, 0);
  rb_recorder_start(argc, argv, recorder);
  return rb_ensure(rb_recorder_yielder, recorder, rb_recorder_stop, recorder);
}


void
Init_screen_recorder()
{
  sel_new = rb_intern("new");

  /*
   * Document-class: ScreenRecorder
   *
   * Screen recordings, easy as pi.
   *
   * Things that you need to be concerned about:
   *
   * * screen going to sleep (we can resolve this issue later)
   * * short recordings (~1 second) don't work too well; it looks like
   *   the last bit of the buffer does not get saved so the last ~0.5
   *   seconds are not saved to disk (we could add a 0.5 second sleep)
   * * small memory leak when a recording starts on Mountain Lion with MacRuby
   * * constantly leaking memory during recording on Lion with MacRuby
   * * run loop hack is not needed if code is already being called from
   *   in a run loop in MacRuby
   * * pausing is not working...not sure why; so it is not exposed for now
   */
  rb_cRecorder = rb_define_class("ScreenRecorder", rb_cObject);

  rb_define_alloc_func(rb_cRecorder, rb_recorder_alloc);

  rb_define_method(rb_cRecorder, "start",      rb_recorder_start,   -1);
  rb_define_method(rb_cRecorder, "started?",   rb_recorder_started,  0);
  rb_define_method(rb_cRecorder, "length",     rb_recorder_length,   0);
  rb_define_method(rb_cRecorder, "size",       rb_recorder_size,     0);
  rb_define_method(rb_cRecorder, "stop",       rb_recorder_stop,     0);
  rb_define_method(rb_cRecorder, "file",       rb_recorder_file,     0);

  rb_define_singleton_method(rb_cRecorder, "record", rb_recorder_record, -1);
}
