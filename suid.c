/* Run commands with generic SUID
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#define	_GNU_SOURCE

#include "linereader.h"
#include "args.h"

#include <pwd.h>
#include <grp.h>

#include "suid_version.h"

#include "osx.h"

#define	CONF	"/etc/suid.conf"
#define	PATH	"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

/* Get next column
 */
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

static int
shellshock(const char *s)
{
  return !memcmp(s, "() {", 4);
}

/* Prepare SUID environment
 */
static void
populate_env(struct args *env, int no_shellshock, int uid, int gid, const char *cwd)
{
  char	**p, *s;

  args_add(env, PATH);
  if ((s=getenv("TERM"))!=0 && (no_shellshock || !shellshock(s)))
    args_addf(env, "TERM=%s", s);
  args_addf(env, "SUIDUID=%d", uid);
  args_addf(env, "SUIDGID=%d", gid);
  if (cwd)
    args_addf(env, "SUIDPWD=%s", cwd);
  for (p=environ; *p; p++)
    {
      s	= strchr(*p, '=');
      if (s && (no_shellshock || !shellshock(s+1)))
        args_addf(env, "SUID_%s", *p);
    }
}

static char *
find_cmd(struct linereader *l, const char *cmd)
{
  char	*line, *pass;

  while ((line = linereader(l))!=0)
    {
      if (*line == '#' || !*line)
	continue;
      pass	= next(line);
      if (!strcmp(cmd, line))
	return pass;
    }
  return 0;
}

/* This routine is too long
 */
int
main(int argc, char **argv)
{
  struct linereader	l = { -1 };
  struct args		args = { 0 }, env = { 0 };
  char			*cmd, *pass, *user, *group, *minmax, *dir, *line, *cwd;
  int			uid, gid, ouid, ogid;
  struct passwd		*pw;
  int			i, minarg, maxarg, debug, no_shellshock;

  if (argc<2)
    {
      /* Avoid to print user defined parameters, so do not output argv[0] here	*/
      OOPS("Usage: suid command [args..]\n"
	   "\t\tVersion " SUID_VERSION " compiled " __DATE__ "\n"
	   "\tConfig is in file " CONF ":\n"
	   "\tcommand:pw:user:grp:minmax:dir:/path/to/binary:args..\n"
	   "\tpw:       currently must be empty ('')\n"
	   "\tuser/grp: '' (suid) * (caller) = (gid of user)\n"
	   "\tminmax:   [D][S][minargs][-[maxargs]]"
           , NULL);
    }

  cmd = argv[1];
  if (strchr(cmd, '/'))
    OOPS("command must not contain '/'", NULL);

  l.name = CONF;
  if ((pass = find_cmd(&l, cmd))==0)
    {
      /* Avoid to print user defined parameters, so not output argv[1] here	*/
      OOPS(CONF, linereader_end(&l) ? "read error" : "command not found", NULL);
    }

  /* command:pw:user:group:minmax:/path/to/binary args..
   */
  user	= next(pass);
  group	= next(user);
  minmax= next(group);
  dir	= next(minmax);
  line	= next(dir);

  if (*pass)
    OOPS(CONF, cmd, "pw not yet supported", pass, NULL);

  debug = 0;
  no_shellshock = 0;
  if (*minmax == 'D')
    {
      minmax++;
      debug = 1;
    }
  if (*minmax == 'S')
    {
      minmax++;
      no_shellshock = 1;
    }

  cwd	= getcwd(NULL, 0);
  if (!cwd)
    OOPS("cannot get current working directory", NULL);

  ouid	= getuid();
  ogid	= getgid();

  if (!*user)
    pw	= getpwuid(geteuid());
  else if (!strcmp(user, "*"))
    pw	= getpwuid(ouid);
  else if (getint(user, &uid))
    pw	= getpwuid(uid);
  else
    pw	= getpwnam(user);

  if (!pw)
    OOPS(CONF, cmd, "user", user, "not found", NULL);

  uid	= pw->pw_uid;

  if (!*group)
    gid	= getegid();
  else if (!strcmp(group, "*"))
    gid = ogid;
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
    OOPS(CONF, cmd, "wrong number of arguments", NULL);

  for (;;)
    {
      args_add(&args, line);
      line	= strchr(line, ':');
      if (!line)
	break;
      *line++	= 0;
    }
  for (i=1; ++i<argc; )
    args_add(&args, argv[i]);

  if (debug)
    {
      fprintf(stderr, "cmd:  %s\n", cmd);
      fprintf(stderr, "uid:  %d\n", uid);
      fprintf(stderr, "gid:  %d\n", gid);
      fprintf(stderr, "args: %d - %d\n", minarg, maxarg);
      fprintf(stderr, "dir:  %s\n", dir);
      for (i=0; args.args[i]; i++)
        printf("%4d: %s\n", i, args.args[i]);
    }

  populate_env(&env, no_shellshock, ouid, ogid, cwd);
  if (*dir && chdir(dir))
    OOPS(dir, "cannot change directory", NULL);

  execve(args.args[0], args.args, env.args);
  OOPS("execve() failed", args.args[0], NULL);
  return 127;	/* resemble shell	*/
}

