/* Run commands with generic SUID
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#define	_GNU_SOURCE
#define	OOPS_FAIL	126

#include "linereader.h"
#include "args.h"

#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include "suid_version.h"

#include "osx.h"

#define	SHELLSHOCK	"() {"

#define	CONF	"/etc/suid.conf"
#define	CONFDIR	"/etc/suid.conf.d"
#define	CONFEXT	".conf"
#define	PATH	"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

struct scan
  {
    char		*pos;
    struct linereader	l;
    char		file[FILENAME_MAX];
  };

/* next arg with deescapement with ESC, but returns NULL on EOL
 * If ESC==0 no deescapement happens.
 * ESC :	gives :
 * ESC ESC :	gives nothing
 * else normal splitting occurs.
 */
static char *
next_deescape(struct scan *scan, char esc)
{
  char	*ptr;

  if (!scan->pos)
    OOPS(scan->file, OOPS_I, scan->l.linenr, "line too short", NULL);

  while ((ptr = strchr(scan->pos, ':'))!=0)
    {
      if (ptr<=scan->pos || ptr[-1]!=esc)	/* <= instead of == for safety	*/
        {
          /* split	*/
          *ptr++	= 0;
          scan->pos	= ptr;
          return ptr;
        }
      if (ptr-1<=scan->pos || ptr[-2]!=esc)
        strcpy(ptr-1, ptr);		/* \: seen but not \\:, remove the \	*/
      else
        {
          ptr -= 2;
          strcpy(ptr, ptr+3);		/* \\: seen, remove it completely	*/
        }
      /* loop after deescapement */
      scan->pos	= ptr;
    }

  scan->pos	= 0;
  return 0;
}

/* command:pw:user:group:minmax:dir:/path/to/binary:args..
 * Get next column
 */
static char *
next(struct scan *scan)
{
  char	*ptr;

  ptr	= next_deescape(scan, 0);
  if (!ptr)
    OOPS(scan->file, OOPS_I, scan->l.linenr, "malformed line", NULL);
  return ptr;
}

/* return 1 if shellshock pattern found
 */
static int
shellshock(const char *s)
{
  return !memcmp(s, SHELLSHOCK, sizeof SHELLSHOCK-1);
}

/* Prepare SUID environment
 */
static void
populate_env(struct args *env, int allow_shellshock, int uid, int gid, const char *cwd)
{
  char	**p, *s;

  /* populate PATH and TERM	*/
  args_add(env, PATH);
  if ((s=getenv("TERM"))!=0 && (allow_shellshock || !shellshock(s)))
    args_addf(env, "TERM=%s", s);

  /* populate SUID* variables	*/
  args_addf(env, "SUIDUID=%d", uid);
  args_addf(env, "SUIDGID=%d", gid);
  if (cwd)
    args_addf(env, "SUIDPWD=%s", cwd);

  /* migrate current environment to SUID_	*/
  for (p=environ; *p; p++)
    {
      s	= strchr(*p, '=');
      if (s && (allow_shellshock || !shellshock(s+1)))
        args_addf(env, "SUID_%s", *p);
    }
}

static int
checkown(const char *path)
{
  struct stat st;

  if (stat(path, &st))
    return 1;
  if (st.st_uid)
    OOPS(path, "wrong ownership", OOPS_I, (int)st.st_uid, NULL);
  if (st.st_uid || st.st_gid)
    OOPS(path, "wrong group", OOPS_I, (int)st.st_gid, NULL);
  if (st.st_mode & 022)
    OOPS(path, "wrong mode", OOPS_O, st.st_mode, NULL);
  return 0;
}

static char *
scan_file(struct scan *scan, const char *cmd)
{
  char	*line;

  if (checkown(scan->file))
    return 0;

  linereader_init(&scan->l, scan->file);
  while ((scan->pos = line = linereader(&scan->l))!=0)
    {
      if (*line == '#' || !*line)
        continue;
      next(scan);
      if (!strcmp(cmd, line))
        break;
    }
  if (linereader_end(&scan->l))
    OOPS(scan->file, OOPS_I, scan->l.linenr, "read error", NULL);
  return scan->pos;	/* 0 on EOF, else position of password	*/
}

