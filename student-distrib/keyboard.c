/*keyboard.c - Functions to interact with the Keyboard */

#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "terminal.h"
#include "filesys.h"
#include "system_call.h"
#include "x86_desc.h"
#include "scheduling.h"
#include "paging.h"

/* flags for function keys initially set to 0 */
int32_t terminal_num = 0;
uint8_t ctrl = 0;
uint8_t shift = 0;
uint8_t caps = 0;
uint8_t alt = 0;

/* character counters for the keyboard and the buffer */
uint8_t row_letter_ct = HEADER_LEN;  //"391OS>" is 6 characters
uint8_t newline_bs_flag = 0;

int i;
char printed_char;


/* helpers */
int is_letter(uint8_t scan_code);
int flag_setters(uint8_t scan_code);
int is_printable(uint8_t scan_code);
void space_handler(void);



/* Scan codes for lower case letters and numbers */
char scan_to_char[0xD8] = {'.', '.', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                          '-','=','.', '.', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
                          'o', 'p', '[', ']', '.', '.', 'a', 's', 'd', 'f', 'g', 'h', 'j',
                          'k', 'l', ';','.','`','.','.', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
                          };

/* Scan codes for upper case letters and shifted characters */
char shift_scan[0xD8] = {'.', '.', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
                          '_','+','.', '.', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
                          'O', 'P', '{', '}', '.', '.', 'A', 'S', 'D', 'F', 'G', 'H', 'J',
                          'K', 'L', ':','.','.','.','.', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
                          };
/* void keyboard_init()
 * Initialize (unmask) keyboard interrupt
 * inputs: none
 * outputs: none
 * side effects: enables irq pin for keyboard on the PIC
 */
void keyboard_init(){
    enable_irq(KEYBOARD_IRQ_NUM);
    int32_t i;
    for(i=0;i<3;i++){
        saved_esp[i] = 0;
        saved_ebp[i] = 0;
    }
    // saved_esp[0] = 0x7FFF08;
    // saved_ebp[0] = 0x7FFF10;
    // saved_esp[1] = 0x7FDF14;
    // saved_ebp[1] = 0x7FDF30;
    // saved_esp[2] = 0x7FBF14;
    // saved_ebp[2] = 0x7FBF30;
}


/* void keyboard_handler()
 * The keyboard interrupt handler
 * inputs: none
 * outputs: none
 * side effects: prints a character once a key is pressed, re-enables interrupts
 */
