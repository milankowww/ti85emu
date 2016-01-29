#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

#include "Z80.h"
#include "ti85.h"
#include "icons.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define NBUTTONS (5*10)

#define VIDEO_EACHN 41

#ifdef GTK_RELEVANT
byte vram_backup[64*16];
int force_refresh = 1;
#endif

static Z80 * cpu = NULL;

struct key_desc {
	char * label;
	int row;
	int column;
	char ** pixmap;
} labels[NBUTTONS] = {
	{"F1",6,4, f1_xpm}, {"F2",6,3, f2_xpm},
	{"F3",6,2, f3_xpm}, {"F4",6,1, f4_xpm},
	{"F5",6,0, f5_xpm},

	{"2nd",6,5,_2nd_xpm}, {"EXIT",6,6,exit_xpm},
	{"MORE",6,7, more_xpm}, {"<",0,1, left_xpm},
	{"^",0,3, up_xpm},

	{"ALPHA",5,7,alpha_xpm}, {"x-VAR",4,7,xvar_xpm},
	{"DEL",3,7,del_xpm}, {"v",0,0,down_xpm},
	{">",0,2,right_xpm},

	{"GRAPH",5,6, graph_xpm}, {"STAT",4,6, stat_xpm},
	{"PRGM",3,6, prgm_xpm}, {"CUSTOM",2,6, custom_xpm},
	{"CLEAR",1,6, clear_xpm},

	{"LOG",5,5, log_xpm}, {"SIN",4,5, sin_xpm},
	{"COS",3,5, cos_xpm}, {"TAN",2,5, tan_xpm},
	{"^",1,5, powerby_xpm},

	{"LN",5,4, ln_xpm}, {"EE",4,4, ee_xpm},
	{"(",3,4, parleft_xpm}, {")",2,4, parright_xpm},
	{"/",1,4, div_xpm},

	{"x^2",5,3, power2_xpm}, {"7",4,3, seven_xpm},
	{"8",3,3, eight_xpm}, {"9",2,3, nine_xpm},
	{"*",1,3, mul_xpm},

	{",",5,2, comma_xpm}, {"4",4,2, four_xpm},
	{"5",3,2, five_xpm}, {"6",2,2, six_xpm},
	{"-",1,2, minus_xpm},

	{"STO>",5,1, sto_xpm}, {"1",4,1, one_xpm},
	{"2",3,1, two_xpm}, {"3",2,1, three_xpm},
	{"+",1,1, plus_xpm},

	{"ON",5,0, on_xpm}, {"0",4,0, zero_xpm},
	{".",3,0, dot_xpm}, {"-",2,0, sign_xpm},
	{"ENTER",1,0, enter_xpm}
};

GtkWidget * box_tophalf;
GtkWidget * mainwin;

GtkWidget * filedialog = NULL;

guchar vram_cooked[128*64];
guchar bytedecoder[256*8];

char * get_fname(char * title)
{
	if (filedialog == NULL) {
		filedialog = gtk_file_selection_new(title);
		// gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedialog)->ok_button), "clicked", (GtkSignalFunc) nieco, NULL);
	}
	gtk_window_set_modal(GTK_WINDOW(filedialog), TRUE);
	gtk_widget_show_all(filedialog);

}

static void button_action(GtkButton * btn, gpointer key_nr)
{
	int i = (int)key_nr;

	if (i < 0)
		TI85_RESKEY(labels[-i-1].row, labels[-i-1].column);
	else
		TI85_SETKEY(labels[i].row, labels[i].column);
}

gint emu(gpointer window)
{
	int i, j;

	static int video_timer = -1;
	static unsigned long elap_tmp = 1;

	signed char c; /* char must be 8bit to make this work;
			  use int8_t or so if it is problem; .. */

	ti85_emu(cpu);

	video_timer = (video_timer + 1) % VIDEO_EACHN;

	if (!video_timer) {
#ifdef GTK_RELEVANT
		if (memcmp(TI85_VRAM, vram_backup, 16*64))
			memcpy(vram_backup, TI85_VRAM, 16*64);
		else if (force_refresh)
			force_refresh = 0;
		else return;
#endif

#if 0
		for (i=0; i<16*64; i++)
			memcpy(vram_cooked+(i<<3), bytedecoder + ((*(TI85_VRAM+i)) << 3), 8);
#endif

#if 1
		j = TI85_IS_ON ? 255 : 192;

		for (i=0; i<16*64*8; ) {
			c = *(TI85_VRAM + (i>>3));
			/* it's OK to overflow here! */
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
			if (c < 0) vram_cooked[i++] = 0;
			else vram_cooked[i++] = j;
			c<<=1;
		}
#endif

	gdk_draw_gray_image(box_tophalf->window, box_tophalf->style->fg_gc[GTK_STATE_NORMAL], 0,0, 128,64, GDK_RGB_DITHER_NORMAL, vram_cooked, 128);

	gtk_widget_show(box_tophalf);
	}

	gtk_idle_add(emu, (gpointer)mainwin);
	return 0;
}

