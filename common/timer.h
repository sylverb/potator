#ifndef __TIMER_H__
#define __TIMER_H__

#include "supervision.h"

extern uint8 supervision_timer_reg;
extern int32 supervision_timer_cycles;
extern BOOL  supervision_timer_activated;

void timer_init();
void timer_done();
void timer_reset();
void timer_write(uint32 addr, uint8 data, uint32 prescale);
uint8 timer_read(uint32 addr);
void timer_exec(uint32 cycles);

#endif
