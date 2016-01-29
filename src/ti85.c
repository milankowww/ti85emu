#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define _TI85_C

#include "Z80.h"
#include "ti85.h"

// #define FRAMETIME 5000
#define FRAMETIME 7500


/* ti85 machine state */

static Z80 *	cpu = NULL;

word		VidOffset = 0;
byte		KeypadMask = 0;
byte		DisplayContrast = 0;
byte		Port3 = 0;
byte		PowerReg = 0;
byte		LinkReg = 0x0f;
byte		CurrentRom = 1;
byte		WasInterrupt = 0;

byte * 		ram;
byte *		rom[8];
#define KBSIZE 8
byte		keyboard[KBSIZE];
byte		once_row, once_col;
int		once_expires;

void (* ti85_emu)(Z80 *) = NULL; /* it is not wise to declare this
				  (as it is slow) and then de-multiplex
				  the ti85_emu_* routines to gain some
				  performance :( Ideas? */

/* routines for local use */
int readblock(int fd, byte * buf, int len)
{
	int i;

	do {
		i = read(fd, buf, len);
		if (i <= 0)
			return -1;
		len -= i;
		buf += i;
	} while (len);
	return 0;
}

int readbyte(int fd, byte * retval)
{
	byte x;
	int i;

	i = read(fd, &x, 1);
	if (i != 1)
		return -1;
	if (retval)
		*retval = x;
	return x;
}

int readword(int fd, word * retval)
{
	pair retword;

	if (readbyte(fd, &(retword.B.l)) == -1)
		return -1;
	if (readbyte(fd, &(retword.B.h)) == -1)
		return -1;
	if (retval)
		*retval = retword.W;
	return retword.W;
}

int writebyte(int fd, byte b)
{
	return write(fd, &b, 1);
}

int writeword(int fd, word w)
{
	if (writebyte(fd, w & 255) != 1)
		return -1;
	return writebyte(fd, w >> 8);
}

int writeblock(int fd, byte * buf, int len)
{
	int i;

	do {
		i = write(fd, buf, len);
		if (i <= 0)
			return -1;
		len -= i;
		buf += i;
	} while (len);
	return 0;
}

/* interface for upper layer (main.c, gui.c) */

void ti85_reset(Z80 * cpu)
{
	int i;

	memset(ram+32768, 0, 32768);
	memcpy(ram, rom[0], 16384);
	memcpy(ram+16384, rom[1], 16384);

	for (i=0; i<8; i++)
		keyboard[i] = 0xff;
	once_row = once_col = 255;

	VidOffset = 0;
	KeypadMask = 0;
	DisplayContrast = 0;
	Port3 = 0;
	PowerReg = 0;
	LinkReg = 0x0f;
	CurrentRom = 1;
	WasInterrupt = 0;

	ResetZ80(cpu);
}

void ti85_emu_full(Z80 * cpu) {
	static struct timeval last_tv = {0, 0, };
	struct timeval new_tv;
	unsigned long elapsed;

	ti85_do(cpu);
	gettimeofday(&new_tv, NULL);
	elapsed = new_tv.tv_usec + 1000000 * (new_tv.tv_sec - last_tv.tv_sec) - last_tv.tv_usec;
	if (elapsed >= 5000) {
		last_tv = new_tv;
		sched_yield();
	}
}

void ti85_emu_freal(Z80 * cpu) {
	static struct timeval last_tv = {0, 0};
	struct timeval new_tv;
	unsigned long elapsed;

	ti85_do(cpu);
	gettimeofday(&new_tv, NULL);
	elapsed = new_tv.tv_usec + 1000000 * (new_tv.tv_sec - last_tv.tv_sec) - last_tv.tv_usec;
	last_tv = new_tv;
	if ((elapsed < FRAMETIME) && WAS_HALT(cpu)) {
		new_tv.tv_usec = FRAMETIME - elapsed;
		new_tv.tv_sec = 0;
		select(0, NULL, NULL, NULL, &new_tv);
	}
}

void ti85_emu_real(Z80 * cpu) {
	static struct timeval last_tv = {0, 0};
	struct timeval new_tv;
	unsigned long elapsed;

	ti85_do(cpu);
	gettimeofday(&new_tv, NULL);
	elapsed = new_tv.tv_usec + 1000000 * (new_tv.tv_sec - last_tv.tv_sec) - last_tv.tv_usec;
	last_tv = new_tv;
	if ((elapsed < FRAMETIME)) {
		new_tv.tv_usec = FRAMETIME - elapsed;
		new_tv.tv_sec = 0;
		select(0, NULL, NULL, NULL, &new_tv);
	}
}

