#include "pit.h"
#include "scheduling.h"

/* void rtc_init()
 * Initialize the RTC
 * inputs: none
 * outputs: none
 * side effects: sets up the PIT and enables IRQ0
 */
void pit_init() {
    /* Disable interrupts */
    cli();

    // /* Enable RTC interrupts */
    enable_irq(PIT_PIC_PIN);

    /* Re-enable interrupts */
    sti();
}

/* void pit_handler()
 * Interrupt handler for PIT
 * inputs: none
 * outputs: none
 * side effects: switches to the next active process using the scheduler function
 */
void pit_handler(){
    cli();
    scheduler(); // call scheduler
    sti();
}

