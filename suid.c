/* Generic secure SUID wrapper with security by default.
 * For programs which are SUID compatible, and those which are not.
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

#if 0
#define	DP(X...)	do { dbprintf(__FILE__, __LINE__, __FUNCTION__, X); } while (0)
static void
dbprintf(const char *file, int line, const char *fn, const char *s, ...)
{
  va_list	list;

  fprintf(stderr, "[%s:%d: %s", file, line, fn);
  va_start(list, s);
  vfprintf(stderr, s, list);
  va_end(list);
  fprintf(stderr, "]\n");
  fflush(stderr);
}
#else
#define	DP(...)	do { ; } while(0)
#endif

/* Simple line scanner with deescapement ******************************/

struct scan
  {
    char		*pos;
    struct linereader	l;
    char		file[FILENAME_MAX];
    const char		*cmd;
  };

static void
strmove(char *to, char *from)
{
  while ((*to++ = *from++)!=0);
}

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
        strmove(ptr-1, ptr);		/* \: seen but not \\:, remove the \	*/
      else
        {
          ptr -= 2;
          strmove(ptr, ptr+3);		/* \\: seen, remove it completely	*/
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

/* Environment ********************************************************/

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
  if (cwd && !shellshock(cwd))
    args_addf(env, "SUIDPWD=%s", cwd);

  /* migrate current environment to SUID_	*/
  for (p=environ; *p; p++)
    {
      s	= strchr(*p, '=');
      if (s && (allow_shellshock || !shellshock(s+1)))
        args_addf(env, "SUID_%s", *p);
    }
}

/* /etc/suid.conf and /etc/suid.conf.d/ *******************************/

static int
checkown(const char *path)
{
  struct stat st;

  if (stat(path, &st))
    return 1;
  if (st.st_uid)
    OOPS(path, "wrong ownership", OOPS_I, (int)st.st_uid, NULL);
  if (st.st_gid)
    OOPS(path, "wrong group", OOPS_I, (int)st.st_gid, NULL);
  if (st.st_mode & 022)
    OOPS(path, "wrong mode", OOPS_O, (unsigned)st.st_mode, NULL);
  return 0;
}

static char *
scan_file(struct scan *scan)
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
      if (!strcmp(scan->cmd, line))
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
find_cmd(struct scan *scan)
{
  struct dirent	**ent;
  int		n;

  /* check /etc/suid.conf	*/
  strcpy(scan->file, CONF);
  if (scan_file(scan))
    return 0;	/* found	*/

  if (checkown(CONFDIR))
    return 1;	/* missing /etc/suid.conf.d/	*/

  /* check /etc/suid.conf.d/ *.conf	*/
  n	= scandir(CONFDIR, &ent, conf_filter, alphasort);
  /* ignore error (n<0) when dir is missing	*/
  for (; --n>=0; ent++)
    {
      snprintf(scan->file, sizeof scan->file, "%s/%s", CONFDIR, (*ent)->d_name);
      if (scan_file(scan))
        return 0;	/* found	*/
    }

  return 1;	/* not found	*/
}

/* Flags **************************************************************/

/* set flags given on argument list in minmax:
 * for each flag in flags there must be an (int *) arg.
 * The arg is set to !=0 if flag is present, 0 otherwise.
 * Returns the next position in minmax (end of flags).
 */
static char *
get_flags(struct scan *scan, char *minmax, const char *flags, ...)
{
  va_list	list;
  int		*i;
  const char	*tmp;

  DP("(%p, %s, %s)", scan, minmax, flags);
  va_start(list, flags);
  for (tmp=flags; *tmp; tmp++)
    *va_arg(list, int *)	= 0;
  va_end(list);
  va_start(list, flags);
  for (tmp=flags; *tmp; tmp++)
    {
      i = va_arg(list, int *);
      if (*minmax == *tmp)
        {
          if (*i)
            OOPS(scan->file, OOPS_I, scan->l.linenr, scan->cmd, "incompatible flag", OOPS_C, *i, OOPS_C, *tmp, NULL);
          minmax++;
          *i	= *tmp;
        }
    }
  va_end(list);
  return minmax;
}

static void
dump_options(FILE *fd)
{
#if 0
  printf("SUID: list of options\n");
#endif
}

/* Modifiers **********************************************************/

