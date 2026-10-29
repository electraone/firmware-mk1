/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 1024K
	RAM  (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 256K
}


SECTIONS
{
	.text : {
		. = 0;
		KEEP(*(.vectors))
		*(.startup*)
		/* TODO: does linker detect startup overflow onto flashconfig? */
		. = 0x400;
		KEEP(*(.flashconfig*))
		*(.text*)
		*(.rodata*)
		. = ALIGN(4);
		KEEP(*(.init))
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	} > FLASH = 0xFF

	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > FLASH
	_etext = .;

	.usbdescriptortable (NOLOAD) : {
		/* . = ORIGIN(RAM); */
		. = ALIGN(512);
		*(.usbdescriptortable*)
	} > RAM

	.dmabuffers (NOLOAD) : {
		. = ALIGN(4);
		*(.dmabuffers*)
	} > RAM

	.usbbuffers (NOLOAD) : {
		. = ALIGN(4);
		*(.usbbuffers*)
	} > RAM

	.data : AT (_etext) {
		. = ALIGN(4);
		_sdata = .; 
		*(.fastrun*)
		*(.data*)
		. = ALIGN(4);
		_edata = .; 
	} > RAM

	.noinit (NOLOAD) : {
		*(.noinit*)
	} > RAM

	/* 0x20000000 is where SRAM_L ends and SRAM_U begins, and an access
	 * spanning the two hard faults - a memcpy or a struct copy into an object
	 * laid across it. Laid out in one run, which object landed on it depended
	 * only on how much static data preceded it, so any change anywhere could
	 * move a buffer there.
	 *
	 * So SRAM_L takes .data and the buffers named .bss.lowram, chosen to
	 * nearly fill it, and the rest of .bss starts at the boundary, where
	 * nothing can straddle it. Only the .bss.lowram list can, by outgrowing
	 * SRAM_L, and tools/check-sram-boundary.py fails the link if it does.
	 * What the list leaves of SRAM_L is a gap below the boundary that holds
	 * nothing; keep it small. The startup code zeroes _sbss to _ebss, gap
	 * included. */
	.bss : {
		. = ALIGN(4);
		_sbss = .;
		__bss_start__ = .;
		*(.bss.lowram*)
		. = ALIGN(4);
		_slowram_end = .;
	} > RAM

	.bss.upper MAX(., 0x20000000) (NOLOAD) : {
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
		__bss_end = .;
		__bss_end__ = .;
	} > RAM

	_estack = ORIGIN(RAM) + LENGTH(RAM);
}




