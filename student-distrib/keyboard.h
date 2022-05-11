/* keyboard.h: defines keyboard interrupt handler, initialization helpers and command and port codes */
/* controller commands*/
#define COMMAND_SET_LED     0xED
#define COMMAND_ECHO        0xEE
#define COMMAND_GET_SCAN    0xF0
#define COMMAND_ID_KEYBOARD 0xF2
#define COMMAND_SET_RATE    0xF3
#define COMMAND_ENABLE_SCAN 0xF4
#define COMMAND_DISABLE_SCAN 0xF5
#define COMMAND_SET_PARAM   0xF6
#define COMMAND_RESEND      0xFE
#define COMMAND_RESET       0xFF

/* ports */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_CMD_PORT 0x64

#define KEYBOARD_IRQ_NUM 1

/*scan codes*/
#define ONE_SCAN_CODE   0x02
#define EQUALS_SCAN_CODE    0x0D
#define Q_SCAN_CODE     0x10
#define CLOSE_SQUARE_BRACKET_SCAN_CODE 0x1B
#define A_SCAN_CODE 0x1E
#define SEMICOLON_SCAN_CODE 0x27
#define Z_SCAN_CODE 0x2C
#define FRONT_SLASH_SCAN_CODE 0x35

#define P_SCAN_CODE 0x19
#define L_SCAN_CODE 0x26
#define Z_SCAN_CODE 0x2C
#define M_SCAN_CODE 0x32
#define W_SCAN_CODE 0x11
#define SINGLE_QUOTE_SCAN_CODE 0x28
#define BACK_TICK_SCAN_CODE 0x29
#define BACKSLASH_SCAN_CODE 0x2B

#define L_SHIFT_ON  0x2A
#define L_SHIFT_OFF 0xAA
#define R_SHIFT_ON  0x36
#define R_SHIFT_OFF 0xB6
#define CAPS_LOCK   0x3A
#define L_ALT_ON    0x38
#define L_ALT_OFF   0xB8
//#define R_ALT_ON    
//#define R_ALT_OFF
#define SPACEBAR 0x39
#define ENTER   0x1C
#define TAB     0x0F
#define ESC     0x01
#define BACKSPACE   0x0E
#define L_CONTROL_ON  0x1D
#define L_CONTROL_OFF 0x9D
#define ENTER_RELEASE 0x9C

/* row and buffer char limits */
#define ROW_LIM 80
#define BUFFER_LIM 128
#define HEADER_LEN 7

void keyboard_handler(void);
void keyboard_init(void);
void clear_buffer();
void switch_terminals(int32_t previous_terminal_num, int32_t next_terminal_num);
// void vidmap_terminal();
uint32_t saved_esp[3];
uint32_t saved_ebp[3];



