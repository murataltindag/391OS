/* idt_handler.c: state machine for interrupt handlers */

#include "idt_number.h"
#include "idt_handler.h"
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "pit.h"

extern int32_t system_halt (uint8_t status);
extern int32_t exception_halt (uint16_t status);

/* void exc_handler(int intr_num)
 * inputs: int intr_num - INT # for exception
 * outputs: void
 * Function: prints the exception name for the given INT number and freezes the system
 */
void exc_handler(int intr_num){
    uint16_t status = EXEC_STATUS;
    switch (intr_num)
    {
    case div_by_zero:
        cli();
        printf("div_by_zero\n");  //print the exception
        exception_halt(status);               //freeze
        sti();
        break;
    case single_step:
        cli();
        printf("single_step\n");
        exception_halt(status);
        sti();
        break;
    case nmi:
        cli();
        printf("nmi\n");
        exception_halt(status);
        sti();
        break;
    case breakpoint:
        cli();
        printf("breakpoint\n");
        exception_halt(status);
        sti();
        break;
    case overflow:
        cli();
        printf("overflow\n");
        exception_halt(status);
        sti();
        break;
    case bound_range:
        cli();
        printf("bound_range\n");
        exception_halt(status);
        sti();
        break;
    case inv_opcode:
        cli();
        printf("inv_opcode\n");
        exception_halt(status);
        sti();
        break;
    case coprocessor_na:
        cli();
        printf("coprocessor_na\n");
        exception_halt(status);
        sti();
        break;
    case double_fault:
        cli();
        printf("double_fault\n");
        exception_halt(status);
        sti();
        break;
    case coprocessor_seg_ovr:
        cli();
        printf("coprocessor_seg_ovr\n");
        exception_halt(status);
        sti();
        break;
    case inv_task:
        cli();
        printf("inv_task\n");
        exception_halt(status);
        sti();
        break;
    case seg_not_present:
        cli();
        printf("seg_not_present\n");
        exception_halt(status);
        sti();
        break;
    case stack_segfault:
        cli();
        printf("stack_segfault\n");
        exception_halt(status);
        sti();
        break;
    case general_protection:
        cli();
        printf("general_protection\n");
        exception_halt(status);
        sti();
        break;
    case page:
        cli();
        printf("page\n");
        exception_halt(status);
        sti();
        break;
    case reserved:
        cli();
        printf("reserved\n");
        exception_halt(status);
        sti();
        break;
    case x87_float:
        cli();
        printf("x87_float\n");
        exception_halt(status);
        sti();
        break;
    case alignment:
        cli();
        printf("alignment\n");
        exception_halt(status);
        sti();
        break;
    case machine:
        cli();
        printf("machine\n");
        exception_halt(status);
        sti();
        break;
    case simd:
        cli();
        printf("simd\n");
        exception_halt(status);
        sti();
        break;
    }
}


/* void hardware_handler(int intr_num)
 * inputs: int intr_num - IRQ # for interrupt
 * outputs: void
 * Function: calls the handler function for the given IRQ
 */
void hardware_handler(int intr_num){
    cli();
    switch (intr_num)
    {
    case IRQ0: // handle the PIT
        pit_handler();
        break;
    case IRQ1:
        keyboard_handler();  //handle the keyboard
        sti();          // start the end of interrupt routine
        send_eoi(KEYBOARD_IRQ_NUM); //send the end of interrupt signal for IRQ1
        break;
    case IRQ8:
        rtc_handler(); //handle the RTC
        sti();          
        send_eoi(RTC_PIC_PIN); 
        break;
    }
}



