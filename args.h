/* Argv-List
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

struct args
  {
    int		n, max;
    const char	**args;
  };

static void
args_add(struct args *a, const char *s)
{
  if (a->max <= a->n+1)
    {
      a->max += a->max + a->n + 2;
      a->args = realloc(a->args, a->max * sizeof *a->args);
      if (!a->args)
	OOPS("out of memory", NULL);
    }
}

