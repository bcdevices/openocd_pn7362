/*
===============================================================================
 Name        : BCFlashStub.c
 Author      : Richard Aplin
 Version     : 1.0
 Copyright   : 2020 Blue Clover Devices
 Description : main definition
===============================================================================
*/

/*
 * Flashing stub for OpenOCD
 * Must be compiled with supplied custom linker script; no vector tables etc.
 * First thing in the binary is executable code.
 * Exits via BKPT instruction which is picked up via SWD
 */

#include <cr_section_macros.h>

#include "phCommon.h"
#include "phRomHal_Eeprom.h"
#include "phRomHal_Flash.h"
#include "ph_Registers.h"

typedef struct {
	uint32_t bufStart;
	uint32_t destAddr;
	uint32_t flashLengthBytes;
}OCDInfo_t;

// This code goes at the bottom 4k 100020 - 100fff
#define RAM_START 0x100020
#define OCDInfo ((OCDInfo_t *)(RAM_START+0x2e00))


int main(void) {
	int returnCode=0;	//0=fail

	phCommon_WaitInit (E_COMMON_CPUSPEED_20MHZ);

	{
		uint32_t retries=5;
    	while(retries--){
			uint8_t *src=(uint8_t *)OCDInfo->bufStart;
			uint8_t *dest=(uint8_t *)OCDInfo->destAddr;
			uint32_t len=OCDInfo->flashLengthBytes;

			if ((retries & 1) ==0){
				//first time around loop check to see if write is necessary
				do{
					if (*src++ != *dest++) break;
				}while(--len);

				if (!len){
					//yes, data is ok, 1=success
					returnCode=1;
					break;
				}
			}else{
				//then on second loop do a write...
				phStatus_t status=PH_STATUS_INTERNAL_ERROR;
				if ((uint32_t)dest>=PH_ROMHAL_EEPROM_MEM_START_ADDRESS && (uint32_t)dest<=PH_ROMHAL_EEPROM_DATA_END_ADDRESS){
					//eeprom (!first int32 is security word!)
					status= phRomHal_Eeprom_WriteBuffer(
					            src,         /**< [in] pointer to buffer to be copied - can be in RAM or EEPROM */
					            dest,         /**< [in] pointer to EEPROM buffer where data will be copied */
					            len  		 /**< [in] number of bytes to be written */
					            );
				}else
				if ((uint32_t)dest>=PH_ROMHAL_FLASH_START_ADDRESS && (uint32_t)dest<=PH_ROMHAL_FLASH_END_ADDRESS){
					status= phRomHal_Flash_WriteBuffer(
								src,
								dest,
								len);
				}
				//'status' return code is irrelevant as we will verify write immediately, retry a couple of times if reqd
				(void)status;
			}
    	}
    }
	OCDInfo->bufStart=returnCode;
	__asm__(
    		"BKPT #0"
 			);	//how we return

}

