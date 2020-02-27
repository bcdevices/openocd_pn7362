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

/* This is a sub-function included twice (once for Flash, once for EEPROM, they're slightly different) */
	cpsid i

	mov 	r9,r3	/* free up r3 (end of buffer ptr)*/

	/* we want a 128-byte, 128-byte aligned buffer in sram, so grab a bit more space on the end of the provided buffer - NOTE this means we will use more ram space than the host knows about */
	lsrs    r7,r3,#PAGE_SIZE_SHIFT
	adds    r7,#1 /* next page so don't hit end of buffer */
	lsls     r7,#PAGE_SIZE_SHIFT  /* shift address back to normal */
	mov 	r10,r7 

.next_page:

	/* first make copy of what's already in flash into our ram page buffer */
	mov 	r3,r10           /* r3 is temp page buffer (we use 128 bytes after end of workarea) */
	/* r5 is copy of dest pointer rounded down to nearest page start */
	lsrs    r5,r4,#PAGE_SIZE_SHIFT
	lsls    r5,#PAGE_SIZE_SHIFT
	movs    r7,#PAGE_SIZE_BYTES/4
.copy_existing:
	ldr	   r6,[r5]
	str    r6,[r3]
	adds   r3,#4
	adds   r5,#4
	subs   r7,#1
	bne    .copy_existing

	movs 	r7,#0
	mov 	r8,r7           /* clear dirty page flag */

	mov 	r3,r10           /* r3 is temp page buffer (we use 128 bytes after end of workarea) */
	movs    r7,#PAGE_SIZE_BYTES-4
	ands    r7,r4      /* we may be starting mid-page, so adjust temp buffer pointer to match */
	add     r3,r7


/* main loop - data is being streamed in via SWD while we flash 128 byte pages */
.wait_fifo:
	ldr 	r6, [r2, #0]	/* read wp */
	cmp 	r6, #0			/* abort if wp == 0 */
	beq 	.exit

	ldr 	r5, [r2, #4]	/* read rp */
	cmp     r5,r6
	beq 	.wait_fifo

	/* we have a 32-bit word to write */

	ldr		r6, [r5]		/* read src buffer */
	ldr     r7, [r3]     	/* read what's already at destination so we can detect if this page even needs actually flashing */
	cmp     r6,r7
	beq     .unchanged_data
	str		r6, [r3] 		/* write to flash dest addr anyway (if page turns out dirty we will have needed to write all data) */
	mov 	r8,r5           /* flag we have a dirty page by setting R8 to a nonzero value */
.unchanged_data:

	adds	r5, #4
	mov 	r7, r9           /* recover end of buffer ptr */
	cmp 	r5, r7			/* wrap rp at end of buffer */
	bne		.no_wrap
	mov		r5, r2			/* back to start of buffer, and skip 8-byte header (rp/wp) */
	adds	r5, #8
.no_wrap:
	str 	r5, [r2, #4]	/* store rp so host can track where we are */

	adds	r3, #4  /* inc buffer dest addr */

	subs	r1, r1, #4	/* total length */
	beq     .all_done

	lsls	r6,r3,#32-PAGE_SIZE_SHIFT /* check low 7 bits of dest address for zero, i.e. did we just finish a page */
	bne 	.wait_fifo  /* no, still in middle of a page, get more data */

.all_done:  /* either all done or just finished a page */


	/* was the page actually changed, or can we skip the (slow) flash operation? r8 is flag */
	mov     r7,r8
	cmp     r7,#0
	/* this is an optimization that should be fine, remove branch if debugging any weirdness */
	beq	    .skip_page_flash 

	/* copy from our buffer to actual flash address space */
	mov 	r3,r10    
	movs    r7,#PAGE_SIZE_BYTES/4
	/* r5 is copy of dest pointer rounded down to nearest page start */
	lsrs    r5,r4,#PAGE_SIZE_SHIFT
	lsls    r5,#PAGE_SIZE_SHIFT
.copy_to_flash:
	ldr	   r6,[r3]
	str    r6,[r5]
	adds   r5,#4
	adds   r3,#4
	subs   r7,#1
	bne    .copy_to_flash


	/* trigger page write */
	ldr		r7,=PN7XXX_FLASH_DYN_TRIGGER_WRITE
	str 	r7, [r0, #PN7XXX_FLASH_DYN_OFFSET]

	/* wait for write start */
	ldr 	r7, =PN7XXX_FLASH_STAT_COD_BUSY_MASK
.wait_for_busy:
	ldr 	r6, [r0, #PN7XXX_FLASH_STAT_COD_OFFSET]
	tst 	r6, r7
	beq 	.wait_for_busy

	/* wait for write complete */
.busy:
	ldr 	r6, [r0, #PN7XXX_FLASH_STAT_COD_OFFSET]	/* wait until both flash blocks not busy */
	tst 	r6, r7
	bne 	.busy

	ldr		r7, =PN7XXX_FLASH_STAT_COD_ERROR_MASK		/* check the error bits */
	tst 	r6, r7
	bne 	.error

   /* verify page */
	mov 	r3,r10    
	movs    r7,#PAGE_SIZE_BYTES/4
	/* r5 is copy of dest pointer rounded down to nearest page start */
	lsrs    r5,r4,#PAGE_SIZE_SHIFT
	lsls    r5,#PAGE_SIZE_SHIFT
.verify_write:
	ldr	   r6,[r3]
	mov    r11,r6
	ldr    r6,[r5]
	cmp    r6,r11
	bne    .error
	adds   r5,#4
	adds   r3,#4
	subs   r7,#1
	bne    .verify_write


.skip_page_flash:
	
	lsrs    r4,#PAGE_SIZE_SHIFT
	adds    r4,#1  /* bump destination to next page */
	lsls    r4,#PAGE_SIZE_SHIFT

	cmp     r1, #0  /* done? */
	bne     .next_page
	b 		.exit

.error:
	movs	r0, #0
	str 	r0, [r2, #4]	/* set rp = 0 on error */
	movs	r0, #1
	bkpt	#0

.exit:
	movs	r0, #0
	bkpt	#0