static int
endswith_i(const char *haystack, const char *tail)
{
  int	pos;

  pos	= strlen(haystack)-strlen(tail);
  return pos>=0 && !strcasecmp(haystack+pos, tail);
}

static int
conf_filter(const struct dirent *dp)
{
  return endswith_i(dp->d_name, CONFEXT);
}

static int
find_cmd(struct scan *scan, const char *cmd)
{
  struct dirent	**ent;
  int		n;

  /* check /etc/suid.conf	*/
  strcpy(scan->file, CONF);
  if (scan_file(scan, cmd))
    return 0;	/* found	*/

  if (checkown(CONFDIR))
    return 1;	/* missing /etc/suid.conf.d/	*/

  /* check /etc/suid.conf.d/ *.conf	*/
  n	= scandir(CONFDIR, &ent, conf_filter, alphasort);
  /* ignore error (n<0) when dir is missing	*/
  for (; --n>=0; ent++)
    {
      snprintf(scan->file, sizeof scan->file, "%s/%s", CONFDIR, (*ent)->d_name);
      if (scan_file(scan, cmd))
        return 0;	/* found	*/
    }

  return 1;	/* not found	*/
}

/* set flags given on argument list in minmax:
 * for each flag in flags there must be an (int *) arg.
 * The arg is set to 1 if flag is present, 0 otherwise.
 * Returns the next position in minmax (end of flags).
 */
static char *
get_flags(char *minmax, const char *flags, ...)
{
  va_list	list;
  int		*i;

  va_start(list, flags);
  for (; *flags; flags++)
    {
      i		= va_arg(list, int *);
      *i	= 0;
      if (*minmax == *flags)
        {
          minmax++;
          *i	= 1;
        }
    }
  return minmax;
}

static void
dump_options(FILE *fd)
{
#if 0
  printf("SUID: list of options\n");
#endif
}

/* This routine is too long
 */
