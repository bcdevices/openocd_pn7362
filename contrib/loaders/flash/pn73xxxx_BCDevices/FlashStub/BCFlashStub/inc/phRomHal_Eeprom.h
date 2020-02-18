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
 * Prototype of EEPROM Controller interface HAL.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18102 $
 * $Date: 2016-03-07 18:44:42 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 * V0.1 | 2013-08-07 | Created initial Draft
 */

#ifndef PHROMHAL_EEPROM_H
#define PHROMHAL_EEPROM_H

/** \defgroup phRomHal_Eeprom EEPROM ROM HAL
 * \ingroup romhal
 * \brief Prototype of the EECTRL ROM HAL for EEPROM
 *
 * This module implements EEPROM Controller interface ROM HAL, specific to EEPROM.
 * This module is not designed for multi user/process environment.
 * It is impossible to have more than one write or read of EEPROM at any moment of time.
 * @{
 */

/*********************************************************************************************************************/
/*   INCLUDES                                                                                                        */
/*********************************************************************************************************************/
#include <ph_Datatypes.h>
#include <ph_Status.h>
#include <phCommon.h>
/*********************************************************************************************************************/
/*   GLOBAL DEFINES                                                                                                  */
/*********************************************************************************************************************/
/* EEPROM PROPERTIES */
#define PH_ROMHAL_EEPROM_MEM_START_ADDRESS     0x00201000UL
#define PH_ROMHAL_EEPROM_USER_START_ADDRESS    (PH_ROMHAL_EEPROM_MEM_START_ADDRESS + 512)
#define PH_ROMHAL_EEPROM_SECURITY_ROW_ADDRESS  0x00201000UL /* Address of security row in EEPROM */
#define PH_ROMHAL_EEPROM_DATA_START_ADDRESS    0x00201004UL
#define PH_ROMHAL_EEPROM_DATA_END_ADDRESS      0x00201FFFUL

#define PH_ROMHAL_EEPROM_DATA_SIZE             (PH_ROMHAL_EEPROM_DATA_END_ADDRESS - PH_ROMHAL_EEPROM_DATA_START_ADDRESS + 1)
#define PH_ROMHAL_EEPROM_SIZE                  (PH_ROMHAL_EEPROM_DATA_END_ADDRESS - PH_ROMHAL_EEPROM_MEM_START_ADDRESS + 1)
#define PH_ROMHAL_EEPROM_PAGE_SIZE             64UL /* EEPROM is organized as 64 pages x 64 bytes = 4096 bytes */
#define PH_ROMHAL_EEPROM_PAGE_COUNT            64
#define PH_ROMHAL_EEPROM_START_PAGE_NUMBER     0


/* If it is not in user acessible user eeprom region, let it crash.
 * Even if it is out of the user accessible region, 
 * let it crash. IDE should have
 * never called this API with wrong address. */
#define WITHIN_EEPROM(ADD) 								\
	(	(ADD) >= PH_ROMHAL_EEPROM_MEM_START_ADDRESS	    \
		&&												\
		(ADD) <= PH_ROMHAL_EEPROM_DATA_END_ADDRESS		\
		)


#define FIND_NEXT_64BYTE_BOUNDARY(x) ((((x) & (~(64UL - 1))) + 64UL) - 1)
#define IS_NOT_2BYTE_ALIGNED(x) ((x) & 0x1)
#define EE_PROG_PREPOLL_DELAY 10 /*Time in us to wait before polling for EEPROM Program busy bit. */
/*********************************************************************************************************************/
/*    GLOBAL FUNCTION PROTOTYPES                                                                                     */
/*********************************************************************************************************************/
/**
 * This function writes a single full 64byte EEPROM page.
 * EEPROM pages are numbered from 0 to 63. Since Page 0 of EEPROM will contain the 4 byte SECROW at the beginning of the page, only 60 bytes from the 4th byte
 * in the given buffer will be written to page 0. For the remaining pages, the first 64 bytes in the buffer will be written.
 * If HV (High Voltage) error is detected, an error code will be returned.
 * \note Parameter checking will not be done by this API.
 * \note It is assumed that if ECC is bypassed for EEPROM write, it will also be bypassed for EEPROM read.
 * \note This API will not write External input parity bits(EDI parity lines) for EEPROM.
 * \note The Write is snchronous i.e. The API will return only after the write operation completes.
 * \note Incase previous write operation is pending, this API will wait for it's completion and then start the new write operation.
 * \note Pre-condition: HV Trim values must have been set correctly prior to using the Write API.(Currently done in boot code)
 * \note Pre-condition: phCommon_WaitInit() must have been called at least once prior to using the write API.(Currently done in boot code)
 * \post EEPROM data page written (if OK).
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if EEPROM Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Eeprom_Write_Page(
            uint8_t page_number, /**< [in] Valid index of a 64byte page in the EEPROM */
            uint8_t *pBuffer     /**< [in] pointer to buffer containing data to be written to EEPROM */
            );

