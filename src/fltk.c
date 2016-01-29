#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

#include "Z80.h"
#include "ti85.h"
#include "icons.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>

#include <FL/fl_draw.H>
#include <FL/fl_file_chooser.H>
#include <FL/fl_message.H>

#define NBUTTONS (5*10)

#define VIDEO_EACHN 41

static Z80 * cpu = NULL;
Fl_Bitmap * fltk_disp;
byte videoram_copy[64*16];
#ifndef FLTK_FLASHING
byte videoram_inv_copy[64*16];
#endif
#ifdef FLTK_RELEVANT
byte vram_backup[64*16];
int force_refresh = 1;
#endif

byte bitturntable[256];

struct key_desc {
	char * label;
	int row;
	int column;
	int shortcut[5];
	char ** pixmap;
} labels[NBUTTONS] = {
	{"F1",6,4, {FL_F+1,0}, f1_xpm},
	{"F2",6,3, {FL_F+2,0}, f2_xpm},
	{"F3",6,2, {FL_F+3,0}, f3_xpm},
	{"F4",6,1, {FL_F+4,0}, f4_xpm},
	{"F5",6,0, {FL_F+5,0}, f5_xpm},

	{"2nd",6,5,{FL_Shift_L,0}, _2nd_xpm},
	{"EXIT",6,6,{FL_Escape, 0}, exit_xpm},
	{"MORE",6,7, {FL_Tab,0}, more_xpm},
	{"<",0,1, {FL_Left,0}, left_xpm},
	{"^",0,3, {FL_Up,0}, up_xpm},

	{"ALPHA",5,7,{FL_Caps_Lock,0}, alpha_xpm},
	{"x-VAR",4,7,{'`', 0}, xvar_xpm},
	{"DEL",3,7,{FL_BackSpace, FL_Delete, 0}, del_xpm},
	{"v",0,0,{FL_Down, 0}, down_xpm},
	{">",0,2,{FL_Right, 0}, right_xpm},

	{"GRAPH",5,6, {FL_SHIFT|'1', 0}, graph_xpm},
	{"STAT",4,6, {FL_SHIFT|'2', 0}, stat_xpm},
	{"PRGM",3,6, {FL_SHIFT|'3', 0}, prgm_xpm},
	{"CUSTOM",2,6, {FL_SHIFT|'4', 0}, custom_xpm},
	{"CLEAR",1,6, {FL_SHIFT|'5', 0}, clear_xpm},

	{"LOG",5,5, {'a', 0}, log_xpm},
	{"SIN",4,5, {'b', 0}, sin_xpm},
	{"COS",3,5, {'c', 0}, cos_xpm},
	{"TAN",2,5, {'d', 0}, tan_xpm},
	{"^",1,5, {'e', FL_SHIFT|'6', 0}, powerby_xpm},

	{"LN",5,4, {'f', 0}, ln_xpm},
	{"EE",4,4, {'g', 0}, ee_xpm},
	{"(",3,4, {'h',FL_SHIFT|'9', 0}, parleft_xpm},
	{")",2,4, {'i',FL_SHIFT|'0', 0}, parright_xpm},
	{"/",1,4, {'j','/', 0}, div_xpm},

	{"x^2",5,3, {'k', 0}, power2_xpm},
	{"7",4,3, {'7',FL_KP+7,'l', 0}, seven_xpm},
	{"8",3,3, {'8',FL_KP+8,'m', 0}, eight_xpm},
	{"9",2,3, {'9',FL_KP+9,'n', 0}, nine_xpm},
	{"*",1,3, {FL_SHIFT|'8','o', 0}, mul_xpm},

	{",",5,2, {',','p', 0}, comma_xpm},
	{"4",4,2, {'4',FL_KP+4,'q', 0}, four_xpm},
	{"5",3,2, {'5',FL_KP+5,'r', 0}, five_xpm},
	{"6",2,2, {'6',FL_KP+6,'s', 0}, six_xpm},
	{"-",1,2, {'-','t', 0}, minus_xpm},

	{"STO>",5,1, {FL_SHIFT|'.','=', 0}, sto_xpm},
	{"1",4,1, {'1',FL_KP+1,'u', 0}, one_xpm},
	{"2",3,1, {'2',FL_KP+2,'v', 0}, two_xpm},
	{"3",2,1, {'3',FL_KP+3,'w', 0}, three_xpm},
	{"+",1,1, {FL_SHIFT|'=', 0}, plus_xpm},

	{"ON",5,0, {FL_Alt_R, 0}, on_xpm},
	{"0",4,0, {'0',FL_KP+0,'y', 0}, zero_xpm},
	{".",3,0, {'.','z', 0}, dot_xpm},
	{"-",2,0, {' ','\\', 0}, sign_xpm},
	{"ENTER",1,0, {FL_Enter, FL_KP_Enter, 0}, enter_xpm}
};

