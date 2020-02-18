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
 * EEPROM Controller HAL implementation.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18101 $
 * $Date: 2016-03-07 18:20:50 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 *
 */

/*********************************************************************************************************************/
/*   INCLUDES                                                                                                        */
/*********************************************************************************************************************/
#include <phCommon.h>
#include <phRomHal_Eeprom.h>
#include <ph_Datatypes.h>
#include <ph_Status.h>
#include <id_nfc_pn640_apb_if_reg.h>
#include <ph_Registers.h>

/*********************************************************************************************************************/
/*   MACRO DEFINITIONS                                                                                                 */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   GLOBAL VARIABLES                                                                                                 */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   LOCAL FUNCTIONS                                                                                                 */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS                                                                                                */
/*********************************************************************************************************************/


/** \brief This function writes a single full 64byte EEPROM page.
*/
phStatus_t phRomHal_Eeprom_Write_Page(
            uint8_t page_number, /**< [in] Valid index of a 64byte page in the EEPROM */
            uint8_t *pBuffer)    /**< [in] pointer to buffer containing data to be written to EEPROM */
{
    /* Local variables */
    uint16_t bytes_remaining;
    uint32_t addr;

    /** Do not Validate input parameters in EEPROM HAL in the ROM code.*/

    /** Write data to page register. */
    /** Calculate the address to be written to, in EEPROM. */
    addr = PH_ROMHAL_EEPROM_MEM_START_ADDRESS + (page_number * PH_ROMHAL_EEPROM_PAGE_SIZE);

    /** If the first page in EEPROM is being written, the 60 bytes starting from the 4th byte only must be written. */
    if(page_number == PH_ROMHAL_EEPROM_START_PAGE_NUMBER)
    {
        addr += 4;
        pBuffer += 4;
        bytes_remaining = PH_ROMHAL_EEPROM_PAGE_SIZE - 4;
    }
    else
        bytes_remaining = PH_ROMHAL_EEPROM_PAGE_SIZE;

    /** if any Program sequence is pending, poll till it completes. */
    while(PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_PROG_DAT_POS))
    {}

    /** Write the data in the buffer to the Page register*/
    /** Write will be done using 16bit(2byte) access for lesser AHB clock cycle count.*/
    while(bytes_remaining)
    {
        *((volatile uint16_t *)addr) = *((uint16_t *)pBuffer);
        pBuffer += 2;
        addr += 2;
        bytes_remaining -= 2;
    }

    /** Initiate Program sequence for the EEPROM Page*/
    PH_HALREG_SETBITW(EE_DYN,EE_DYN_PROG_DAT_POS);

    /** It takes minimum 200ns for Program command to be generated. Wait for 10us.
        The minimum time for a Page write is anyway 2.9 ms. So the fixed delay of 10us will not add additional delays. */
    phCommon_Wait(EE_PROG_PREPOLL_DELAY);

    /** if the Program sequence is pending, poll till it completes. */
    while(PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_PROG_DAT_POS))
    {}

    /** Check if HV Error occured. */
    if (PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_HVERR_DAT_POS))
        return PH_STATUS_MEMORY_ERROR;

    /** Return Success status code on completion. */
    return PH_STATUS_SUCCESS;
}
/*********************************************************************************************************************/

/** \brief This function writes one byte to the EEPROM memory.
*/
phStatus_t phRomHal_Eeprom_WriteByte(
            uint8_t bByte,           /**< [in]  byte to be written */
            uint8_t *pEeprom)        /**< [in] pointer to EEPROM where data will be copied */
{
    return phRomHal_Eeprom_WriteBuffer((uint8_t *)(&bByte),pEeprom,(uint16_t)(sizeof(uint8_t)));
}
/*********************************************************************************************************************/

/** \brief This function write one word (16 bit) to the EEPROM memory.
*/
phStatus_t phRomHal_Eeprom_WriteWord(
            uint16_t wWord,          /**< [in]  word to be written */
            uint8_t *pEeprom)        /**< [in] pointer to EEPROM where data will be copied */
{
    return phRomHal_Eeprom_WriteBuffer((uint8_t *)(&wWord),pEeprom,(uint16_t)(sizeof(uint16_t)));
}
/*********************************************************************************************************************/

/** \brief This function write one double word (32 bit) to the EEPROM memory.
*/
phStatus_t phRomHal_Eeprom_WriteDoubleWord(
            uint32_t dwDoubleWord,   /**< [in]  double word to be written */
            uint8_t *pEeprom)        /**< [in] pointer to EEPROM where data will be copied */
{
    return phRomHal_Eeprom_WriteBuffer((uint8_t *)(&dwDoubleWord),pEeprom,(uint16_t)(sizeof(uint32_t)));
}
/*********************************************************************************************************************/

