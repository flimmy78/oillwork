#ifndef __Idiab_stdarg
#define __Idiab_stdarg

#ifdef	__cplusplus
extern "C" {
#endif

#include "va_list.h"
#if defined(__EXCLUDE_VA_LIST) && defined(__Iva_list)
typedef _va_list va_list;
#endif

#ifdef __LITTLE_ENDIAN__
#define __generic_va_start(list, parmN) (list = (va_list) ((char *)&parmN + ((sizeof(parmN) + 3) & ~3)))
#else
#define __generic_va_start(list, parmN) (list = (va_list) ((char *)&parmN + sizeof(parmN)))
#endif

#if defined(__lint)
/* trick lint */

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list) ((void)0)
#define va_arg(list, mode) (((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1])

#elif defined(m88k) || defined(__m88k)

#pragma pure_function __mem_ptr, __reg_ptr

extern int *__mem_ptr(), *__reg_ptr();

#define __gnuc_va_list va_list

#define va_start(list,parmN) {\
	(list).mem_ptr = __mem_ptr();\
	(list).reg_ptr = __reg_ptr();\
	(list).next_arg = (int *) ((char *)&parmN + sizeof(parmN)) - (list).mem_ptr;\
}
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
	sizeof(mode,2) <= 3 || sizeof(mode,2) == 44 ? (	/* char-short */\
		(list).next_arg < 8 ? (\
			*(mode*)((char*)(&(list).reg_ptr[++(list).next_arg])-sizeof(mode))\
		) : (\
			*(mode*)((char*)(&(list).mem_ptr[++(list).next_arg])-sizeof(mode))\
		)\
	) : sizeof(mode) == 4 && sizeof(mode,1) == 4 ? (/* 4 byte types */\
		(list).next_arg < 8 ? (\
			*(mode*)&(list).reg_ptr[(list).next_arg++]\
		) : (\
			*(mode*)&(list).mem_ptr[(list).next_arg++]\
		)\
	) : sizeof(mode,2) == 15  ? (			/* double */\
		((list).next_arg & 1) && (list).next_arg++,\
		(list).next_arg < 8 ? (\
			(list).next_arg += 2,\
			*(mode*)&(list).reg_ptr[(list).next_arg-2]\
		) : (\
			(list).next_arg += 2,\
			*(mode*)&(list).mem_ptr[(list).next_arg-2]\
		)\
	) : sizeof(mode,2) == 32 ? (			/* non-aggregates */\
		(list).next_arg < 8 ? (\
			**(mode**)&(list).reg_ptr[(list).next_arg++]\
		) : (\
			**(mode**)&(list).mem_ptr[(list).next_arg++]\
		)\
	) : (						/* structs */\
		(void)(sizeof(mode,1) == 8 && ((list).next_arg & 1) && (list).next_arg++),\
		(list).next_arg += (sizeof(mode)+3)>>2,\
		*(mode*)&(list).mem_ptr[(list).next_arg-((sizeof(mode)+3)>>2)]\
	)\
)

#elif defined(__ppc) && defined(__EABI__)

#pragma pure_function __va_mem, __va_reg, __va_gpr, __va_fpr

extern int __va_gpr(void), __va_fpr(void), __va_vr(void);
extern char *__va_mem(void), *__va_reg(void);
extern void *__va_arg(va_list, int);

/* undef to avoid warnings (vxWorks) */
#undef __va_one_gpr
#undef __va_two_gpr

#ifdef __softfp_abi
#define __va_one_gpr(mode)	(sizeof(mode,2) <= 7 || sizeof(mode,2) == 14 || sizeof(mode,2) == 19 || sizeof(mode,2) == 34 || sizeof(mode,2) == 44)
#define __va_two_gpr(mode)	(sizeof(mode,2) <= 9 || sizeof(mode,2) == 15 || sizeof(mode,2) == 16 || sizeof(mode,2) == 12)
#else
#ifdef __softfp
#define __va_one_gpr(mode)	(sizeof(mode,2) <= 7 || sizeof(mode,2) == 14 || sizeof(mode,2) == 19 || sizeof(mode,2) == 34 || sizeof(mode,2) == 44)
#define __va_two_gpr(mode)	(sizeof(mode,2) <= 9 || sizeof(mode,2) == 15 || sizeof(mode,2) == 16 || sizeof(mode,2) == 12)
#else
#define __va_one_gpr(mode)	(sizeof(mode,2) <= 7 || sizeof(mode,2) == 19 || sizeof(mode,2) == 34 || sizeof(mode,2) == 44)
#define __va_two_gpr(mode)	(sizeof(mode,2) <= 9 || sizeof(mode,2) == 12)
#endif
#endif

