
#ifndef _SCHEDULING
#define _SCHEDULING

#include "types.h"
#define SCHEDULED_TASKS_NUM 2

void scheduler(void);

volatile int32_t schedule_idx;
int32_t pit_count;
int32_t schedule_array[SCHEDULED_TASKS_NUM+1];

#endif 
