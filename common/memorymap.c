////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#include "memorymap.h"
#include "wsv_sound.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uint8	*memorymap_programRom;
uint32   memorymap_programRomSize;
#ifndef POTATOR_NO_MALLOC
uint8	*memorymap_lowerRam; /* 0x0000-0x1FFF WRAM */
uint8	*memorymap_regs;     /* 0x2000-0x3FFF Hardware registers */
uint8	*memorymap_upperRam; /* 0x4000-0x5FFF VRAM */
#else
uint8	memorymap_lowerRam[0x2000]; /* 0x0000-0x1FFF WRAM */
uint8	memorymap_regs[0x2000];     /* 0x2000-0x3FFF Hardware registers */
uint8	memorymap_upperRam[0x2000]; /* 0x4000-0x5FFF VRAM */
#endif
uint8	*memorymap_lowerRomBank;    /* 0x8000-0xBFFF Paged ROM Bank */
uint8	*memorymap_upperRomBank;    /* 0xC000-0xFFFF Fixed ROM Bank */

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint8 *memorymap_getRomPointer(void)
{
	return(memorymap_programRom);
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
void memorymap_init()
{
	//fprintf(log_get(), "memorymap: init\n");
#ifndef POTATOR_NO_MALLOC
	memory_malloc_secure((void**)&memorymap_lowerRam, 0x2000, "Lower ram");
	memory_malloc_secure((void**)&memorymap_upperRam, 0x2000, "Upper ram");
	memory_malloc_secure((void**)&memorymap_regs,     0x2000, "Internal registers");
#else
	memset(memorymap_lowerRam,0x00,0x2000);
	memset(memorymap_upperRam,0x00,0x2000);
	memset(memorymap_regs,0x00,0x2000);
#endif
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
void memorymap_done()
{
	//fprintf(log_get(), "memorymap: done\n");
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
void memorymap_reset()
{
	//fprintf(log_get(), "memorymap: reset\n");
	memorymap_lowerRomBank = memorymap_programRom + 0x0000;
	memorymap_upperRomBank = memorymap_programRom + (memorymap_programRomSize-0x4000);//==0x10000?0xc000:0x4000);

	memset(memorymap_lowerRam, 0x00, 0x2000);
	memset(memorymap_upperRam, 0x00, 0x2000);
}

void memorymap_setTimerBit() {
	memorymap_regs[0x27] = memorymap_regs[0x27]|0x01;
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
uint8  memorymap_registers_read(uint32 Addr)
{
	uint8 data = memorymap_regs[Addr&0x1fff]; 
	switch (Addr&0x1fff)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
			data = gpu_read(Addr);
			break;
	case 0x20:
			data = controls_read(Addr);
			break;
	case 0x21:
			data = io_read(Addr);
			break;
	case 0x24: // reset timer status
			memorymap_regs[0x27] = memorymap_regs[0x27]&0xFE;
			break;
	case 0x25: // reset audio dma status
			memorymap_regs[0x27] = memorymap_regs[0x27]&0xFD;
			break;
	}
//	printf("read[%x]=%x\n",Addr,data);
//	iprintf("regs: reading 0x%.2x from 0x%.4x\n", data, Addr);
	return(data);
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
void memorymap_registers_write(uint32 Addr, uint8 Value)
{
	memorymap_regs[Addr&0x1fff] = Value;
//	printf("write[%x]=%x\n",Addr,Value);

	switch (Addr&0x1fff)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:  gpu_write(Addr, Value);
				break;
	case 0x22:	io_write(Addr, Value);
				break;
	case 0x23:  if (memorymap_regs[0x26]&0x10) {
					timer_write(Addr, Value,16384);
				} else {
					timer_write(Addr, Value,256);
				}
				break;
	case 0x24: memorymap_regs[0x27] = memorymap_regs[0x27]&0xFE; // reset timer_status
				break;
	case 0x26: /*SYS_CONFIG*/
	{
			int bank = ((Value & 0xe0) >> 5) % (memorymap_programRomSize / 0x4000);
			//fprintf(log_get(), "memorymap: writing 0x%.2x to rom bank register\n", Value);
			memorymap_lowerRomBank = memorymap_programRom + (bank * 0x4000);
			// Fixed rom bank always pointing to last 16kB of rom memory
			memorymap_upperRomBank = memorymap_programRom + (memorymap_programRomSize-0x4000);
			return;
	}
	case 0x27: /*SYS_STATUS*/
				//fprintf(log_get(), "regs: writing 0x%.2x from 0x%.4x\n", Value, Addr);		
				break;
	case 0x10:
	case 0x11:
	case 0x12:	
	case 0x13:	// ALEK
	case 0x14:
	case 0x15:
	case 0x16:	
	case 0x17:		// ALEK
		soundport_w(((Addr&0x4)>>2), Addr&3, Value); break;
		//sound_write(Addr&7, Value); break;
	case 0x28:
	case 0x29:
	case 0x2a: 
		svision_noise_w(Addr&0x07, Value); break;
		//sound_noise_write(Addr&0x07, Value); break;
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
#ifdef GP2X
		if(currentConfig.enable_sound) sound_audio_dma(Addr&0x07, Value); break;
#else
		svision_sounddma_w(Addr&0x07, Value); break;
		//sound_audio_dma(Addr&0x07, Value); break;
#endif
	}
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
void Wr6502(register word Addr,register byte Value)
{
//	printf("Wr6502[%x] (%x) : %x\n",Addr,Addr>>12,Value);

	Addr&=0xffff;
	switch (Addr>>12)
	{
	case 0x0:
	case 0x1:
		memorymap_lowerRam[Addr] = Value;
		return;
	case 0x2:
	case 0x3:
		memorymap_registers_write(Addr, Value);
		return;
	case 0x4:
	case 0x5:
		memorymap_upperRam[Addr&0x1fff] = Value;
		return;
	}
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
byte Rd6502(register word Addr)
{
	Addr&=0xffff;
	byte value = 0xff;
	char trace = 1;

	switch (Addr>>12)
	{
	case 0x0:
	case 0x1:
		value = memorymap_lowerRam[Addr];
		break;
	case 0x2:
	case 0x3:
		value = memorymap_registers_read(Addr);
		break;
	case 0x4:
	case 0x5:
		value = memorymap_upperRam[Addr&0x1fff];
		break;
	case 0x6:
	case 0x7:
		value = memorymap_programRom[Addr&0x1fff];
		break;
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
		value = memorymap_lowerRomBank[Addr&0x3fff];
		break;
	case 0xc:
	case 0xd:
	case 0xe:
	case 0xf:
		value = memorymap_upperRomBank[Addr&0x3fff];
		break;
	}
//	if (trace)
//		printf("Rd6502[%x] (%x) = %x\n",Addr,Addr>>12,value);

	return(value);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
void memorymap_load(uint8 *rom, uint32 size)
{
	memorymap_programRomSize = size;
	memorymap_programRom = rom;
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
uint8 *memorymap_getUpperRamPointer(void)
{
	return(memorymap_upperRam);
}
uint8 *memorymap_getLowerRamPointer(void)
{
	return(memorymap_lowerRam);
}
uint8 *memorymap_getUpperRomBank(void)
{
	return(memorymap_upperRomBank);
}
uint8 *memorymap_getLowerRomBank(void)
{
	return(memorymap_lowerRomBank);
}
uint8 *memorymap_getRegisters(void)
{
	return(memorymap_regs);
}
