#ifndef __Iva_list
#define __Iva_list

/*
 * This file is required by stdio.h, stdarg.h and varargs.h
 * It should not generally be included directly.
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__lint)
/* trick lint */

typedef char *_va_list;

#elif defined(m88k) || defined(__m88k)

typedef struct {
	int	next_arg;
	int	*mem_ptr;
	int	*reg_ptr;
} _va_list;

#elif defined(__ppc) && defined(__EABI__)

typedef struct va_list_t {
	char	__gpr;
	char	__fpr;
	char	__vr;
	char	__filler ; /* necessary if compiled with packed */
	char	*__mem;
	char	*__reg;
} _va_list[1], __gnuc_va_list[1];

#elif defined(__arm)

typedef char *_va_list;

#elif defined(__mips) || defined(__mcore)

typedef char *_va_list;

#elif defined(__sparc) /* difference from mcore/mips is that structs are passed by address */

typedef char *_va_list;

#elif defined(__nec)

typedef char *_va_list;

#elif defined(__m32r) || defined (__sh)
#if defined(__SH4__) || defined(__SH2A__)
typedef long __va_greg;
typedef float __va_freg;
typedef struct {
  __va_greg * __va_next_o;		 
  __va_greg * __va_next_o_limit;	 
  __va_freg * __va_next_fp;		 
  __va_freg * __va_next_fp_limit;	 
  __va_greg * __va_next_stack;
} _va_list;

#else
typedef char *_va_list;
#endif
#elif defined(__nec)

typedef char *_va_list;

#else

typedef char *_va_list;

#endif

#ifndef __EXCLUDE_VA_LIST
typedef _va_list va_list;
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __Iva_list */
