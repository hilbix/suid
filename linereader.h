/* Easy to use line reader
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#include "oops.h"

#ifndef	LINEREADER_MAX
#define	LINEREADER_MAX	BUFSIZ
#endif

struct linereader
  {
    int		fd, pos, fill, eof, err;
    int		linenr;
    char	buf[LINEREADER_MAX], *line;
    const char	*name;
  };

static void
linereader_init(struct linereader *l, const char *name)
{
  memset(l, 0, sizeof *l);
  l->fd		= -1;
  l->name	= name;
}

/* Close the linereader and get the error state	*/
static int
linereader_end(struct linereader *l)
{
  if (l->fd >= 0)
    {
      if (closex(l->fd))
        l->err = errno;
      l->fd = -1;
    }
  if (l->err)
    errno = l->err;
  return l->err;
}

/* struct linereader l;
 * linereader_init(&l, "FILENAME");
 * while (linereader(&l))
 *   process(l.line, l.linenr);
 * if (linereader_end(&l))
 *   error(errno);
 *
 * This OOPSes at:
 * - Stray NUL
 * - Line too long
 * - too many loops (EINTR or broken reads)
 */
static char *
linereader(struct linereader *l)
{
  int	i, loop;

  if (l->fd<0 && (l->fd=open(l->name, O_RDONLY))<0)
    {
      l->err = errno;
      return 0;
    }
  
  for (loop=MAXLOOPS; --loop>=0; )
    {
      int	tmp;

      if (l->pos >= l->fill)
        {
          l->pos	= 0;
          l->fill	= 0;
        }
      FATAL(l->pos<0 || l->fill<0 || l->pos>sizeof l->buf || l->fill>sizeof l->buf);

      /* hunt for complete line */
      for (i=l->pos; i<l->fill; i++)
        switch (l->buf[i])
          {
          case '\n':
            tmp		= l->pos;
            l->pos	= i+1;
            l->buf[i]	= 0;			/* NUL terminated line	*/
            l->linenr++;
            return l->line = l->buf+tmp;	/* return line	*/

          case 0:
            OOPS(l->name, OOPS_I, l->linenr+1, "contains stray NUL", NULL);
          }

      /* free no more needed buffer space */
      if (l->pos)
        {
          /* l->pos < l->fill, see above	*/
          memmove(l->buf, l->buf + l->pos, l->fill - l->pos);
          l->fill	-= l->pos;
          l->pos	= 0;
        }

      /* l->pos == 0	*/

      if (l->fill >= sizeof l->buf)
        OOPS(l->name, OOPS_I, l->linenr+1, "line too long", NULL);

      if (l->eof)
        {
          /* l->fill < sizeof l->buf	*/
          l->buf[l->pos = l->fill] = 0;
          return l->fill ? l->buf : 0;	/* return last line without LF, or EOF	*/
        }

      /* read more data */
      /* l->fill < sizeof l->buf	*/
      tmp = read(l->fd, l->buf + l->fill, (sizeof l->buf) - l->fill);

      if (tmp<0)
        {
          if (errno==EINTR)
            continue;	/* ignore EINTR	*/
          l->err	= errno;
          return 0;
        }

      l->fill += tmp;
      if (!tmp)
        l->eof = 1;
    }
  OOPS(l->name, OOPS_I, l->linenr+1, "too many interrupts", NULL);
  return 0;
}

