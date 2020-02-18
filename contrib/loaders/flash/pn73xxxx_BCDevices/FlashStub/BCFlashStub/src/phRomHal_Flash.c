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
#include <phRomHal_Flash.h>
#include <ph_Datatypes.h>
#include <ph_Status.h>
#include <id_nfc_pn640_apb_if_reg.h>
#include <ph_Registers.h>
#include <phCommon.h>
/*********************************************************************************************************************/
/*   MACRO DEFINITIONS                                                                                               */
/*********************************************************************************************************************/


/*********************************************************************************************************************/
/*   GLOBAL VARIABLES                                                                                                */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   LOCAL FUNCTIONS                                                                                                 */
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS                                                                                                */
/*********************************************************************************************************************/


/** \brief This function write one double word (32 bit) to the FLASH memory.
*/
phStatus_t phRomHal_Flash_WriteDoubleWord(
        uint32_t dwWord, /**< [in] DOUBLE word to be written */
        uint8_t *pFlash) /**< [in] pointer to FLASH location where data will be written. */
{
    return (phRomHal_Flash_WriteBuffer((uint8_t *)(&dwWord),pFlash,(uint32_t)(sizeof(uint32_t))));
}
/*********************************************************************************************************************/

/** \brief This function writes a given Pattern to a specified region in the PAGEFLASH.
*/
phStatus_t phRomHal_Flash_FillRegion(
            uint8_t *pFlash,         /**< [in] 4byte aligned address in PageFlash starting from which, the pattern will be filled */
            uint32_t bytes_to_fill,  /**< [in] number of bytes to be written. Must be Non zero, multiple of 4. */
            uint32_t pattern_32bit)  /**< [in] A 32 bit pattern that will be written to the specified region in the PAGEFLASH */
{
    /* Local variables */
    bool hv_error_occured = false;
    uint32_t innerloop_tempAddr,temp_addr,start_address, end_address;
    uint32_t block128_end;

    /** Do not Validate input parameters in PAGE FLASH HAL in the ROM code.*/

    /** Find the range of addresses that the user wants to fill in Flash. */
    start_address = (uint32_t)pFlash;
    end_address = (uint32_t)pFlash + bytes_to_fill - 1;
    temp_addr = start_address;

    /** if any Program sequence is pending for PageFlash 0 or PageFlash1 poll till it completes. */
    while(PH_HALREG_GETFIELD(EE_STAT_COD, (EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK)))
    {}

    while(bytes_to_fill)
    {
        /** PAGEFLASH must be written 128 bytes at a time.*/
        /** Find the 128 byte Start boundary address*/
        innerloop_tempAddr = FIND_128BYTE_START_BOUNDARY(temp_addr);
        /** Find the 128 byte end boundary address*/
        block128_end = innerloop_tempAddr + PH_ROMHAL_FLASH_PAGE_SIZE - 1;

        /** Within a 128 byte block, write data to be written to the program register using 4byte access.
        Unlike EEPROM, Untouched data must be read and written back to avoid erasure during the programming phase.*/
        while(innerloop_tempAddr <= block128_end)
        {
            if((innerloop_tempAddr >= start_address) && (innerloop_tempAddr <= end_address))
            {
                *((volatile uint32_t *)innerloop_tempAddr) = pattern_32bit;
                bytes_to_fill -= 4;
            }
            else
            {
                uint32_t temp_data = *((volatile uint32_t *)innerloop_tempAddr);
                *((volatile uint32_t *)innerloop_tempAddr) = temp_data;
            }
            innerloop_tempAddr += 4;
        }

        /** Initiate Program sequence for the PageFlash 0 and 1 Pages*/
        PH_HALREG_SETREG(EE_DYN,(EE_DYN_PROG_0_COD_MASK | EE_DYN_PROG_1_COD_MASK));

        /** It takes minimum 200ns for Program command to be generated. Wait for 10us.
            The minimum time for a Page write is anyway 2.9 ms. So the fixed delay of 10us will not add additional delays. */
        phCommon_Wait(PF_PROG_PREPOLL_DELAY);

        /** if the Program sequence is pending, poll till it completes. */
        while(PH_HALREG_GETFIELD(EE_STAT_COD, (EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK)))
        {}

        /** Check if HV Error occured during the program phase of PageFlash 0 and 1 */
        if((PH_HALREG_TESTBITN(EE_STAT_COD,EE_STAT_COD_HVERR_0_COD_POS)) ||
            (PH_HALREG_TESTBITN(EE_STAT_COD,EE_STAT_COD_HVERR_1_COD_POS)))
        {
            hv_error_occured = true;
            break;
        }
        temp_addr = block128_end + 1;
    }

    /** Check if HV Error occured during the entire write operation. */
    if (hv_error_occured)
        return PH_STATUS_MEMORY_ERROR;

    /** Return Success status code on completion. */
    return PH_STATUS_SUCCESS;
}
/*********************************************************************************************************************/

