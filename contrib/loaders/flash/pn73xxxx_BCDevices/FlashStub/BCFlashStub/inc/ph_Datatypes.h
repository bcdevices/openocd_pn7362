/*
 *                  Copyright (c), NXP Semiconductors
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */
/** \file
 * Global data types.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18115 $
 * $Date: 2016-03-08 16:17:25 +0530 (Tue, 08 Mar 2016) $
 *
 * History:
 *
 */

#ifndef PH_DATATYPES_H
#define PH_DATATYPES_H


#if defined(PHFL_CONFIG_PLATFORM_INCLUDE) && PHFL_CONFIG_PLATFORM_INCLUDE != 0
#   include <ph_Platform.h>
#endif

/** \defgroup phDatatypes Global data types
 * \ingroup types
 * \brief Contains definitions for portable data types used within this firmware.
 *
 * The data types defined here have to be used instead of standard C data types (e.g. int, char)
 * to allow easier portability of the firmware.
 * @{
 */

#include <ph_Config.h>

#if (PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_ARMGCC) \
	|| ( PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_IAR)
#include <stdint.h>
#else

#ifndef __uint8_t_defined
#define __uint8_t_defined
/**
* \brief 8 bit unsigned integer
*/
typedef unsigned char uint8_t;
#endif

#ifndef __uint16_t_defined
#define __uint16_t_defined
/**
* \brief 16 bit unsigned integer
*/
typedef unsigned short uint16_t;
#endif

#ifndef __uint32_t_defined
#define __uint32_t_defined
/**
* \brief 32 bit unsigned integer
*/
typedef unsigned long uint32_t;
#endif

#ifndef __int8_t_defined
#define __int8_t_defined
/**
* \brief 8 bit signed integer
*/
typedef signed char int8_t;
#endif

#ifndef __int16_t_defined
#define __int16_t_defined
/**
* \brief 16 bit signed integer
*/
typedef signed short int16_t;
#endif

#ifndef __int32_t_defined
#define __int32_t_defined
/**
* \brief 32 bit signed integer
*/
typedef signed int int32_t;
#endif
#endif

#if !defined(__cplusplus) || defined(__arm__)

#ifndef __BOOL_DEFINED
#define __BOOL_DEFINED 1

/**
* \brief Boolean type
*/
#ifndef false
#define false    0
#endif
#ifndef true
#define true     (!false)
#endif

typedef uint32_t bool;

#endif /* __BOOL_DEFINED */

#endif /* !defined(__cplusplus) || defined(__arm__) */

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifndef NULL
#define NULL ((void*)0)  /**< Value used for NULL pointers */
#endif

#ifndef BYTE_SIZE
#define BYTE_SIZE 0x08U
#endif

/**
 * \brief Declaration of interrupt service routines.
 * This macro should be used for declaration of interrupt service routines. Example:
 * \code
PH_INTERRUPT(phSwp_Isr)
{
   ...
}
\endcode
 */
#define PH_INTERRUPT(ISR) void (ISR)(void)

#define STRINGIFY(a) #a

#if (PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_DS5)
#    define PH_ALIGN(X)           __align(X)
#    define PH_NOINIT             __attribute__ ((section (".noinit")))
#    define PH_NOINIT_ALIGNED     __attribute__ ((aligned (4),section(".noinit")))
#    define PH_PACK_STRUCT_BEGIN  __packed
#    define PH_PACK_STRUCT_END
#    define PH_PLACE_FUNCTION_TO_SECTION(SECTIONNAME) \
                                  __attribute__ ((section (SECTIONNAME)))
#    define PH_UNUSED             __attribute__((unused))
#    define PH_USED               __attribute__((used))
#    define PH_WEAK               __attribute__((weak))
#elif (PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_ARMGCC)
#    define PH_ALIGN(a)           __attribute__ ((aligned (a)))
#    define PH_NOINIT             __attribute__ ((section (".noinit")))
#    define PH_NOINIT_ALIGNED     __attribute__ ((aligned (4),section(".noinit")))
#    define PH_PACK_STRUCT_BEGIN
#    define PH_PACK_STRUCT_END    __attribute__ (( packed ))
#    define PH_PLACE_FUNCTION_TO_SECTION(SECTIONNAME)                  \
                                  __attribute__ ((section (SECTIONNAME)))
#    define PH_UNUSED             __attribute__((unused))
#    define PH_USED               __attribute__((used))
#    define PH_WEAK               __attribute__((weak))
#elif (PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_IAR)
#    define PH_ALIGN(X)           _Pragma(STRINGIFY(data_alignment = X))
#    define PH_NOINIT             __no_init
#    define PH_NOINIT_ALIGNED     _Pragma(STRINGIFY(data_alignment = 4)) __no_init
#    define PH_PLACE_FUNCTION_TO_SECTION(SECTION) \
                                  _Pragma(STRINGIFY(location = # SECTION))
#    define PH_PACK_STRUCT_BEGIN  __packed
#    define PH_PACK_STRUCT_END
#    define PH_UNUSED             /* Not used API parameters */
#    define PH_USED               /* TBU */
#    define PH_WEAK               __weak
#else /* PHFL_CONFIG_TARGET_PLATFORM */
#    error "Platform/configuration not supported"
#endif /* PHFL_CONFIG_TARGET_PLATFORM */


#define UNUSED(PARAM) (PARAM) = (PARAM)

/**
 * @}
 */

#endif /* PH_DATATYPES_H */
