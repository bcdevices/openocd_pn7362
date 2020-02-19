/***************************************************************************
 *   Copyright (C) 2020 by Richard Aplin                                   *
 *   drtune@gmail.com                                                      *
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

/* NXP PN7xxx series NFC controllers
 * Flash register base on PN7xxx is 0x00200000, passed in R0 
 * PN7xxx flash has a couple of quirks;
 * Must write 128-byte aligned pages, 32-bit writes only
 * Any addresses within page not written will be blanked (ff?),
      so if you want part of page to remain unchanged, untouched words must be read/rewritten.

  This code handles the partial page writes but requires 32-bit aligned dest, and 32-bit writes.
*/ 

	.text
	.syntax unified
	.cpu cortex-m0
	.thumb

	/* Params:
	 * r0 - flash base (in), status (out)
	 * r1 - count (bytes, but must be multiple of 4 currently)
	 * r2 - workarea start
	 * r3 - workarea end (moved to r9, then r3 is used again)
	 * r4 - target address (32-bit aligned, within Flash section)
	 * Clobbered:
	 * r5 - rp
	 * r6 - wp, tmp
	 * r7 - tmp
	 * r8 - dirty page flag
     * r9  - tmp
	 * r10 - tmp
	 * r11  - tmp
	 */


#define PN7XXX_FLASH_STAT_COD_OFFSET		0x0c

/*EE_STAT_COD_PROG_0_COD_MASK | EE_STAT_COD_PROG_1_COD_MASK */
#define PN7XXX_FLASH_STAT_COD_BUSY_MASK		0x4002

/* EE_STAT_COD_HVERR_1_COD_MASK | EE_STAT_COD_HVERR_0_COD_MASK */
#define PN7XXX_FLASH_STAT_COD_ERROR_MASK	0x2001

/* offset of EE_DYN register from flash reg base */
#define PN7XXX_FLASH_DYN_OFFSET 				0x04 

/* EE_DYN_PROG_0_COD_MASK | EE_DYN_PROG_1_COD_MASK */
#define PN7XXX_FLASH_DYN_TRIGGER_WRITE			(0x00010000|0x00100000)


	.thumb_func
	.global _start
_start:
	mov 	r9,r3	/* free up r3 */

	/* we want a 128-byte, 128-byte aligned buffer in sram, so grab a bit more space on the end of the provided buffer - NOTE this means we will use more ram space than the host knows about */
	lsrs    r7,r3,#7
	adds    r7,#1 /* next page so don't hit end of buffer */
	lsls     r7,#7  /* shift address back to normal */
	mov 	r10,r7 

next_page:

	/* first make copy of what's already in flash into our ram page buffer */
	mov 	r3,r10           /* r3 is temp page buffer (we use 128 bytes after end of workarea) */
	mov     r5,r4 /* r5 is copy of dest pointer rounded down to nearest page start */
	ldr 	r7,=0xffffff80
	ands     r5,r7
	movs    r7,#0x80/4
copy_existing:
	ldr	   r6,[r5]
	str    r6,[r3]
	adds   r3,#4
	adds   r5,#4
	subs   r7,#1
	bne    copy_existing

	movs 	r7,#0
	mov 	r8,r7           /* clear dirty page flag */

	mov 	r3,r10           /* r3 is temp page buffer (we use 128 bytes after end of workarea) */
	movs    r7,#0x7f
	ands    r7,r4      /* we may be starting mid-page, so adjust temp buffer pointer to match */
	add     r3,r7


/* main loop - data is being streamed in via SWD while we flash 128 byte pages */
wait_fifo:
	ldr 	r6, [r2, #0]	/* read wp */
	cmp 	r6, #0			/* abort if wp == 0 */
	beq 	exit

	ldr 	r5, [r2, #4]	/* read rp */
	cmp     r5,r6
	beq 	wait_fifo

	/* we have a 32-bit word to write */

	ldr		r6, [r5]		/* read src buffer */
	ldr     r7, [r3]     	/* read what's already at destination so we can detect if this page even needs actually flashing */
	cmp     r6,r7
	beq     unchanged_data
	str		r6, [r3] 		/* write to flash dest addr anyway (if page turns out dirty we will have needed to write all data) */
	mov 	r8,r5           /* flag we have a dirty page by setting R8 to a nonzero value */
unchanged_data:

	adds	r5, #4
	mov 	r7, r9           /* recover end of buffer ptr */
	cmp 	r5, r7			/* wrap rp at end of buffer */
	bcc		no_wrap
	mov		r5, r2			/* back to start of buffer, and skip 8-byte header (rp/wp) */
	adds	r5, #8
no_wrap:
	str 	r5, [r2, #4]	/* store rp so host can track where we are */

	adds	r3, #4  /* inc buffer dest addr */

	subs	r1, r1, #4	/* total length */
	beq     all_done

	lsls	r6,r3,#32-7 /* check low 7 bits of dest address for zero, i.e. did we just finish a page */
	bne 	wait_fifo  /* no, still in middle of a page, get more data */

all_done:  /* either all done or just finished a page */


	/* was the page actually changed, or can we skip the (slow) flash operation? r8 is flag */
	mov     r7,r8
	cmp     r7,#0
	beq	    skip_page_flash

	/* copy from our buffer to actual flash address space */
	mov 	r3,r10    
	movs    r7,#0x80/4
	mov     r5,r4
copy_to_flash:
	ldr	   r6,[r3]
	str    r6,[r5]
	adds   r5,#4
	adds   r3,#4
	subs   r7,#1
	bne    copy_to_flash


	/* trigger page write */
	ldr		r7,=PN7XXX_FLASH_DYN_TRIGGER_WRITE
	str 	r7, [r0, #PN7XXX_FLASH_DYN_OFFSET]

	/* wait for write start */
	ldr 	r7, =PN7XXX_FLASH_STAT_COD_BUSY_MASK
wait_for_busy:
	ldr 	r6, [r0, #PN7XXX_FLASH_STAT_COD_OFFSET]
	tst 	r6, r7
	beq 	wait_for_busy

	/* wait for write complete */
busy:
	ldr 	r6, [r0, #PN7XXX_FLASH_STAT_COD_OFFSET]	/* wait until both flash blocks not busy */
	tst 	r6, r7
	bne 	busy

	ldr		r7, =PN7XXX_FLASH_STAT_COD_ERROR_MASK		/* check the error bits */
	tst 	r6, r7
	bne 	error

   /* verify page */
	mov 	r3,r10    
	movs    r7,#0x80/4
	mov     r5,r4
verify_write:
	ldr	   r6,[r3]
	mov    r11,r6
	ldr    r6,[r5]
	cmp    r6,r11
	bne    error
	adds   r5,#4
	adds   r3,#4
	subs   r7,#1
	bne    verify_write


skip_page_flash:
	
	movs     r7,#0x80
	add     r4,r7 /*bump dest */

	cmp     r1, #0  /* done? */
	bne     next_page
	b 		exit

error:
	movs	r0, #0
	str 	r0, [r2, #4]	/* set rp = 0 on error */
	movs	r0, #1
	bkpt	#0

exit:
	movs	r0, #0
	bkpt	#0
