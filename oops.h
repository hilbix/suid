/* generic functions
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	OOPS_I		(char *)1	/* OOPS("int", OOPS_I, 1, NULL)	*/
#define	OOPS_LLU	(char *)2	/* OOPS("long", OOPS_LLU, 2ll, NULL)	*/
#define	OOPS_O		(char *)3	/* OOPS("oct", OOPS_O, 012, NULL)	*/
#define	OOPS_C		(char *)4	/* OOPS("char", OOPS_C, 'a', NULL)	*/

#define	FATAL(X)	do { if (X) OOPS(__FILE__, __FUNCTION__, "internal error", #X, NULL); } while (0)

#ifndef MAXLOOPS
#define	MAXLOOPS	(2*BUFSIZ)
#endif

#ifndef	OOPS_FAIL
#define	OOPS_FAIL	23
#endif

static inline void IGUR() {}	/* IGnore Unused Return: IGUR(fn(args))	*/

/* why isn't there a stdlib for this?	*/
static int
writex(int fd, const void *ptr, size_t len)
{
  int		loops, pos;
  const char	*p;

  p	= ptr;
  pos	= 0;
  for (loops=0; pos<len; loops++)
    {
      int put;

      put = write(fd, p+pos, len-pos);
      if (!put)
        break;
      if (put<0)
        {
          if (loops>MAXLOOPS || errno!=EINTR)
            return put;
          else
            continue;
        }
      pos += put;
    }
  return pos;
}

#if 0
/* why isn't there a stdlib for this?	*/
static char *
strxcat(char *buf, const char *add, size_t max)
{
  if (!buf || !max)
    OOPS("strxcat() with NULL buffer", NULL);
  for (i=0; i<max; i++)
    if (!buf[i])
      {
        strncpy(buf+i, add, max-i);
        if (buf[max-1])
          OOPS("strxcat() buffer overflow", NULL);
        return buf;
      }
  OOPS("strxcat() string not terminated with NUL", NULL);
}
#endif

/* why isn't there a stdlib for this?	*/
static int
closex(int fd)
{
  while (close(fd))
    if (errno!=EINTR)
      return -1;
  return 0;
}

/* why isn't there a stdlib for this?	*/
static int
writes(int fd, const char *s)
{
  return s ? writex(fd, s, strlen(s)) : 0;
}

static void
ERRORv(int e, const char *s, va_list list)
{
  const char *sep;

  for (sep=0; s; s=va_arg(list, const char *))
    {
      char	buf[22];

      if (s==OOPS_I)
        {
          snprintf(buf, sizeof buf, "%d", va_arg(list, int));
          s	= buf;
        }
      else if (s==OOPS_LLU)
        {
          snprintf(buf, sizeof buf, "%llu", va_arg(list, unsigned long long));
          s	= buf;
        }
      else if (s==OOPS_O)
        {
          snprintf(buf, sizeof buf, "0%03o", va_arg(list, int));
          s	= buf;
        }
      else if (s==OOPS_C)
        {
          snprintf(buf, sizeof buf, "%c", va_arg(list, int));	/* char is passed as int	*/
          s	= buf;
        }
      writes(2, sep);
      sep = ": ";
      writes(2, s);
    }

  if (e)
    {
      writes(2, sep);
      writes(2, strerror(e));
    }
  writes(2, "\n");
}

static void
ERROR(const char *s, ...)
{
  int 		e = errno;
  va_list	list;

  va_start(list, s);
  ERRORv(e, s, list);
  va_end(list);
}

static void
STDERR(const char *s, ...)
{
  va_list	list;

  va_start(list, s);
  ERRORv(0, s, list);
  va_end(list);
}

__attribute__((noreturn))
static void
OOPS(const char *bug, ...)
{
  int 		e = errno;
  va_list	list;

  va_start(list, bug);
  ERRORv(e, bug, list);
  va_end(list);
  exit(OOPS_FAIL);
}

static void *
re_alloc(void *buf, size_t len)
{
  buf	= realloc(buf, len);
  if (!buf)
    OOPS("out of memory", OOPS_LLU, (unsigned long long)len, NULL);
  return buf;
}

static char *
stralloc(const char *s)
{
  size_t	len;
  char		*buf;

  len	= strlen(s)+1;
  buf	= re_alloc(NULL, len);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
  /* GCC does not grok that `buf` must have length `len` now, so following is safe: */
  strncpy(buf, s, len);
#pragma GCC diagnostic pop
  FATAL(buf[len-1]);
  return buf;
}

static int
getint(const char *s, int *val)
{
  long long	l;
  char		*end;

  l = strtoll(s, &end, 10);
  if (!end || *end)
    return 0;
  if (l != (long long)(int)l)
    return 0;
  *val	= (int)l;
  return 1;
}

static int
fetchint(char **s, int def)
{
  long long	l;
  char		*end;

  l = strtoll(*s, &end, 10);
  return end && end != *s ? (*s=end), l : def;
}

