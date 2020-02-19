/***************************************************************************
 *   Copyright (C) 2005 by Dominic Rath                                    *
 *   Dominic.Rath@gmx.de                                                   *
 *                                                                         *
 *   Copyright (C) 2008 by Spencer Oliver                                  *
 *   spen@spen-soft.co.uk                                                  *
 *                                                                         *
 *   Copyright (C) 2011 by Andreas Fritiofson                              *
 *   andreas.fritiofson@gmail.com                                          *
 *
 *
 *   PN73xxx.c driver created from bare-bones of stm32f1x.c, originally by the above authors
 *   Richard Aplin for BC Devices Feb 2020
 *
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

//temporarily turn off nags during port, todo tidy and remove these
#pragma GCC diagnostic ignored  "-Wall"
//#pragma GCC diagnostic ignored  "-Wunused-function"
//#pragma GCC diagnostic ignored  "-Wunused-variable"
//#pragma GCC diagnostic ignored  "-Wunused-but-set-variable"
//#pragma GCC diagnostic ignored  "-Wunused-label"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imp.h"
#include <helper/binarybuffer.h>
#include <target/algorithm.h>
#include <target/armv7m.h>



// PN definitions. It has a rom loader that runs first
// Some varients have half as much Flash (80k), this is defined in the .cfg file not in here
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


#define PH_ROMHAL_FLASH_START_ADDRESS  0x00203000
#define PH_ROMHAL_FLASH_END_ADDRESS    (PH_ROMHAL_FLASH_START_ADDRESS + (158*1024) -1)
#define PH_ROMHAL_FLASH_SIZE    (PH_ROMHAL_FLASH_END_ADDRESS - PH_ROMHAL_FLASH_START_ADDRESS + 1) /* Total Size of the PAGEFLASH = 160 KBytes */
#define PH_ROMHAL_FLASH_PAGE_SIZE          128 /* FLASH is organized as 1280 pages x 128 bytes = 160KBytes */
#define PH_ROMHAL_FLASH_PAGE_COUNT         1280

#define PN73_RAM_START 0x100020
#define PN73_RAM_SIZE  0x2e00

//#define PN73_OCDInfo ((OCDInfo_t *)RAM_START+0x2e00)
#define PN73_OCDInfo (PN73_RAM_START+0x2e00)
#define PN73_STACK (PN73_RAM_START+0x2d00)
#define PN73_BUFFER_SIZE 0x1000

#define PN74_FLASH_START PH_ROMHAL_FLASH_START_ADDRESS
#define PN74_FLASH_SIZE  PH_ROMHAL_FLASH_SIZE

#define PN73_FLASH_REGISTER_BASE 0x00200000


/* timeout values */

#define FLASH_WRITE_TIMEOUT 10
#define FLASH_ERASE_TIMEOUT 100

//somewhat redundant 
struct pn73x_flash_bank {
	int ppage_size;
	int probed;
	uint32_t user_bank_size;
};

//static int pn73x_mass_erase(struct flash_bank *bank);
static int pn73x_get_device_id(struct flash_bank *bank, uint32_t *device_id);
static int pn73x_write_block(struct flash_bank *bank, const uint8_t *buffer,
		uint32_t address, uint32_t count);

/* flash bank pn73x <base> <size> 0 0 <target#>
 */
FLASH_BANK_COMMAND_HANDLER(pn73x_flash_bank_command)
{
	struct pn73x_flash_bank *pn73x_info;

	if (CMD_ARGC < 6)
		return ERROR_COMMAND_SYNTAX_ERROR;

	pn73x_info = malloc(sizeof(struct pn73x_flash_bank));

	bank->driver_priv = pn73x_info;
	pn73x_info->probed = 0;
	pn73x_info->user_bank_size = bank->size;

	return ERROR_OK;
}

static int pn73x_protect_check(struct flash_bank *bank)
{
	return ERROR_OK;
}

static int pn73x_erase(struct flash_bank *bank, int first, int last)
{
	return ERROR_OK;
}

static int pn73x_protect(struct flash_bank *bank, int set, int first, int last)
{
	return ERROR_OK;
}

