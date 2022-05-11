/* rtc.c: functions dealing with the Real-Time Clock */

#include "rtc.h"
#include "lib.h"
#include "filesys.h"
#include "i8259.h"
#include "system_call.h"
#include "scheduling.h"

uint32_t desired_virtualized_frequency[3] = {2, 2, 2};
// uint32_t interrupt_flag = 1;
// uint32_t interrupt_count = 0;
uint32_t rtc_idx = 0;
uint32_t interrupt_flag[3] = {1, 1, 1};
uint32_t interrupt_count[3] = {0, 0, 0};

/* void rtc_init()
 * Initialize the RTC
 * inputs: none
 * outputs: none
 * side effects: sets up the RTC and enables IRQ8
 */
void rtc_init() {
    /* Disable interrupts */
    cli();

    // Set RTC frequency to 1024hz	
    outb(NMI_A_REG, INDEX_PORT);		// set index to register A, disable NMI
    unsigned char prev = inb(CMOS_PORT);	// get initial value of register A, prev = 0x26
    outb(NMI_A_REG, INDEX_PORT);		// reset index to A, enable NMI
    outb((prev & TOP_FOUR_BITS) | RTC_FREQUENCY, CMOS_PORT); //write only our rate to A. Note, rate is the bottom 4 bits.

    outb(NMI_A_REG, INDEX_PORT);		// set index to register A, disable NMI
    prev = inb(CMOS_PORT);	// get initial value of register A, prev = 0x26
    

    /* Turn on periodic interrupts */
    outb(NMI_B_REG, INDEX_PORT); /* Disable NMI */
    prev = inb(CMOS_PORT); 
    outb(NMI_B_REG, INDEX_PORT);
    outb((prev | SECOND_MOST_SIG_BIT), CMOS_PORT);

    outb(NMI_B_REG, INDEX_PORT); /* Disable NMI */
    prev = inb(CMOS_PORT); 

    /* Re-enable NMI */
    outb(inb(INDEX_PORT) & MASK_MOST_SIG_BIT, INDEX_PORT);

    // Initial virtualized frequency
    desired_virtualized_frequency[schedule_idx] = 2; //2hz

    // /* Enable RTC interrupts */
    enable_irq(RTC_PIC_PIN);


    /* Re-enable interrupts */
    sti();
}

/* void rtc_read()
 * Read function returns after an interrupt has occured
 * inputs: none
 * outputs: always return 0
 * side effects: set interrupt_flag that will be changed by rtc_write
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    cli();
    while(1){
        cli();
        // disable_irq(0);
        if(interrupt_flag[schedule_idx] == 1){
            sti();
            // enable_irq(0);
        }else{
            break;
        }
    }  
    // while(interrupt_flag == 1){
    //     //wait until interrupt handler clears it
    // }
    // while(interrupt_flag[schedule_idx] == 1){
    //     //wait until interrupt handler clears it
    // }
    // interrupt_flag = 1;
    interrupt_flag[schedule_idx] = 1;
    sti();
    // enable_irq(0);
    return 0;
}


/* void rtc_write()
 * Accept only a 4-byte integer specifying the interrupt rate in Hz, and should set the rate of periodic interrupts accordingly.
 * inputs:  fd      -     file descriptor
 *          buf     -     desired rtc frequency
 *          nbytes  -     number of bytes used in the buffer
 * outputs: -1      -     invalid frequency
 *          0       -     success
 * side effects: clear interrupt_flag that will be read by rtc_read
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){ 
    cli();
    if(nbytes != 4) return -1;
    int32_t interrupt_rate = *((int32_t*)buf);

    // Should limit this further to 1024 Hz and should be a power of 2
    if(interrupt_rate > MAX_FREQUENCY || interrupt_rate <= 0 || !((interrupt_rate != 0) && ((interrupt_rate & (interrupt_rate - 1)) == 0))){
        return -1;
    }
    desired_virtualized_frequency[schedule_idx] = interrupt_rate;
    sti();
    
    return 0;
}

/* void rtc_open()
 * Open a file for RTC
 * inputs:  fd      -     file descriptor
 *          filename-     name of the rtc file
 * outputs: -1      -     invalid name or file type or fd
 *          0       -     success
 * side effects: set file descriptor flag to 1
 */
int32_t rtc_open(const uint8_t* filename){
    cli();
    // disable_irq(0);
    desired_virtualized_frequency[schedule_idx] = 2;
    interrupt_count[schedule_idx] = 0;
    interrupt_flag[schedule_idx] = 1;
    // interrupt_count = 0;
    // interrupt_flag = 1;
    
    dentry_t dentry;
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(terminal[schedule_idx].curr_pid +1));   
    // pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));

    int i;
    int32_t fd = -1;   //invalid fd (used if all fd slots are open/unavailable)  

    for(i = FD_MIN; i <= FD_MAX; i++){     //Find an open slot in the fd
        if(pcb->file_descriptor[i].flags == 0){
            fd = i;
            break;
        }
    }

    if((fd == -1) || fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        // enable_irq(0);
        sti();
        return -1;
    }
    if(read_dentry_by_name(filename, &dentry) == -1){ // If the named file does not exist
        // enable_irq(0);
        sti();
        return -1; 
    }
    if(dentry.file_type != RTC_FILE_TYPE){ // If wrong file type
        // enable_irq(0);
        sti();
        return -1;
    }

    if(pcb->file_descriptor[fd].flags == 0){
        pcb->file_descriptor[fd].file_operations_table_pointer = &rtc_fop;
        pcb->file_descriptor[fd].inode = 0; // 0 for directories and RTC
        pcb->file_descriptor[fd].file_position = 0; // starts at the initial position
        pcb->file_descriptor[fd].flags = 1; // descriptor in-use
    }else{
        // enable_irq(0);
        sti();
        return -1; // descriptor is not free
    }
    // enable_irq(0);
    sti();
    return fd;
}

/* void rtc_close()
 * Close a file for RTC
 * inputs:  fd      -     file descriptor
 * outputs: -1      -     invalid fd
 *          0       -     success
 * side effects: set file descriptor flag to 0
 */
int32_t rtc_close(int32_t fd){
    cli();
    pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(terminal[schedule_idx].curr_pid +1));   
    // pcb_struct* pcb = (pcb_struct*)(EIGHT_MB - EIGHT_KB*(process_num+1));

    if(fd < FD_MIN || fd > FD_MAX){ // invalid descriptor (none existing, stdin, stdout)
        sti();
        return -1;
    }
    
    if(pcb->file_descriptor[fd].flags == 1){
        pcb->file_descriptor[fd].flags = 0; // descriptor not used
    }else{
        // Do nothing, descriptor is free already
    }
    sti();
    return 0;
}

/* void rtc_handler()
 * handler for the RTC
 * inputs: none
 * outputs: none
 * side effects: increments video memory every time it recieves an RTC interrupt
 */
void rtc_handler(){
    // disable_irq(0);
    cli();
    interrupt_count[rtc_idx]++;
    if(rtc_idx < 2){
        rtc_idx++;
    }else{
        rtc_idx = 0;
    }
    
    // interrupt_count++;
    if(interrupt_count[schedule_idx] >= MAX_FREQUENCY/(desired_virtualized_frequency[schedule_idx]*3)){
        interrupt_flag[schedule_idx] = 0;
        interrupt_count[schedule_idx] = 0;
    }
    outb(C_REG, INDEX_PORT);	// select register C
    inb(CMOS_PORT);		        // just throw away contents
    // enable_irq(0);
    sti();
}

