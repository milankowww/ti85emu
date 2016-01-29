TI85 Emulator for Unix, (c) 2001 Milan Pikula

0. build

	Requirements:
	* ROM code from your real TI85 emulator (!)
	(I can't give you mine because of legal issues,
	don't even ask)
	* and FLTK or GTK library

	Configuration:
	* easy way: ./configure
	* configure with your own options - follow the help,
	  which shows on ./configure --help
	  Most important is the GUI option: --with-gui=gtk,
	  or --with-gui=fltk.

	Installation:
	* make install
	* move your ti85 rom image to /usr/share/ti85rom.bin

1. history and current status

	At first, I wanted to order my Agenda VR3d - a linux PDA.
	I wanted to use it as a calculator - and I wanted it NOW.
	So the first step was to look at existing calculators
	- didn't find good one.  Second step - try to port
	some existing emulator of real calculator, e.g. TI85.
	I soon found, that there is NO good emulator of TI85.
	First I tried to port some, but I wasn't happy with it.

	So I picked the good ideas from 'ti85' by Justesen Dines,
	few other available sources and the Z80 emulation core from
	the MSX portable emulator and cooked the preliminary
	version during one afternoon. There were some problems with
	ports (seems like noone really knows what that bits really
	mean; except me, of course;) so next version came soon.
	So.. here is x-th version, which is still a piece of crap,
	but it's the only emulator of TI85 on Linux and it's
	best emulator in terms of port3 emulation I've ever found.

	This emulator works on common Linux boxes and is in use on
	Agenda VR3 (www.agendacomputing.com) too. The X toolkit can
	be easily replaced by the Xlib or something else; if
	you don't like FLTK or GTK, write your own GUI (and contact me:)

	License: GNU GPL. Don't blame me for bugs, I've coded it
	to fulfill my needs and I have more important projects, which
	I want to work on.

	Any volunteer to clean up the sources, add some features,
	play with it? :)

	March 2001, Milan Pikula <www@fornax.sk>

2. To Do & bugs
	- bug: snapshot save/load is somewhat corrupted
	- finish the GTK port (buttons for load/save/reset/quit)
	- ti82,86 emulation
	- speed improvements (mallocs in banking etc.)
	- theme support
	- run-time options panel - speed, type of emulation, etc.

