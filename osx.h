t/* This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#ifdef __APPLE__
#ifdef __MACH__

/* IGUR is meant to be called with any number of arguments to ignore them.
 * But clang issues following warning, and I found no way to suppress this:
 * warning: too many arguments in call to 'IGUR'
 * (IGUR is a workaround to mark, that return values are intentionally ignored.)
 */
extern char **environ;

#endif
#endif

