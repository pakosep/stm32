/************************************************************************************
 * nuttx/arch/arm/src/armv7-m/up_memcpy.S
 *
 * armv7m-optimised memcpy, contributed by Mike Smith.  Apparently in the public
 * domain and is re-released here under the modified BSD license:
 *
 * Obtained via a posting on the Stellaris forum:
 *  http://e2e.ti.com/support/microcontrollers/\
 *       stellaris_arm_cortex-m3_microcontroller/f/473/t/44360.aspx
 *
 * Posted by rocksoft on Jul 24, 2008 10:19 AM
 *
 *   Hi,
 *
 *   I recently finished a "memcpy" replacement and thought it might be useful for
 *   others...
 *
 *   I've put some instructions and the code here:
 *
 *   http://www.rock-software.net/downloads/memcpy/
 *
 *   Hope it works for you as well as it did for me.
 *
 *   Liam.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

/************************************************************************************
 * Global Symbols
 ************************************************************************************/

	.global		memcpy

	.syntax		unified
	.thumb
	.cpu		cortex-m3
	.file		"up_memcpy.S"

/************************************************************************************
 * .text
 ************************************************************************************/

	.text

/************************************************************************************
 * Private Constant Data
 ************************************************************************************/

/* We have 16 possible alignment combinations of src and dst, this jump table
 * directs the copy operation
 *
 * Bits:  Src=00, Dst=00 - Long to Long copy
 * Bits:  Src=00, Dst=01 - Long to Byte before half word
 * Bits:  Src=00, Dst=10 - Long to Half word
 * Bits:  Src=00, Dst=11 - Long to Byte before long word
 * Bits:  Src=01, Dst=00 - Byte before half word to long
 * Bits:  Src=01, Dst=01 - Byte before half word to byte before half word -
 *                         Same alignment
 * Bits:  Src=01, Dst=10 - Byte before half word to half word
 * Bits:  Src=01, Dst=11 - Byte before half word to byte before long word
 * Bits:  Src=10, Dst=00 - Half word to long word
 * Bits:  Src=10, Dst=01 - Half word to byte before half word
 * Bits:  Src=10, Dst=10 - Half word to half word - Same Alignment
 * Bits:  Src=10, Dst=11 - Half word to byte before long word
 * Bits:  Src=11, Dst=00 - Byte before long word to long word
 * Bits:  Src=11, Dst=01 - Byte before long word to byte before half word
 * Bits:  Src=11, Dst=11 - Byte before long word to half word
 * Bits:  Src=11, Dst=11 - Byte before long word to Byte before long word -
 *                         Same alignment
 */

MEM_DataCopyTable:
	.byte	(MEM_DataCopy0 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy1 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy2 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy3 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy4 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy5 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy6 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy7 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy8 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy9 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy10 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy11 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy12 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy13 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy14 - MEM_DataCopyJump) >> 1
	.byte	(MEM_DataCopy15 - MEM_DataCopyJump) >> 1

	.align 2

MEM_LongCopyTable:
	.byte	(MEM_LongCopyEnd - MEM_LongCopyJump) >> 1	/* 0 bytes left */
	.byte	0					/* 4 bytes left */
	.byte	(1 * 10) >> 1		/* 8 bytes left */
	.byte	(2 * 10) >> 1		/* 12 bytes left */
	.byte	(3 * 10) >> 1		/* 16 bytes left */
	.byte	(4 * 10) >> 1		/* 20 bytes left */
	.byte	(5 * 10) >> 1		/* 24 bytes left */
	.byte	(6 * 10) >> 1		/* 28 bytes left */
	.byte	(7 * 10) >> 1		/* 32 bytes left */
	.byte	(8 * 10) >> 1		/* 36 bytes left */

	.align 2

/************************************************************************************
 * Public Functions
 ************************************************************************************/
/************************************************************************************
 * Name: memcpy
 *
 * Description:
 *   Optimised "general" copy routine
 *
 * Input Parameters:
 *   r0 = destination, r1 = source, r2 = length
 *
 * Returned Value:
 *   r0 = destination r1-r3 burned
 *
 ************************************************************************************/

	.align 4
	.thumb_func

memcpy:
	push	{r14}
	push    {r0}
	bl      _do_memcpy
	pop     {r0}
	pop     {pc}

	.align 4

	.thumb_func
_do_memcpy:
	push    {r14}

	/* This allows the inner workings to "assume" a minimum amount of bytes */
	/* Quickly check for very short copies */

	cmp		r2, #4
	blt.n	MEM_DataCopyBytes

	and		r14, r0, #3		 		/* Get destination alignment bits */
	bfi		r14, r1, #2, #2	 		/* Get source alignment bits */
	ldr		r3, =MEM_DataCopyTable	/* Jump table base */
	tbb		[r3, r14]		   		/* Perform jump on src/dst alignment bits */
MEM_DataCopyJump:

	.align 4

/* Bits:  Src=01, Dst=01 - Byte before half word to byte before half word - Same alignment
 * 3 bytes to read for long word aligning
 */

MEM_DataCopy5:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #	 0x01

/* Bits:  Src=10, Dst=10 - Half word to half word - Same Alignment
 * 2 bytes to read for long word aligning
 */

MEM_DataCopy10:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=11, Dst=11 - Byte before long word to Byte before long word - Same alignment
 * 1 bytes to read for long word aligning
 */

MEM_DataCopy15:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=00, Dst=00 - Long to Long copy */

MEM_DataCopy0:
	/* Save regs that may be used by memcpy */

	push	{r4-r12}

	/* Check for short word-aligned copy */

	cmp		r2, #0x28
	blt.n	MEM_DataCopy0_2

	/* Bulk copy loop */

