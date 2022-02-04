#ifndef __MEMORYMAP_H__
#define __MEMORYMAP_H__

#include "supervision.h"

void memorymap_init();
void memorymap_done();
void memorymap_reset();
uint8  memorymap_registers_read(uint32 Addr);
void memorymap_registers_write(uint32 Addr, uint8 Value);
void memorymap_load(uint8 *rom, uint32 size);

void memorymap_setTimerBit();

uint8 *memorymap_getUpperRamPointer(void);
uint8 *memorymap_getLowerRamPointer(void);
uint8 *memorymap_getUpperRomBank(void);
uint8 *memorymap_getLowerRomBank(void);
uint8 *memorymap_getRegisters(void);
uint8 *memorymap_getRomPointer(void);

extern uint8	*memorymap_programRom;
extern uint32   memorymap_programRomSize;
#ifndef POTATOR_NO_MALLOC
extern uint8	*memorymap_lowerRam;
extern uint8	*memorymap_upperRam;
extern uint8	*memorymap_regs;
#else
extern uint8	memorymap_lowerRam[0x2000];
extern uint8	memorymap_upperRam[0x2000];
extern uint8	memorymap_regs[0x2000];
#endif
extern uint8	*memorymap_lowerRomBank;
extern uint8	*memorymap_upperRomBank;

#endif

