////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#include "supervision.h"
#include "memorymap.h"
#include "wsv_sound.h"
#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef GP2X
#include "menues.h"
#include "minimal.h"
#endif
#ifdef NDS
#include <nds.h>
#endif

static M6502	m6502_registers;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
byte Loop6502(register M6502 *R)
{
	return(INT_QUIT);
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
void supervision_init(void)
{
	//fprintf(log_get(), "supervision: init\n");
	#ifndef DEBUG
	//iprintf("supervision: init\n");
	#endif

	memorymap_init();
	io_init();
	gpu_init();
	timer_init();
	controls_init();
	supervision_sound_init();
	interrupts_init();
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
BOOL supervision_load(uint8 *rom, uint32 romSize)
{
	//uint32 supervision_programRomSize;
	//uint8 *supervision_programRom = memorymap_rom_load(szPath, &supervision_programRomSize);
	#ifdef DEBUG
	//iprintf("supervision: load\n");
	#endif

	memorymap_load(rom, romSize);
	supervision_reset();

	return(TRUE);
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
void supervision_reset(void)
{
	//fprintf(log_get(), "supervision: reset\n");


	memorymap_reset();
	io_reset();
	gpu_reset();
	timer_reset();
	controls_reset();
	supervision_sound_reset();
	interrupts_reset();

	Reset6502(&m6502_registers);
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
void supervision_reset_handler(void)
{
	//fprintf(log_get(), "supervision: reset handler\n");
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
void supervision_done(void)
{
	//fprintf(log_get(), "supervision: done\n");
	memorymap_done();
	io_done();
	gpu_done();
	timer_done();
	controls_done();
	sound_done();
	interrupts_done();
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
void supervision_set_colour_scheme(int sv_colourScheme)
{
	gpu_set_colour_scheme(sv_colourScheme);
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
M6502	*supervision_get6502regs(void)
{
	return(&m6502_registers);
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
BOOL supervision_update_input(void)
{
	return(controls_update());
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
void supervision_exec(int16 *backbuffer, BOOL bRender)
{
	uint32 supervision_scanline, scan1=0;

	for (supervision_scanline = 0; supervision_scanline < 160; supervision_scanline++)
	{
		m6502_registers.ICount = 512; 
		timer_exec(m6502_registers.ICount);
#ifdef GP2X
		if(currentConfig.enable_sound) sound_exec(11025/160);
#else
		//sound_exec(22050/160);
#endif
		Run6502(&m6502_registers);
#ifdef NDS
		gpu_render_scanline(supervision_scanline, backbuffer);
		backbuffer += 160+96;
#elif defined (__linux__)

		gpu_render_scanline(supervision_scanline, backbuffer);
		backbuffer += 160;
#else
		gpu_render_scanline_fast(scan1, backbuffer);
		backbuffer += 160;
		scan1 += 0x30;
#endif
	}

	if (Rd6502(0x2026)&0x01)
		Int6502(supervision_get6502regs(), INT_NMI);
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
void supervision_turnSound(BOOL bOn)
{
	audio_turnSound(bOn);
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

#ifndef POTATOR_NO_FS
int	sv_loadState(char *statepath, int id)
{
	FILE* fp;
	char newPath[256];

	strcpy(newPath,statepath);
	sprintf(newPath+strlen(newPath)-3,".s%d",id);

#ifdef GP2X
	gp2x_printf(0,10,220,"newPath = %s",newPath);
	gp2x_video_RGB_flip(0);
#endif
#ifdef NDS
	iprintf("\nnewPath = %s",newPath);
#endif

	fp=fopen(newPath,"rb");

	if (fp) {
		fread(&m6502_registers, 1, sizeof(m6502_registers), fp);
		fread(memorymap_programRom, 1, sizeof(memorymap_programRom), fp);
		fread(memorymap_lowerRam, 1, 0x2000, fp);
		fread(memorymap_upperRam, 1, 0x2000, fp);
		fread(memorymap_lowerRomBank, 1, sizeof(memorymap_lowerRomBank), fp);
		fread(memorymap_upperRomBank, 1, sizeof(memorymap_upperRomBank), fp);
		fread(memorymap_regs, 1, 0x2000, fp);
		fclose(fp);
	}

#ifdef GP2X
	sleep(1);
#endif

	return(1);
}
#else
int	sv_loadState_flash(uint8 *src_buffer)
{
	uint16 offset = 0;
	int8 bank;
	if (memcmp(src_buffer,"WSV",3) == 0) {
		offset+=3;
		memcpy(supervision_gpu_regs,src_buffer+offset,4*sizeof(int8));
		offset+=4*sizeof(int8);
		memcpy(&supervision_io_data,src_buffer+offset,sizeof(int8));
		offset+=sizeof(int8);
		memcpy(&supervision_timer_reg,src_buffer+offset,sizeof(int8));
		offset+=sizeof(int8);
		memcpy(&supervision_timer_cycles,src_buffer+offset,sizeof(int32));
		offset+=sizeof(int32);
		memcpy(&supervision_timer_activated,src_buffer+offset,sizeof(BOOL));
		offset+=sizeof(BOOL);
		memcpy(memorymap_lowerRam,src_buffer+offset,0x2000);
		offset+=0x2000;
		memcpy(memorymap_upperRam,src_buffer+offset,0x2000);
		offset+=0x2000;
		memcpy(memorymap_regs,src_buffer+offset,0x2000);
		offset+=0x2000;
		memcpy(&m6502_registers,src_buffer+offset,sizeof(m6502_registers));
		offset+=sizeof(m6502_registers);

		// update bank address
		bank = ((memorymap_regs[0x26] & 0xe0) >> 5) % (memorymap_programRomSize / 0x4000);
		//fprintf(log_get(), "memorymap: writing 0x%.2x to rom bank register\n", Value);
		memorymap_lowerRomBank = memorymap_programRom + (bank * 0x4000);
		// Fixed rom bank always pointing to last 16kB of rom memory
		memorymap_upperRomBank = memorymap_programRom + (memorymap_programRomSize-0x4000);
	}

	return(offset);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

#ifndef POTATOR_NO_FS
int	sv_saveState(char *statepath, int id)
{
	FILE* fp;
	char newPath[256];

	strcpy(newPath,statepath);
	sprintf(newPath+strlen(newPath)-3,".s%d",id);

#ifdef GP2X
	gp2x_printf(0,10,220,"newPath = %s",newPath);
	gp2x_video_RGB_flip(0);
#endif
#ifdef NDS
	iprintf("\nnewPath = %s",newPath);
#endif

	fp=fopen(newPath,"wb");

	if (fp) {
		fwrite(&m6502_registers, 1, sizeof(m6502_registers), fp);
		fwrite(memorymap_programRom, 1, sizeof(memorymap_programRom), fp);
		fwrite(memorymap_lowerRam, 1, 0x2000, fp);
		fwrite(memorymap_upperRam, 1, 0x2000, fp);
		fwrite(memorymap_lowerRomBank, 1, sizeof(memorymap_lowerRomBank), fp);
		fwrite(memorymap_upperRomBank, 1, sizeof(memorymap_upperRomBank), fp);
		fwrite(memorymap_regs, 1, 0x2000, fp);
		fflush(fp);
		fclose(fp);
#ifdef GP2X
		sync();
#endif
	}

#ifdef GP2X
	sleep(1);
#endif

	return(1);
}
#else
int	sv_saveState_flash(uint8 *dest_buffer)
{
	uint16 offset = 0;
	memcpy(dest_buffer+offset,"WSV",3);
	offset+=3;
	memcpy(dest_buffer+offset,supervision_gpu_regs,4*sizeof(int8));
	offset+=4*sizeof(int8);
	memcpy(dest_buffer+offset,&supervision_io_data,sizeof(int8));
	offset+=sizeof(int8);
	memcpy(dest_buffer+offset,&supervision_timer_reg,sizeof(int8));
	offset+=sizeof(int8);
	memcpy(dest_buffer+offset,&supervision_timer_cycles,sizeof(int32));
	offset+=sizeof(int32);
	memcpy(dest_buffer+offset,&supervision_timer_activated,sizeof(BOOL));
	offset+=sizeof(BOOL);
	memcpy(dest_buffer+offset,memorymap_lowerRam,0x2000);
	offset+=0x2000;
	memcpy(dest_buffer+offset,memorymap_upperRam,0x2000);
	offset+=0x2000;
	memcpy(dest_buffer+offset,memorymap_regs,0x2000);
	offset+=0x2000;
	memcpy(dest_buffer+offset,&m6502_registers,sizeof(m6502_registers));
	offset+=sizeof(m6502_registers);
	return(offset);
}
#endif