int misc_action(int ev)
{
	int key = Fl::event_key() | (Fl::event_state()&FL_SHIFT);
	int i, j;

#if 0
    int Fl::compose(int& del)

	       Use of this function is very simple. Any text editing widget should call this for each FL_KEYBOARD event.  If true is returned, then it has modified the Fl::event_text() and Fl::event_length() to a set of bytes to insert (it may be of zero length!). In will also set the "del" parameter to the number of bytes to the left of the cursor to delete, this is used to delete the results of the previous call to Fl::compose().  If false is returned, the keys should be treated as function keys, and del is set to zero. You could insert the text anyways, if you dont know what else to do.  Though the current implementation returns immediately, future versions may take quite awhile, as they may pop up a window or do other user-interface things to allow characters to be selected.  int Fl::compose_reset() If the user moves the cursor, be sure to call Fl::compose_reset(). The next call to Fl::compose() will start out in an initial state. In particular it will not set "del" to non-zero. This call is very fast so it is ok to call it many times and in many places.
							   
#endif
	

	// printf("ev %d\n", ev);
	if ((ev != FL_SHORTCUT) && (ev != FL_NO_EVENT))
		return 0;
	if (((key < FL_Button) || (key > FL_Button + 3)) && (key != 0)) {
		for (i=0; i<NBUTTONS; i++)
			for (j=0; labels[i].shortcut[j]; j++)
				if (labels[i].shortcut[j] == key)
					if (ev == FL_SHORTCUT) {
						// printf ("sending one-time key %d,%d\n", labels[i].row, labels[i].column);
						TI85_SETKEY(labels[i].row, labels[i].column);
					}
					else
						TI85_RESKEY(labels[i].row, labels[i].column);
	}
	return 1;
}

static void mainw_action(Fl_Widget * window, void * v)
{
	int key = Fl::event_key();

	/* ignore ESC exit */
	if (key == FL_Escape)
		return;

	SaveSnap(SNAPFILE, 1);
	Fl::atclose((Fl_Window *)window, v);
}

static void button_action(Fl_Widget * widg, void * key_nr)
{
	if (((Fl_Button *)widg)->value()) {
		TI85_SETKEY(labels[(int)key_nr].row, labels[(int)key_nr].column);
		// printf ("Key %s turned on\n", labels[(int)key_nr].label);
	} else {
		TI85_RESKEY(labels[(int)key_nr].row, labels[(int)key_nr].column);
		// printf ("Key %s turned off\n", labels[(int)key_nr].label);
	}
}

static void button_action_load(Fl_Widget * widg, void * tmp)
{
	char * fname;
	fname = fl_file_chooser("Load file...", "*", NULL);

	if (!fname)
		return;

	if (LoadSnap(fname) == -1)
		fl_alert("Cannot load the file %s", fname);
}

static void button_action_save(Fl_Widget * widg, void * tmp)
{
	char * fname;
	fname = fl_file_chooser("Save file...", "*", NULL);

	if (!fname)
		return;

	if (SaveSnap(fname, 1) == -1)
		fl_alert("Cannot save the file %s", fname);
}

static void button_action_reset(Fl_Widget * widg, void * tmp)
{
	if (fl_ask("Reset?")) {
		ti85_reset((Z80 *)tmp);
	}
}

static void button_action_quit(Fl_Widget * widg, void * tmp)
{
	if (fl_ask("Quit?")) {
		SaveSnap(SNAPFILE, 1);
		exit(0);
	}
}