/** \brief This function writes a given pattern to a specified region in the EEPROM.
*/
phStatus_t phRomHal_Eeprom_FillRegion(
        uint8_t *pEeprom,       /**< [in] 2byte aligned address in EEPROM starting from which, the pattern will be filled. */
        uint16_t bytes_to_fill, /**< [in] number of bytes to be written. Must be multiple of 2. */
        uint16_t pattern_16bit) /**< [in] A 16 bit pattern that will be written to the specified region in the EEPROM */
{
    /* Local variables */
    uint32_t next_block64_end;
    bool hv_error_occured = false;

    /** Do not Validate input parameters in EEPROM HAL in the ROM code.*/

    /** if any Program sequence is pending, poll till it completes. */
    while(PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_PROG_DAT_POS))
    {}

    while(bytes_to_fill)
    {
        /** EEPROM must be written 64 bytes at a time.*/
        /** Find the next 64 byte end boundary address*/
        next_block64_end = FIND_NEXT_64BYTE_BOUNDARY((uint32_t)pEeprom);

        /** Within a 64 byte block, write data to the program register using 2 byte access.*/
        while((((uint32_t)pEeprom) <= next_block64_end) && bytes_to_fill)
        {
            *((volatile uint16_t *)pEeprom) = pattern_16bit;
            pEeprom += 2;
            bytes_to_fill -= 2;
        }

        /** Initiate Program sequence for the EEPROM Page*/
        PH_HALREG_SETBITW(EE_DYN,EE_DYN_PROG_DAT_POS);

        /** It takes minimum 200ns for Program command to be generated. Wait for 10us.
            The minimum time for a Page write is anyway 2.9 ms. So the fixed delay of 10us will not add additional delays. */
        phCommon_Wait(EE_PROG_PREPOLL_DELAY);

        /** if the Program sequence is pending, poll till it completes. */
        while(PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_PROG_DAT_POS))
        {}

        /** Check if HV Error occurred during the last program phase. Exit API immediately on error */
        if (PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_HVERR_DAT_POS))
        {
            hv_error_occured = true;
            break;
        }
    }

    /** Check if HV Error occurred during the write operation. */
    if (hv_error_occured)
        return PH_STATUS_MEMORY_ERROR;

    /** Return Success status code on completion. */
    return PH_STATUS_SUCCESS;
}
/*********************************************************************************************************************/

/** \brief This function writes given data to EEPROM memory.
*/
phStatus_t phRomHal_Eeprom_WriteBuffer(
            uint8_t *pBuffer,         /**< [in] pointer to buffer containing data to be written to EEPROM */
            uint8_t *pEeprom,         /**< [in] pointer to EEPROM buffer where data will be copied */
            uint16_t bytes_to_write)    /**< [in] number of bytes to be written */
{
    /* Local variables */
    bool hv_error_occured = false;
    uint32_t next_block64_end;

    /** Do not Validate input parameters in EEPROM HAL in the ROM code.*/

    /** if any Program sequence is pending, poll till it completes. */
    while(PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_PROG_DAT_POS))
    {}

    while(bytes_to_write)
    {
        /** EEPROM must be written 64 bytes at a time.*/
        /** Find the next 64 byte end boundary address*/
        next_block64_end = FIND_NEXT_64BYTE_BOUNDARY((uint32_t)pEeprom);

        /** Within a 64 byte block, write data to the program register using byte access.*/
        while((((uint32_t)pEeprom) <= next_block64_end) && bytes_to_write)
        {
            *((volatile uint8_t *)pEeprom) = *((uint8_t *)pBuffer);
            ++pBuffer;
            ++pEeprom;
            --bytes_to_write;
        }

        /** Initiate Program sequence for the EEPROM Page*/
        PH_HALREG_SETBITW(EE_DYN,EE_DYN_PROG_DAT_POS);

        /** It takes minimum 200ns for Program command to be generated. Wait for 10us.
            The minimum time for a Page write is anyway 2.9 ms. So the fixed delay of 10us will not add additional delays. */
        phCommon_Wait(EE_PROG_PREPOLL_DELAY);

        /** if the Program sequence is pending, poll till it completes. */
        while(PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_PROG_DAT_POS))
        {}

        /** Check if HV Error occured during the last program phase. */
        if (PH_HALREG_TESTBITN(EE_STAT_DAT,EE_STAT_DAT_HVERR_DAT_POS))
        {
            hv_error_occured = true;
            break;
        }
    }

    /** Check if HV Error occured during the entire write operation. */
    if (hv_error_occured)
        return PH_STATUS_MEMORY_ERROR;

    /** Return Success status code on completion. */
    return PH_STATUS_SUCCESS;
}
/*********************************************************************************************************************/
