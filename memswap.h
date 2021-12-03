/* memory helper
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

/* abcdef -> fedcba
 */
static void
memrev(void *base, size_t len)
{
  char	*a, *b, c;

  if (len)
    for (a = base, b = a+len-1; a<b; a++, b--)
      c = *a, *a = *b, *b = c;
}

/* Swap memory area within itself.
 * The both parts do not need to be of equal size.
 *
 *           !off!
 * before:   abcdEF.
 * after:    EFabcd.
 *           !-len-!
 */
static void *
memswap(void *base, size_t off, size_t len)
{
  if (!base || !off || off>=len)
    return base;

  /* Optimization to just touch each element once:
   *
   * XXX TODO XXX Handle special case where GCD(off,len)==off
   * XXX TODO XXX Handle special case where GCD(off,len)==1
   * XXX TODO XXX Handle common case  with  GCD(off,len) iterations
   *
   * For now, touch each element twice:
   */

  memrev(base, off);			/* abcdEFG -> dcbaEFG	*/
  memrev(((char *)base)+off, len-off);	/* dcbaEFG -> dcbaGFE	*/
  memrev(base, len);			/* dcbaGFE -> EFGabcd	*/
  return base;
}