void keyboard_handler(){
    cli();
    uint32_t scan_code = inb(KEYBOARD_DATA_PORT); //read the scan code from the port
    
    //enter
    if(scan_code == ENTER){
        // disable_irq(0);
        terminal[terminal_num].enter_read = 1;                 // turn the enter flag on for terminal_read to continue 
        //clear_buffer();                 //clear the keyboard buffer
        printf("\n");               //new line
        //printf("391OS>");
        newline_bs_flag = 1;    //turn on the newline backsapce flag so backspace doesn't delete past the start of the new line
        row_letter_ct = HEADER_LEN;      //reset the row (391OS> is 6 characters long)
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();              // start the end of interrupt routine
        return;
    }


    if((alt)&&(scan_code == 0x3B)){         //upon alt+f1
        // if(terminal_num == 0){
        //     sti();          // start the end of interrupt routine
        //     send_eoi(KEYBOARD_IRQ_NUM);
        //     return;  
        // }
        // saved_terminal_num = terminal_num;
        // terminal_num = 0;
        // switch_terminals();
        
        switch_terminals(terminal_num,  0);

        send_eoi(KEYBOARD_IRQ_NUM);
        sti();          // start the end of interrupt routine
        return;
    }
    
    if((alt)&&(scan_code == 0x3C)){         //upon alt+f2
        // if(terminal_num == 1){
        //     sti();          // start the end of interrupt routine
        //     send_eoi(KEYBOARD_IRQ_NUM);
        //     return;  
        // }
        // saved_terminal_num = terminal_num;
        // terminal_num = 1;
        // switch_terminals();
        
        switch_terminals(terminal_num, 1);
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();          // start the end of interrupt routine
        return;
    }

    if((alt)&&(scan_code == 0x3D)){         //upon alt+f3
        // if(terminal_num == 2){
        //     sti();          // start the end of interrupt routine
        //     send_eoi(KEYBOARD_IRQ_NUM);
        //     return;  
        // }
        // saved_terminal_num = terminal_num;
        // terminal_num = 2;
        // switch_terminals();

        switch_terminals(terminal_num, 2);

        send_eoi(KEYBOARD_IRQ_NUM);
        sti();          // start the end of interrupt routine
        return;
    }

    //clear
    if((ctrl)&&(scan_code == L_SCAN_CODE)){         //upon ctrl + l
        clear();        //clear screen
        row_letter_ct = HEADER_LEN;      //reset the row (391OS> is 6 characters long)
        //clear_buffer();         
        printf("391OS> ");
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();          // start the end of interrupt routine
        return;
        
    }



    
    // enter released
    // if(scan_code == ENTER_RELEASE){
    //     terminal_read
    //     terminal[terminal_num].enter_read = 0;                 // turn the enter flag on for terminal_read to continue 
    //     clear_buffer();                 //clear the keyboard buffer
    //     printf("\n");               //new line
    //     printf("391OS>");
    //    newline_bs_flag = 1;    //turn on the newline backsapce flag so backspace doesn't delete past the start of the new line
    //    row_letter_ct = 7;      //reset the row (391OS> is 6 characters long)
    //    sti();              // start the end of interrupt routine
    //     send_eoi(KEYBOARD_IRQ_NUM);
    //     return;
    // }



   // set flags
    if(flag_setters(scan_code)){        //if a flag was set
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();              // start the end of interrupt routine
        return;
    }

    //backspace
    if(scan_code == BACKSPACE){
        if(ctrl || alt){
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();          // start the end of interrupt routine
            return;
        }
        if(terminal[terminal_num].buffer_ct != 0){         //if there's anything in the buffer
            if(row_letter_ct == 0 && newline_bs_flag == 1){      //if you're at the start of the buffer
                    //do nothing
            }
            if(terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct-1] == '\t'){       // if you're at a tab
                putc('\b');     // delete 4 spaces at once
                putc('\b');
                putc('\b');
                putc('\b');
                row_letter_ct--;        //decrement row and buffer  //TODO Might have to decrement this by 4 unless it reaches negative
                terminal[terminal_num].buffer_ct--;
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = NULL;      //replace that char by null
            }
            
            else{
                putc('\b'); // delete 1 char
                row_letter_ct--;    //decrement row and buffer
                terminal[terminal_num].buffer_ct--;
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = NULL;  //replace that char by null
            }
        }
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();              // start the end of interrupt routine
        return;
    }


    //spacebar
    if(scan_code == SPACEBAR){
        if(ctrl || alt || (terminal[terminal_num].buffer_ct == BUFFER_LIM -1)){ //if ctrl or alt are pressed or the buffer is full
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();          // start the end of interrupt routine
            return;
        }
        space_handler();        // print a space
        if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){       // fill the buffer accordingly
            terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = ' ';
            terminal[terminal_num].buffer_ct++;
        }
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();              // start the end of interrupt routine
        return;
    }



    //tab
    if(scan_code == TAB){
        if(ctrl || alt || (terminal[terminal_num].buffer_ct == BUFFER_LIM -1)){
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();              // start the end of interrupt routines
            return;
        }
        if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){       // fill the buffer accordingly
            terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '\t';
            terminal[terminal_num].buffer_ct++;
        }
        printed_char = '\t';    
        putc(printed_char);     // print 4 spaces
        // if(row_letter_ct == ROW_LIM){        //newline if you're at the end of the line
        //     printf("\n");
        //     newline_bs_flag = 0;        
        //     row_letter_ct = 0;
        // }
        // else{
        //     row_letter_ct += 4;     //increment row char ct by 4
        // }
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();              // start the end of interrupt routine
        return;
    }

    //single quote
    if(scan_code == SINGLE_QUOTE_SCAN_CODE){
        if(ctrl || alt || (terminal[terminal_num].buffer_ct == BUFFER_LIM - 1)){
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();          // start the end of interrupt routine
            return;
        }
        if (row_letter_ct == ROW_LIM){      //newline if you're end of the line
            printf("\n");
            newline_bs_flag = 0;
            row_letter_ct = 0;
        }
        if(shift){          //print the appropriate char according to the shift flag
            printf("\"");
            if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '\"';
                terminal[terminal_num].buffer_ct++;
            }
        }
        else{
            printf("\'");
            if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '\'';
                terminal[terminal_num].buffer_ct++;
            }
        }
        row_letter_ct++;
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();              // start the end of interrupt routine
        return;
    }

    //back tick
    if(scan_code == BACK_TICK_SCAN_CODE){
        if(ctrl || alt || (terminal[terminal_num].buffer_ct == BUFFER_LIM - 1)){
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();          // start the end of interrupt routine
            return;
        }
        if (row_letter_ct == ROW_LIM){      //newline if you're end of the line
            printf("\n");
            newline_bs_flag = 0;
            row_letter_ct = 0;
        }
        if(shift){          //print the appropriate char according to the shift flag
            printf("~");
            if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '~';
                terminal[terminal_num].buffer_ct++;
            }
        }
        else {
            printf("`");
            if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '`';
                terminal[terminal_num].buffer_ct++;
            }   
        }
        row_letter_ct++;
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();          // start the end of interrupt routine
        return;
    }

    //backslash
    if(scan_code == BACKSLASH_SCAN_CODE){
        if(ctrl || alt || (terminal[terminal_num].buffer_ct == BUFFER_LIM - 1)){
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();      // start the end of interrupt routine
            return;
        }
        if (row_letter_ct == ROW_LIM){          //newline if you're end of the line
            printf("\n");
            newline_bs_flag = 0;
            row_letter_ct = 0;
        }
        if(shift){              //print the appropriate char according to the shift flag
            printf("|");
            if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '|';
                terminal[terminal_num].buffer_ct++;
            }
        }
        else {
            printf("\\");
            if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
                terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = '\\';
                terminal[terminal_num].buffer_ct++;
            }
        }
        row_letter_ct++;
        send_eoi(KEYBOARD_IRQ_NUM);
        sti();          // start the end of interrupt routine
        return;
    }

    // printable characters
    
    if(is_printable(scan_code)){  //check the scan code boundaries
        if(ctrl || alt || (terminal[terminal_num].buffer_ct == BUFFER_LIM - 1)){
            send_eoi(KEYBOARD_IRQ_NUM);
            sti();      // start the end of interrupt routine
            return;
        }


        if (row_letter_ct == ROW_LIM){
            printf("\n");
            newline_bs_flag = 0;
            row_letter_ct = 0;
        }
        if((caps == 0) && (shift == 0)){  //default    hello world 1234
            printed_char = scan_to_char[scan_code];     
        }
        else if((shift == 0) && is_letter(scan_code)){   // caps on, shift off, is letter      HELLO WORLD
            printed_char = shift_scan[scan_code];
        }
        else if(shift == 0){            // caps on, shift off, is not letter           1234
            printed_char = scan_to_char[scan_code]; 
        }
        else if(caps == 0){             // caps off, shift on              HELLO WORLD !@#$
            printed_char = shift_scan[scan_code];
        }
        else if(is_letter(scan_code)){      // caps on, shift on, is letter    hello world 
            printed_char = scan_to_char[scan_code]; 
        }
        else {                               // caps on, shift on, is not letter     !@#$      
            printed_char = shift_scan[scan_code];
        }
        row_letter_ct++;
        if(terminal[terminal_num].buffer_ct < (BUFFER_LIM - 1)){
            terminal[terminal_num].keyboard_buffer[terminal[terminal_num].buffer_ct] = printed_char;
            terminal[terminal_num].buffer_ct++;
        }
        putc(printed_char);
        
    }





    send_eoi(KEYBOARD_IRQ_NUM); //send the end of interrupt signal for IRQ1
    sti();          // start the end of interrupt routine
}