#define va_start(list,parmN) {\
	(list[0]).__mem = __va_mem();\
	(list[0]).__reg = __va_reg();\
	(list[0]).__gpr = (char)__va_gpr();\
	(list[0]).__fpr = (char)__va_fpr();\
	(list[0]).__vr = (char)__va_vr();\
}
#define va_end(list)  ((void)0)
#ifdef __SINGLE_ONLY
#define va_arg(list, mode) (\
	(sizeof(mode,2) == 15) ? (	/* double -> float */\
        	(*(mode *)((char *)__va_arg(list, 1))) \
	) : __va_one_gpr(mode) ? (			/* char-ulint + ptr (float if SWFP) */\
		(*(mode *)((char *)__va_arg(list, 1)+sizeof(int)-sizeof(mode)))\
	) : __va_two_gpr(mode) ? (		/* llint-ullint (double if SWFP) */\
		(*(mode *)__va_arg(list, 2))\
	) : (sizeof(mode,2) == 14) ? (		/* float (602, -Xno-double) */\
		(*(mode *)__va_arg(list, 4))\
	) : (sizeof(mode,2) == 16) ? (		/* long double */\
		(*(mode *)__va_arg(list, 3))\
	) : (					/* structs and others passed as pointers */\
		(*(mode *)__va_arg(list, 0))\
	)\
)
#else
#if defined(__LITTLE_ENDIAN__)
#	define	__VA_ONE_GPR_ADDEND(mode)	0
#else
#	define	__VA_ONE_GPR_ADDEND(mode)	(sizeof(int)-sizeof(mode))
#endif
#define va_arg(list, mode) (\
	 __va_one_gpr(mode) ? (			/* char-ulint + ptr (float if SWFP) */\
		 (*(mode *)((char *)__va_arg(list, 1) +__VA_ONE_GPR_ADDEND(mode))) \
	) : __va_two_gpr(mode) ? (		/* llint-ullint (double if SWFP) */\
		(*(mode *)__va_arg(list, 2))\
	) : (sizeof(mode,2) == 14) ? (		/* float (602, -Xno-double) */\
		(*(mode *)__va_arg(list, 5))\
	) : (sizeof(mode,2) == 15 || sizeof(mode,2) == 16) ? (		/* double */\
		(*(mode *)__va_arg(list, 3))\
	) : (sizeof(mode,2) == 17) ? (		/* vector */\
		(*(mode *)__va_arg(list, 4))\
	) : (					/* structs and others passed as pointers */\
		(*(mode *)__va_arg(list, 0))\
	)\
)
#endif
#if !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE) && \
    !defined(_POSIX_AEP_RT_CONTROLLER_C_SOURCE)
#ifdef __SINGLE_ONLY
#define va_arg_addr(list, mode) (\
	(sizeof(mode,2) == 15) ? (      /* double -> float */\
		((mode *)((char *)__va_arg(list, 1)))\
	) : __va_one_gpr(mode) ? (			/* char-ulint + ptr (float if SWFP) */\
		((mode *)((char *)__va_arg(list, 1)+sizeof(int)-sizeof(mode)))\
	) : __va_two_gpr(mode) ? (		/* llint-ullint (double if SWFP) */\
		((mode *)__va_arg(list, 2))\
	) : (sizeof(mode,2) == 14) ? (		/* float (602, -Xno-double) */\
		((mode *)__va_arg(list, 5))\
	) : (sizeof(mode,2) == 16) ? (		/* long double */\
		((mode *)__va_arg(list, 3))\
	) : (sizeof(mode,2) == 17) ? (		/* vector */\
		((mode *)__va_arg(list, 4))\
	) : (					/* structs and others passed as pointers */\
		((mode *)__va_arg(list, 0))\
	)\
)
#else
#define va_arg_addr(list, mode) (\
	 __va_one_gpr(mode) ? (			/* char-ulint + ptr (float if SWFP) */\
		((mode *)((char *)__va_arg(list, 1)+sizeof(int)-sizeof(mode)))\
	) : __va_two_gpr(mode) ? (		/* llint-ullint (double if SWFP) */\
		((mode *)__va_arg(list, 2))\
	) : (sizeof(mode,2) == 14) ? (		/* float (602, -Xno-double) */\
		((mode *)__va_arg(list, 5))\
	) : (sizeof(mode,2) == 15 || sizeof(mode,2) == 16) ? (		/* double */\
		((mode *)__va_arg(list, 3))\
	) : (sizeof(mode,2) == 17) ? (		/* vector */\
		((mode *)__va_arg(list, 4))\
	) : (					/* structs and others passed as pointers */\
		((mode *)__va_arg(list, 0))\
	)\
)
#endif
#endif  /* _POSIX_xxx_SOURCE */
#elif defined(__arm)
#if defined(__LITTLE_ENDIAN__)
#define va_arg(list, mode) (\
              (sizeof(mode) < sizeof(int)) ? (\
			    ((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-(sizeof(int)/sizeof(mode))]\
		  ) : (list = (va_list) ((unsigned int)(list+sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1)), ((mode *)list)[-1])\
              )
