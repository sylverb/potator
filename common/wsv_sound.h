#ifndef __WSV_SOUND_H__
#define __WSV_SOUND_H__

#include "supervision.h"

#define BPS			44100


void supervision_sound_init();
void supervision_sound_reset();
void sound_done();
void sound_write(uint32 Addr, uint8 data);
void sound_noise_write(uint32 Addr, uint8 data);
void sound_audio_dma(uint32 Addr, uint8 data);
void sound_exec(uint32 cycles);
void audio_turnSound(BOOL bOn);

#endif