/* int flag_setters(uint8_t scan_code)
 * Sets the flags for shift, control, alt and caps lock
 * inputs: uint8_t scan_code
 * outputs: 1 -- if flags changed
 *          0 -- if they didn't change
 * side effects: flips the flags when shift/alt/ctrl buttons are pressed or unpressed, and if caps_lock is pressed
 */
int flag_setters(uint8_t scan_code){
    switch (scan_code)
    {
    case L_SHIFT_ON:
        shift = 1;
        return 1;
        break;
    case L_SHIFT_OFF:
        shift = 0;
        return 1;
        break;
    case R_SHIFT_ON:
        shift = 1;
        return 1;
        break;
    case R_SHIFT_OFF:
        shift = 0;
        return 1;
        break;
    case L_CONTROL_ON:
        ctrl = 1;
        return 1;
        break;
    case L_CONTROL_OFF:
        ctrl = 0;
        return 1;
        break;
    case L_ALT_ON:
        alt = 1;
        return 1;
        break;
    case L_ALT_OFF:
        alt = 0;
        return 1;
        break;
    case CAPS_LOCK:
        if(caps){
            caps = 0;
        }
        else{
            caps = 1;
        }
        return 1;
        break;
    default:
        return 0;
        break;
    }
}


/* int is_letter(uint8_t scan_code)
 * Checks if the scan_code corresponds to a alphanumerical + other printable symbols
 * inputs: uint8_t scan_code
 * outputs: 1 -- if printable
 *          0 -- if not printable
 * side effects: none
 */
