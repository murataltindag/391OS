/* terminal.h: defines terminal functions */

#define BUFFER_LIM 128
#define ROW_LIM 80
#define SIX_BYTE 6
#define HELLO_LINE_COUNT 7
#define HELLO_BYTE_COUNT 22
#define COUNTER_BYTE_COUNT 58
#define EXIT_BYTE_COUNT 5

int32_t terminal_open();
int32_t terminal_close();
int32_t terminal_read(int fd, void* buf, int n);
int32_t terminal_write(int fd, const void* buf, int n);