MEM_DataCopy0_1:
	ldmia	r1!, {r3-r12}
	stmia	r0!, {r3-r12}
	sub		r2, r2, #0x28
	cmp		r2, #0x28
	bge.n	MEM_DataCopy0_1

	/* Copy remaining long words */

MEM_DataCopy0_2:
	/* Copy remaining long words */

	ldr		r14, =MEM_LongCopyTable
	lsr		r11, r2, #0x02
	tbb		[r14, r11]

	/* longword copy branch table anchor */

MEM_LongCopyJump:
	ldr.w	r3, [r1], #0x04		/* 4 bytes remain */
	str.w	r3, [r0], #0x04
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r4}		/* 8 bytes remain */
	stmia.w	r0!, {r3-r4}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r5}		/* 12 bytes remain */
	stmia.w	r0!, {r3-r5}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r6}		/* 16 bytes remain */
	stmia.w	r0!, {r3-r6}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r7}		/* 20 bytes remain */
	stmia.w	r0!, {r3-r7}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r8}		/* 24 bytes remain */
	stmia.w	r0!, {r3-r8}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r9}		/* 28 bytes remain */
	stmia.w	r0!, {r3-r9}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r10}		/* 32 bytes remain */
	stmia.w	r0!, {r3-r10}
	b.n		MEM_LongCopyEnd
	ldmia.w	r1!, {r3-r11}		/* 36 bytes remain */
	stmia.w	r0!, {r3-r11}

MEM_LongCopyEnd:
	pop		{r4-r12}
	and		r2, r2, #0x03		/* All the longs have been copied */

	/* Deal with up to 3 remaining bytes */

MEM_DataCopyBytes:
	/* Deal with up to 3 remaining bytes */

	cmp		r2, #0x00
	it		eq
	popeq	{pc}
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	subs	r2, r2, #0x01
	it		eq
	popeq	{pc}
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	subs	r2, r2, #0x01
	it		eq
	popeq	{pc}
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	pop		{pc}

 .align 4

/* Bits:  Src=01, Dst=11 - Byte before half word to byte before long word
 * 3 bytes to read for long word aligning the source
 */

MEM_DataCopy7:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=10, Dst=00 - Half word to long word
 * 2 bytes to read for long word aligning the source
 */

MEM_DataCopy8:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=11, Dst=01 - Byte before long word to byte before half word
 * 1 byte to read for long word aligning the source
 */

MEM_DataCopy13:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=00, Dst=10 - Long to Half word */

MEM_DataCopy2:
	cmp		r2, #0x28
	blt.n	MEM_DataCopy2_1

	/* Save regs */

	push	{r4-r12}

	/* Bulk copy loop */

MEM_DataCopy2_2:
	ldmia	r1!, {r3-r12}

	strh	r3, [r0], #0x02

	lsr		r3, r3, #0x10
	bfi		r3, r4, #0x10, #0x10
	lsr		r4, r4, #0x10
	bfi		r4, r5, #0x10, #0x10
	lsr		r5, r5, #0x10
	bfi		r5, r6, #0x10, #0x10
	lsr		r6, r6, #0x10
	bfi		r6, r7, #0x10, #0x10
	lsr		r7, r7, #0x10
	bfi		r7, r8, #0x10, #0x10
	lsr		r8, r8, #0x10
	bfi		r8, r9, #0x10, #0x10
	lsr		r9, r9, #0x10
	bfi		r9, r10, #0x10, #0x10
	lsr		r10, r10, #0x10
	bfi		r10, r11, #0x10, #0x10
	lsr		r11, r11, #0x10
	bfi		r11, r12, #0x10, #0x10
	stmia	r0!, {r3-r11}
	lsr		r12, r12, #0x10
	strh	r12, [r0], #0x02

	sub		r2, r2, #0x28
	cmp		r2, #0x28
	bge.n	MEM_DataCopy2_2
	pop		{r4-r12}

MEM_DataCopy2_1: /* Read longs and write 2 x half words */
	cmp		r2, #4
	blt.n	MEM_DataCopyBytes
	ldr		r3, [r1], #0x04
	strh	r3, [r0], #0x02
	lsr		r3, r3, #0x10
	strh	r3, [r0], #0x02
	sub		r2, r2, #0x04
	b.n		MEM_DataCopy2

/* Bits:  Src=01, Dst=00 - Byte before half word to long
 * Bits:  Src=01, Dst=10 - Byte before half word to half word
 * 3 bytes to read for long word aligning the source
 */

MEM_DataCopy4:
MEM_DataCopy6:
	/* Read B and write B */

	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=10, Dst=01 - Half word to byte before half word
 * Bits:  Src=10, Dst=11 - Half word to byte before long word
 * 2 bytes to read for long word aligning the source
 */

MEM_DataCopy9:
MEM_DataCopy11:
	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=11, Dst=00 -chm Byte before long word to long word
 * Bits:  Src=11, Dst=11 - Byte before long word to half word
 * 1 byte to read for long word aligning the source
 */

MEM_DataCopy12:
MEM_DataCopy14:
	/* Read B and write B */

	ldrb	r3, [r1], #0x01
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x01

/* Bits:  Src=00, Dst=01 - Long to Byte before half word
 * Bits:  Src=00, Dst=11 - Long to Byte before long word
 */

MEM_DataCopy1: /* Read longs, write B->H->B */
MEM_DataCopy3:
	cmp		r2, #4
	blt		MEM_DataCopyBytes
	ldr		r3, [r1], #0x04
	strb	r3, [r0], #0x01
	lsr		r3, r3, #0x08
	strh	r3, [r0], #0x02
	lsr		r3, r3, #0x10
	strb	r3, [r0], #0x01
	sub		r2, r2, #0x04
	b.n		MEM_DataCopy3

	.size	memcpy, .-memcpy
	.end
