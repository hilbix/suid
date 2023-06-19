/* Some experiments with SUID status on Linux
 *
 * Not copyrighted - Public Domain
 *
 * See suid.conf.d.example/linuxid.conf.ex
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int
main()
{
  FILE	*fd;
  char	line[BUFSIZ];
  int	uid;

  fd = fopen("/proc/self/status", "rt");
  if (!fd)
    return 1;
  while (fgets(line, sizeof line, fd))
    if (!strncmp(line, "Uid:", 4) || !strncmp(line, "Gid:", 4))
      fputs(line, stdout);
  fclose(fd);
  printf("eugid = %10d %10d\n", (int)geteuid(), (int)getegid());
  printf("rugid = %10d %10d\n", (int)getuid(), (int)getgid());
  uid	= getuid();
  if (!seteuid(0)) printf("seteuid(0) worked\n");
  if (!seteuid(1)) printf("seteuid(1) worked\n");
  if (!seteuid(uid)) printf("seteuid(%d) worked\n", uid);
  if (!seteuid(1)) printf("seteuid(1) worked\n");
  fflush(stdout);
  return 0;
}

