// CSyd's own double-precision math: musl's (MIT, see COPYRIGHT.musl), compiled into CSyd so a
// render computes the same bits on every platform (macOS, Linux, WebAssembly) instead of
// whatever each platform's libm rounds to. sqrt, fabs and floor need no copy: IEEE 754 defines
// their results exactly.

#ifndef _SMATH_H
#define _SMATH_H

#ifdef __cplusplus
extern "C" {
#endif

double sm_pow(double, double);
double sm_exp(double);
double sm_log(double);
double sm_log10(double);
double sm_sin(double);
double sm_cos(double);
double sm_tan(double);
double sm_sinh(double);
double sm_cosh(double);
double sm_tanh(double);
double sm_asin(double);
double sm_acos(double);
double sm_atan(double);

#ifdef __cplusplus
}
#endif

#endif
