#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "Z80.h"
#include "ti85.h"

extern int gui_init(int * argcp, char *** argvp, Z80 * cpu);
extern void gui_run(Z80 * p);

int main(int argc, char ** argv) {
	Z80 * cpu;
	int retval;

	cpu = ti85_init();
	if (cpu == NULL) {
		puts("cannot initialize ti85");
		return -1;
	}

	retval = gui_init(&argc, &argv, cpu);
	if (retval)
		return retval;
	gui_run(cpu);

	ti85_destroy(cpu);
	return retval;
}
