/* Argv-List
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#include "memswap.h"

struct args
  {
    int		n, max;
    char	**args;
  };

/* This adds a string to the given args,
 * without allocating!
 * So it can be manipulated later on.
 */
static struct args *
args_add(struct args *a, char *s)
{
  if (a->max <= a->n+1)
    {
      a->max += a->max + a->n + 2;
      a->args = re_alloc(a->args, a->max * sizeof *a->args);
    }
  a->args[a->n]		= s;
  a->args[++a->n]	= 0;
  return a;
}

/* This adds a printf formatted string to args,
 * with allocating!
 */
static struct args *
args_addf(struct args *a, const char *s, ...)
{
  char	*buf;
  int	n, k;

  buf	= 0;
  for (n=BUFSIZ;; n+=k)
    {
      va_list	list;

      buf	= re_alloc(buf, n);
      va_start(list, s);
      k = vsnprintf(buf, n, s, list);
      va_end(list);
      if (k<n)
        break;
    }
  k = strlen(buf);
  if (k+1<n)
    buf = re_alloc(buf, k+1);
  return args_add(a, buf);
}

static struct args *
args_prepend(struct args *a, ...)
{
  va_list	list;
  char		*s;
  int		n;

  n	= a->n;

  va_start(list, a);
  while ((s = va_arg(list, char *))!=0)
    args_add(a, s);
  va_end(list);
  memswap(a->args, n*sizeof *a->args, a->n*sizeof *a->args);
  return a;
}

static struct args *
args_pop(struct args *a, int n)
{
  if (n >= a->n)
    {
      a->n	= 0;
      return a;
    }
  a->n	-= n;
  memmove(a->args, &a->args[n], a->n*sizeof *a->args);
  return a;
}

