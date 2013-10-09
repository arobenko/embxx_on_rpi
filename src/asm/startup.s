;@
;@ Copyright 2013 (C). Alex Robenko. All rights reserved.
;@

;@ This file is free software: you can redistribute it and/or modify
;@ it under the terms of the GNU General Public License as published by
;@ the Free Software Foundation, either version 3 of the License, or
;@ (at your option) any later version.
;@
;@ This program is distributed in the hope that it will be useful,
;@ but WITHOUT ANY WARRANTY; without even the implied warranty of
;@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;@ GNU General Public License for more details.
;@
;@ You should have received a copy of the GNU General Public License
;@ along with this program.  If not, see <http://www.gnu.org/licenses/>.

.extern __stack;
.extern main
.extern interruptHandler

	.section .init
	.globl _start

_start:

    ldr pc,reset_handler_ptr        ;@  Processor Reset handler
    ldr pc,undefined_handler_ptr    ;@  Undefined instruction handler
    ldr pc,swi_handler_ptr          ;@  Software interrupt
    ldr pc,prefetch_handler_ptr     ;@  Prefetch/abort handler.
    ldr pc,data_handler_ptr         ;@  Data abort handler/
    ldr pc,unused_handler_ptr       ;@
    ldr pc,irq_handler_ptr          ;@  IRQ handler
    ldr pc,fiq_handler_ptr          ;@  Fast interrupt handler.

    ;@ Set the branch addresses
reset_handler_ptr:      .word reset
undefined_handler_ptr:  .word hang
swi_handler_ptr:        .word hang
prefetch_handler_ptr:   .word hang
data_handler_ptr:       .word hang
unused_handler_ptr:     .word hang
irq_handler_ptr:        .word irq_handler
fiq_handler_ptr:        .word hang

reset:
    ;@ Disable interrupts
    cpsid if

    ;@ Copy interrupt vector to its place
    ldr r0,=_start
    mov r1,#0x0000

    ;@  Here we copy the branching instructions
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}

    ;@  Here we copy the branching addresses
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}

    ;@ Set interrupt stacks
    ldr r0,=__stack;
    mov r1,#0x1000 ;@ interrupt stacks have 4K size

    ;@ FIQ mode
    cps 0x11
    mov sp,r0
    sub r0,r0,r1

    ;@ IRQ mode
    cps 0x12
    mov sp,r0
    sub r0,r0,r1

    ;@ Supervisor mode with disabled interrupts
    cpsid if,0x13
    mov sp,r0

    bl main
    b reset ;@ restart if main function returns

    .section .text

hang:
	b hang

irq_handler:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    bl interruptHandler
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    subs pc,lr,#4

