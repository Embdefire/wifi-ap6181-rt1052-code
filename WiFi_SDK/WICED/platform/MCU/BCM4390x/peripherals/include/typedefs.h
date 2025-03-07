/*
 * Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 * $Id: typedefs.h 452599 2014-01-31 07:25:38Z sunilkum $
 */

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_
#define MAX_EVENT 16   /* Maximum number of event bit 128 */

#ifdef SITE_TYPEDEFS

/*
 * Define SITE_TYPEDEFS in the compile to include a site-specific
 * typedef file "site_typedefs.h".
 *
 * If SITE_TYPEDEFS is not defined, then the code section below makes
 * inferences about the compile environment based on defined symbols and
 * possibly compiler pragmas.
 *
 * Following these two sections is the Default Typedefs section.
 * This section is only processed if USE_TYPEDEF_DEFAULTS is
 * defined. This section has a default set of typedefs and a few
 * preprocessor symbols (TRUE, FALSE, NULL, ...).
 */

#include "site_typedefs.h"

#else

/*
 * Infer the compile environment based on preprocessor symbols and pragmas.
 * Override type definitions as needed, and include configuration-dependent
 * header files to define types.
 */

#ifdef __cplusplus

#define TYPEDEF_BOOL
#ifndef FALSE
#define FALSE	false
#endif
#ifndef TRUE
#define TRUE	true
#endif

#else	/* ! __cplusplus */

#if defined(_WIN32)

#define TYPEDEF_BOOL
typedef	unsigned char	bool;			/* consistent w/BOOL */

#endif /* _WIN32 */

#endif	/* ! __cplusplus */

#if defined(_WIN64) && !defined(EFI)
/* use the Windows ULONG_PTR type when compiling for 64 bit */
#include <basetsd.h>
#define TYPEDEF_UINTPTR
typedef ULONG_PTR uintptr;
#elif defined(__x86_64__)
#define TYPEDEF_UINTPTR
typedef unsigned long long int uintptr;
#endif

#if defined(_HNDRTE_) && !defined(_HNDRTE_SIM_)
#define _NEED_SIZE_T_
#endif

#if defined(_MINOSL_)
#define _NEED_SIZE_T_
#endif

#if defined(EFI) && !defined(_WIN64)
#define _NEED_SIZE_T_
#endif

#if defined(TARGETOS_nucleus)
/* for 'size_t' type */
#include <stddef.h>

/* float_t types conflict with the same typedefs from the standard ANSI-C
** math.h header file. Don't re-typedef them here.
*/
#define TYPEDEF_FLOAT_T
#endif   /* TARGETOS_nucleus */

#if defined(TARGETOS_nuttx)
/* These float32 and float64 types conflict with the same typedefs in
** stddef.h. Don't re-typedef them here.
*/
#define TYPEDEF_FLOAT32
#define TYPEDEF_FLOAT64
/* bool available in stdbool.h. */
#include <stdbool.h>
#define TYPEDEF_BOOL
#endif   /* TARGETOS_nuttx */

#if defined(_NEED_SIZE_T_)
typedef long unsigned int size_t;
#endif

#ifdef _MSC_VER	/* Microsoft C */
#define TYPEDEF_INT64
#define TYPEDEF_UINT64
typedef signed __int64	int64;
typedef unsigned __int64 uint64;
#endif

#if defined(MACOSX)
#define TYPEDEF_BOOL
#endif

#if defined(__NetBSD__)
#define TYPEDEF_BOOL
#ifndef _KERNEL
#include <stdbool.h>
#endif
#define TYPEDEF_UINT
#define TYPEDEF_USHORT
#define TYPEDEF_ULONG
#endif /* defined(__NetBSD__) */

#if (defined(__FreeBSD__))
#include <sys/param.h>
#if (__FreeBSD_version == 901000)
#define TYPEDEF_BOOL
#endif /* (__FreeBSD_version == 901000) */
#endif /* (defined(__FreeBSD__)) */

#if defined(__sparc__)
#define TYPEDEF_ULONG
#endif

#if defined(vxworks)
#define TYPEDEF_USHORT
typedef unsigned int socklen_t;
#endif

#ifdef	linux
/*
 * If this is either a Linux hybrid build or the per-port code of a hybrid build
 * then use the Linux header files to get some of the typedefs.  Otherwise, define
 * them entirely in this file.  We can't always define the types because we get
 * a duplicate typedef error; there is no way to "undefine" a typedef.
 * We know when it's per-port code because each file defines LINUX_PORT at the top.
 */