/**
 * This function writes data to EEPROM.
 * Use this API, when the length of data to be written is less than or greater than 1 page(64 bytes).
 * If HV (High Voltage) error is detected, an error code will be returned.
 * \note Parameter checking will not be done by this API.
 * \note The Write is synchronous i.e. The API will return only after the write operation completes.
 * \note It is assumed that if ECC is bypassed for EEPROM write, it will also be bypassed for EEPROM read.
 * This API will not write External input parity bits(EDI parity lines) for EEPROM.
 * \note Pre-condition: HV Trim values must have been set correctly prior to using the Write API.(Currently done in boot code)
 * \note Pre-condition: phCommon_WaitInit() must have been called at least once prior to using the write API.(Currently done in boot code)
 * \post EEPROM data written (if OK).
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if EEPROM Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Eeprom_WriteBuffer(
            uint8_t *pBuffer,         /**< [in] pointer to buffer to be copied - can be in RAM or EEPROM */
            uint8_t *pEeprom,         /**< [in] pointer to EEPROM buffer where data will be copied */
            uint16_t wData_Length     /**< [in] number of bytes to be written */
            );

/**
 * This function write one byte to the EEPROM memory.
 * \note This function internally calls phRomHal_Eeprom_WriteBuffer (See description of this API)
 * \post EEPROM data written if OK, Assertion if error occured.
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if EEPROM Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Eeprom_WriteByte(
            uint8_t bByte,           /**< [in]  byte to be written */
            uint8_t *pEeprom         /**< [in] pointer to EEPROM where data will be copied */
            );

/**
 * This function write one word (16 bit) to the EEPROM memory.
 * \note This function internally calls phRomHal_Eeprom_WriteBuffer (See description of this API)
 * \post EEPROM data written if OK, Assertion if error occured.
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if EEPROM Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Eeprom_WriteWord(
            uint16_t wWord,          /**< [in]  word to be written */
            uint8_t *pEeprom         /**< [in] pointer to EEPROM where data will be copied */
            );

/**
 * This function write one double word (32 bit) to the EEPROM memory.
 * \note This function internally calls phRomHal_Eeprom_WriteBuffer (See description of this API)
 * \post EEPROM data written if OK, Assertion if error occured.
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if EEPROM Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Eeprom_WriteDoubleWord(
            uint32_t dwDoubleWord,   /**< [in]  double word to be written */
            uint8_t *pEeprom         /**< [in] pointer to EEPROM where data will be copied */
            );

/**
 * This function fills a specified region in the EEPROM with a given 16 bit pattern .
 * phRomHal_Eeprom_WriteBuffer() API can be used to achieve the same result, but a user buffer would have to be allocated
 * and filled with the data pattern.
 * If HV (High Voltage) error is detected, an error code will be returned.
 * \note Parameter checking will not be done by this API.
 * \note The Write is synchronous i.e. The API will return only after the write operation completes.
 * This API will not write External input parity bits(EDI parity lines) for EEPROM.
 * \note Pre-condition: HV Trim values must have been set correctly prior to using the Write API.(Currently done in boot code)
 * \note Pre-condition: phCommon_WaitInit() must have been called at least once prior to using the write API.(Currently done in boot code)
 * \post EEPROM data written (if OK).
 * \return Status code of type phStatus_t.
 * \retval #PH_STATUS_SUCCESS if EEPROM Write Operation completed successfully.
 * \retval #PH_STATUS_MEMORY_ERROR if High Voltage error is detected.
 */
extern phStatus_t phRomHal_Eeprom_FillRegion(
            uint8_t *pEeprom,      /**< [in] 2byte aligned address in EEPROM starting from which, the pattern will be filled. */
            uint16_t wData_Length, /**< [in] number of bytes to be written. Must be multiple of 2. */
            uint16_t pattern_16bit /**< [in] A 16 bit pattern that will be written to the specified region in the EEPROM */
            );

/**
 * This Macro reads one byte of EEPROM data without any parameter checking or ECC error reporting.
 * \note Upper Software layer must take care of validating the read data, if desired.
 * \note Input parameter is the address to be read in EEPROM and is not checked for validity.
 * \post EEPROM data read.
 * \return Unsigned char
 * \retval Byte read from the EEPROM.
 */
#define phRomHal_Eeprom_ReadByte(pEeprom) ((uint8_t)(*((uint8_t*)(pEeprom))))

/**
 * This Macro reads one word (16 bit) from the EEPROM memory without any parameter checking or ECC error reporting.
 * \note Upper Software layer must take care of validating the read data, if desired.
 * \note Input parameter is the address to be read in EEPROM and is not checked for validity.
 * \post EEPROM data read.
 * \return Unsigned short integer
 * \retval 16bit word read from the EEPROM.
 */
#define phRomHal_Eeprom_ReadWord(pEeprom) ((uint16_t)(((uint8_t *)(pEeprom))[0] | (((uint16_t)(((uint8_t *)(pEeprom))[1]) << 8) & 0xff00)))

/**
 * This Macro resets the EEPROM controller hardware.
 * \return None.
 */
#define phRomHal_Eeprom_HwReset()    PH_HALREG_SETBITW(EE_DYN,EE_DYN_EE_RST_DAT_POS)


/** @} */

#endif /* PHROMHAL_EEPROM_H */