/* NOTE the count must be multiple of 4 bytes, and address on 4 byte boundary */
static int pn73x_write_block(struct flash_bank *bank, const uint8_t *buffer,
		uint32_t address, uint32_t count)
{
	struct target *target = bank->target;
	uint32_t buffer_size = PN73_BUFFER_SIZE+12;  /* +12 makes the actual target data buffer itself a nice round size (4096 bytes) */
	struct working_area *write_algorithm;
	struct working_area *source;
	//uint32_t address = bank->base + offset;
	struct reg_param reg_params[5];
	struct armv7m_algorithm armv7m_info;
	int retval = ERROR_OK;


	static const uint8_t pn73xxxx_flash_write_code[] = 
	{
		//Compiled from BCFlashStub project. This does not support async write mode but it wouldn't be too hard to add
		//the stub could be implemented in assembler much like the other OpenOCD ones, this is not part of basic functionality
#include "../../../contrib/loaders/flash/pn73xxxx/pn7xxxx.inc"
	};

	/* flash write code */
	if (target_alloc_working_area(target, sizeof(pn73xxxx_flash_write_code),
			&write_algorithm) != ERROR_OK) {
		LOG_WARNING("no working area available, can't do block memory writes");
		return ERROR_TARGET_RESOURCE_NOT_AVAILABLE;
	}

	retval = target_write_buffer(target, write_algorithm->address,
			sizeof(pn73xxxx_flash_write_code), pn73xxxx_flash_write_code);
	if (retval != ERROR_OK)
		return retval;

	/* memory buffer */
	while (target_alloc_working_area_try(target, buffer_size, &source) != ERROR_OK) {
		buffer_size /= 2;
		if (buffer_size <= 256) {
			/* we already allocated the writing code, but failed to get a
			 * buffer, free the algorithm */
			target_free_working_area(target, write_algorithm);

			LOG_WARNING(
				"no large enough working area available, can't do block memory writes");
			return ERROR_TARGET_RESOURCE_NOT_AVAILABLE;
		}
	}

	armv7m_info.common_magic = ARMV7M_COMMON_MAGIC;
	armv7m_info.core_mode = ARM_MODE_THREAD;

	init_reg_param(&reg_params[0], "r0", 32, PARAM_IN_OUT);	/* flash base (in), status (out) */
	init_reg_param(&reg_params[1], "r1", 32, PARAM_OUT);	/* count (bytes) */
	init_reg_param(&reg_params[2], "r2", 32, PARAM_OUT);	/* buffer start */
	init_reg_param(&reg_params[3], "r3", 32, PARAM_OUT);	/* buffer end */
	init_reg_param(&reg_params[4], "r4", 32, PARAM_IN_OUT);	/* target address */

	buf_set_u32(reg_params[0].value, 0, 32, PN73_FLASH_REGISTER_BASE);
	buf_set_u32(reg_params[1].value, 0, 32, count<<1);
	buf_set_u32(reg_params[2].value, 0, 32, source->address);
	buf_set_u32(reg_params[3].value, 0, 32, source->address + source->size);
	buf_set_u32(reg_params[4].value, 0, 32, address);

	armv7m_info.common_magic = ARMV7M_COMMON_MAGIC;
	armv7m_info.core_mode = ARM_MODE_THREAD;

	retval = target_run_flash_async_algorithm(target, buffer,
			count/2, 4,	//block count, block size
			0, NULL,	//mem params
			5, reg_params, //reg params
			source->address, source->size,
			write_algorithm->address, 0,
			&armv7m_info);

	if (retval == ERROR_FLASH_OPERATION_FAILED) {
		LOG_ERROR("flash write failed at address 0x%"PRIx32,
				buf_get_u32(reg_params[4].value, 0, 32));
	}

	destroy_reg_param(&reg_params[0]);
	destroy_reg_param(&reg_params[1]);
	destroy_reg_param(&reg_params[2]);
	destroy_reg_param(&reg_params[3]);
	destroy_reg_param(&reg_params[4]);

	target_free_working_area(target, source);
	target_free_working_area(target, write_algorithm);

	return retval;
}





