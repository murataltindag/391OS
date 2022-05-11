#include "paging.h"
#include "filesys.h"
#include "types.h"

#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H
#define SUCCESS 1
#define FAIL_NEG_ONE    -1
#define EIGHT_MB    0x800000
#define FOUR_MB     0x400000
#define TWELVE_MB   0xC00000
#define BUF_SIZE    32
#define USER_ADDR   0x08000000
#define EXC_META    40
#define EIGHT_KB    8192
#define FD_STDIN    0
#define ENTRY_POINT_BYTE    24
#define FOUR_BYTES    4
#define FOURTY_BYTES    40
#define EIGHT_BYTES    8
#define ONE_TWENTLY_EIGHT_MB    128
#define MB_128  0x8000000
#define ONE_THIRTY_TWO_MB   0x8400000
#define ONE_THIRTY_SIX_MB   0x8800000
#define FOUR_MEGABYTES    4
#define MAGIC_NUM_ZERO    0x7f
#define MAGIC_NUM_ONE    0x45
#define MAGIC_NUM_TWO    0x4c
#define MAGIC_NUM_THREE    0x46
#define EXECUTE_ADDR    0x08048000
#define BUFSIZE 1024
#define MAX_PCB     512
#define SHELL_LIMIT 6
#define PROGRAM_LIMIT 6
#define FIVE_BYTES 5
#define TWO 2
#define RTC 0
#define DIRECTORY 1
#define FILE 2
#define PROCESS_NUMBER_PIT 3
#define CAT_STRLEN 3

int32_t process_num; // for each terminal
int32_t process_count;
int32_t saved_status_num;
uint8_t stored_buf[BUFSIZE];
int32_t shell_halt_flag;
int32_t shell_process_count;
int32_t first_call;
int32_t base_shell_id;
extern int32_t terminal_num; 
int32_t check_for_enter;
int32_t cat_flag[PROCESS_NUMBER_PIT]; 

// helper functions
const uint8_t* get_file_name(const uint8_t* command);
void parse_args(const uint8_t* command);
void paging_execute(int32_t local_process_num);
int32_t is_executable(uint8_t* buffer);
extern void flush_tlb();
extern void context_switch();
void disable_child_page();

// system call functions
extern int32_t system_halt (uint8_t status);
extern int32_t exception_halt(uint16_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t n);
extern int32_t write (int32_t fd, const void* buf, int32_t n);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);
int32_t invalid_terminal_read(int32_t fd, void* buf, int32_t n);
int32_t invalid_terminal_write(int32_t fd, const void* buf, int32_t n);


int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
// int32_t switch_vidmap(uint32_t terminal_num);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

/* file operations tables for different types of files
 *  stdin_fop  -- read-only terminal
 *  stdout_fop -- write-only terminal
 *  rtc_fop -- real-time clock files
 *  directory_fop -- directories
 *  file_fop -- regular files
 */
extern fop_table_t stdin_fop;
extern fop_table_t stdout_fop;
extern fop_table_t rtc_fop;
extern fop_table_t directory_fop;
extern fop_table_t file_fop;

#endif /* SYSTEM_CALL_H */


