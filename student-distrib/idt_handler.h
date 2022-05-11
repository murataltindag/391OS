/* idt_handler.c: defines assembly linkage functions for the idt and interrupt handlers */

#ifndef IDT_HANDLER_H
#define IDT_HANDLER_H

#include "types.h"


#define EXEC_STATUS 256

void exc_handler(int intr_num);
void hardware_handler(int intr_num);
// int32_t sys_call_handler(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx);

// EXCEPTIONS
extern void div_by_zero_linkage(void);
extern void single_step_linkage(void);
extern void nmi_linkage(void);
extern void breakpoint_linkage(void);
extern void overflow_linkage(void);
extern void bound_range_linkage(void);
extern void inv_opcode_linkage(void);
extern void coprocessor_na_linkage(void);
extern void double_fault_linkage(void);
extern void coprocessor_seg_ovr_linkage(void);
extern void inv_task_linkage(void);
extern void seg_not_present_linkage(void);
extern void stack_segfault_linkage(void);
extern void general_protection_linkage(void);
extern void page_linkage(void);
extern void reserved_linkage(void);
extern void x87_float_linkage(void);
extern void alignment_linkage(void);
extern void machine_linkage(void);
extern void simd_linkage(void);

// HARDWARE INTERRUPTS
extern void irq0_linkage(void);
extern void irq1_linkage(void);
extern void irq2_linkage(void);
extern void irq3_linkage(void);
extern void irq4_linkage(void);
extern void irq5_linkage(void);
extern void irq6_linkage(void);
extern void irq7_linkage(void);
extern void irq8_linkage(void);
extern void irq9_linkage(void);
extern void irq10_linkage(void);
extern void irq11_linkage(void);
extern void irq12_linkage(void);
extern void irq13_linkage(void);
extern void irq14_linkage(void);
extern void irq15_linkage(void);

// SYSTEM CALL (0x80)
extern void sys_linkage(void);

#endif