void emu(void * window)
{
	int i;

	static int video_timer = -1;
	static unsigned long elap_tmp = 0;

	ti85_emu(cpu);

	video_timer = (video_timer + 1) % VIDEO_EACHN;

	if (!video_timer) {
#ifdef FLTK_RELEVANT
		if (memcmp(TI85_VRAM, vram_backup, 16*64))
			memcpy(vram_backup, TI85_VRAM, 16*64);
		else if (force_refresh)
			force_refresh = 0;
		else return;
#endif

#ifdef FLTK_FLASHING
		for (i=0; i<16*64; i++)
			videoram_copy[i] = bitturntable[*(TI85_VRAM+i)];
#else
		
		for (i=0; i<16*64; i++)
			videoram_inv_copy[i] = ~(videoram_copy[i] = bitturntable[*(TI85_VRAM+i)]);
#endif
		// videoram_copy[0] ^= 255;
		((Fl_Window *)window)->make_current();
		if (TI85_IS_ON)
			fl_color(FL_WHITE);
		else
			fl_color(FL_GRAY);
#ifdef FLTK_FLASHING
		fl_rectf(1, 1, 16*8, 64);
#else
		fltk_disp = new Fl_Bitmap(videoram_inv_copy, 16*8, 64);
		fltk_disp->draw(1,1);
		delete(fltk_disp);
#endif
		fl_color(FL_BLACK);
		fltk_disp = new Fl_Bitmap(videoram_copy, 16*8, 64);
		fltk_disp->draw(1,1);
		delete(fltk_disp);
	}
}

int gui_init(int * argcp, char ***argvp, Z80 * cpu)
{
	int i;

	Fl_Window * window;
	Fl_Button * tmp;
 
#define TURN(X) ((((X)&1) << 7) | (((X)&2) << 5) | (((X)&4) << 3) | (((X)&8) << 1) | \
		(((X)&16) >> 1) | (((X)&32) >> 3) | (((X)&64) >> 5) | (((X)&128) >> 7))
	for (i=0; i<256; i++)
		bitturntable[i] = TURN(i);
	
	window = new Fl_Window(160,240);
	Fl::add_idle(emu, (void *)window);

	for (i=0; i<NBUTTONS; i++) {
		if (i/5 == 0)
			tmp=new Fl_Button((25*(i%5))+3, 64+2, 25, 14, labels[i].label);
		else if (i == 8)
			tmp = new Fl_Button(102, 80+3+9, 18,18,labels[i].label);
		else if (i == 9)
			tmp = new Fl_Button(102+18, 80+3, 18,18,labels[i].label);
		else if (i == 13)
			tmp = new Fl_Button(102+18, 80+3+18, 18,18,labels[i].label);
		else if (i == 14)
			tmp = new Fl_Button(102+36, 80+3+9, 18,18,labels[i].label);
		else
			tmp=new Fl_Button((i%5)*32, 64+6+(i/5)*17, 32, 16, labels[i].label);
		if (labels[i].pixmap)
			(new Fl_Pixmap(labels[i].pixmap))->label(tmp);
		tmp->when(FL_WHEN_CHANGED);
		tmp->callback(button_action, (void *)i);
		tmp->labelsize(8);
	}
	tmp = new Fl_Button(128+3, 1, 28, 13, "Load");
	tmp->when(FL_WHEN_RELEASE);
	tmp->callback(button_action_load, NULL);
	tmp->labelsize(8);
	tmp = new Fl_Button(128+3, 1+14, 28, 13, "Save");
	tmp->when(FL_WHEN_RELEASE);
	tmp->callback(button_action_save, NULL);
	tmp->labelsize(8);
	tmp = new Fl_Button(128+3, 1+2*14, 28, 13, "Reset");
	tmp->when(FL_WHEN_RELEASE);
	tmp->callback(button_action_reset, (void *)cpu); // dirty.. fixme some day
	tmp->labelsize(8);
	tmp = new Fl_Button(128+3, 1+3*14, 28, 13, "Quit");
	tmp->when(FL_WHEN_RELEASE);
	tmp->callback(button_action_quit, NULL);
	tmp->labelsize(8);
	window->callback(mainw_action, NULL);
	window->end();
	Fl::add_handler(misc_action);

	LoadSnap(SNAPFILE);

	window->show(*argcp, *argvp);
	return 0;
}

void gui_run(Z80 * p)
{
	cpu = p;
#ifdef FLTK_RELEVANT
	force_refresh = 1;
#endif
	Fl::run();
}
