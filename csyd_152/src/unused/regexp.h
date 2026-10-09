// U-REGEXP.H
// #include "C-LOCAL.H"

char *re_comp(char *pat);
int re_exec(register char *lp);
void re_modw(register char *s);
int re_subs(register char *src, register char *dst);