Z80 * ti85_init(void)
{
	int i, j;

	cpu = (Z80 *)malloc(sizeof(Z80));
	if (!cpu)
		return NULL;
	ResetZ80(cpu);
	cpu->IPeriod = 30000;

	if ((ram = (byte *)malloc(65536)) == NULL)
		return NULL;

#ifdef SPEED_FULL
	ti85_emu = ti85_emu_full;
#else
#ifdef SPEED_FREAL
	ti85_emu = ti85_emu_freal;
#else
#ifdef SPEED_REAL
	ti85_emu = ti85_emu_real;
#else
#error Run at which speed?
#endif
#endif
#endif

	j = open(ROMFILE, O_RDONLY | O_BINARY);

	for (i=0; i<8; i++) {
		if ((rom[i] = (byte *)malloc(16384)) == NULL)
			return NULL;
		if (readblock (j, rom[i], 16384) == -1) {
			puts("cannot load rom image");
			return NULL;
		}
	}

	ti85_reset(cpu);
	return cpu;
}

void ti85_do(Z80 * cpu)
{
	if (WasInterrupt)
		IntZ80(cpu, INT_IRQ);
	RunZ80(cpu);
	// printf("%d %d          \r", cpu->PC.W, VidOffset);
}

void ti85_destroy(Z80 * cpu)
{
	free(cpu);
}

/* support routines */

void MapROM(register byte NewRom)
{
	NewRom &= 7;
	if (CurrentRom != NewRom) {
		CurrentRom = NewRom;
		memcpy(ram+16384, rom[NewRom], 16384);
	}
}

#define SNAP_PASSWORD "ti85emu snap"

int LoadSnap(char * filename)
{
	int i;
	struct stat st;
	byte x;
	char tmp[32];

	i = open(filename, O_RDONLY | O_BINARY);
	if (i == -1)
		return -1;

	memset(keyboard, 0xff, KBSIZE);
	once_row = once_col = 255;
	WasInterrupt = WASTIMERINTERRUPT;

	fstat(i, &st);
	if (st.st_size == 0x8028) /* ti8xemu savfile */ {
		readblock(i, ram+32768, 32768);
		readbyte(i,&(cpu->AF.B.h)); readbyte(i,&(cpu->AF.B.l));
		readbyte(i,&(cpu->BC.B.h)); readbyte(i,&(cpu->BC.B.l));
		readbyte(i,&(cpu->DE.B.h)); readbyte(i,&(cpu->DE.B.l));
		readbyte(i,&(cpu->HL.B.h)); readbyte(i,&(cpu->HL.B.l));
		readbyte(i,&(cpu->IX.B.h)); readbyte(i,&(cpu->IX.B.l));
		readbyte(i,&(cpu->IY.B.h)); readbyte(i,&(cpu->IY.B.l));
		readbyte(i,&(cpu->PC.B.h)); readbyte(i,&(cpu->PC.B.l));
		readbyte(i,&(cpu->SP.B.h)); readbyte(i,&(cpu->SP.B.l));
		readbyte(i,&(cpu->AF1.B.h)); readbyte(i,&(cpu->AF1.B.l));
		readbyte(i,&(cpu->BC1.B.h)); readbyte(i,&(cpu->BC1.B.l));
		readbyte(i,&(cpu->DE1.B.h)); readbyte(i,&(cpu->DE1.B.l));
		readbyte(i,&(cpu->HL1.B.h)); readbyte(i,&(cpu->HL1.B.l));
		readbyte(i,&(cpu->IFF));
		readbyte(i,&(cpu->IFF)); /* XXX is this ok? */
		readbyte(i,&(x)); if (x) {cpu->IFF |= 0x80; cpu->ICount=0;}
		readbyte(i,&(x));
		switch (x) {
			case 0: cpu->IFF&=~(0x02|0x04); break;
			case 1: cpu->IFF|=0x02; break;
			case 2: cpu->IFF|=0x04; break;
		}
		readbyte(i,&(cpu->I));
		readbyte(i,&(x)); cpu->R = x & 127; readbyte(i,&(x)); cpu->R |= x & 128;

		Port3 = 0;
		readbyte(i, &x); /* KeypadMask */ KeypadMask = x;
		readbyte(i, &x); /* ONmask */ if (x) Port3 |= 1; else Port3 &= ~1;
		readbyte(i, &x); /* LinkReg - broken */ // LinkReg = x;
		LinkReg = 0x0f;
		readbyte(i, &x); /* LCDmask ;) */ if (x) Port3 |= 2; else Port3 &= ~2;
		readbyte(i, &x); /* LCDon */ if (x) Port3 |= 8; else Port3 &= ~8;
		readbyte(i, &x); // Contrast
		DisplayContrast = x;

		readbyte(i, &x); /* ROM bank */ OutZ80(5, x);
		readbyte(i, &x); /* VidOffset */ OutZ80(0, x);
		readbyte(i, &x); /* PowerReg */ PowerReg = x;

		close(i);
		return 0;
	} else if (st.st_size == 0x802F) /* our own snap file */ {
		readblock(i, (byte *)tmp, strlen(SNAP_PASSWORD));
		if (!strcmp(tmp, SNAP_PASSWORD)) {
			close(i);
			return -1;
		}

		readblock(i, ram+32768, 32768);
		readword(i,&(cpu->AF.W)); readword(i,&(cpu->BC.W));
		readword(i,&(cpu->DE.W)); readword(i,&(cpu->HL.W));
		readword(i,&(cpu->IX.W)); readword(i,&(cpu->IY.W));
		readword(i,&(cpu->PC.W)); readword(i,&(cpu->SP.W));
		readword(i,&(cpu->AF1.W)); readword(i,&(cpu->BC1.W));
		readword(i,&(cpu->DE1.W)); readword(i,&(cpu->HL1.W));
		readbyte(i,&(cpu->IFF)); readbyte(i,&(cpu->I));
		readbyte(i,&(cpu->R));

		readword(i, &VidOffset); readbyte(i, &KeypadMask);
		readbyte(i, &DisplayContrast); readbyte(i, &Port3);
		readbyte(i, &PowerReg); readbyte(i, &LinkReg);
		readbyte(i, &x); OutZ80(5, x);

		close(i);
		return 0;
	//} else if (st.st_size == INE) /* vti state file */ {
	//	close(i);
	//	return 0;
	}
	close(i);
	return -1;
}

