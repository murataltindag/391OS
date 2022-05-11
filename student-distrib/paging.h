#ifndef paging_test
#define paging_test 

#include "types.h"

#define KB      1024
#define VIDEO   0xB8000
#define FOUR_KB 4096
#define FIRST_TERMINAL_BUF (0xB8000 + 4096) 
#define SECOND_TERMINAL_BUF (0xB8000 + 4096 * 2) 
#define THIRD_TERMINAL_BUF (0xB8000 + 4096 * 3)

/* Page directory entry that points to a 4KB page table */
typedef struct pd_entry_pt {
    union {
        uint32_t val;
        struct {
            uint32_t present            : 1;
            uint32_t r_w                : 1;
            uint32_t u_s                : 1;
            uint32_t pwt                : 1;
            uint32_t pcd                : 1;
            uint32_t accessed           : 1;
            uint32_t ignored            : 1; 
            uint32_t page_size          : 1;
            uint32_t global             : 1;
            uint32_t avl                : 3;  // avl has 3 bits
            uint32_t page_base_31_12    : 20; // base address is 20 bits
        } __attribute__((packed));
    };
} pd_entry_pt;

/* Page directory entry that points to a 4MB page */
typedef struct pd_entry_page {
    union {
        uint32_t val;
        struct {
            uint32_t present            : 1;
            uint32_t r_w                : 1;
            uint32_t u_s                : 1;
            uint32_t pwt                : 1;
            uint32_t pcd                : 1;
            uint32_t accessed           : 1;
            uint32_t dirty              : 1;
            uint32_t page_size          : 1;
            uint32_t global             : 1;
            uint32_t avl                : 3; // avl has 3 bits
            uint32_t pat                : 1;
            uint32_t reserved_21_13     : 9; // reserved has 9 bits
            uint32_t page_base_31_22    : 10; // base address is 10 bits
        } __attribute__((packed));
    };
} pd_entry_page;

/* Page table entry that points to a 4KB page */
typedef struct pt_entry_page {
    union {
        uint32_t val;
        struct {
            uint32_t present            : 1;
            uint32_t r_w                : 1;
            uint32_t u_s                : 1;
            uint32_t pwt                : 1;
            uint32_t pcd                : 1;
            uint32_t accessed           : 1;
            uint32_t dirty              : 1;
            uint32_t pat                : 1;
            uint32_t global             : 1;
            uint32_t avl                : 3; // avl has 3 bits
            uint32_t page_base_31_12    : 20; // base address is 20 bits
        } __attribute__((packed));
    };
} pt_entry_page;

/* Page directory */
uint32_t page_dir[KB] __attribute__((aligned (FOUR_KB)));

/* First page table */
uint32_t first_page_table[KB] __attribute__((aligned (FOUR_KB)));

/* Video memory page table */
uint32_t vidmap_page_table[KB] __attribute__((aligned (FOUR_KB)));

/* Initialize paging */
void init_paging();

extern uint32_t* page_dir_pointer;

extern void enable_paging(uint32_t* page_dir_pointer);
extern void flush_tlb(); // helper function to flush tlb

#endif