#else
#define va_arg(list, mode) (\
                (sizeof(mode,2) > 19 && sizeof(mode) < sizeof(int)) ? (\
		    ((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-(sizeof(int)/sizeof(mode))]\
		   ) : (list = (va_list) ((unsigned int)(list+sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1)), ((mode *)list)[-1])\
              )
#endif /* __LITTLE_ENDIAN__ */
#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list)  ((void)0)

#elif defined(__mcore)

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list)  ((void)0)

#if defined(__LITTLE_ENDIAN__)
#define va_arg(list, mode) (\
	sizeof(mode,2) == 32 ? (			/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
	sizeof(mode,1) >= 8 ? (				/* align on double */\
		list = (va_list)((int)((char *)list + sizeof(double) - 1) & -(int)sizeof(double)),\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
	sizeof(mode) >= sizeof(int) ? (			/* align on double */\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
		((mode *)((int *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))) - 1))[0]\
	)))\
)
#else
#define va_arg(list, mode) (\
	sizeof(mode,2) == 32 ? (			/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
	sizeof(mode,1) >= 8 ? (			/* align on double */\
		list = (va_list)((int)((char *)list + sizeof(double) - 1) & -(int)sizeof(double)),\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
        (sizeof(mode) >= sizeof(int) || sizeof(mode,2) != 23) ? (			/* align on double */\
		(list = (va_list) ((unsigned int)(list+sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1)), ((mode *)list)[-1])\
	) : (\
		((mode *)((int *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))) - 1))[0]\
	)))\
)
#endif

#elif defined(__mips) && defined(__MIPS64BIT__)

#define va_start(list,parmN) (list = (va_list)(((unsigned int)&parmN + sizeof(parmN) + 7) & ~7))
#define va_end(list)  ((void)0)

#if defined(__LITTLE_ENDIAN__)
#define va_arg(list, mode) (\
	sizeof(mode)>=8 ? ((mode *)((list += ((sizeof(mode) + 7) & ~7)) - ((sizeof(mode) + 7) & ~7)))[0] : \
	((mode *)((list += sizeof(double)) - sizeof(double)))[0])
#else
#define va_arg(list, mode) (\
	sizeof(mode)>=8 ? ((mode *)((list += ((sizeof(mode) + 7) & ~7)) - ((sizeof(mode) + 7) & ~7)))[0] : \
	((mode *)((list += sizeof(double)) - sizeof(mode)))[0])
#endif

#elif defined(__mips)

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list)  ((void)0)

#if defined(__LITTLE_ENDIAN__)
#define va_arg(list, mode) (\
	sizeof(mode,2) == 32 ? (			/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
	sizeof(mode,1) >= 8 ? (				/* align on double */\
		list = (va_list)((int)((char *)list + sizeof(double) - 1) & -(int)sizeof(double)),\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
	sizeof(mode) >= sizeof(int) ? (			/* align on double */\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
		((mode *)((int *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))) - 1))[0]\
	)))\
)
#else
#define va_arg(list, mode) (\
	sizeof(mode,2) == 32 ? (			/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
	sizeof(mode,1) >= 8 ? (			/* align on double */\
		list = (va_list)((int)((char *)list + sizeof(double) - 1) & -(int)sizeof(double)),\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
        (sizeof(mode) >= sizeof(int) || sizeof(mode,2) != 23) ? (			/* align on double */\
		list = (va_list) ((unsigned int)(list+sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1)), \
		((mode *)list)[-1]\
	) : (\
		((mode *)((int *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))) - 1))[0]\
	)))\
)
#endif