#if !defined(LINUX_HYBRID) || defined(LINUX_PORT)
#define TYPEDEF_UINT
#ifndef TARGETENV_android
#define TYPEDEF_USHORT
#define TYPEDEF_ULONG
#endif /* TARGETENV_android */
#ifdef __KERNEL__
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19))
#define TYPEDEF_BOOL
#endif	/* >= 2.6.19 */
/* special detection for 2.6.18-128.7.1.0.1.el5 */
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 18))
#include <linux/compiler.h>
#ifdef noinline_for_stack
#define TYPEDEF_BOOL
#endif
#endif	/* == 2.6.18 */
#endif	/* __KERNEL__ */
#endif  /* !defined(LINUX_HYBRID) || defined(LINUX_PORT) */
#endif	/* linux */

#if defined(__ECOS)
#define TYPEDEF_UCHAR
#define TYPEDEF_UINT
#define TYPEDEF_USHORT
#define TYPEDEF_ULONG
#define TYPEDEF_BOOL
#endif

#if !defined(linux) && !defined(vxworks) && !defined(_WIN32) && !defined(_CFE_) && \
!defined(_HNDRTE_) && !defined(_MINOSL_) && !defined(__DJGPP__) && \
!defined(__IOPOS__) && !defined(__ECOS) && !defined(__BOB__) && \
!defined(TARGETOS_nucleus) && !defined(EFI) && !defined(__FreeBSD__) && \
!defined(TARGETOS_nuttx)
#define TYPEDEF_UINT
#define TYPEDEF_USHORT
#endif

#if defined(vxworks)
/* Do not support the int64 type in VxWorks */
#define TYPEDEF_INT64
#endif

/* Do not support the (u)int64 types with strict ansi for GNU C */
#if defined(__GNUC__) && defined(__STRICT_ANSI__)
#if !defined(__FreeBSD__)
#define TYPEDEF_INT64
#define TYPEDEF_UINT64
#endif /* !defined(__FreeBSD__) */
#endif /* defined(__GNUC__) && defined(__STRICT_ANSI__) */

/* ICL accepts unsigned 64 bit type only, and complains in ANSI mode
 * for signed or unsigned
 */
#if defined(__ICL)

#define TYPEDEF_INT64

#if defined(__STDC__)
#define TYPEDEF_UINT64
#endif

#endif /* __ICL */

#if !defined(_WIN32) && !defined(_CFE_) && !defined(_HNDRTE_) && !defined(_MINOSL_) && \
	!defined(__DJGPP__) && !defined(__IOPOS__) && !defined(__BOB__) && \
	!defined(TARGETOS_nucleus) && !defined(EFI)

/* pick up ushort & uint from standard types.h */
#if defined(linux) && defined(__KERNEL__)

/* See note above */
#if !defined(LINUX_HYBRID) || defined(LINUX_PORT)
#ifdef USER_MODE
#include <sys/types.h>
#else
#include <linux/types.h>	/* sys/types.h and linux/types.h are oil and water */
#endif /* USER_MODE */
#endif /* !defined(LINUX_HYBRID) || defined(LINUX_PORT) */

#else

#if defined(__ECOS)
#include <pkgconf/infra.h>
#include <cyg/infra/cyg_type.h>
#include <stdarg.h>
#endif

#include <sys/types.h>

#endif /* linux && __KERNEL__ */

#endif /* !_WIN32 && !PMON && !_CFE_ && !_HNDRTE_  && !_MINOSL_ && !__DJGPP__ */

#if defined(MACOSX)

#ifdef __BIG_ENDIAN__
#define IL_BIGENDIAN
#else
#ifdef IL_BIGENDIAN
#error "IL_BIGENDIAN was defined for a little-endian compile"
#endif
#endif /* __BIG_ENDIAN__ */

#if !defined(__cplusplus)

#if defined (__i386__)
typedef unsigned char bool;
#else
typedef unsigned int bool;
#endif
#define TYPE_BOOL 1
enum {
    false	= 0,
    true	= 1
};

#if defined(KERNEL)
#include <IOKit/IOTypes.h>
#endif /* KERNEL */

#endif /* __cplusplus */

#endif /* MACOSX */

#if defined(vxworks)
#include <private/cplusLibP.h>
#endif

/* use the default typedefs in the next section of this file */
#define USE_TYPEDEF_DEFAULTS

#endif /* SITE_TYPEDEFS */


/*
 * Default Typedefs
 */

#ifdef USE_TYPEDEF_DEFAULTS
#undef USE_TYPEDEF_DEFAULTS