int gui_init(int * argcp, char ***argvp, Z80 * cpu)
{
	GtkWidget * button;
	GtkWidget * box_main;
	GtkWidget * tmp;

	GtkStyle * style;
	int i;

	gtk_init(argcp, argvp);

	for (i=0; i<256; i++) {
		bytedecoder[(i<<3) + 0] = (i&128)? 0 : 255;
		bytedecoder[(i<<3) + 1] = (i& 64)? 0 : 255;
		bytedecoder[(i<<3) + 2] = (i& 32)? 0 : 255;
		bytedecoder[(i<<3) + 3] = (i& 16)? 0 : 255;
		bytedecoder[(i<<3) + 4] = (i&  8)? 0 : 255;
		bytedecoder[(i<<3) + 5] = (i&  4)? 0 : 255;
		bytedecoder[(i<<3) + 6] = (i&  2)? 0 : 255;
		bytedecoder[(i<<3) + 7] = (i&  1)? 0 : 255;
	}
	
	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainwin), "TI85Emu");
	gtk_window_set_default_size(GTK_WINDOW(mainwin), 160, 240);
	gtk_idle_add(emu, (gpointer)mainwin);

	box_main = gtk_vbox_new(FALSE, 1);

	box_tophalf = gtk_vbox_new(TRUE, 0);
	gtk_widget_set_usize(box_tophalf, 160, 64);

	gtk_box_pack_start(GTK_BOX(box_main), box_tophalf, TRUE, TRUE, 0);

	tmp = gtk_fixed_new();
	for (i=0; i<NBUTTONS; i++) {
		GdkPixmap * pixmap;
		GtkWidget * pixmap_w;
		GdkBitmap * mask;
		GtkStyle * style;

		style = gtk_widget_get_style(tmp);

		if (labels[i].pixmap) {
			button = gtk_button_new();
			pixmap = gdk_pixmap_create_from_xpm_d(tmp->window,
					&mask,
					&style->bg[GTK_STATE_NORMAL],
					(gchar **)labels[i].pixmap);
			pixmap_w = gtk_pixmap_new(pixmap, mask);
			//gtk_widget_show(pixmap_w);
			gtk_container_add(GTK_CONTAINER(button), pixmap_w);
		} else {
			button = gtk_button_new_with_label(labels[i].label);
		}

		gtk_signal_connect(GTK_OBJECT(button), "pressed",
				GTK_SIGNAL_FUNC(button_action), GINT_TO_POINTER(i));
		gtk_signal_connect(GTK_OBJECT(button), "released",
				GTK_SIGNAL_FUNC(button_action), GINT_TO_POINTER(-i-1));
		gtk_signal_connect(GTK_OBJECT(button), "leave",
				GTK_SIGNAL_FUNC(button_action), GINT_TO_POINTER(-i-1));

		if (i/5 == 0) {
			gtk_widget_set_usize(button, 25, 14);
			gtk_fixed_put(GTK_FIXED(tmp), button, (25*(i%5))+3, 2);
		} else if (i == 8) {
			gtk_widget_set_usize(button, 18, 18);
			gtk_fixed_put(GTK_FIXED(tmp), button, 102, 16+3+9);
		} else if (i == 9) {
			gtk_widget_set_usize(button, 18, 18);
			gtk_fixed_put(GTK_FIXED(tmp), button, 102+18, 16+3);
		} else if (i == 13) {
			gtk_widget_set_usize(button, 18, 18);
			gtk_fixed_put(GTK_FIXED(tmp), button, 102+18, 16+3+18);
		} else if (i == 14) {
			gtk_widget_set_usize(button, 18, 18);
			gtk_fixed_put(GTK_FIXED(tmp), button, 102+36, 16+3+9);
		} else {
			gtk_widget_set_usize(button, 32, 16);
			gtk_fixed_put(GTK_FIXED(tmp), button, (i%5)*32, 6+(i/5)*17);
		}
	}
	gtk_box_pack_start(GTK_BOX(box_main), tmp, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(mainwin), box_main);

	gdk_rgb_init();
	gtk_widget_set_default_colormap (gdk_rgb_get_cmap());
	gtk_widget_set_default_visual (gdk_rgb_get_visual());

	gtk_widget_show_all(mainwin);
	return 0;
}

void gui_run(Z80 * p)
{
	cpu = p;
	gtk_main();
}
