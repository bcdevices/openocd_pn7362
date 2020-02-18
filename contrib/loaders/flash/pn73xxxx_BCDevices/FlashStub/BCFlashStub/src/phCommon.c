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

/** \file phCommon.c
 * Common tools and utils.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18102 $
 * $Date: 2016-03-07 18:44:42 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 *
 */

/*********************************************************************************************************************/
/*   INCLUDES                                                                                                        */
/*********************************************************************************************************************/
#include "ph_Config.h"
#include "ph_Datatypes.h"
#include "ph_Status.h"
#include "phCommon.h"
#include "phRomHal_Eeprom.h"

#include <string.h>

/*********************************************************************************************************************/
/*   LOCAL DEFINES                                                                                                   */
/*********************************************************************************************************************/
#define STACK_SIZE 1000U
#define MAX_LOG_STRING_LENGTH 500U

/*********************************************************************************************************************/
/*   LOCAL DATATYPES                                                                                                 */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   LOCAL VARIABLES                                                                                                 */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   LOCAL FUNCTION PROTOTYPES                                                                                       */
/*********************************************************************************************************************/


/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS                                                                                                */
/*********************************************************************************************************************/

//Code Review Status: Closed
void phCommon_WaitInit(phCommon_CpuSpeed_t eCpuSpeed)
{
    /* Nothing to do */
}

#if PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_ARMGCC
#    define VOLATILE_LOOP_COUNTER volatile
#elif  PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_IAR
#    define VOLATILE_LOOP_COUNTER volatile
#elif  PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_DS5
#    define VOLATILE_LOOP_COUNTER /* Not needed */
#else
#    eror Unsupported platform
#endif


void phCommon_Wait(uint32_t dwUSec)
{
    VOLATILE_LOOP_COUNTER uint32_t dwUSec_vol = dwUSec;

#if PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_ARMGCC
    dwUSec_vol >>= 2; /* Compensate based on Instruction */
#elif  PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_IAR
    dwUSec_vol >>= 2; /* Compensate based on Instruction */
#elif  PHFL_CONFIG_TARGET_PLATFORM == PH_CONFIG_TARGET_PLATFORM_DS5
    dwUSec_vol >>= 1; /* Compensate based on Instruction */
#else
    /* No compensation needed */
#endif

    while(dwUSec_vol)
    {
        /* to be qmore compliant, to the subtract not in the loop header */
        dwUSec_vol--;
    }
}

void phCommon_Memcpy(void* dst, void* src, uint32_t length)
{
	if((NULL != dst) && (NULL != src) && ( 0 != length) )
	{
		/* link against standard ARM libraries. */
		(void)memcpy(dst,src,length);
	}
	return;
}

uint32_t phCommon_Memcmp(void* buf, void* cmp_buf, uint32_t length)
{
	/* link against standard ARM libraries. */
	return memcmp(buf,cmp_buf,length);
}

void phCommon_Memset(void* buf, uint32_t val , uint32_t length)
{
	if((NULL != buf) && ( 0 != length) )
	{
		/* link against standard ARM libraries. */
		(void)memset(buf,val,length);
	}
	return;
}