#ifndef TYPEDEF_BOOL
typedef	/* @abstract@ */ unsigned char	bool;
#endif

/* define uchar, ushort, uint, ulong */

#ifndef TYPEDEF_UCHAR
typedef unsigned char	uchar;
#endif

#ifndef TYPEDEF_USHORT
typedef unsigned short	ushort;
#endif

#ifndef TYPEDEF_UINT
typedef unsigned int	uint;
#endif

#ifndef TYPEDEF_ULONG
typedef unsigned long	ulong;
#endif

/* define [u]int8/16/32/64, uintptr */

#ifndef TYPEDEF_UINT8
typedef unsigned char	uint8;
#endif

#ifndef TYPEDEF_UINT16
typedef unsigned short	uint16;
#endif

#ifndef TYPEDEF_UINT32
typedef unsigned int	uint32;
#endif

#ifndef TYPEDEF_UINT64
typedef unsigned long long uint64;
#endif

#ifndef TYPEDEF_UINTPTR
typedef unsigned int	uintptr;
#endif

#ifndef TYPEDEF_INT8
typedef signed char	int8;
#endif

#ifndef TYPEDEF_INT16
typedef signed short	int16;
#endif

#ifndef TYPEDEF_INT32
typedef signed int	int32;
#endif

#ifndef TYPEDEF_INT64
typedef signed long long int64;
#endif

/* define float32/64, float_t */

#ifndef TYPEDEF_FLOAT32
typedef float		float32;
#endif

#ifndef TYPEDEF_FLOAT64
typedef double		float64;
#endif

/*
 * abstracted floating point type allows for compile time selection of
 * single or double precision arithmetic.  Compiling with -DFLOAT32
 * selects single precision; the default is double precision.
 */

#ifdef MACOSX
/* float_t types conflict with the same typedefs from the standard ANSI-C
** math.h header file. Don't re-typedef them here.
*/
#define TYPEDEF_FLOAT_T
#endif /* MACOSX */

#ifndef TYPEDEF_FLOAT_T

#if defined(FLOAT32)
typedef float32 float_t;
#else /* default to double precision floating point */
typedef float64 float_t;
#endif

#endif /* TYPEDEF_FLOAT_T */

/* define macro values */

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1  /* TRUE */
#endif

#ifndef NULL
#define	NULL	0
#endif

#ifndef OFF
#define	OFF	0
#endif

#ifndef ON
#define	ON	1  /* ON = 1 */
#endif

#define	AUTO	(-1) /* Auto = -1 */

/* define PTRSZ, INLINE */

#ifndef PTRSZ
#define	PTRSZ	sizeof(char*)
#endif


/* Detect compiler type. */
#ifdef _MSC_VER
	#define BWL_COMPILER_MICROSOFT
#elif defined(__GNUC__) || defined(__lint)
	#define BWL_COMPILER_GNU
#elif defined(__CC_ARM) && __CC_ARM
	#define BWL_COMPILER_ARMCC
#else
	#error "Unknown compiler!"
#endif /* _MSC_VER */


#ifndef INLINE
	#if defined(BWL_COMPILER_MICROSOFT)
		#define INLINE __inline
	#elif defined(BWL_COMPILER_GNU)
		#define INLINE __inline__ __attribute__((always_inline))
	#elif defined(BWL_COMPILER_ARMCC)
		#define INLINE	__inline
	#else
		#define INLINE
	#endif /* _MSC_VER */
#endif /* INLINE */

#undef TYPEDEF_BOOL
#undef TYPEDEF_UCHAR
#undef TYPEDEF_USHORT
#undef TYPEDEF_UINT
#undef TYPEDEF_ULONG
#undef TYPEDEF_UINT8
#undef TYPEDEF_UINT16
#undef TYPEDEF_UINT32
#undef TYPEDEF_UINT64
#undef TYPEDEF_UINTPTR
#undef TYPEDEF_INT8
#undef TYPEDEF_INT16
#undef TYPEDEF_INT32
#undef TYPEDEF_INT64
#undef TYPEDEF_FLOAT32
#undef TYPEDEF_FLOAT64
#undef TYPEDEF_FLOAT_T

#endif /* USE_TYPEDEF_DEFAULTS */

/* Suppress unused parameter warning */
#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

/* Avoid warning for discarded const or volatile qualifier in special cases (-Wcast-qual) */
#define DISCARD_QUAL(ptr, type) ((type *)(uintptr)(ptr))

#endif /* _TYPEDEFS_H_ */