#elif defined(__sparc) /* difference from mcore/mips is that structs are passed by address */

#pragma pure_function __va_mem

extern char* __va_mem(void);
#define va_start(list,parmN) (list) = __va_mem(),(void)0
#define va_end(list)  ((void)0)
#define va_arg(list, mode) (\
	sizeof(mode,2) == 32 || sizeof(mode,2) == 23 || sizeof(mode,2) == 24 || sizeof(mode,2) == 43 ? (/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	)\
)
#elif defined(__m32r) || defined (__sh)

#if defined(__SH4__) || defined(__SH2A__)
extern double __va_dbl_align;
extern __va_greg *__va_next_o(void);
extern __va_greg *__va_next_o_limit(void);
extern __va_freg *__va_next_fp(void);
extern __va_freg *__va_next_fp_limit(void);
extern __va_greg *__va_next_stack(void);
extern void *__va_arg(va_list *, int);

#ifdef __softfp
#define __va_one_gpr(mode)	(sizeof(mode,2) <= 7 || sizeof(mode,2) == 14 || sizeof(mode,2) == 19 || sizeof(mode,2) == 34 || sizeof(mode,2) == 44)
#define __va_two_gpr(mode)	(sizeof(mode,2) <= 9 || sizeof(mode,2) == 15 || sizeof(mode,2) == 16)
#else
#define __va_one_gpr(mode)	(sizeof(mode,2) <= 7 || sizeof(mode,2) == 19 || sizeof(mode,2) == 34 || sizeof(mode,2) == 44)
#define __va_two_gpr(mode)	(sizeof(mode,2) <= 9)
#endif
#define va_start(list,parmN) {\
	list.__va_next_o = __va_next_o();		\
	list.__va_next_o_limit = __va_next_o_limit();	\
	list.__va_next_fp = __va_next_fp();		\
	list.__va_next_fp_limit = __va_next_fp_limit();	\
	list.__va_next_stack = __va_next_stack();	\
}
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
	 __va_one_gpr(mode) ? (			/* char-ulint + ptr (float if SWFP) */\
		(*(mode *)((char *)__va_arg(&list, 1)+sizeof(int)-sizeof(mode)))\
	) : __va_two_gpr(mode) ? (		/* llint-ullint (double if SWFP) */\
		(*(mode *)__va_arg(&list, 2))\
	) : (sizeof(mode,2) == 14) ? (		/* float (forced to DOUBLE) */\
		(*(mode *)__va_arg(&list, 3))\
	) : (sizeof(mode,2) == 15 || sizeof(mode,2) == 16) ? (		/* double */\
		(*(mode *)__va_arg(&list, 3))\
	) : (sizeof(mode,2) == 17) ? (		/* vector */\
		(*(mode *)__va_arg(&list, 4))\
	) : (					/* structs and others passed as pointers */\
		(*(mode *)__va_arg(&list, 0))\
	)\
)
#if !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE) && \
    !defined(_POSIX_AEP_RT_CONTROLLER_C_SOURCE)
#define va_arg_addr(list,mode) (\
	 __va_one_gpr(mode) ? (			/* char-ulint + ptr (float if SWFP) */\
		((mode *)((char *)__va_arg(&list, 1)+sizeof(int)-sizeof(mode)))\
	) : __va_two_gpr(mode) ? (		/* llint-ullint (double if SWFP) */\
		((mode *)__va_arg(&list, 2))\
	) : (sizeof(mode,2) == 14) ? (		/* float */\
		((mode *)__va_arg(&list, 3))\
	) : (sizeof(mode,2) == 15 || sizeof(mode,2) == 16) ? (		/* double */\
		((mode *)__va_arg(&list, 3))\
	) : (sizeof(mode,2) == 17) ? (		/* vector */\
		((mode *)__va_arg(&list, 4))\
	) : (					/* structs and others passed as pointers */\
		((mode *)__va_arg(&list, 0))\
	)\
)
#endif  /* _POSIX_xxx_SOURCE */
#else

