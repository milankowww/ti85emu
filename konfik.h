/* Configuration file for a TI85 emulator */

/*************************/
/** general GUI options **/
/*************************/

/*
   uncomment this, if you have a fast CPU, but slow X, or you send
   the emulator over a network. Uncommenting this option will cause
   ti85emu to avoid refreshing of an unchanged screen.
 */
/* #define DONT_SHOW_NOTHING */

/*
   if you want the emulation o
 */
#define FULLSPEED_HACK

fltk.c:#ifdef DONT_SHOW_NOTHING
fltk.c:#ifdef FULLSPEED_HACK
fltk.c:#ifdef FULLSPEED_HACK
fltk.c:#ifdef DONT_SHOW_NOTHING
fltk.c:#ifdef FLASHING_BUT_FASTER
fltk.c:#ifdef FLASHING_BUT_FASTER
fltk.c:#ifdef DONT_SHOW_NOTHING
gtk-old.c:#ifdef FULLSPEED_HACK
gtk-old.c:#ifdef FULLSPEED_HACK
gtk-old.c:#ifdef DONT_SHOW_NOTHING
gtk-old.c:#ifdef FLASHING_BUT_FASTER
gtk-old.c:#ifdef FLASHING_BUT_FASTER
gtk.c:#ifdef FULLSPEED_HACK
gtk.c:#ifdef FULLSPEED_HACK
gtk.c:#ifdef DONT_SHOW_NOTHING
main.c:#ifdef USE_FLTK
fltk.c:#define NBUTTONS (5*10)
fltk.c:#define SNAPFILE "85snap.sna"
fltk.c:#undef	DONT_SHOW_NOTHING	/* define this to avoid unnecessary screen redrawing */
fltk.c:#define	FLASHING_BUT_FASTER	/* define this to get faster display redraw */
fltk.c:// #define FRAMETIME 5000
fltk.c:#define FRAMETIME 7500
fltk.c:#define VIDEO_EACHN 41
fltk.c:#define TURN(X) ((((X)&1) << 7) | (((X)&2) << 5) | (((X)&4) << 3) | (((X)&8) << 1) | \
fltk.h:#define _FLTK_H
gtk-old.c:#define NBUTTONS (5*10)
gtk-old.c:#define SNAPFILE "85snap.sna"
gtk-old.c:#undef	DONT_SHOW_NOTHING	/* define this to avoid unnecessary screen redrawing */
gtk-old.c:#define	FLASHING_BUT_FASTER	/* define this to get faster display redraw */
gtk-old.c:// #define FRAMETIME 5000
gtk-old.c:#define FRAMETIME 7500
gtk-old.c:#define VIDEO_EACHN 41
gtk.c:#define NBUTTONS (5*10)
gtk.c:#define SNAPFILE "85snap.sna"
gtk.c:#undef	DONT_SHOW_NOTHING	/* define this to avoid unnecessary screen redrawing */
gtk.c:#define	FLASHING_BUT_FASTER	/* define this to get faster display redraw */
gtk.c:// #define FRAMETIME 5000
gtk.c:#define FRAMETIME 7500
gtk.c:#define VIDEO_EACHN 41
