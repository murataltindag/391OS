#include "paging.h"
uint32_t* page_dir_pointer = page_dir;

/* void init_paging(void)
 * inputs: void
 * outputs: void
 * Function: Initializes a page entry
 */
void init_paging() {
    int i;
    pt_entry_page page_table_entry;
    pd_entry_pt first_pd_entry;
    pd_entry_page kernel_page;
    pd_entry_page reset_page_directory_entry;
    
    /* Initialize all pd entries. Set each page dir entry to not present */    
    reset_page_directory_entry.present           =  0; // 0 clear means not in memory (present)
    reset_page_directory_entry.r_w               =  0;
    reset_page_directory_entry.u_s               =  0;
    reset_page_directory_entry.pwt               =  0;
    reset_page_directory_entry.pcd               =  0;
    reset_page_directory_entry.accessed          =  0;
    reset_page_directory_entry.dirty             =  0;
    reset_page_directory_entry.page_size         =  1; // using 4MB to set
    reset_page_directory_entry.global            =  0;
    reset_page_directory_entry.avl               =  0;
    reset_page_directory_entry.pat               =  0;
    reset_page_directory_entry.reserved_21_13    =  0;
    reset_page_directory_entry.page_base_31_22   =  0;
    for(i = 0; i < KB; i++) { 
        page_dir[i] = reset_page_directory_entry.val;
    }

    /* Initialize 1st pd entry (points to page tables) */ // Added this since present needs to be set to 1
    first_pd_entry.page_base_31_12 = ((uint32_t)first_page_table/FOUR_KB); // 4GB/(2^20) = 4096
    first_pd_entry.global          = 0; // Only Kernel should be set to 1 (shared by other processes)
    first_pd_entry.page_size       = 0; // 0 for page table page directory entry
    first_pd_entry.ignored         = 0; // Ignored
    first_pd_entry.accessed        = 0; // access bit, not used in mp3
    first_pd_entry.pcd             = 0; // page cach is not enabled (enabled for kernel pages, program pages)
    first_pd_entry.pwt             = 0; //caching is writeback
    first_pd_entry.u_s             = 0; //supervisor only
    first_pd_entry.r_w             = 1; // 1 for read/write
    first_pd_entry.present         = 1; // PDE does exist
    first_pd_entry.avl             = 0; // Not used for us
    page_dir[0] = first_pd_entry.val;

    /* Initialize 4kb pages for the 1st pd entry */
    page_table_entry.global          = 0; //TLB is cleared when reloading CR3s
    page_table_entry.pat             = 0; // Page Attribute Table index, not used
    page_table_entry.dirty           = 0; // 1 if page matched by this PTE has been written to
    page_table_entry.accessed        = 0; // access bit, not used in mp3
    page_table_entry.pcd             = 0; // page cach is disabled
    page_table_entry.pwt             = 0; //caching is writeback
    page_table_entry.u_s             = 0; //supervisor only
    page_table_entry.r_w             = 1; // 1 for read/write
    page_table_entry.avl             = 0; // Not used for us
    for(i = 0; i < KB; i++) {
        if(i == VIDEO/FOUR_KB || i == FIRST_TERMINAL_BUF/FOUR_KB || i == SECOND_TERMINAL_BUF/FOUR_KB  || i == THIRD_TERMINAL_BUF/FOUR_KB ){ // each page is 4kb
            page_table_entry.present = 1; /* Initialize video memory to present */
        }else{
            page_table_entry.present = 0; // PDE does not exist
        }
        page_table_entry.page_base_31_12 = i; //address of each of the page table entry physical address, each page is 4kb = i = 4GB/(2^20)
        first_page_table[i] = page_table_entry.val;
    }
    
    /* Initialize 2nd pd entry (kernel page) */
    kernel_page.present         = 1; // PDE does exist
    kernel_page.r_w             = 1; // 1 for read/write
    kernel_page.u_s             = 0; //supervisor only
    kernel_page.pwt             = 0; //caching is writeback
    kernel_page.pcd             = 1; // page cach is not enabled (enabled for kernel pages, program pages)
    kernel_page.accessed        = 0; // access bit, not used in mp3
    kernel_page.page_size       = 1; // 0 for page table page directory entry
    kernel_page.global          = 1; // Only Kernel should be set to 1 (shared by other processes)
    kernel_page.avl             = 0; // Not used for us
    kernel_page.dirty           = 0; // 1 if page matched by this PTE has been written to
    kernel_page.pat             = 0; // Page Attribute Table index, not used
    kernel_page.reserved_21_13  = 0; // Not used
    kernel_page.page_base_31_22 = 1; // Starts at 4MB, 2^10 bits is 1024 so 4GB/1024 = 4MB
    page_dir[1] = kernel_page.val;

    /* Load page directory and enable paging */
    enable_paging(page_dir);
}


