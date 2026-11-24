/* SPDX-License-Identifier: GPL-3.0-only
 * A RAM app (m-bootloader SPEC 11.8): linked at 0x01C30000, the entry at its first byte, everything (code, data,
 * .bss, our stack) below 0x01C7FD80. The loader writes only the file's bytes: .bss is cleared by our start. */
ENTRY(_start)
MEMORY { slot (rwx) : ORIGIN = 0x01C30000, LENGTH = 0x4FD80 }
SECTIONS {
  .entry : { KEEP(*(.entry)) } > slot
  .text : { *(.text*) *(.rodata*) . = ALIGN(4); } > slot
  .data : { *(.data*) *(.sdata*) . = ALIGN(4); } > slot
  __image_end = .;
  .bss (NOLOAD) : { . = ALIGN(8); __bss_start = .; *(.bss*) *(.sbss*) *(COMMON) . = ALIGN(8); __bss_end = .; } > slot
  .stack (NOLOAD) : { . = ALIGN(8); . += 0x200; __sstack_top = .; . += 0x4000; __stack_top = .; } > slot   /* (3.4.1: the interrupts' stack below) */
  ASSERT(__stack_top <= 0x01C7FD80, "the RAM app does not fit its slot")
}
