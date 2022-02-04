#include "timer.h"
#include <stdlib.h>
#include <stdio.h>

uint8 supervision_timer_reg;
int32 supervision_timer_cycles;
BOOL  supervision_timer_activated;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void timer_init()
{
	//fprintf(log_get(), "timer: init\n");
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void timer_done()
{
	//fprintf(log_get(), "timer: done\n");
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void timer_reset()
{
	//fprintf(log_get(), "timer: reset\n");
	supervision_timer_reg = 0x00;

	supervision_timer_cycles = 0;
	supervision_timer_activated = FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void timer_write(uint32 addr, uint8 data, uint32 prescale)
{
	//iprintf("timer: writing 0x%.2x at 0x%.4x\n", data, addr);
	supervision_timer_reg = data;
	supervision_timer_cycles = ((uint32)data)*prescale;
	supervision_timer_activated = TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint8 timer_read(uint32 addr)
{
	return(supervision_timer_reg);
}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void timer_exec(uint32 cycles)
{
	if (supervision_timer_activated)
	{
		supervision_timer_cycles-=cycles;
		
		if (supervision_timer_cycles<0)
		{
			supervision_timer_reg = 0;
			memorymap_setTimerBit();
//			fprintf(log_get(), "timer: irq\n");
			interrupts_irq();
			supervision_timer_activated=FALSE;
		}
	}
}
