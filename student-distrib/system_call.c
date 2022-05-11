/* system_call.c - Functions to call with INT 0x80
 */
#include "x86_desc.h"
#include "filesys.h"
#include "rtc.h"
#include "system_call.h"
#include "lib.h"
#include "terminal.h"
#include "scheduling.h"
#include "i8259.h"

#define FIRST_TERMINAL_BUF (0xB8000 + 4096) 
#define SECOND_TERMINAL_BUF (0xB8000 + 4096 * 2) 
#define THIRD_TERMINAL_BUF (0xB8000 + 4096 * 3)

/* file operations tables for different types of files
 *  stdin_fop  -- read-only terminal
 *  stdout_fop -- write-only terminal
 *  rtc_fop -- real-time clock files
 *  directory_fop -- directories
 *  file_fop -- regular files
 */
fop_table_t stdin_fop = {
    terminal_open, 
    terminal_close,
    terminal_read, 
    invalid_terminal_write 
};
fop_table_t stdout_fop = {
    terminal_open,
    terminal_close,
    invalid_terminal_read,
    terminal_write
};
fop_table_t rtc_fop = {
    rtc_open,
    rtc_close,
    rtc_read,
    rtc_write
};
fop_table_t directory_fop = {
    directory_open,
    directory_close,
    directory_read,
    directory_write
};
fop_table_t file_fop = {
    file_open,
    file_close,
    file_read,
    file_write
};


/* int32_t exception_halt(uint16_t status);
 * Inputs: uint8_t status
 * Return Value: uint32_t status_32_bit
 *            == -1 terminated abnormally
 *            == 0-255 regular halt
 *            == 256 exception    
 *  Function: The halt system call terminates a process, returning the specified value to its parent process */

int32_t exception_halt (uint16_t status){
    cli();
    int i;
    process_count--;
    saved_status_num = status;
    pcb_struct* current_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1)); // get ptr to parent pcbb

    // Restore the TSS and paging
    if(current_pcb->is_base_shell == 0 ){     //check for shell
        tss.esp0 = EIGHT_MB - EIGHT_KB*(current_pcb->parent_id) - FOUR_BYTES; // restore parent's kernel-mode stack
        tss.ss0 = KERNEL_DS; //pointer to kernel’s stack segment
        paging_execute(current_pcb->parent_id);       // restore parent program file
        disable_child_page();   // disable child program page
    }else{
        shell_halt_flag = 1;
        tss.ss0 = KERNEL_DS; //pointer to kernel’s stack segment
        tss.esp0 = EIGHT_MB - EIGHT_KB*(current_pcb->pid) - FOUR_BYTES; // restore parent's kernel-mode stack
    }
    file_descriptor* curr_fd = current_pcb->file_descriptor;   // get ptr to current fd

    // close open fds
    for(i = 0; i < FD_MAX; i++) {
        if(curr_fd[i].flags == 1) curr_fd[i].file_operations_table_pointer->close(i);
    }
    
    if(current_pcb->is_base_shell == 0  ){     //If not base shell case
        pcb_struct* parent_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(current_pcb->parent_id+1));     // get parent process pcb
        schedule_array[current_pcb->terminal_num] = parent_pcb->pid;                                // update queue       
        terminal[current_pcb->terminal_num].curr_pid = current_pcb->parent_id;        
        current_pcb->active = 0;
        parent_pcb->active = 1; 
        process_num = current_pcb->parent_id;  

        if(current_pcb->is_shell == 1){
            shell_process_count--;
        }
        //restore esp and ebp, move into eax
        asm volatile ("movl %0, %%esp;" 
                        :                /* output */
                        :"r"(current_pcb->saved_parents_esp)  /* input */
                        :"%eax"          /* clobbered register */
                        );      
        asm volatile ("movl %0, %%ebp;"  
                        :                 /* output */
                        :"r"(current_pcb->saved_parents_ebp)   /* input */
                        :"%eax"           /* clobbered register */
                        ); 
        asm volatile ("movl %0, %%eax;\n\
                        jmp return_to_exe;"
                        :                /* output */
                        :"r"(saved_status_num)  /* input */
                        :"%eax"          /* clobbered register */
                        );        
    }else{ // Base shell case
        base_shell_id = current_pcb->pid;
        asm volatile ("movl %0, %%eax;\n\
                       movl %1, %%ebx;\n\
                        jmp shell_halt;"
                        :                /* output */
                        :"r"(0), "r"(current_pcb->entry_point)  /* input */
                        : "%eax", "%ebx"         /* clobbered register */
                        );  
    }
    return (uint32_t)status;
}

/* int32_t system_halt(uint8_t status);
 * Inputs: uint8_t status
 * Return Value: uint32_t status_32_bit
 *            == -1 terminated abnormally
 *            == 0-255 regular halt
 *            == 256 exception    
 *  Function: The halt system call terminates a process, returning the specified value to its parent process */
int32_t system_halt (uint8_t status){
    cli();
    int i;
    process_count--;
    saved_status_num = status;
    pcb_struct* current_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1)); // get ptr to parent pcbb

    // Restore the TSS and paging
    if(current_pcb->is_base_shell == 0 ){     //check for shell
        tss.esp0 = EIGHT_MB - EIGHT_KB*(current_pcb->parent_id) - FOUR_BYTES; // restore parent's kernel-mode stack
        tss.ss0 = KERNEL_DS; //pointer to kernel’s stack segment
        paging_execute(current_pcb->parent_id);       // restore parent program file
        disable_child_page();   // disable child program page
    }else{
        shell_halt_flag = 1;
        tss.ss0 = KERNEL_DS; //pointer to kernel’s stack segment
        tss.esp0 = EIGHT_MB - EIGHT_KB*(current_pcb->pid) - FOUR_BYTES; // restore parent's kernel-mode stack
    }
    file_descriptor* curr_fd = current_pcb->file_descriptor;   // get ptr to current fd

    // close open fds
    for(i = 0; i < FD_MAX; i++) {
        if(curr_fd[i].flags == 1) curr_fd[i].file_operations_table_pointer->close(i);
    }
    
    if(current_pcb->is_base_shell == 0  ){     //If not base shell case
        pcb_struct* parent_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(current_pcb->parent_id+1));     // get parent process pcb
        schedule_array[current_pcb->terminal_num] = parent_pcb->pid;                                // update queue       
        terminal[current_pcb->terminal_num].curr_pid = current_pcb->parent_id;        
        current_pcb->active = 0;
        parent_pcb->active = 1; 
        process_num = current_pcb->parent_id;  

        if(current_pcb->is_shell == 1){
            shell_process_count--;
        }
        //restore esp and ebp, move into eax
        asm volatile ("movl %0, %%esp;" 
                        :                /* output */
                        :"r"(current_pcb->saved_parents_esp)  /* input */
                        :"%eax"          /* clobbered register */
                        );      
        asm volatile ("movl %0, %%ebp;"  
                        :                 /* output */
                        :"r"(current_pcb->saved_parents_ebp)   /* input */
                        :"%eax"           /* clobbered register */
                        ); 
        asm volatile ("movl %0, %%eax;\n\
                        jmp return_to_exe;"
                        :                /* output */
                        :"r"(saved_status_num)  /* input */
                        :"%eax"          /* clobbered register */
                        );        
    }else{ // Base shell case
        base_shell_id = current_pcb->pid;
        asm volatile ("movl %0, %%eax;\n\
                       movl %1, %%ebx;\n\
                        jmp shell_halt;"
                        :                /* output */
                        :"r"(0), "r"(current_pcb->entry_point)  /* input */
                        : "%eax", "%ebx"         /* clobbered register */
                        );  
    }
    return (uint32_t)status;
}

/* int32_t execute(const uint8_t* command);
 * Inputs: const uint8_t* command  -- command to be executed
 * Return Value: uint32_t status
 *            == -1 terminated abnormally
 *            == 0-255 regular halt
 *            == 256 exception    
 *  Function: The execute system call attempts to load and execute a new program, handing off the processor to the new program
 * until it terminates. */
int32_t execute (const uint8_t* command){
    cli(); 

    // get args 
    parse_args(command);

    // get file name
    const uint8_t* file_name = get_file_name(command);
    shell_halt_flag = 0;
    uint8_t is_shell_flag = 0;

    // Check if we are on the 6th shell and limit to that
    if(!strncmp((const int8_t*)file_name, "shell", FIVE_BYTES)){
        if(shell_process_count >= SHELL_LIMIT){
            printf("Already at 6th Shell! Can't open more \n");
            asm volatile ("jmp return_to_exe;"
                            :                /* output */
                            :  /* input */
                            : "%eax"     /* clobbered register */
                            );  
        }else{
            is_shell_flag = 1;
            shell_process_count++;
        }
    }

    // For Cat program, set flags
    if(!strncmp((const int8_t*)file_name, "cat", CAT_STRLEN)){
        cat_flag[schedule_idx] = 1;
    }
    else{
        cat_flag[schedule_idx] = 0;
    }
    
    // check if file exists/can be found
    dentry_t curr_dentry;
    if(read_dentry_by_name(file_name, &curr_dentry) == -1){
        return FAIL_NEG_ONE;
    }

    // check if file is executable
    uint8_t buffer[EXC_META];
    uint32_t entry_point;
    read_data(curr_dentry.inode_num, 0, buffer, FOURTY_BYTES); // read first forty bytes of program
    if(is_executable(buffer) == 0) {
        memcpy(&entry_point, &buffer[ENTRY_POINT_BYTE], FOUR_BYTES); // get entry point of program (bytes 24-27, which is 4 bytes)
    }
    else{
        return FAIL_NEG_ONE;      //if not executable, return -1
    } 

    // Find available PCB to allocate a new one to
    int32_t temp_process_num = process_num;
    pcb_struct* next_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(temp_process_num+TWO));  // check if the next pcb is active
    while((next_pcb->active)&&(temp_process_num<MAX_PCB)){ // there can be at most 512 processes
        temp_process_num++;
        next_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(temp_process_num+TWO));
    }

    // If reaches maximum PCB
    if (temp_process_num == MAX_PCB){
         asm volatile ("jmp return_to_exe;"
                        :                /* output */
                        :                /* input */
                        :"%eax"          /* clobbered register */
                        );      
    }
    int32_t local_process_num = temp_process_num+1;

    // Set up paging for current program
    paging_execute(local_process_num);

    // Load program into physical memory
    //128 MB address + 288 KB offset
    read_data(curr_dentry.inode_num, 0, (uint8_t*)EXECUTE_ADDR, ((inodes_struct_ptr + curr_dentry.inode_num))->length); 

    asm volatile ("movl %0, %%eax; \n\
                    movl %1, %%ebx;"
                    :                /* output */
                    :"r"(local_process_num), "r" (entry_point)  /* input */
                    :"%eax", "%ebx"           /* clobbered register */
                    );  
    asm volatile ("shell_halt:" 
                    :  /* output */
                    :                 /* input */
                    :  "%eax", "%ebx"         /* clobbered register */
    );        
    asm volatile ("movl %%eax, %0; \n\
                    movl %%ebx, %1;" 
                    :"=r"(local_process_num), "=r"(entry_point)  /* output */
                    :                /* input */
                    :"%eax", "%ebx"         /* clobbered register */
                    );    

    // Create PCBs and Open FDs
    pcb_struct* pcb;
    if(shell_halt_flag == 1){
        local_process_num = base_shell_id; 
        pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(local_process_num+1));
        entry_point = pcb->entry_point;
        pcb->is_base_shell = 1;
        pcb->parent_id = -1;
        shell_halt_flag = 0; //reset to 0
    }else{
        pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(local_process_num+1));
        pcb->entry_point = entry_point;
    }

    if(pcb->parent_id != -1){
        pcb->parent_id = process_num; //current process numb 
    }

    // setup the PCB for the general case
    pcb->pid = local_process_num; // zero-indexed
    pcb->active = 1; // activated process, useful for later checkpoints
    asm volatile ("movl %%esp, %0;" 
                    :"=r"(pcb->saved_parents_esp)  /* output */
                    :                /* input */
                    :"%eax"          /* clobbered register */
                    );      
    asm volatile ("movl %%ebp, %0;"  
                    :"=r"(pcb->saved_parents_ebp)   /* output */
                    :                 /* input */
                    :"%eax"           /* clobbered register */
                    );      
    pcb->file_descriptor[0].file_operations_table_pointer = &stdin_fop; //manually open stdin
    pcb->file_descriptor[0].flags = 1;
    pcb->file_descriptor[1].file_operations_table_pointer = &stdout_fop; //manually open stdout
    pcb->file_descriptor[1].flags = 1;
    if(is_shell_flag == 1){
        pcb->is_shell = 1;
    }else{
        pcb->is_shell = 0;
    }

    // setup the pcb for the first three terminals and update scheduler array
    if(pit_count < PROCESS_NUMBER_PIT){ // first three calls
        pcb->terminal_num = pit_count;
        pcb->is_base_shell = 1;
        pcb->parent_id = -1;
        schedule_array[pcb->terminal_num] = pit_count;
        terminal[pit_count].curr_pid = schedule_array[pit_count];
        pit_count++;
    }else{
        if(pcb->is_base_shell != 1){
            pcb_struct* parent_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(pcb->parent_id+1));     // get parent process pcb
            pcb->terminal_num = parent_pcb->terminal_num;                                       // set new process terminal_num
        }else{
            pcb->terminal_num = pcb->terminal_num;
        }
        schedule_array[pcb->terminal_num] = local_process_num;                              // update queue               
        terminal[pcb->terminal_num].curr_pid = schedule_array[pcb->terminal_num];
    }

    // Context Switch and IRET
    tss.ss0 = KERNEL_DS; //pointer to kernel’s stack segment
    tss.esp0 = EIGHT_MB - EIGHT_KB*(local_process_num) - FOUR_BYTES; //pointer to the process’s kernel-mode stack,

    // increment process_num
    process_num = local_process_num;
    context_switch(pcb->entry_point); //entry point from bytes 24-27

    asm volatile ("return_to_exe: \n\
                    leave; \n\
                    ret;"  
                    :  /* output */
                    :                 /* input */
                    :  "%eax"         /* clobbered register */
    );    

    return 0;
}


/* const uint8_t* parse_args(const uint8_t* command);
 * Inputs: const uint8_t* command  -- command to be executed
 * Return Value: const uint8_t* args   
 *  Function: Parsing the arguements for execute to use
 * until it terminates. */
void parse_args(const uint8_t* command){ 
    uint8_t* args = (uint8_t*)command;
    uint32_t idx = 0;
    while (*args == ' ') args++;              // skip front spaces
    while (*args != ' '){    // skip till the argument
        if(*args == '\0'){ // if there is no argument after the command
            memset(stored_buf, 0, 1024);
            return;
        }
        args++;
    }

    args++; // element after space is arg
    while(*args != '\0' && (idx<BUFSIZE)){
        stored_buf[idx] = *args;
        args++;
        idx++;
    }
    if(idx<BUFSIZE-1){ // put \0 in buffer too
        stored_buf[idx] = *args; 
    }
    
    return;
}

/* const uint8_t* get_file_name(const uint8_t* command);
 * Inputs: const uint8_t* command  -- command to be executed
 * Return Value: const uint8_t* file_name  
 *  Function: Gets the file name that corresponds to the command
 * until it terminates. */
const uint8_t* get_file_name(const uint8_t* command){
    const uint8_t* file_name = command;
    const uint8_t* ptr = file_name;
    while (*ptr == ' ') ptr++;              // skip front spaces
    while (*ptr != ' ' && *ptr != '\0' && *ptr != '\n'){    // skip over cmd
        ptr++;
    }
    memset((void*)ptr, '\0', 1);            // put EOS after cmd
    return file_name;
}

/* int32_t is_executable(uint8_t* buffer);
 * Inputs:  uint8_t* buffer  -- physical memory buffer
 * Return Value: 0 -- if the file is executable
 *               -1 -- if it is not   
 *  Function: checks bytes 24-27 of the file to see if the file is executable
 */
int32_t is_executable(uint8_t* buffer) {
    uint32_t magic_num_0 = buffer[0]; //byte 24
    uint32_t magic_num_1 = buffer[1]; //byte 25
    uint32_t magic_num_2 = buffer[2]; //byte 26
    uint32_t magic_num_3 = buffer[3]; //byte 27
    if((magic_num_0 == MAGIC_NUM_ZERO) && (magic_num_1 == MAGIC_NUM_ONE) && (magic_num_2 == MAGIC_NUM_TWO) && (magic_num_3 == MAGIC_NUM_THREE)){ //check for executable magic numbers
        return 0;     //file executable
    } else return -1; // file not executable
}

/* void disable_child_page(void);
 * Inputs:  uint8_t* buffer  -- physical memory buffer
 * Return Value: none 
 *  Function: During halt, the assigned page for the program is set to be non-present
 */
void disable_child_page() { //TODO: pass in a process num variable (not the global one)
    pd_entry_page page_directory;
    uint32_t physical_memory_addr = EIGHT_BYTES + ((process_num+1) * FOUR_BYTES);
    uint32_t phys_mem_val = physical_memory_addr/FOUR_BYTES;

    /* Initialize pd entry for user programs */
    page_directory.present         = 0; // PDE does exist
    page_directory.r_w             = 0; // 0 for read/write
    page_directory.u_s             = 0; //supervisor only
    page_directory.pwt             = 0; //caching is writeback
    page_directory.pcd             = 0; // page cach is enabled (enabled for kernel pages, program pages)
    page_directory.accessed        = 0; // access bit, not used in mp3
    page_directory.page_size       = 0; // 0 for page table page directory entry
    page_directory.global          = 0; // Only Kernel should be set to 1 (shared by other processes)
    page_directory.avl             = 0; // Not used for us
    page_directory.dirty           = 0; // 1 if page matched by this PTE has been written to
    page_directory.pat             = 0; // Page Attribute Table index, not used
    page_directory.reserved_21_13  = 0; // Not used
    page_directory.page_base_31_22 = phys_mem_val; // Starts at 8MB, 2^10 bits is 1024 so 4GB/1024 = 4MB
    page_dir[phys_mem_val] = page_directory.val;
}

/* void paging_execute(int32_t local_process_num);
 * Inputs:  int32_t local_process_num
 * Return Value: none 
 *  Function: During execute, the paging is set up for the file to be executed
 */
void paging_execute(int32_t local_process_num){
    pd_entry_page page_directory;
    uint32_t physical_memory_addr = EIGHT_BYTES + (local_process_num * FOUR_BYTES);
    uint32_t phys_mem_val = physical_memory_addr/FOUR_BYTES;

    /* Initialize pd entry for user programs */
    page_directory.present         = 1; // PDE does exist
    page_directory.r_w             = 1; // 0 for read/write
    page_directory.u_s             = 1; // user 
    page_directory.pwt             = 0; //caching is writeback
    page_directory.pcd             = 1; // page cach is enabled (enabled for kernel pages, program pages)
    page_directory.accessed        = 0; // access bit, not used in mp3
    page_directory.page_size       = 1; // 0 for page table page directory entry
    page_directory.global          = 0; // Only Kernel should be set to 1 (shared by other processes)
    page_directory.avl             = 0; // Not used for us
    page_directory.dirty           = 0; // 1 if page matched by this PTE has been written to
    page_directory.pat             = 0; // Page Attribute Table index, not used
    page_directory.reserved_21_13  = 0; // Not used
    page_directory.page_base_31_22 = phys_mem_val; // Starts at 8MB, 2^10 bits is 1024 so 4GB/1024 = 4MB
    page_dir[ONE_TWENTLY_EIGHT_MB/FOUR_MEGABYTES] = page_directory.val;

    flush_tlb();
}

/* int32_t read(int32_t fd, void* buf, int32_t n);
 * Inputs: int32_t fd  -- file descriptor number
 *         void *buf  -- buffer
 *         int32_t n  -- number of bytes to be read
 * Return Value: -1 -- read failed
 *          n -- number of bytes read   
 *  Function: The read system call reads data from the keyboard, a file, device (RTC), or directory
 */
int32_t read (int32_t fd, void* buf, int32_t n){
    cli();
    disable_irq(0); // Disable PIT to avoid interrupt
    if(fd != 0){
        enable_irq(0);
    }   
    cli();
    
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(terminal[schedule_idx].curr_pid +1));   
    // pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));  // Create PCB
    int32_t bytes_read;
    if(fd < FD_STDIN || fd > FD_MAX || buf == NULL || n < 0 || pcb->file_descriptor[fd].flags == 0){  //Check for bad input
        sti();
        return FAIL_NEG_ONE;
    }
    
    bytes_read = pcb->file_descriptor[fd].file_operations_table_pointer->read(fd, buf, n); //make pcb fop point to the corresponding table's read function
    if(bytes_read == 0){
        sti();
        return 0;
    }else if (bytes_read == -1){
        sti();
        return -1;
    }
    else {
        sti();
        return n;
    }
}

/* int32_t write(int32_t fd, void* buf, int32_t n);
 * Inputs: int32_t fd  -- file descriptor number
 *         void *buf  -- buffer
 *         int32_t n  -- number of bytes to be written
 * Return Value: -1 -- write failed
 *          n -- number of bytes written  
 *  Function: The write system call writes data to the terminal or to a device (RTC).
 */
int32_t write (int32_t fd, const void* buf, int32_t n){
    cli();
    
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(terminal[schedule_idx].curr_pid +1));   
    // pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));  // Create PCB

    if(fd < FD_STDIN || fd > FD_MAX || buf == NULL || n < 0 || pcb->file_descriptor[fd].flags == 0){ //Check for bad input
        // printf("ERROR: Invalid input\n");
        sti();
        return FAIL_NEG_ONE;
    }
    int32_t write_success;
    write_success = pcb->file_descriptor[fd].file_operations_table_pointer->write(fd, buf, n);//make pcb fop point to the corresponding table's write function
    if (write_success == FAIL_NEG_ONE){
        // printf("ERROR: It's a read only system\n");
        sti();
        return FAIL_NEG_ONE;
    }
    sti();
    return n;
}

/* int32_t open(const uint8_t* filename);
 * Inputs: const uint8_t* filename -- name of the file to be opened
 * Return Value: -1 -- open failed
 *          fd -- file descriptor number that was opened
 *  Function: The open system call provides access to the file system.
 */
int32_t open (const uint8_t* filename){
    cli();
    // pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1)); //Create PCB
    int8_t open_success;
    dentry_t dentry;

    if((char*)filename == NULL){    //check for NULL input
        sti();
        return FAIL_NEG_ONE;
    }
    if(read_dentry_by_name(filename, &dentry) == -1){ // If the named file does not exist
        sti();
        return FAIL_NEG_ONE; 
    }

    if(dentry.file_type == RTC){       //open the right file by setting up the appropriate fop table
       open_success = rtc_open(filename);//rtc
    }
    else if(dentry.file_type == DIRECTORY){
       open_success = directory_open(filename);//directory
    }
    else if(dentry.file_type == FILE){
       open_success = file_open(filename);//regular
    }
    sti();
    return open_success;
}

/* int32_t close(int32_t fd);
 * Inputs: int32_t fd -- file descriptor number to be closed
 * Return Value: -1 -- close failed
 *          0 -- close successful
 *  Function: The close system call closes the specified file descriptor and makes it available for return from later calls to open
 */
int32_t close (int32_t fd){
    cli();
    // pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1)); //Create PCB
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(terminal[schedule_idx].curr_pid +1));   
    
    if(pcb->file_descriptor[fd].flags == 0 || fd < FD_MIN || fd > FD_MAX){        // Check for bad input
        sti();
        return FAIL_NEG_ONE;
    }
    pcb->file_descriptor[fd].file_operations_table_pointer->close(fd);  //make pcb fop point to the corresponding table's close function
    sti();
    return SUCCESS;
}


/* int32_t getargs (uint8_t* buf, int32_t nbytes)
 * Inputs: none used
 * Return Value: 0
 *  Function: get passed in argument from command into the user buffer
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
    cli();
    uint32_t i = 0;
    uint32_t count = 0; 
    while(i < nbytes){  // clear any extra newlines
        if(stored_buf[i] == '\n'){
            stored_buf[i] = '\0';
        }
        i++;
    }
    uint8_t* ptr = stored_buf;
    while (*ptr != '\0'){    // get command into user buffer
        if(count < nbytes){
            buf[count] = *ptr;
            ptr++;
            count++;
        }else{
            sti();
            return -1; // if arguments do not fit in the buffer
        }
    }
    if(count == 0){
        printf("Error, No arguments! \n");
        sti();
        return -1; // if no arguments
    }

    if(count == nbytes){
        sti();
        return -1; // if argument and a terminal NULL (0-byte) do not fit in the buffer
    }else{
        buf[count] = *ptr;
    }
    sti();
    return 0; //success
}

/* int32_t vidmap(uint8_t** screen_start)
 * Inputs: memory location
 * Return Value: -1 or 0
 *  Function: 
 */
