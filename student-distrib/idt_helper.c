   
#include "lib.h"
#include "x86_desc.h"  
#include "idt_handler.h"
#include "idt_number.h"
#define INT_START 32
#define INT_END 47
#define USER_LEVEL_PRIORITY 3
   
/* void init_idt(void);
 * Inputs: void
 * Return Value: void
 *  Function: Construct an IDT entry */
void init_idt(){
    int i;
    for(i=0; i<NUM_VEC; i++){    
        idt[i].seg_selector = KERNEL_CS; // what's going to be the segment sel
        //idt[i].reserved4; // why are there 5 reserved 
        idt[i].reserved3 = 1; // 1 for TRAP, 0 for interrupt
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1 ;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0; //Kernel priority level is 0
        idt[i].present = 1;
    }

    for(i=INT_START; i<=INT_END; i++) {
        idt[i].seg_selector = KERNEL_CS; // what's going to be the segment sel
        //idt[i].reserved4; // why are there 5 reserved 
        idt[i].reserved3 = 0; // 1 for TRAP, 0 for interrupt
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1 ;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0; //Kernel priority level is 0
        idt[i].present = 1;
    }

    idt[SYS_CALL].seg_selector = KERNEL_CS; // what's going to be the segment sel
    //idt[i].reserved4; // why are there 5 reserved 
    idt[SYS_CALL].reserved3 = 1; // 1 for TRAP, 0 for interrupt
    idt[SYS_CALL].reserved2 = 1;
    idt[SYS_CALL].reserved1 = 1;
    idt[SYS_CALL].size = 1 ;
    idt[SYS_CALL].reserved0 = 0;
    idt[SYS_CALL].dpl = USER_LEVEL_PRIORITY; //User priority level is 3 for system call
    idt[SYS_CALL].present = 1;    
}

/* void populate_idt(void);
 * Inputs: void
 * Return Value: void
 *  Function: Populate the IDT entry */
void populate_idt(){
    // EXCEPTIONS
    SET_IDT_ENTRY(idt[div_by_zero], div_by_zero_linkage);
    SET_IDT_ENTRY(idt[single_step], single_step_linkage);
    SET_IDT_ENTRY(idt[nmi], nmi_linkage);
    SET_IDT_ENTRY(idt[breakpoint], breakpoint_linkage);
    SET_IDT_ENTRY(idt[overflow], overflow_linkage);
    SET_IDT_ENTRY(idt[bound_range], bound_range_linkage);
    SET_IDT_ENTRY(idt[inv_opcode], inv_opcode_linkage);
    SET_IDT_ENTRY(idt[coprocessor_na], coprocessor_na_linkage);
    SET_IDT_ENTRY(idt[double_fault], double_fault_linkage);
    SET_IDT_ENTRY(idt[coprocessor_seg_ovr], coprocessor_seg_ovr_linkage);
    SET_IDT_ENTRY(idt[inv_task], inv_task_linkage);
    SET_IDT_ENTRY(idt[seg_not_present], seg_not_present_linkage);
    SET_IDT_ENTRY(idt[stack_segfault], stack_segfault_linkage);
    SET_IDT_ENTRY(idt[general_protection], general_protection_linkage);
    SET_IDT_ENTRY(idt[page], page_linkage);
    SET_IDT_ENTRY(idt[reserved], reserved_linkage);
    SET_IDT_ENTRY(idt[x87_float], x87_float_linkage);
    SET_IDT_ENTRY(idt[alignment], alignment_linkage);
    SET_IDT_ENTRY(idt[machine], machine_linkage);
    SET_IDT_ENTRY(idt[simd], simd_linkage);

    // HARDWARE INTERRUPTS
    SET_IDT_ENTRY(idt[IRQ0], irq0_linkage);
    SET_IDT_ENTRY(idt[IRQ1], irq1_linkage);
    SET_IDT_ENTRY(idt[IRQ2], irq2_linkage);
    SET_IDT_ENTRY(idt[IRQ3], irq3_linkage);
    SET_IDT_ENTRY(idt[IRQ4], irq4_linkage);
    SET_IDT_ENTRY(idt[IRQ5], irq5_linkage);
    SET_IDT_ENTRY(idt[IRQ6], irq6_linkage);
    SET_IDT_ENTRY(idt[IRQ7], irq7_linkage);
    SET_IDT_ENTRY(idt[IRQ8], irq8_linkage);
    SET_IDT_ENTRY(idt[IRQ9], irq9_linkage);
    SET_IDT_ENTRY(idt[IRQ10], irq10_linkage);
    SET_IDT_ENTRY(idt[IRQ11], irq11_linkage);
    SET_IDT_ENTRY(idt[IRQ12], irq12_linkage);
    SET_IDT_ENTRY(idt[IRQ13], irq13_linkage);
    SET_IDT_ENTRY(idt[IRQ14], irq14_linkage);
    SET_IDT_ENTRY(idt[IRQ15], irq15_linkage);

    // SYSTEM CALL (0x80)
    SET_IDT_ENTRY(idt[SYS_CALL], sys_linkage);
}