extern double __va_dbl_align;
extern void *__sp();

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list) ((void)0)
#if defined(__sh)
#if defined(__LITTLE_ENDIAN__)
#define va_arg(list, mode) (\
              (sizeof(mode) < sizeof(int)) ? (\
			    ((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-(sizeof(int)/sizeof(mode))]\
		  ) : (((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1] )\
              )
#else
#undef  va_start
#define va_start(list, parmN) (list = (va_list) ((int)((char *)&parmN + sizeof(parmN) + (sizeof(int) - 1)) & ~(sizeof(int)-1)))
#define va_arg(list, mode) (\
                (sizeof(mode,2) > 19 && sizeof(mode) < sizeof(int)) ? (\
		    ((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-(sizeof(int)/sizeof(mode))]\
		   ) : (((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1] )\
              )
#endif /* __LITTLE_ENDIAN__ */
#else
#define va_arg(list, mode) (\
	sizeof(mode,2) == 32 || sizeof(mode,2) == 23 || sizeof(mode,2) == 24 || sizeof(mode,2) == 43 ? (/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
	sizeof(mode,1) >= 8 ? (			/* align on double */\
		list += (unsigned)list & 4,\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	))\
)
#endif
#endif
#elif defined(__nec)

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
	(sizeof(mode,2) == 32)? (	/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (				/* aggregates (struct and union) */\
	(sizeof(mode,2) == 23 || sizeof(mode,2) == 24) ? (	/* align on int */\
		list = (va_list)((int)((char *)list + sizeof(int) - 1) & -(int)sizeof(int)),\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
	sizeof(mode,1) >= 8 ? (				/* align on double */\
		list = (va_list)((int)((char *)list + sizeof(double) - 1) & -(int)sizeof(double)),\
		((mode *)(list += ((sizeof(mode)+sizeof(mode,1)-1) & ~(sizeof(mode,1)-1))))[-1]\
	) : (\
	sizeof(mode) >= sizeof(int) ? (			/* align on double */\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
		((mode *)((int *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))) - 1))[0]\
	))))\
)

#elif defined(__sc)
#define va_start(list,parmN) (list = (va_list) ((char *)&parmN))
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
        (sizeof(mode,1) == 8)  ? (		/* align on double */\
		((mode *) (list  -= (sizeof(double) + (((unsigned int)list) & 7))))[0]\
	) : (sizeof(mode,2) == 32) ? (		/* non-aggregates. */\
		*((mode **)(list -= ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[0]\
	) : (\
		((mode *)(list -= ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[0]\
	)\
)

#elif defined(__m68k)

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
	(sizeof(mode,2) == 32) ? (		/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (sizeof(mode,2) == 23 || sizeof(mode,2) == 24) ? ( 	/* align structs/unions on int */\
		((mode *)((list += ((sizeof(mode) + 3) & -4)) - ((sizeof(mode) + 3) & -4)))[0]\
	) : (\
		(list = (va_list) ((unsigned int)(list+sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1)), ((mode*)list)[-1])\
	)\
)
#elif defined(__tc)
#pragma pure_function __va_mem
extern char* __va_mem(void);
#define va_start(list,parmN) (list) = __va_mem(),(void)0
#define va_arg(list, mode) (\
              (sizeof(mode) < sizeof(int)) ? (\
			    ((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-(sizeof(int)/sizeof(mode))]\
		  ) : (((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1] )\
              )
#define va_end(list) ((void)0)

#elif defined(__386)

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
	(sizeof(mode,2) == 32) ? (		/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	)\
)

#else

#define va_start(list,parmN) __generic_va_start(list, parmN)
#define va_end(list) ((void)0)
#define va_arg(list, mode) (\
	(sizeof(mode,2) == 32) ? (		/* non-aggregates. */\
		*((mode **)(list += ((sizeof(mode *)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	) : (\
		((mode *)(list += ((sizeof(mode)+sizeof(int)-1) & ~(sizeof(int)-1))))[-1]\
	)\
)
#endif

#ifndef _DINKUM_STDARG_PROPERTIES
#define _DINKUM_STDARG_PROPERTIES
/* These definitions have been extracted from the Dinkumware yvals.h */
/* Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V4.00:1278 */
                /* stdarg PROPERTIES */
#ifndef __VA_LIST
#define __VA_LIST
typedef va_list _Va_list;
#endif

 #if defined(__RTP__) || (defined(__DCC__) && defined(_C99))

 #if __EDG__
  #undef va_copy
 #endif /* __EDG__ */

 #ifndef va_copy
void _Vacopy(va_list *, va_list);
  #define va_copy(apd, aps)     _Vacopy(&(apd), aps)
 #endif /* va_copy */

 #endif /* __RTP__ */
/* end definitions extracted from Dinkumware yvals.h */
#endif /* _DINKUM_STDARG_PROPERTIES */

#ifdef	__cplusplus
}
#endif
#endif
