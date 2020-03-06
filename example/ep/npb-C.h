/*
  NAS Parallel Benchmarks 2.3 Serial Versions in C - common part
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/* data types */
typedef int boolean;
typedef struct { float real; float imag; } fcomplex;
typedef struct { double real; double imag; } dcomplex;

/* constants */
#define TRUE	1
#define FALSE	0

/* macro functions */
#define max(a,b) ((a) > (b)) ? (a) : (b)
#define min(a,b) ((a) < (b)) ? (a) : (b)

#define get_real(c) c.real
#define get_imag(c) c.imag
#define cadd(c,a,b) (c.real = a.real + b.real, c.imag = a.imag + b.imag)
#define csub(c,a,b) (c.real = a.real - b.real, c.imag = a.imag - b.imag)
#define cmul(c,a,b) (c.real = a.real * b.real - a.imag * b.imag, \
                     c.imag = a.real * b.imag + a.imag * b.real)
#define crmul(c,a,b) (c.real = a.real * b, c.imag = a.imag * b)

/* external functions - move to npb-C.h */
extern double randlc(double *, double);
extern void vranlc(int, double *, double, double *);
extern void timer_clear(int);
extern void timer_start(int);
extern void timer_stop(int);
extern double timer_read(int);
