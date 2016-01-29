noinst_LIBRARIES = @msxfile@
EXTRA_LIBRARIES = libmsx.a libxmsx.a

libmsx_a_SOURCES = Z80.c
libmsx_a_DEPENDENCIES = Codes.h CodesED.h CodesXX.h \
	CodesCB.h CodesXCB.h Tables.h Z80.h

libxmsx_a_SOURCES =
libxmsx_a_LIBADD = x-Z80.o
libxmsx_a_DEPENDENCIES = x-Z80.o Codes.h CodesED.h CodesXX.h \
	CodesCB.h CodesXCB.h Tables.h Z80.h

x-Z80.o:	Z80.c
	$(CXX) $(CXXFLAGS) -o x-Z80.o -c Z80.c

