/* rtc.h: defining the functions dealing with the Real-Time Clock */
#include "types.h"

#define INDEX_PORT 0x70
#define CMOS_PORT 0x71
#define NMI_B_REG           0x8B
#define NMI_A_REG           0x8A
#define C_REG               0x0C
#define MASK_MOST_SIG_BIT   0x7F
#define RTC_PIC_PIN         8
#define RTC_FREQUENCY       0x06 // 1024hz 
#define SECOND_MOST_SIG_BIT 0x40
#define TOP_FOUR_BITS       0xF0
#define ASCII_ZERO 48
#define ASCII_NINE 57
#define MAX_FREQUENCY 1024

void rtc_init(void);
void rtc_handler(void);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);


