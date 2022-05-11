.globl enable_paging
.globl flush_tlb

# flush tlb
# inputs: void
# outputs: void
# function: Flushes translation lookaside buffer by writing to page directory base register
flush_tlb:
    movl	%cr3,%eax
	movl	%eax,%cr3
    RET
    
# enable_paging 
# inputs: void
# outputs: void
# function: Enables paging by modifying the control registers

enable_paging:
    pushl %ebp
    movl %esp,%ebp 

    # Load page directory
    movl 8(%ebp), %edx               
    movl %edx, %cr3            
    
    # Enable 4MB pages and Global flag
    movl %cr4, %edx           
    orl $0x00000090, %edx      
    movl %edx, %cr4   

    # Enable paging
    movl %cr0, %edx           
    orl $0x80000001, %edx      
    movl %edx, %cr0   
    

    LEAVE
    RET