enum suid_type
  {
    TYPE_NORMAL,
    TYPE_SUID,	/* Call if it has SUID flags	*/
    TYPE_ROOT,	/* As TYPE_SUID, but with modified original UID/GID */
    TYPE_SH,	/* /bin/sh does not support suid	*/
    TYPE_BASH,	/* As TYPE_SH, but supports modified arg0	*/
  };

static enum suid_type
modifier(struct args *args, enum suid_type type)
{
  if (args->n!=1)
    return type;

  const char	*cmd	= args->args[0];

  DP("('%p', %d) n=%d cmd='%s'", args, type, args->n, cmd);

  if (     !strcmp(cmd, "suid"))	type	= TYPE_SUID;
  else if (!strcmp(cmd, "root"))	type	= TYPE_ROOT;
  else if (!strcmp(cmd, "sh"  ))	type	= TYPE_SH;
  else if (!strcmp(cmd, "bash"))	type	= TYPE_BASH;
  else
    return type;

  args->n	= 0;
  DP("()=%d n=%d", type, args->n);
  return type;
}

/* Helpers ************************************************************/

static char *
file_name(char *s)
{
  char	*tmp;

  tmp	= strrchr(s, '/');
  return tmp ? tmp+1 : s;
}

static int
checkfile(int uid, int gid, struct args *args, int insecure, int wrap)
{
  struct stat	st;
  int		f, d;
  char		*dir;
  char		*name, *orig;

  DP("(%d,%d,%p,%d)", uid, gid, args, insecure);
  /* calculate everything about what we try to execute	*/
  orig	= args->args[0];
  if ((dir=realpath(orig, NULL))==0 || (name=file_name(dir))==dir || *dir!='/')
    OOPS(orig, "path resolution failed", dir, NULL);
  args->args[0]	= stralloc(dir);	/* return, what needs to be executed	*/
  name[-1]	= 0;

  DP("() dir '%s'", dir);
  /* be careful what to access	*/
  d	= openat(AT_FDCWD, dir, O_RDONLY|O_DIRECTORY|O_CLOEXEC|(wrap ? 0 : O_NOFOLLOW));
  if (d<0)
    OOPS(dir, "cannot access directory", NULL);

  DP("() file '%s'", name);
  f	= openat(d, name, O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
  if (f<0)
    OOPS(orig, "cannot access file", name, NULL);

  if (insecure)
    goto insecure_return;

  /* check the file entry	*/
  if (fstat(f, &st))
    OOPS(orig, "cannot stat", NULL);

  if ((st.st_mode & S_IFMT)!=S_IFREG)
    OOPS(orig, "not a regular file", NULL);
  if (st.st_mode & S_IWOTH)
    OOPS(orig, "is globally writeable", OOPS_O, (unsigned)st.st_mode, NULL);
  if (st.st_uid && st.st_uid!=uid)
    OOPS(orig, "wrong user id", OOPS_I, (int)st.st_uid, "expected", OOPS_I, uid, NULL);
  if ((st.st_mode & S_IWGRP) && st.st_gid && st.st_gid!=gid)
    OOPS(orig, "wrong group id", OOPS_I, (int)st.st_gid, "expected", OOPS_I, gid, NULL);

  if (fstat(d, &st))
    OOPS(dir, "cannot stat directory", NULL);

  for (;;)
    {
      struct stat	st2;
      int		p;

      if ((st.st_mode & S_IFMT)!=S_IFDIR)
        OOPS(dir, "not a directory", NULL);	/* WTF? Just to be sure ..	*/
      if ((st.st_mode & (S_IWOTH|S_ISVTX))==S_IWOTH)
        OOPS(dir, "is globally writeable", OOPS_O, (unsigned)st.st_mode, NULL);
      if (st.st_uid && st.st_uid!=uid)
        OOPS(dir, "wrong user id", OOPS_I, (int)st.st_uid, "expected", OOPS_I, uid, NULL);
      if ((st.st_mode & S_IWGRP) && st.st_gid && st.st_gid!=gid)
        OOPS(dir, "wrong group id", OOPS_I, (int)st.st_gid, "expected", OOPS_I, gid, NULL);

      if (name==dir)
        break;

      p	= openat(d, "..", O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW);
      if (p<0)
        OOPS(dir, "cannot access parent directory", NULL);
      close(d);

      if (fstat(p, &st2))
        OOPS(dir, "cannot stat parent directory", NULL);
      close(p);

      name	= file_name(dir);
      name--;
      name[name==dir ? 1 : 0]	= 0;

      DP("() parent '%s' name '%s'", dir, name);

      d	= openat(AT_FDCWD, dir, O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW);
      if (d<0)
        OOPS(dir, "cannot access directory", NULL);
      if (fstat(d, &st))
        OOPS(dir, "cannot stat directory", NULL);

      if (st.st_dev  != st2.st_dev  ||
          st.st_ino  != st2.st_ino  ||
          st.st_mode != st2.st_mode ||
          st.st_uid  != st2.st_uid  ||
          st.st_gid  != st2.st_gid)
        OOPS(dir, "directory stat mismatch on path and parent of", name, NULL);
    }

insecure_return:
  close(d);
  free(dir);
  DP("() %d", f);
  return f;

}

/* Main ***************************************************************/

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
  int			i, minarg, maxarg, debug, suid_cmd, insecure, allow_shellshock;
  int			allow_tiocsti;
  int			wrap;
  enum suid_type	suid_type;
  int			runfd;
  char			*orig;

  if (argc<2)
    {
      dump_options(stdout);	/* dump this to stdout	*/
      /* Avoid to print user defined parameters, so do not output argv[0] here	*/
      OOPS("Usage: suid command [args..]\n"
           "\t\tVersion " SUID_VERSION " compiled " __DATE__ "\n"
           "\tConfig is in file " CONF " or dir " CONFDIR "/*" CONFEXT ":\n"
           "\tcommand:pw:user:grp:minmax:dir:/path/to/bin:args..\n"
           "\tpw:       currently must be empty\n"
           "\tuser/grp: '' (suid) * (caller) = (gid of user)\n"
           "\tminmax:   [CDIFNRSTW][minargs][-[maxargs]]\n"
           "\t          Cmd/Filename/Next/Realpath is arg0, other flags:\n"
           "\t          Debug/Insecure/ShellShock/TIOCSTI/Wrap\n"
           "\targs..:   optional list of ':' separated args\n"
           "\t          '\\:' escapes ':', '\\\\:' is swallowed\n"
           "\t          (Use '\\\\:' to disambiguate)\n"
           "\n"
           "\t'suid:' before '/path/to/bin' for suid-capable bin.\n"
           "\t'root:' to call as root and drop to the given user.\n"
           "\t'sh:' to feed though /bin/sh (no arg0 handling)\n"
           "\t'bash:' to feed though /bin/bash (allows arg0 handling)\n"
           "\tClean Env: SUIDUID/SUIDGID/SUIDPWD/TERM.  Others\n"
           "\tare prefixed with SUID_ (Shellshock save unless S)\n"
           "\n"
           "\tsuid usually returns the value of the bin, except:\n"
           "\t126 if suid fails/usage  (see: bash -c /dev/null)\n"
           "\t127 if command not found (see: bash -c /notfound)\n"
           , NULL);
    }

  cmd = argv[1];
#if 1	/* rly?	*/
  if (strchr(cmd, '/'))
    OOPS(argv[0], "command must not contain '/'", NULL);
#endif

  /* scan /etc/suid.conf and /etc/suid.conf.d/	*/
  scan.cmd	= cmd;
  if (find_cmd(&scan))
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
   * Empty for now.  In future you list Options (like Namespaces) here
   */
  if (*pass)
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "pw not yet supported", pass, NULL);

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * early process optional flags, which are before min-max (flags must be sorted ABC):
   * Cmd/Filename/Next/Realpath defines how arg0 is set, default is what is in the config
   * Debug
   * Insecure
   * ShellShock
   * TIOCSTI
   * Wrap
   */
  minmax = get_flags(&scan, minmax, "CDIFNRSTW", &suid_cmd, &debug, &insecure, &suid_cmd, &suid_cmd, &suid_cmd, &allow_shellshock, &allow_tiocsti, &wrap);

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
  while (*minmax && *minmax==' ')
    minmax++;
  if (*minmax)
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "wrong minmax", minmax, NULL);

  /* check that number of arguments are within given min-max	*/
  if (argc-2<minarg || (maxarg>=0 && argc-2>maxarg))
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "wrong number of arguments", NULL);

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process dir
   *
   * . becomes $HOME
   */
  if (!strcmp(dir, "."))
    dir	= pw->pw_dir;

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process args..
   *
   * De-Escape \: to : and \\: to nothing
   */
  suid_type	= TYPE_NORMAL;
  while (line)
    {
      if (!args.n)
        while (*line==' ') line++;
      args_add(&args, line);
      line	= next_deescape(&scan, '\\');
      suid_type	= modifier(&args, suid_type);
    }
  if (args.n<1)
    OOPS(scan.file, OOPS_I, scan.l.linenr, cmd, "missing command", NULL);

  /* Apply the new uid/gid for normal commands.
   * On "suid" commands, apply it such, as if command was invoked with suid flags.
   */
  switch (suid_type)
    {
    default:	FATAL(suid_type);

    case TYPE_ROOT:
      if (!uid || !gid)
        OOPS(scan.file, OOPS_I, scan.l.linenr, "'root:' needs nonprivileged user/group, not", OOPS_I, uid, OOPS_I, gid, NULL);
      if (setregid(gid, egid))
        OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot move to group", OOPS_I, gid, NULL);
      if (setreuid(uid, euid))
        OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot move to user", OOPS_I, uid, NULL);
      break;

    case TYPE_SUID:
      if (egid != gid && setegid(gid))
        OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot set effective group", OOPS_I, gid, NULL);
      if (euid != uid && seteuid(gid))
        OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot set effective user", OOPS_I, uid, NULL);
      /* we can now switch between getuid() (caller) and geteuid() (config) as in SUID programs
       * using seteuid() - you can use setuid() in case getuid()/geteuid() is not root (0)
       */
      break;

    case TYPE_SH:
    case TYPE_BASH:
    case TYPE_NORMAL:
      if (setgid(gid))
        OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot drop group priv", OOPS_I, gid, NULL);
      if (setuid(uid))
        OOPS(scan.file, OOPS_I, scan.l.linenr, "cannot drop user priv", OOPS_I, uid, NULL);
      break;
    }
  /* note: according to the manual following is the case:
   *
   * When euid is privileged, setuid() drops all privileges (uid, euid and saved uid).
   * Only if euid is not privileged, setuid() allows to regain saved privileges again.
   * As euid is always privileged in our case, setuid() completely drops the privileges.
   */

  /* append user arguments from commandline	*/
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
  else if (cmd[0]=='!')	/* Disallow direct call to Options, except when debugging */
    OOPS(argv[0], "command must not start with '!'", NULL);

  /* command:pw:user:group:minmax:dir:/path/to/binary:args..
   * process dir
   */
  if (*dir && chdir(dir))
    OOPS(dir, "cannot change directory", NULL);

  /* check that command is safe to use
   *
   * Command must either be owned by root,
   * or by the effective user.
   *
   * Directories, which contain the command, must fulfill the same.
   * (We can stop searching if we hit a 755 root:root directory.)
   */
  orig	= args.args[0];
  runfd	= checkfile(uid, gid, &args, insecure, wrap);

  /* args.args[0] was populated with the full path
   * of the file which is referenced by runfd
   */
  switch (suid_cmd)
    {
    default:	FATAL(suid_cmd); 					/* catch programming errors			*/
    case 0:	args.args[0]	= orig;				break;	/* (no flag) switch back to orig		*/
    case 'C':	args.args[0]	= cmd;				break;	/* use (C)md from commandline as arg0		*/
    case 'F':	args.args[0]	= file_name(args.args[0]);	break;	/* only use the (F)ile name portion for arg0	*/
    case 'N':	args_pop(&args, 1);				break;	/* use the "(N)ext" arg (ignore first arg)	*/
    case 'R':							break;	/* use the (R)ealpath == no change		*/
    }

  /* handle special modifiers	*/
  switch (suid_type)
    {
    default: break;	/* quiesce compiler	*/

    case TYPE_SH:
      args_prepend(&args, "/bin/sh", "-c", "--", "exec \"$@\"", NULL);
      if (suid_cmd)
        OOPS(scan.file, OOPS_I, scan.l.linenr, "modifier 'sh:' does not support flag", OOPS_C, suid_cmd, NULL);
      break;
    case TYPE_BASH:
      args_prepend(&args, "/bin/bash", "-c", "--", "exec -a \"$0\" -- \"$@\"", NULL);
      break;
    }

  /* fill target environment	*/
  populate_env(&env, allow_shellshock, ouid, ogid, cwd);

  /* fix various security related things */
  if (!allow_tiocsti)
    setsid();	/* fails if we are already session leader	*/

  /* invoke command	*/
  fexecve(runfd, args.args, env.args);
  OOPS("execve() failed", args.args[0], NULL);
  return 127;	/* resemble shell	*/
}

