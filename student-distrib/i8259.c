/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = INITIAL_MASK; /* IRQs 0-7  */
uint8_t slave_mask = INITIAL_MASK;  /* IRQs 8-15 */

/* void i8259_init(void);
 * Inputs: void
 * Return Value: void
 *  Function: Initialize the 8259 PIC */
void i8259_init(void) {

    /* mask all of 8259-1, 8259-2 */
    outb(INITIAL_MASK, MASTER_8259_PORT_DATA);
    outb(INITIAL_MASK, SLAVE_8259_PORT_DATA);

    /* Initialize PIC 1 */
    outb(ICW1, MASTER_8259_PORT_CTRL);           // ICW1
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);    // ICW2 
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);    // ICW3
    outb(ICW4, MASTER_8259_PORT_DATA);           // ICW4

    /* Initialize PIC 2 */
    outb(ICW1, SLAVE_8259_PORT_CTRL);            // ICW1
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);      // ICW2 
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);      // ICW3
    outb(ICW4, SLAVE_8259_PORT_DATA);            // ICW4

    /* Init cascade IRQ */
    enable_irq(SLAVE_PIN);
}

/* void enable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num - the IRQ number of the interrupt
 * Return Value: void
 *  Function: unmask the IRQ bit for the given interrupt */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    if(irq_num < PIC_PIN_NUMBERS) { // For the master PIC
        port = MASTER_8259_PORT_DATA;
        value = master_mask & ~(1 << irq_num);
        master_mask = value;
    } else { // For the slave PIC
        port = SLAVE_8259_PORT_DATA;
        irq_num -= PIC_PIN_NUMBERS;
        value = slave_mask & ~(1 << irq_num);
        slave_mask = value;
    }
    outb(value, port);  
}

/* void disable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num - the IRQ number of the interrupt
 * Return Value: void
 *  Function: mask the IRQ bit for the given interrupt */
void disable_irq(uint32_t irq_num) {
    cli();
    uint16_t port;
    uint8_t value;
    if(irq_num < PIC_PIN_NUMBERS) { // For the master PIC
        port = MASTER_8259_PORT_DATA;
        value = master_mask | (1 << irq_num);
        master_mask = value;
    } else { // For the slave PIC
        port = SLAVE_8259_PORT_DATA;
        irq_num -= PIC_PIN_NUMBERS;
        value = slave_mask | (1 << irq_num);
        slave_mask = value;
    }
    outb(value, port);  
    sti();
}

/* void send_eoi(uint32_t irq_num);
 * Inputs: uint32_t irq_num - the IRQ number of the interrupt
 * Return Value: void
 * Function: Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	if(irq_num >= PIC_PIN_NUMBERS){ // For the slave PIC
        irq_num -= PIC_PIN_NUMBERS;
		outb((EOI | irq_num), SLAVE_8259_PORT_CTRL);
	    outb((EOI | SLAVE_PIN), MASTER_8259_PORT_CTRL);
    } // For the master PIC
    else outb((EOI | irq_num), MASTER_8259_PORT_CTRL);
}
