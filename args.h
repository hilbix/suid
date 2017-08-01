/* Argv-List
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

struct args
  {
    int		n, max;
    char	**args;
  };

static void
args_add(struct args *a, char *s)
{
  if (a->max <= a->n+1)
    {
      a->max += a->max + a->n + 2;
      a->args = re_alloc(a->args, a->max * sizeof *a->args);
    }
  a->args[a->n]		= s;
  a->args[++a->n]	= 0;
}

static void
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
  args_add(a, buf);
}

