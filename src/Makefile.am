## Makefile.am --[automake]--> Makefile.in

bin_PROGRAMS = @guifile@
EXTRA_PROGRAMS = ti85emu_gtk ti85emu_fltk

INCLUDES = -I.. -I./MSX
SUBDIRS = MSX


ti85emu_gtk_SOURCES = main.c ti85.c gtk.c \
			ti85.h icons.h
ti85emu_gtk_LDADD = MSX/libmsx.a

ti85emu_fltk_SOURCES = ti85.h icons.h
ti85emu_fltk_LDADD = x-main.o x-ti85.o x-fltk.o \
			MSX/libxmsx.a
ti85emu_fltk_DEPENDENCIES = x-main.o x-ti85.o x-fltk.o

x-main.o:	main.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o x-main.o -c main.c

x-ti85.o:	ti85.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o x-ti85.o -c ti85.c

x-fltk.o:	fltk.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o x-fltk.o -c fltk.c