static int pn73x_write(struct flash_bank *bank, const uint8_t *buffer,
		uint32_t offset, uint32_t count)
{
	uint8_t *new_buffer = NULL;

	if (bank->target->state != TARGET_HALTED) {
		LOG_ERROR("Target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	if (offset & 0x3) {
		LOG_ERROR("offset 0x%" PRIx32 " breaks required 4-byte alignment", offset);
		return ERROR_FLASH_DST_BREAKS_ALIGNMENT;
	}

	/* pad to nearest 4 bytes */
	if (count & 3) {
		new_buffer = malloc((count + 3) & 0xfffffffc);
		if (new_buffer == NULL) {
			LOG_ERROR("need to pad, and no memory for padding buffer");
			return ERROR_FAIL;
		}
		LOG_INFO("padding with 0xff");
		buffer = memcpy(new_buffer, buffer, count);
		while(count & 3){
			new_buffer[count++] = 0xff;
		}
	}

	uint32_t words_remaining = count / 2;
	int retval;

	/* try using a block write */
	retval = pn73x_write_block(bank, buffer, bank->base + offset, words_remaining);

	if (new_buffer)
		free(new_buffer);

	return retval;
}

static int pn73x_get_device_id(struct flash_bank *bank, uint32_t *device_id)
{
	/* This check the device CPUID core register*/

	struct target *target = bank->target;
	uint32_t cpuid;

	/* Get the CPUID from the ARM Core
	 * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0432c/DDI0432C_cortex_m0_r0p0_trm.pdf 4.2.1 */
	int retval = target_read_u32(target, 0xE000ED00, &cpuid);
	if (retval != ERROR_OK)
		return retval;

	if (((cpuid >> 4) & 0xFFF) == 0xC20) {
		/* 0xC20 is M0 devices */
		//cannot find suitable register yet on PN7xxxxx series 
		//device_id_register = 0x40015800;
		*device_id=0;
		retval=ERROR_OK;
	} else {
		LOG_ERROR("Cannot identify target as a pn73x");
		return ERROR_FAIL;
	}

	return retval;
}

static int pn73x_get_flash_size(struct flash_bank *bank, uint16_t *flash_size_in_kb)
{
	*flash_size_in_kb = PN74_FLASH_SIZE/1024;
	return ERROR_OK;
}

static int pn73x_probe(struct flash_bank *bank)
{
	// This is a bit untidy
	struct pn73x_flash_bank *pn73x_info = bank->driver_priv;
	uint16_t flash_size_in_kb;
	uint32_t device_id;
	int page_size;
	uint32_t base_address = PN74_FLASH_START;
	
	pn73x_info->probed = 0;
	

	/* read pn73 device id register */
	int retval = pn73x_get_device_id(bank, &device_id);
	if (retval != ERROR_OK)
		return retval;

	LOG_INFO("device id = 0x%08" PRIx32 "", device_id);

	switch (device_id & 0xfff) {
	case 0:
		page_size = 1024;
		pn73x_info->ppage_size = 4;
		break;
	

	default:
		LOG_WARNING("Cannot identify target as a pn73 family.");
		return ERROR_FAIL;
	}
	/* get flash size from target. */
	retval = pn73x_get_flash_size(bank, &flash_size_in_kb);
	
	LOG_INFO("flash size = %dkbytes", flash_size_in_kb);
	/* did we assign flash size? */
	assert(flash_size_in_kb != 0xffff);

	/* calculate numbers of pages */
	int num_pages = flash_size_in_kb * 1024 / page_size;

	/* check that calculation result makes sense */
	assert(num_pages > 0);

	bank->base = base_address;
	bank->size = (num_pages * page_size);

	bank->num_sectors = num_pages;
	bank->sectors = alloc_block_array(0, page_size, num_pages);
	if (!bank->sectors)
		return ERROR_FAIL;

	pn73x_info->probed = 1;

	return ERROR_OK;
}

static int pn73x_auto_probe(struct flash_bank *bank)
{
	struct pn73x_flash_bank *pn73x_info = bank->driver_priv;
	if (pn73x_info->probed)
		return ERROR_OK;
	return pn73x_probe(bank);
}

static int get_pn73x_info(struct flash_bank *bank, char *buf, int buf_size)
{
	uint32_t dbgmcu_idcode=0;

	uint16_t rev_id = dbgmcu_idcode >> 16;
	const char *device_str;
	const char *rev_str = NULL;

	//temporary
	{
		device_str = "pn73xxxx";
		rev_str = "Not implemented";
	}

	
	if (rev_str != NULL)
		snprintf(buf, buf_size, "%s - Rev: %s", device_str, rev_str);
	else
		snprintf(buf, buf_size, "%s - Rev: unknown (0x%04x)", device_str, rev_id);

	return ERROR_OK;
}

#if 0
COMMAND_HANDLER(pn73x_handle_lock_command)
{
	return ERROR_OK;
}

COMMAND_HANDLER(pn73x_handle_unlock_command)
{
	return ERROR_OK;
}
#endif

static const struct command_registration pn73x_exec_command_handlers[] = {
/*	Not yet supported
	{
		.name = "lock",
		.handler = pn73x_handle_lock_command,
		.mode = COMMAND_EXEC,
		.usage = "bank_id",
		.help = "Lock entire flash device.",
	},
	{
		.name = "unlock",
		.handler = pn73x_handle_unlock_command,
		.mode = COMMAND_EXEC,
		.usage = "bank_id",
		.help = "Unlock entire protected flash device.",
	},
*/
	COMMAND_REGISTRATION_DONE
};

static const struct command_registration pn73x_command_handlers[] = {
	{
		.name = "pn73xxxx",
		.mode = COMMAND_ANY,
		.help = "pn73xxxx flash command group",
		.usage = "",
		.chain = pn73x_exec_command_handlers,
	},
	COMMAND_REGISTRATION_DONE
};

const struct flash_driver pn73xxxx_flash = {
	// Not all fully supported, this is minimal flash driver currently
	.name = "pn73xxxx",
	.commands = pn73x_command_handlers,
	.flash_bank_command = pn73x_flash_bank_command,
	.erase = pn73x_erase,
	.protect = pn73x_protect,
	.write = pn73x_write,
	.read = default_flash_read,
	.probe = pn73x_probe,
	.auto_probe = pn73x_auto_probe,
	.erase_check = default_flash_blank_check,
	.protect_check = pn73x_protect_check,
	.info = get_pn73x_info,
	.free_driver_priv = default_flash_free_driver_priv,
};
