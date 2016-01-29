#ifndef _TI85_H
#define _TI85_H

/* this is to be used by arch-dependent I/O routines */
Z80 * ti85_init(void);
void ti85_do(Z80 * cpu);
void ti85_reset(Z80 * cpu);
void ti85_destroy(Z80 * cpu);
int LoadSnap(char * filename);
int SaveSnap(char * filename, int overwrite);
extern void (* ti85_emu)(Z80 *);

#ifndef _TI85_C
extern byte * ram;
extern word VidOffset;
extern byte Port3;
#endif

#define TI85_VRAM (ram+VidOffset)
#define TI85_IS_ON ((Port3 & 8) != 0)
#define TI85_IS_OFF ((Port3 & 8) == 0)

#ifndef _TI85_C
extern byte WasInterrupt;
#endif

#define WASTIMERINTERRUPT	0x04
#define WASONINTERRUPT		0x01
#define TI85_ONINTERRUPT() do { if (Port3 & 1) WasInterrupt = WASONINTERRUPT; } while (0)

#ifndef _TI85_C
extern byte keyboard[];
extern byte once_row, once_col;
extern int once_expires;
#endif
#define ONCE_REPEAT 3
#define TI85_ISKEY(row,column) (!(keyboard[(row)] & (1 << (column))))
#define TI85_SETKEY_CORE(row,column) do { keyboard[(row)] &= ~(1 << (column)); } while(0)
#define TI85_SETKEY(row,column) do { if (((row)==5) && ((column)==0) && !TI85_ISKEY(row,column)) TI85_ONINTERRUPT(); TI85_SETKEY_CORE((row),(column)); } while(0)
#define TI85_SETKEYONCE(row,column) do { if (once_row != 255) break; TI85_SETKEY((row),(column)); once_row = (row); once_col = (column); once_expires = ONCE_REPEAT; } while(0)
#define TI85_RESKEY(row,column) do { keyboard[(row)] |= (1 << (column)); } while (0)

#define WAS_HALT(cpu) ((ram[(cpu)->PC.W] == 118) && ((cpu)->IFF & 0x80))

#endif
