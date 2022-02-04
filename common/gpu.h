#ifndef __GPU_H__
#define __GPU_H__

#include "supervision.h"

#ifndef POTATOR_NO_MALLOC
extern uint16	*supervision_palette;
#else
extern uint16	supervision_palette[4];
#endif
extern uint8    supervision_gpu_regs[4];

void gpu_init(void);
void gpu_done(void);
void gpu_reset(void);
void gpu_write(uint32 addr, uint8 data);
uint8 gpu_read(uint32 addr);
void gpu_render_scanline(uint32 scanline, int16 *backbuffer);
void gpu_render_scanline_fast(uint32 scanline, uint16 *backbuffer);
//void gpu_render_scanline(uint32 scanline, int16 *backbuffer); //fast
void gpu_set_colour_scheme(int ws_colourScheme);
int gpu_get_colour_scheme();


#endif
