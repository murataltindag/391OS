
#define ASM     1
#include "x86_desc.h"

.globl  context_switch

# context_switch
# inputs: entry_point  -- bytes 24-27 of the copied user program
# outputs: void
# function: Hands off the processor to the new program until it terminates
context_switch:

    movl 4(%esp), %ebx
    # push SS
    pushl $USER_DS

    # push ESP
    pushl $0x83FFFFC # User stack pointer at 132MB - 4

    # push EFLAGS
    pushfl
    popl %edx
    orl $0x0200, %edx # Enable interrupts
    push %edx

    # push CS
    pushl $USER_CS

    # push EIP
    pushl %ebx
    
    IRET