int is_printable(uint8_t scan_code){
    if(((scan_code >= ONE_SCAN_CODE) && (scan_code <= EQUALS_SCAN_CODE)) || 
    ((scan_code >= Q_SCAN_CODE) && (scan_code <= CLOSE_SQUARE_BRACKET_SCAN_CODE)) ||
    ((scan_code >= A_SCAN_CODE) && (scan_code <= SEMICOLON_SCAN_CODE)) || 
    ((scan_code >= Z_SCAN_CODE) && (scan_code <= FRONT_SLASH_SCAN_CODE))){
        return 1;  //1 if printable
    }
    else return 0;  //0 if not printable
}

/* int is_letter(uint8_t scan_code)
 * Checks if the scan_code corresponds to a alphabetical char or not
 * inputs: uint8_t scan_code
 * outputs: 1 -- if letter
 *          0 -- if not letter
 * side effects: none
 */
int is_letter(uint8_t scan_code){
    if( ((scan_code >= Q_SCAN_CODE) && (scan_code <= P_SCAN_CODE)) || ((scan_code >= A_SCAN_CODE) && (scan_code <= L_SCAN_CODE)) ||  ((scan_code >= Z_SCAN_CODE) && (scan_code <= M_SCAN_CODE)) ){
        return 1;  //return 1 if letter
    }
    else return 0; // 0 if not letter
}

/* void space_handler()
 * Helper for spacebar and TAB cases
 * inputs: none
 * outputs: none
 * side effects: prints a space
 */
void space_handler(void){
    if (row_letter_ct == ROW_LIM){  //print new line if you're end of the line
       printf("\n");
       newline_bs_flag = 0;
       row_letter_ct = 0;
    }
    printf(" ");    //print a space
    row_letter_ct++;    //increment row letter ct
}

/* void clear_buffer()
 * Clear the pending buffer
 * inputs: none
 * outputs: none
 * side effects: fills up terminal[terminal_num].keyboard_buffer with NULLs
 */
void clear_buffer(){
    for(i=0; i<(BUFFER_LIM - 1); i++){ // for (BUFFER_LIM - 1) chars
        terminal[terminal_num].keyboard_buffer[i] = NULL;  // fill it up with NULLs
    }
    terminal[terminal_num].buffer_ct = 0;  //reset the counter
}

/* void switch_terminals()
 * switches cursor postions and video memory of previous and next terminal
 * inputs: int32_t previous_terminal_num -- index of previous terminal 
 *         int32_t next_terminal_num -- index of current terminal
 * outputs: none
 * side effects: copies current video screen of terminal to terminal video memory and copies terminal video memory to current video screen
 */
void switch_terminals(int32_t previous_terminal_num, int32_t next_terminal_num){
    switch_video_mem(previous_terminal_num, next_terminal_num);
    // save current terminal screen to video page assigned for it
    memcpy((void*)(VIDEO + (previous_terminal_num+1)*FOUR_KB), (const void*)(VIDEO), (uint32_t)FOUR_KB);
    memcpy((void*)(VIDEO), (const void*)(VIDEO + (next_terminal_num+1)*FOUR_KB), (uint32_t)FOUR_KB);
    terminal_num = next_terminal_num; // update terminal_num
}


