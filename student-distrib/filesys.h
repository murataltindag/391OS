#ifndef FILESYS
#define FILESYS

#include "types.h"
#include "lib.h"

#define BB_RESERVED             52
#define BB_DENTRIES             63
#define DENTRY_FILE_NAME_LEN    32
#define DENTRY_RESERVED         24
#define INODE_DATA_BLOCKS       1023
#define FOUR_KB                 4096
#define KB                      1024
#define RTC_FILE_TYPE           0
#define DIRECTORY_FILE_TYPE     1
#define REGULAR_FILE_TYPE       2
#define FD_MIN                  2
#define FD_MAX                  7
#define EIGHT_MB    0x800000
#define FOUR_MB     0x400000
#define TWELVE_MB   0xC00000
#define EIGHT_KB    8192

extern int32_t process_num;

typedef struct dentry_t {
    uint8_t filename[DENTRY_FILE_NAME_LEN];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[DENTRY_RESERVED];
}dentry_t;

typedef struct boot_block {
    uint32_t dir_count;
    uint32_t inode_count;
    uint32_t data_count;
    uint8_t reserved[BB_RESERVED];
    dentry_t direntries[BB_DENTRIES];
}boot_block;

typedef struct inode {
    uint32_t length;
    uint32_t data_blocks[1023];
}inode;

typedef struct {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t n);
    int32_t (*write)(int32_t fd, const void* buf, int32_t n);
} fop_table_t;

typedef struct file_descriptor {
    fop_table_t* file_operations_table_pointer;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags; // Open == 1 (in-use)
}file_descriptor;

typedef struct file_array {
    file_descriptor stdin;
    file_descriptor stdout;
    file_descriptor file_descriptor[6];
}file_array;

typedef struct pcb_struct {
    int32_t pid;                            // Process ID
    int32_t parent_id;                      // Parent ID
    uint32_t saved_esp;                     // Saved ESP of the current process
    uint32_t saved_ebp;                     // Saved EBP of the current process
    uint32_t saved_parents_esp;             // Saved parent's esp
    uint32_t saved_parents_ebp;             // Saved parent's ebp
    uint32_t active;                        // Active flag, not used in the end    
    uint32_t entry_point;                   // Entry point for the start of the program
    uint32_t is_shell;                      // If the program is a shell, this is 1
    uint32_t is_base_shell;                 // If the program is a base shell, this is 1
    uint32_t saved_esp0;                    // Saved esp0
    uint32_t saved_ss0;                     // Saved ss0
    uint32_t terminal_num;                  // Process's terminal number
    uint32_t pingpong;                      // Label this as pingpong for terminal
    file_descriptor file_descriptor[8];     // File descriptor array
}pcb_struct;

/* Initialize file system */
void init_filesys(uint32_t* addr);

/* Module Functions */
uint32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
uint32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
uint32_t read_data(uint32_t inode_index, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t directory_open(const uint8_t* filename);
int32_t directory_close(int32_t fd);

/* Pointers to beginning of blocks */
boot_block* bb;
uint32_t* inodes;
inode* inodes_struct_ptr;
uint32_t* data;

#endif
