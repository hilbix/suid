/* Run commands with generic SUID
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#include "linereader.h"
#include "args.h"

#include <pwd.h>
#include <grp.h>

#define	CONF	"/etc/suid.conf"

static char *
next(char *s)
{
  char *ptr;

  ptr	= strchr(s, ':');
  if (!ptr)
    OOPS(CONF, "malformed line", NULL);
  *ptr++	= 0;
  return ptr;
}

int
main(int argc, char **argv)
{
  struct linereader	l = { -1 };
  struct args		args = { 0 };
  char			*cmd, *pwd, *user, *group, *minmax, *line;
  int			uid, gid;
  struct passwd		*pw;
  int			i, minarg, maxarg;

  if (argc<2)
    {
      /* Avoid to print user defined parameters, so do not output argv[0] here	*/
      OOPS("Usage: suid command [args..]", NULL);
    }

  cmd = argv[1];
  if (strchr(cmd, '/'))
    OOPS("command must not contain '/'", NULL);

  l.name = CONF;

  do
    {
      if ((line = linereader(&l))==0)
	{
	  /* Avoid to print user defined parameters, so not output argv[1] here	*/
          OOPS(CONF, linereader_end(&l) ? "read error" : "command not found", NULL);
	}

      pwd	= next(line);
    } while (strcmp(cmd, line));

  /* command:pw:user:group:minmax:/path/to/binary args..
   */
  user	= next(pwd);
  group	= next(user);
  minmax= next(group);
  line	= next(minmax);

  if (!*user)
    pw	= getpwuid(getuid());
  else if (!strcmp(user, "*"))
    pw	= getpwuid(geteuid());
  else if (getint(user, &uid))
    pw	= getpwuid(uid);
  else
    pw	= getpwnam(user);

  if (!pw)
    OOPS(CONF, cmd, "user", user, "not found", NULL);

  uid	= pw->pw_uid;

  if (!*group)
    gid	= getgid();
  else if (!strcmp(group, "*"))
    gid = getegid();
  else if (!strcmp(group, "="))
    gid	= pw->pw_gid;
  else if (!getint(group, &gid))
    {
      struct group *gr;

      gr	= getgrnam(group);
      if (!gr)
        OOPS(CONF, cmd, "group", group, "not found", NULL);
      gid	= gr->gr_gid;
    }

  /* Apply the new uid/gid	*/
  IGUR(setuid(uid));
  IGUR(setgid(gid));

  minarg = fetchint(&minmax, 0);
  maxarg = minarg;
  if (*minmax=='-')
    {
      minmax++;
      maxarg	= fetchint(&minmax, -1);
    }
  if (*minmax)
    OOPS(CONF, cmd, "wrong minmax", minmax, NULL);
 
  if (argc-2<minarg || (maxarg>=0 && argc-2>maxarg))
    OOPS(CONF, cmd, "wront number of arguments", NULL);

  for (;;)
    {
      args_add(&args, line);
      line	= strchr(line, ' ');
      if (!line)
	break;
      *line++	= 0;
      while (*line==' ')
	line++;
      if (!*line)
	break;
    }
  for (i=1; ++i<argc; )
    args_add(&args, argv[i]);

  execv(argv[0], argv);
  perror(argv[0]);
  return 127;	/* resemble shell	*/
}

