/*terminal.c - Functions to interact with the terminal */

#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "filesys.h"
#include "terminal.h"
#include "scheduling.h"
#include "system_call.h"

// Global variables used by different terminals
int32_t bytes_written, line_ct;     
extern int32_t terminal_num;
extern uint8_t row_letter_ct;
int32_t pingpong_terminal;
int8_t hello_flag = 0;

/* int terminal_open()
 * opens the terminal
 * inputs: none
 * outputs: 0
 * side effects: none
 */
int32_t terminal_open(const uint8_t* filename){
    // Set up the 3 terminals
    terminal[0].save_x = 0;
    terminal[0].save_y = 0;
    terminal[0].enter_read = 0;
    terminal[0].active = 0;
    terminal[1].save_x = 0;
    terminal[1].save_y = 0;
    terminal[1].enter_read = 0;
    terminal[1].active = 0;
    terminal[2].save_x = 0;
    terminal[2].save_y = 0;
    terminal[2].enter_read = 0;
    terminal[2].active = 1;
    return 0;
}

/* int terminal_closes()
 * closes the terminal
 * inputs: none
 * outputs: 0
 * side effects: none
 */
int32_t terminal_close(int32_t fd){
    return -1;
}

/* int terminal_read()
 * copies the keyboard buffer into buf
 * inputs: char buf[128] -- the buffer to fill
 *          n -- how many chars to copy
 * outputs: buffer_ct + 1 -- current size of the copied buffer
 * side effects: none
 */
int32_t terminal_read(int32_t fd, void* buf, int n){
    disable_irq(0);
    int i;
    uint8_t saved_buffer_ct;

    // Get the pcb of the scheduled process for later use
    pcb_struct* current_pcb_local;
    current_pcb_local = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(terminal[schedule_idx].curr_pid +1));   
    terminal[current_pcb_local->terminal_num].terminal_buf = (uint8_t*)buf;
    
    // Wait until the enter is pressed and terminal number matches with schedule index
    while(1){
        disable_irq(0);
        if(!terminal[current_pcb_local->terminal_num].enter_read || !(terminal_num == schedule_idx)){
            enable_irq(0);
        }else{
            break;
        }
    }  

    //check for bad input
    if((n<0) || (fd>1)){        
        return -1;
    }

    // Store the keyboard buffer into the buffer
    if (n>BUFFER_LIM){      //check if the buffer is too long
        for(i = 0; i<BUFFER_LIM - 1; i++){
            ((char*)buf)[i] = terminal[current_pcb_local->terminal_num].keyboard_buffer[i];
        }
        ((char*)buf)[terminal[current_pcb_local->terminal_num].buffer_ct] = '\n';    //add the newline
    }
    else {
        for(i = 0; i<n-1; i++){     
            ((char*)buf)[i] = terminal[current_pcb_local->terminal_num].keyboard_buffer[i];
        }
        ((char*)buf)[terminal[current_pcb_local->terminal_num].buffer_ct] = '\n';   //add the newline
    }
    saved_buffer_ct = terminal[current_pcb_local->terminal_num].buffer_ct;
    
    //If it's exit, add a \0 to the end
    if(strncmp(buf, (char*)"exit\n", SIX_BYTE) == 0){
        ((char*)buf)[terminal[terminal_num].buffer_ct] = '\0';   //add the newline   
    }     
    
    clear_buffer();  //clear the keyboard_buffer
    terminal[current_pcb_local->terminal_num].enter_read = 0;    //flip enter flag
    
    enable_irq(0);
    return saved_buffer_ct+1;
} 

/* int terminal_write()
 * writes buf into the terminal
 * inputs: char buf[128] -- the input buffer
 *          n -- how many chars to write
 * outputs: n -- current size of the printed buffer                 
 * side effects: none
 */
int32_t terminal_write(int32_t fd, const void* buf, int n){
    bytes_written = 0;
    line_ct = 0;
    int index;

    //check for bad input
    if((buf == NULL) || (n<0) || (fd>1)){       
        return -1;
    }

    // Cases for differnt program's buffer line count
    if(hello_flag == 1){ 
        line_ct = HELLO_LINE_COUNT;
        hello_flag = 0;
    }
    if(strncmp(buf, (char*)"Hi, what's your name? ", HELLO_BYTE_COUNT+1) == 0){
        row_letter_ct = HELLO_BYTE_COUNT; 
    }else if(strncmp(buf, (char*)"Enter the Test Number: (0): 100, (1): 10000, (2): 100000\n", COUNTER_BYTE_COUNT) == 0) {
        row_letter_ct = 0; 
    }  
   
    // Two cases: write to the current terminal buffer or write to the background terminal buffer if there
    // is a terminal number and schedule index mismatch for write.
    for(index = 0; index<n; index++){
        if((line_ct == ROW_LIM) && (((uint8_t*)buf)[index] != '\0') && (((uint8_t*)buf)[index] != '\n')){
            if(schedule_idx != terminal_num){
                background_putc('\n', schedule_idx);
            }
            else{
                putc('\n');
            }
            line_ct = 0;
        }
        else if ((((uint8_t*)buf)[index]) == '\n'){
            line_ct = 0;
        }
        if(((uint8_t*)buf)[index] != '\0'){
            bytes_written++;
            line_ct++;
            if(schedule_idx != terminal_num){
                background_putc(((uint8_t*)buf)[index],schedule_idx);
            }
            else{
                putc(((uint8_t*)buf)[index]);
            }
        }
    }  

    // Set different flags or add new lines for different programs
    if(strncmp(buf, (char*)"Hello, ", HELLO_LINE_COUNT+1) == 0){
        hello_flag = 1; 
    }else{
        hello_flag = 0;
    }
    if(strncmp(buf, (char*)"exit", EXIT_BYTE_COUNT) == 0){
        putc('\n');
    } 

    if(cat_flag[schedule_idx] == 1 && strncmp(buf, (char*)"391OS> ", 9)){
        putc('\n');
    }else{
        cat_flag[schedule_idx] = 0;
    }

    return bytes_written;
}


