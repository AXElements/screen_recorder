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
      path_url = [NSURL fileURLWithPath:path];
      result = [OBJC_UNWRAP(self) start:path_url];
      [path release];
      [path_url release];
    }

  return (result ? Qtrue : Qfalse);
}

static
VALUE
rb_recorder_started(VALUE self)
{
  return ([OBJC_UNWRAP(self) isStarted] ? Qtrue : Qfalse);
}

static
VALUE
rb_recorder_length(VALUE self)
{
  return DBL2NUM([OBJC_UNWRAP(self) length]);
}

static
VALUE
rb_recorder_size(VALUE self)
{
  return SIZET2NUM([OBJC_UNWRAP(self) size]);
}

static
VALUE
rb_recorder_stop(VALUE self)
{
  return ([OBJC_UNWRAP(self) stop] ? Qtrue : Qfalse);
}

static
VALUE
rb_recorder_file(VALUE self)
{
  NSString* name = [[OBJC_UNWRAP(self) file] path];
  if (name)
    return rb_str_new_cstr([name cStringUsingEncoding:NSUTF8StringEncoding]);
  else
    return Qnil;
}

static
VALUE
rb_recorder_yielder(VALUE self)
{
  rb_yield(self);
  return rb_recorder_file(self);
}

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
   * Record screens 'n shit.
   *
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