/** \brief This function writes data to PAGEFLASH.
*/
phStatus_t phRomHal_Flash_WriteBuffer(
            uint8_t *pBuffer,       /**< [in] pointer to buffer containing data to be written to PAGEFLASH */
            uint8_t *pFlash,        /**< [in] Valid address of a byte in the PAGEFLASH. Given address must be 4byte aligned. */
            uint32_t bytes_to_write)/**< [in] number of bytes to be written. Must be Non zero, multiple of 4. */
{
    /* Local variables */
    bool hv_error_occured = false;
    uint32_t innerloop_tempAddr,temp_addr,start_address, end_address;
    uint32_t block128_end;

    /** Do not Validate input parameters in PAGE FLASH HAL in the ROM code.*/

    /** Find the range of addresses that the user wants to write in Flash. */
    start_address = (uint32_t)pFlash;
    end_address = (uint32_t)pFlash + bytes_to_write - 1;
    temp_addr = start_address;

    /** if any Program sequence is pending for PageFlash 0 or PageFlash1 poll till it completes. */
    while(PH_HALREG_GETFIELD(EE_STAT_COD, (EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK)))
    {}

    while(bytes_to_write)
    {
        /** PAGEFLASH must be written 128 bytes at a time.*/
        /** Find the 128 byte Start boundary address*/
        innerloop_tempAddr = FIND_128BYTE_START_BOUNDARY(temp_addr);
        /** Find the 128 byte end boundary address*/
        block128_end = innerloop_tempAddr + PH_ROMHAL_FLASH_PAGE_SIZE - 1;

        /** Within a 128 byte block, write data to be written to the program register using 4byte access.
        Unlike EEPROM, Untouched data must be read and written back to avoid erasure during the programming phase.*/
        while(innerloop_tempAddr <= block128_end)
        {
            if((innerloop_tempAddr >= start_address) && (innerloop_tempAddr <= end_address))
            {
                *((volatile uint32_t *)innerloop_tempAddr) = *((uint32_t *)pBuffer);
                pBuffer += 4;
                bytes_to_write -= 4;
            }
            else
            {
                uint32_t temp_data = *((volatile uint32_t *)innerloop_tempAddr);
                *((volatile uint32_t *)innerloop_tempAddr) = temp_data;
            }
            innerloop_tempAddr += 4;
        }

        /** Initiate Program sequence for the PageFlash 0 and 1 Pages */
        PH_HALREG_SETREG(EE_DYN,(EE_DYN_PROG_0_COD_MASK | EE_DYN_PROG_1_COD_MASK));

        /** It takes minimum 200ns for Program command to be generated. Wait for 10us.
            The minimum time for a Page write is anyway 2.9 ms. So the fixed delay of 10us will not add additional delays. */
        phCommon_Wait(PF_PROG_PREPOLL_DELAY);

        /** if the Program sequence is pending, poll till it completes. */
        while(PH_HALREG_GETFIELD(EE_STAT_COD, (EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK)))
        {}

        /** Check if HV Error occured during the program phase of PageFlash 0 and 1 */
        if((PH_HALREG_TESTBITN(EE_STAT_COD,EE_STAT_COD_HVERR_0_COD_POS)) ||
            (PH_HALREG_TESTBITN(EE_STAT_COD,EE_STAT_COD_HVERR_1_COD_POS)))
        {
            hv_error_occured = true;
            break;
        }

        temp_addr = block128_end + 1;
    }

    /** Check if HV Error occured during the entire write operation. */
    if (hv_error_occured)
        return PH_STATUS_MEMORY_ERROR;

    /** Return Success status code on completion. */
    return PH_STATUS_SUCCESS;
}
/*********************************************************************************************************************/

/** \brief This function writes a single full 128byte PAGEFLASH page.
*/
phStatus_t phRomHal_Flash_Write_Page(
            uint32_t page_number, /**< [in] Valid index of a 128byte page in the PAGEFLASH */
            uint8_t *pBuffer)    /**< [in] pointer to buffer containing data to be written to PAGEFLASH */
{
    /* Local variables */
    uint16_t bytes_remaining = PH_ROMHAL_FLASH_PAGE_SIZE;
    uint32_t addr;

    /** Do not Validate input parameters in PAGE FLASH HAL in the ROM code.*/

    /** Write data to page register. */
    /** Calculate the address to be written to, in EEPROM. */
    addr = PH_ROMHAL_FLASH_START_ADDRESS + (page_number * PH_ROMHAL_FLASH_PAGE_SIZE);

    /** if any Program sequence is pending for PageFlash 0 or PageFlash1 poll till it completes. */
    while(PH_HALREG_GETFIELD(EE_STAT_COD, (EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK)))
    {}

    /** Write the data in the buffer to the Page register*/
    /** Write must be done using 32bit(4byte) access.*/
    while(bytes_remaining)
    {
        *((volatile uint32_t *)addr) = *((uint32_t *)pBuffer);
        pBuffer += 4;
        addr += 4;
        bytes_remaining -= 4;
    }

    /** Initiate Program sequence for the PageFlash 0 and 1 Pages */
    PH_HALREG_SETREG(EE_DYN,(EE_DYN_PROG_0_COD_MASK | EE_DYN_PROG_1_COD_MASK));

    /** It takes minimum 200ns for Program command to be generated. Wait for 10us.
        The minimum time for a Page write is anyway 2.9 ms. So the fixed delay of 10us will not add additional delays. */
    phCommon_Wait(PF_PROG_PREPOLL_DELAY);

    /** if the Program sequence is pending, poll till it completes. */
    while(PH_HALREG_GETFIELD(EE_STAT_COD, (EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK)))
    {}

    /** Check if HV Error occured during the program phase of PageFlash 0 and 1 */
    if((PH_HALREG_TESTBITN(EE_STAT_COD,EE_STAT_COD_HVERR_0_COD_POS)) ||
        (PH_HALREG_TESTBITN(EE_STAT_COD,EE_STAT_COD_HVERR_1_COD_POS)))
        return PH_STATUS_MEMORY_ERROR;

    /** Return Success status code on completion. */
    return PH_STATUS_SUCCESS;
}
/*********************************************************************************************************************/

/** \brief This function resets the PageFlash controller hardware.
*/
void phRomHal_Flash_HwReset(void)
{
    /** Trigger Reset of the PageFlash controller. */
    PH_HALREG_SETBITW(EE_DYN,EE_DYN_EE_RST_0_COD_POS);
    PH_HALREG_SETBITW(EE_DYN,EE_DYN_EE_RST_1_COD_POS);
    return;
}
/*********************************************************************************************************************/