int
main(int argc, char **argv)
{
  struct scan		scan = { 0 };
  struct args		args = { 0 }, env = { 0 };
  char			*cmd, *pass, *user, *group, *minmax, *dir, *line, *cwd;
  int			uid, gid, ouid, ogid, euid, egid;
  struct passwd		*pw;
  int			i, minarg, maxarg, debug, allow_shellshock;

  if (argc<2)
    {
      dump_options(stdout);	/* dump this to stdout	*/
      /* Avoid to print user defined parameters, so do not output argv[0] here	*/
      OOPS("Usage: suid command [args..]\n"
           "\t\tVersion " SUID_VERSION " compiled " __DATE__ "\n"
           "\tConfig is in file " CONF " or dir " CONFDIR "/*" CONFEXT ":\n"
           "\n"
           "\tcommand:pw:user:grp:minmax:dir:/path/to/binary:args..\n"
           "\tpw:       comma separated list of options\n"
           "\tuser/grp: '' (suid) * (caller) = (gid of user)\n"
           "\tminmax:   [D][S][minargs][-[maxargs]]\n"
           "\targs..:   optional list of : separated args\n"
           "\t          Escape ':' with '\\:' and '\\' with '\\\\:'\n"
           "\t          'a:b'  must be written as 'a\\:b'\n"
           "\t          'a\\b'  can  be written as 'a\\\\:b'\n"
           "\t          'a\\:b' must  be written as 'a\\\\:\\:b'\n"
           "\n"
           "\t!opt:option:value:!flags:dir:/path/to/checkscript:args..\n"
           "\toption/value: optional, see list of options above\n"
           "\t!flags:    ![D][S]\n"
           "\tIf binary is given and fails, the whole process fails\n"
           "\n"
           "\tsuid usually returns the value of the binary, except:\n"
           "\t125 for suid failure (like this usage)\n"
           "\t126 if option fails (see: bash -c /dev/null)\n"
           "\t127 if command not found (see: bash -c /notfound)\n"
           , NULL);
    }

  cmd = argv[1];
#if 1	/* rly?	*/
  if (strchr(cmd, '/'))
    OOPS(argv[0], "command must not contain '/'", NULL);
#endif

  /* scan /etc/suid.conf and /etc/suid.conf.d/	*/
  if (find_cmd(&scan, cmd))
    {
      /* Avoid to print user defined parameters, so not output argv[1] here	*/
      OOPS(CONF, "command not found", NULL);
    }

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * scan.pos is at: pw
   */
  pass	= scan.pos;
  user	= next(&scan);
  group	= next(&scan);
  minmax= next(&scan);
  dir	= next(&scan);
  line	= next(&scan);

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * check pw
   */
  if (*pass)
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "pw not yet supported", pass, NULL);

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * early process optional flags, which are before min-max (flags must be sorted ABC):
   * DS
   */
  minmax = get_flags(minmax, "DS", &debug, &allow_shellshock);

  /* get current settings	*/
  cwd	= getcwd(NULL, 0);
  if (!cwd)
    OOPS("cannot get current working directory", NULL);

  ouid	= getuid();
  ogid	= getgid();
  euid	= geteuid();
  egid	= getegid();

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process user
   */
  if (!*user)
    pw	= getpwuid(euid);
  else if (!strcmp(user, "*"))
    pw	= getpwuid(ouid);
  else if (getint(user, &uid))
    pw	= getpwuid(uid);
  else
    pw	= getpwnam(user);

  if (!pw)
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "user", user, "not found", NULL);

  uid	= pw->pw_uid;

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process group
   */
  if (!*group)
    gid	= egid;
  else if (!strcmp(group, "*"))
    gid = ogid;
  else if (!strcmp(group, "="))
    gid	= pw->pw_gid;
  else if (!getint(group, &gid))
    {
      struct group *gr;

      gr	= getgrnam(group);
      if (!gr)
        OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "group", group, "not found", NULL);
      gid	= gr->gr_gid;
    }

  /* Apply the new uid/gid.
   * Only apply the new ID, if it is NOT already the effective ID.
   * This means, we can move "suid" things into the cloned process.
   * (this is no security problem, as we are already privileged)
   */
  if (egid != gid && setgid(gid))
    OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot drop group priv", OOPS_I, gid, NULL);
  if (euid != uid && setuid(uid))
    OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot drop user priv", OOPS_I, uid, NULL);
  /* note: according to the manual following is the case:
   *
   * When euid is privileged, setuid() drops all privileges (uid, euid and saved uid).
   * Only if euid is not privileged, setuid() allows to regain saved privileges again.
   * As euid is always privileged in our case, setuid() completely drops the privileges.
   */

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process minmax (everything after flags)
   */
  minarg = fetchint(&minmax, 0);
  maxarg = minarg;
  if (*minmax=='-')
    {
      minmax++;
      maxarg	= fetchint(&minmax, -1);
    }
  if (*minmax)
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "wrong minmax", minmax, NULL);

  /* check that number of arguments are within given min-max	*/
  if (argc-2<minarg || (maxarg>=0 && argc-2>maxarg))
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "wrong number of arguments", NULL);

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process args..
   *
   * De-Escape \: to : and \\: to nothing
   */
  while (line)
    {
      args_add(&args, line);
      line	= next_deescape(&scan, '\\');
    }
  /* append user arguments to commandline	*/
  for (i=1; ++i<argc; )
    args_add(&args, argv[i]);

  /* show debug info (flag D)	*/
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

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process dir
   */
  if (*dir && chdir(dir))
    OOPS(dir, "cannot change directory", NULL);

  /* fill target environment	*/
  populate_env(&env, allow_shellshock, ouid, ogid, cwd);

  /* invoke command	*/
  execve(args.args[0], args.args, env.args);
  OOPS("execve() failed", args.args[0], NULL);
  return 127;	/* resemble shell	*/
}

