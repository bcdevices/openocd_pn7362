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
 * Prototype of EEPROM Controller interface HAL for PageFlash Access.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18102 $
 * $Date: 2016-03-07 18:44:42 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 * V0.1 | 2013-08-07 | Created initial Draft
 */

#ifndef PHROMHAL_FLASH_H
#define PHROMHAL_FLASH_H

/** \defgroup phRomHalFlash FLASH ROM HAL
 * \ingroup romhal
 * \brief Prototype of the FLASH ROM HAL
 *
 * This module implements FLASH interface ROM HAL.
 * This module is not designed for multi user/process environment.
 * It is possible to have only one write or read of PAGEFLASH at any moment of time.
 * FLASH HAL exposes the PAGEFLASH as a single Flash memory, not as separate Flash 0, Flash 1 memories.
 * @{
 */

/*********************************************************************************************************************/
/*   INCLUDES                                                                                                        */
/*********************************************************************************************************************/
#include <ph_Datatypes.h>
#include <ph_Status.h>

/*********************************************************************************************************************/
/*   GLOBAL DEFINES                                                                                                  */
/*********************************************************************************************************************/
/* PAGE FLASH PROPERTIES */
#define PH_ROMHAL_FLASH_START_ADDRESS  0x00203000
#define PH_ROMHAL_FLASH_END_ADDRESS    (PH_ROMHAL_FLASH_START_ADDRESS + (158*1024) -1)
#define PH_ROMHAL_FLASH_SIZE    (PH_ROMHAL_FLASH_END_ADDRESS - PH_ROMHAL_FLASH_START_ADDRESS + 1) /* Total Size of the PAGEFLASH = 160 KBytes */
#define PH_ROMHAL_FLASH_PAGE_SIZE          128 /* FLASH is organized as 1280 pages x 128 bytes = 160KBytes */
#define PH_ROMHAL_FLASH_PAGE_COUNT         1280

/* If it is above User Accessible and the physical end address 
 * Even if it is out of the varaint, let it crash. IDE should have
 * never called this API with wrong address. */
#define WITHIN_PAGEFLASH(ADD) 						\
	((ADD) >= PH_ROMHAL_FLASH_START_ADDRESS			\
		  &&										\
	   (ADD) <= PH_ROMHAL_FLASH_END_ADDRESS )


#define FIND_128BYTE_START_BOUNDARY(x) ((x) & (~(128UL - 1)))
#define IS_NOT_4BYTE_ALIGNED(x) ((x) & 0x3)
#define IS_EVEN(x) (!((x) & 0x1))
#define TOTAL_NUM_FLASH 2
#define PF_PROG_PREPOLL_DELAY 10 /*Time in us to wait before polling for Page flash Program busy bit.*/
/*********************************************************************************************************************/
/*    GLOBAL FUNCTION PROTOTYPES                                                                                     */
/*********************************************************************************************************************/
/**
 * This function writes data to PAGEFLASH.
 * Use this API, when the length of data to be written is less than or greater than 1 page(128 bytes).
 * Using this API for Page writes will work, but will not be as optimized as phRomHal_Flash_Write_Page().
 * If HV (High Voltage) error is detected, an error code will be returned.
 *
 * \note Given address must be 4byte aligned.
 * \note Size of data to be written must be multiple of 4. Cannot be zero.
 * \note The Write is synchronous i.e. The API will return only after the write operation completes.
 * \note Pre-condition: HV Trim values must have been set correctly prior to using the Write API.(Currently done in boot code)
 * \note Pre-condition: phCommon_WaitInit() must have been called at least once prior to using the write API.(Currently done in boot code)
 * \post PAGEFLASH data written (if OK).
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if PAGEFLASH Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Flash_WriteBuffer(
            uint8_t *pBuffer,     /**< [in] pointer to buffer containing data to be written to PAGEFLASH */
            uint8_t *pFlash,      /**< [in] Valid address of a byte in the PAGEFLASH. Given address must be 4byte aligned. */
            uint32_t wData_Length /**< [in] number of bytes to be written. Must be Non zero, multiple of 4. */
            );

/**
 * This function writes a given 32 bit Pattern to a specified region in the PAGEFLASH.
 * If HV (High Voltage) error is detected, an error code will be returned.
 * \note Given address must be 4byte aligned.
 * \note Size of data to be written must be multiple of 4. Cannot be zero.
 * \note The Write is synchronous i.e. The API will return only after the write operation completes.
 * \note Pre-condition: HV Trim values must have been set correctly prior to using the Write API.(Currently done in boot code)
 * \note Pre-condition: phCommon_WaitInit() must have been called at least once prior to using the write API.(Currently done in boot code)
 * \post PAGEFLASH data written (if OK).
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if PAGEFLASH write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Flash_FillRegion(
            uint8_t *pFlash,       /**< [in] 4byte aligned address in PageFlash starting from which, the pattern will be filled */
            uint32_t wData_Length, /**< [in] number of bytes to be written. Must be Non zero, multiple of 4. */
            uint32_t pattern_32bit /**< [in] A 32 bit pattern that will be written to the specified region in the PAGEFLASH */
            );

/**
 * This function writes one double word (32 bit) to the FLASH memory.
 * \note Given address must be 4byte aligned.
 * \note This function internally calls phRomHal_Flash_WriteBuffer() (See description of this API)
 * \post FLASH data written (if OK)
 * \return Status code.
 * \retval #PH_STATUS_SUCCESS if PAGEFLASH write completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Flash_WriteDoubleWord(
            uint32_t dwWord,        /**< [in] DOUBLE word to be written */
            uint8_t *pFlash         /**< [in] pointer to FLASH location where data will be written. */
            );

/**
 * This function writes a single full 128byte PAGEFLASH page.
 * PAGEFLASH pages are numbered from 0 to 1279.
 * If HV (High Voltage) error is detected, an error code will be returned.
 * \note It is assumed that if ECC is bypassed for PAGEFLASH write, it will also be bypassed for PAGEFLASH read.
 * This API will not write External input parity bits(EDI parity lines) for PAGEFLASH.
 * \note The Write is synchronous i.e. The API will return only after the write operation completes.
 * \note In case previous write operation is pending, this API will wait for it's completion and then start the new write operation.
 * \note Pre-condition: HV Trim values must have been set correctly prior to using the Write API.(Currently done in boot code)
 * \note Pre-condition: phCommon_WaitInit() must have been called at least once prior to using the write API.(Currently done in boot code)
 * \post PAGEFLASH data page written (if OK).
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if PAGEFLASH Read Operation completed successfully.
 * \retval #PH_STATUS_PARAMETER_ERROR if parameters passed are invalid.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Flash_Write_Page(
            uint32_t page_number, /**< [in] Valid index of a 128byte page in the PAGEFLASH */
            uint8_t *pBuffer     /**< [in] pointer to buffer containing data to be written to PAGEFLASH */
            );

/**
 * This function resets the PageFlash controller hardware.
 * \return None.
 */
extern void phRomHal_Flash_HwReset(void);



/** @} */
#endif /* PHROMHAL_FLASH_H */
