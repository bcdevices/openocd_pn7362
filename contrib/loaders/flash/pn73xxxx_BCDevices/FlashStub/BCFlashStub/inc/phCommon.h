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
/** \file phCommon.h
 * Common tools and utils.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18101 $
 * $Date: 2016-03-07 18:20:50 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 *
 */

#ifndef PHCOMMON_H
#define PHCOMMON_H

/*********************************************************************************************************************/
/*   INCLUDES                                                                                                        */
/*********************************************************************************************************************/
#include <ph_Datatypes.h>
#include <ph_Status.h>

/*********************************************************************************************************************/
/*   GLOBAL DATATYPES                                                                                                 */
/*********************************************************************************************************************/
/** \name CPU speed
 * @{
 */
typedef enum
{
  E_COMMON_CPUSPEED_20MHZ = 0,
  E_COMMON_CPUSPEED_10MHZ = 1,
  E_COMMON_CPUSPEED_5MHZ  = 2
} phCommon_CpuSpeed_t;
/**
 * @}
 */


/*********************************************************************************************************************/
/*   GLOBAL FUNCTION PROTOTYPES                                                                                      */
/*********************************************************************************************************************/

/**
 * Optimized memcpy for cortex M0.
 * @param dst Destination Address
 * @param src Source Address
 * @param length Number of bytes to copy.
 */
void phCommon_Memcpy(void* dst, void* src, uint32_t length);
uint32_t phCommon_Memcmp(void* buf, void* cmp_buf, uint32_t length);
void phCommon_Memset(void* buf, uint32_t val , uint32_t length);

void phCommon_WaitInit(phCommon_CpuSpeed_t dwCpuSpeed);
void phCommon_Wait(uint32_t dwUSec);


#endif /* PHCOMMON_H */
