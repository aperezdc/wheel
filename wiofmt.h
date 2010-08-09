
#ifndef __wiofmt_h__
#define __wiofmt_h__ 1

#include <stddef.h>
#include <stdarg.h>

/* ..scanf */
struct _w_arg_scanf {
  void *data;
  int (*getch)(void*);
  int (*putch)(int,void*);
};

int _w__v_scanf(struct _w_arg_scanf* fn, const char *format, va_list arg_ptr);

struct _w_arg_printf {
  void *data;
  int (*put)(void*,size_t,void*);
};

int _w__v_printf(struct _w_arg_printf* fn, const char *format, va_list arg_ptr);


#endif /* !__wiofmt_h__ */

