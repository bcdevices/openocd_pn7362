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
 * Default global configuration. Do NOT modify this file. ph_Config.h shall be modified instead.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18102 $
 * $Date: 2016-03-07 18:44:42 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 *
 */

#ifndef PH_DEFAULTCONFIG_H
#define PH_DEFAULTCONFIG_H

#ifndef PHFL_CONFIG_SYSCLK
#   define PHFL_CONFIG_SYSCLK 20000000UL
#endif /* PHFL_CONFIG_SYSCLK */

/** Compiling code for KEIL/AMRCC/DS5  */
#define PH_CONFIG_TARGET_PLATFORM_DS5    1
/** Compiling code for gnu-arm-gcc / LPCXpresso */
#define PH_CONFIG_TARGET_PLATFORM_ARMGCC 3
/** Compiling code for IAR */
#define PH_CONFIG_TARGET_PLATFORM_IAR    4

#ifndef PHFL_CONFIG_TARGET_PLATFORM
#   ifdef __ARMCC_VERSION
#       define PHFL_CONFIG_TARGET_PLATFORM PH_CONFIG_TARGET_PLATFORM_DS5
#   endif
#endif /* PHFL_CONFIG_TARGET_PLATFORM  */
#ifndef PHFL_CONFIG_TARGET_PLATFORM
#   ifdef __GNUC__
#       define PHFL_CONFIG_TARGET_PLATFORM PH_CONFIG_TARGET_PLATFORM_ARMGCC
#   endif
#endif /* PHFL_CONFIG_TARGET_PLATFORM  */
#ifndef PHFL_CONFIG_TARGET_PLATFORM
#   ifdef __ICCARM__
#       define PHFL_CONFIG_TARGET_PLATFORM PH_CONFIG_TARGET_PLATFORM_IAR
#   endif
#endif
#ifndef PHFL_CONFIG_TARGET_PLATFORM
#   error Why here?  Platform should have been identified by now.
#endif

#define PH_RAM_START_ADDRESS         (0x0100000UL)

#define PH_RAM_END_ADDRESS           (0x0103000UL)

#define PH_RAM_USER_OFFSET           (0x0000020UL)


#endif /* PH_DEFAULTCONFIG_H */
