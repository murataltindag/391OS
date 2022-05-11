#include "idt_number.h"

# Exception_Linkage Macro
# inputs: name -- name of the assembly linkage function
#         func -- name of the exception handler
#         number -- exception number
# outputs: void
# function: Macro for assembly linkage functions for the exception handlers
#define EXC_LINK(name,func,number)   \
.GLOBL name                   ;\
name:                         ;\
    PUSHAL                    ;\
    PUSHFL                    ;\
    push $number              ;\
    call func                 ;\
    addl $4, %esp             ;\
	POPFL                     ;\
    POPAL                     ;\
	iRET

# Hardware_Linkage Macro
# inputs: name -- name of the assembly linkage function
#         func -- name of the hardware handler
#         number -- IRQ number
# outputs: void
# function: Macro for assembly linkage functions for the hardware handlers
#define HARDWARE_LINK(name,func,number)   \
.GLOBL name                   ;\
name:                         ;\
    PUSHAL                    ;\
    PUSHFL                    ;\
    push $number              ;\
    call func                 ;\
    addl $4, %esp             ;\
	POPFL                     ;\
    POPAL                     ;\
	iRET

# System_Call_Linkage Macro
# inputs: name -- name of the assembly linkage function
# outputs: void
# function: Macro for assembly linkage functions for the system call handlers
#define SYS_CALL_LINK(name)   \
.GLOBL name                   ;\
name:                         ;\
    pushl %esi                    ;\
    pushl %edi                    ;\
    PUSHL %ebx                ;\
    PUSHFL                      ;\
    PUSHL %edx                ;\
    PUSHL %ecx                ;\
    PUSHL %ebx                ;\
    cmpl $0,%eax             ;\
    jle invalid_number      ;\
    cmpl $11, %eax           ;\
    jge  invalid_number     ;\
    call *syscall_jump_table(,%eax,4) ;\
    jmp end_sys

invalid_number:     ;\
    movl $-1, %eax  ;\
    jmp end_sys

end_sys:
    addl $12, %esp          ;\
    POPFL                      ;\
    POPl %ebx                ;\
    POPl %edi                     ;\
    POPl %esi                     ;\
	iRET


// EXCEPTIONS
EXC_LINK(div_by_zero_linkage, exc_handler, div_by_zero);
EXC_LINK(single_step_linkage, exc_handler, single_step);
EXC_LINK(nmi_linkage, exc_handler, nmi);
EXC_LINK(breakpoint_linkage, exc_handler, breakpoint);
EXC_LINK(overflow_linkage, exc_handler, overflow);
EXC_LINK(bound_range_linkage, exc_handler, bound_range);
EXC_LINK(inv_opcode_linkage, exc_handler, inv_opcode);
EXC_LINK(coprocessor_na_linkage, exc_handler, coprocessor_na);
EXC_LINK(double_fault_linkage, exc_handler, double_fault);
EXC_LINK(coprocessor_seg_ovr_linkage, exc_handler, coprocessor_seg_ovr);
EXC_LINK(inv_task_linkage, exc_handler, inv_task);
EXC_LINK(seg_not_present_linkage, exc_handler, seg_not_present);
EXC_LINK(stack_segfault_linkage, exc_handler, stack_segfault);
EXC_LINK(general_protection_linkage, exc_handler, general_protection);
EXC_LINK(page_linkage, exc_handler, page);
EXC_LINK(reserved_linkage, exc_handler, reserved);
EXC_LINK(x87_float_linkage, exc_handler, x87_float);
EXC_LINK(alignment_linkage, exc_handler, alignment);
EXC_LINK(machine_linkage, exc_handler, machine);
EXC_LINK(simd_linkage, exc_handler, simd);

// HARDWARE INTERRUPTS
HARDWARE_LINK(irq0_linkage,hardware_handler,IRQ0);
HARDWARE_LINK(irq1_linkage,hardware_handler,IRQ1);
HARDWARE_LINK(irq2_linkage,hardware_handler,IRQ2);
HARDWARE_LINK(irq3_linkage,hardware_handler,IRQ3);
HARDWARE_LINK(irq4_linkage,hardware_handler,IRQ4);
HARDWARE_LINK(irq5_linkage,hardware_handler,IRQ5);
HARDWARE_LINK(irq6_linkage,hardware_handler,IRQ6);
HARDWARE_LINK(irq7_linkage,hardware_handler,IRQ7);
HARDWARE_LINK(irq8_linkage,hardware_handler,IRQ8);
HARDWARE_LINK(irq9_linkage,hardware_handler,IRQ9);
HARDWARE_LINK(irq10_linkage,hardware_handler,IRQ10);
HARDWARE_LINK(irq11_linkage,hardware_handler,IRQ11);
HARDWARE_LINK(irq12_linkage,hardware_handler,IRQ12);
HARDWARE_LINK(irq13_linkage,hardware_handler,IRQ13);
HARDWARE_LINK(irq14_linkage,hardware_handler,IRQ14);
HARDWARE_LINK(irq15_linkage,hardware_handler,IRQ15);

// SYS CALL (0x80)
SYS_CALL_LINK(sys_linkage);

# syscall_jump_table
# inputs: void
# outputs: void
# function: Jump table used by the assembly linkage function to jump to the correct system call
syscall_jump_table:
    .long   0x0000, system_halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn




