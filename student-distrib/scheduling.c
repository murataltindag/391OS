#include "scheduling.h"
#include "system_call.h"
#include "x86_desc.h"
#include "i8259.h"
#include "keyboard.h"

#define PIT_PIC_PIN 0

uint32_t i = 1;
pcb_struct* pcb_scheduling;
uint32_t curr_esp_scheduling;
uint32_t curr_ebp_scheduling;


/* void scheduler()
 * Inputs: none
 * Outputs: none
 * Return Value: none
 * Function: This is called by the PIT handler. The first three pits execute the first
 *           three shells. The rest of the PIT calls just switch between processes.
 * */
void scheduler(){
    if(pit_count == 0){ // execute the first shell
        send_eoi(PIT_PIC_PIN); 
        clear();  // clear video memory
        switch_terminals(0, 0); // switch to the first terminal
        execute((uint8_t*)"shell");
    }else if(pit_count < SCHEDULED_TASKS_NUM+1){ // second and third shell
        pcb_scheduling = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));
        asm volatile ("movl %%esp, %0;" 
                        :"=r"(curr_esp_scheduling)  /* output */
                        :                /* input */
                        :"%eax"          /* clobbered register */
                        );      
        asm volatile ("movl %%ebp, %0;"  
                        :"=r"(curr_ebp_scheduling)   /* output */
                        :                 /* input */
                        :"%eax"           /* clobbered register */
                        );    
        pcb_scheduling->saved_ebp = curr_ebp_scheduling;
        pcb_scheduling->saved_esp = curr_esp_scheduling;
        pcb_scheduling->is_shell = 1;
        pcb_scheduling->active = 1;
        send_eoi(PIT_PIC_PIN); 
        schedule_idx++; 
        switch_terminals(pit_count-1, pit_count); // switch to the second terminal
        execute((uint8_t*)"shell");               // launch new terminals
    }else{
        // switching between different programs after terminals are all launched
        pcb_scheduling = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1)); // Save the current esp and ebps for restoration
        asm volatile ("movl %%esp, %0;" 
                        :"=r"(pcb_scheduling->saved_esp)  /* output */
                        :                /* input */
                        :"%eax"          /* clobbered register */
                        );      
        asm volatile ("movl %%ebp, %0;"  
                        :"=r"(pcb_scheduling->saved_ebp)   /* output */
                        :                 /* input */
                        :"%eax"           /* clobbered register */
                        );     

        if(pit_count == (SCHEDULED_TASKS_NUM+1)){ // third shell's pcb setup
            pcb_scheduling->is_shell = 1;
            pcb_scheduling->is_base_shell = 1;
            pcb_scheduling->parent_id = -1;
            pcb_scheduling->active = 1;
            pit_count++;
        }

        //Increment the scheduler process idx that represents which terminal is being actively running
        if(schedule_idx < SCHEDULED_TASKS_NUM){
            schedule_idx++;
        }else{
            schedule_idx = 0;
        }

        //Get the pid: Get scheduler process idx -> access scheduler struct array with that idx to get the next pid
        uint32_t next_pid = schedule_array[schedule_idx];
        process_num = next_pid;

        //Find PCB of the next process 
        pcb_struct* next_process_pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(next_pid+1));
        
        // Set up the page table entry for switching between video memories for different process
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

        // Switch between different video memory location for the program to write even in background
        if (next_process_pcb->terminal_num == terminal_num){
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
        } else {
            for(i = 0; i < KB; i++) {
                if(i == 0){ // setting vid mem at first 4kb page at 132MB
                    vidmap_table_entry.present = 1; /* Initialize video memory to present */
                    vidmap_table_entry.u_s = 1;     // user level privilege
                    vidmap_table_entry.page_base_31_12 = VIDEO/FOUR_KB + (next_process_pcb->terminal_num)+1;
                }else{
                    vidmap_table_entry.present = 0; // PTE does not exist
                    vidmap_table_entry.page_base_31_12 = i; //address of each of the page table entry physical address, each page is 4kb = i = 4GB/(2^20)
                }
                vidmap_page_table[i] = vidmap_table_entry.val;
            }
        }

        // Set up the 4kb page directory entry to switch to
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
        flush_tlb();
        
        //Setup paging for the next process + Flush TLB 
        paging_execute(next_process_pcb->pid);

        //Switch the kernel stack to the next process’s kernel stack (from next process’s PCB)
        //Restore next process’ TSS
        tss.ss0 = KERNEL_DS; //pointer to kernel’s stack segment
        tss.esp0 = EIGHT_MB - EIGHT_KB*(next_process_pcb->pid) - FOUR_BYTES; // restore parent's kernel-mode stack
        asm volatile ("movl %0, %%esp;"
                        :                /* output */
                        :"r"(next_process_pcb->saved_esp)  /* input */
                        :"%eax"          /* clobbered register */
                        );      
        asm volatile ("movl %0, %%ebp;"  
                        :                 /* output */
                        :"r"(next_process_pcb->saved_ebp)   /* input */
                        :"%eax"           /* clobbered register */
                        ); 
        send_eoi(PIT_PIC_PIN); 
    }
}