int SaveSnap(char * filename, int overwrite)
{
	int i;

	i = open(filename, O_WRONLY | O_BINARY | O_CREAT, 0644);
	if (i == -1)
		return -1;

#define W(x) writeblock(i, (byte *)(x), strlen(x))
	W(SNAP_PASSWORD);
#undef W
	writeblock(i, ram+32768, 32768);
	writeword(i,cpu->AF.W); writeword(i,cpu->BC.W);
	writeword(i,cpu->DE.W); writeword(i,cpu->HL.W);
	writeword(i,cpu->IX.W); writeword(i,cpu->IY.W);
	writeword(i,cpu->PC.W); writeword(i,cpu->SP.W);
	writeword(i,cpu->AF1.W); writeword(i,cpu->BC1.W);
	writeword(i,cpu->DE1.W); writeword(i,cpu->HL1.W);
	writebyte(i,cpu->IFF); writebyte(i,cpu->I);
	writebyte(i,cpu->R);

	writeword(i,VidOffset); writebyte(i, KeypadMask);
	writebyte(i, DisplayContrast); writebyte(i, Port3);
	writebyte(i, PowerReg); writebyte(i, LinkReg);
	writebyte(i, CurrentRom);

	close(i);
	return 0;
}

/* interface for the lower layer (msx.c) */

#if 0 /* now deifined directly in MSX/Z80.c */
void WrZ80(register word Addr, register byte Value)
{
	if (Addr < 32768)
		return;
	ram[Addr] = Value;
}

byte RdZ80(register word Addr)
{
	return ram[Addr];
}
#endif

void OutZ80(register word Port, register byte Value)
{
	switch (Port) {
	case 0:	VidOffset = ((Value&0x3f) + 0xc0) << 8;
		break;
	case 1:	KeypadMask = Value;
		break;
	case 2:	DisplayContrast = Value;
		break;	
	case 3:	Port3 = Value;
		// printf ("OUT 3, %.2x\n", Port3);
		break;
	case 4:
		break;
	case 5:	MapROM(Value);
		break;
	case 6:	PowerReg = Value;
		break;
	case 7:	LinkReg=(LinkReg & 0xF3) | (Value & 0x0C);
		break;
	}
}

byte InZ80(register word Port)
{
	int j;
	
	switch (Port & 7) {
	case 1:	j = 0xff;
		if (!(KeypadMask & 1))	j &= keyboard[0];
		if (!(KeypadMask & 2))	j &= keyboard[1];
		if (!(KeypadMask & 4))	j &= keyboard[2];
		if (!(KeypadMask & 8))	j &= keyboard[3];
		if (!(KeypadMask & 16))	j &= keyboard[4];
		if (!(KeypadMask & 32))	j &= keyboard[5];
		if (!(KeypadMask & 64))	j &= keyboard[6];
		// if (!(KeypadMask & 128))j &= keyboard[7];
		if ((once_row != 255) && !( KeypadMask&(1<<once_row) ))
			if (once_expires-- == 0) {
			TI85_RESKEY(once_row, once_col);
			once_row = once_col = 255;
		}
		return j;
	case 2:	return DisplayContrast & 0x1f;
	case 3:	j = WasInterrupt; WasInterrupt = 0;
		if (j == WASONINTERRUPT) {
			Port3 |= 4;
			once_expires = 3000;
		}
		if (TI85_ISKEY(5,0) /* ON key is held down */) {
			if ((once_row == 5) && (once_col == 0))
		       		if (once_expires-- == 0) {
				TI85_RESKEY(once_row, once_col);
				once_row = once_col = 255;
			}
		} else
			j |= 8;
		if (Port3 & 8 /* LCD is ON */)
			j |= Port3 & 2;
		// printf ("IN  3, %.2x\n", j);
		return j;
	case 5:	return CurrentRom;
	case 6:	return PowerReg;
	case 7:	return LinkReg;
	}
	return 0xff;
}

void PatchZ80(register Z80 *R)
{
	return;
}

byte DebugZ80(register Z80 *R)
{
	return 1;
}

word LoopZ80(register Z80 *R)
{
	if (Port3 & 2)
		WasInterrupt = WASTIMERINTERRUPT;
	else
		WasInterrupt = 0;
	return INT_QUIT;
}
