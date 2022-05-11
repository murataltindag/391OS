#ifndef INTR_LINKAGE_H
#define INTR_LINKAGE_H

// EXCEPTIONS
#define div_by_zero 0
#define single_step 1
#define nmi 2
#define breakpoint 3
#define overflow 4
#define bound_range 5
#define inv_opcode 6
#define coprocessor_na 7
#define double_fault 8
#define coprocessor_seg_ovr 9
#define inv_task 10
#define seg_not_present 11
#define stack_segfault 12
#define general_protection 13
#define page 14
#define reserved 15
#define x87_float 16
#define alignment 17
#define machine 18
#define simd 19

// HARDWARE INTERRUPTS
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

// SYSTEM CALL (0x80)
#define SYS_CALL 128


#endif
