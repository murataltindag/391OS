/* pit.h: defining the functions dealing with the PIT */
#include "types.h"
#include "lib.h"
#include "i8259.h"

#define PIT_PIC_PIN     0

void pit_init(void);
void pit_handler(void);