int32_t vidmap (uint8_t** screen_start){
    cli();
    // check if memory location is valid (check if address falls within address range covered by single user-level page)
    // return -1 if not valid
    // range covered by single user-level page 8 MB - 12 MB???
    int i;
    if((uint32_t)screen_start < MB_128 || (uint32_t)screen_start > ONE_THIRTY_TWO_MB){
        sti();
        return FAIL_NEG_ONE;
    } else {
        pt_entry_page vidmap_table_entry;
        vidmap_table_entry.global          = 0; //TLB is cleared when reloading CR3
        vidmap_table_entry.pat             = 0; // Page Attribute Table index, not used
        vidmap_table_entry.dirty           = 0; // 1 if page matched by this PTE has been written to
        vidmap_table_entry.accessed        = 0; // access bit, not used in mp3
        vidmap_table_entry.pcd             = 0; // page cach is disabled
        vidmap_table_entry.pwt             = 0; //caching is writeback
        vidmap_table_entry.u_s             = 0; //supervisor only
        vidmap_table_entry.r_w             = 1; // 1 for read/write
        vidmap_table_entry.avl             = 0; // Not used for us

        for(i = 0; i < KB; i++) {
                if(i == 0){ // setting vid mem at first 4kb page at 132MB
                    vidmap_table_entry.present = 1; /* Initialize video memory to present */
                    vidmap_table_entry.u_s = 1;     // user level privilege
                    vidmap_table_entry.page_base_31_12 = VIDEO/FOUR_KB;
                }else{
                    vidmap_table_entry.present = 0; // PTE does not exist
                    vidmap_table_entry.page_base_31_12 = i; //address of each of the page table entry physical address, each page is 4kb = i = 4GB/(2^20)
                }
                vidmap_page_table[i] = vidmap_table_entry.val;
        }

        pd_entry_pt vid_mem_pt;
        vid_mem_pt.page_base_31_12 = ((uint32_t)vidmap_page_table/FOUR_KB); // 4GB/(2^20) = 4096
        vid_mem_pt.global          = 0; // Only Kernel should be set to 1 (shared by other processes)
        vid_mem_pt.page_size       = 0; // 0 for page table page directory entry
        vid_mem_pt.ignored         = 0; // Ignored
        vid_mem_pt.accessed        = 0; // access bit, not used in mp3
        vid_mem_pt.pcd             = 0; // page cach is not enabled (enabled for kernel pages, program pages)
        vid_mem_pt.pwt             = 0; //caching is writeback
        vid_mem_pt.u_s             = 1; // user level
        vid_mem_pt.r_w             = 1; // 1 for read/write
        vid_mem_pt.present         = 1; // PDE does exist
        vid_mem_pt.avl             = 0; // Not used for us
        page_dir[ONE_THIRTY_TWO_MB/FOUR_MB] = vid_mem_pt.val;
        
        *screen_start = (uint8_t*)ONE_THIRTY_TWO_MB; // set address
        flush_tlb();
        sti();
        return 0;
    }
}

/* int32_t set_handler(int32_t signum, void* handler_address)
 * Inputs: not used, meant for extra credit signaling
 * Return Value: -1 failure
 * Function: none
 * */
int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}

/* int32_t sigreturn(void);
 * Inputs: not used, meant for extra credit signaling
 * Return Value: -1 failure
 * Function: none
 */
int32_t sigreturn (void){
    return -1;
}

/* int32_t invalid_terminal_read(int32_t fd, void* buf, int32_t n);
 * Inputs: not used, meant for extra credit signaling
 * Return Value: -1 -- stdin is read-only
 *  
 */
int32_t invalid_terminal_write(int32_t fd, const void* buf, int32_t n){
    return FAIL_NEG_ONE;
}

/* int32_t invalid_terminal_read(int32_t fd, void* buf, int32_t n);
 * Inputs: none used
 * Return Value: -1 -- stdout is write-only
 *  Function: Since stdin is write-only, the corresponding read function is invalid, thus returning -1
 */
int32_t invalid_terminal_read(int32_t fd, void* buf, int32_t n){  
    return FAIL_NEG_ONE;
}








