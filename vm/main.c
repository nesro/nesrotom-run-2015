/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "loader.h"
#include "vm.h"

void usage(const char *progname) {
	printf("Usage:\n");
	printf("%s -f [filename]\n", progname);
	exit(1);
}

void free_globals() {
	uint32_t i;

	if (classes_g == NULL) {
		return;
	}

	for (i = 0; i < classes_cnt_g; i++) {
		class_free(&classes_g[i]);
	}
}

int main(int argc, char *argv[]) {
	int c;
	char *filename = NULL;

	while ((c = getopt(argc, argv, "f:")) != -1) {
		switch (c) {
		case 'f':
			filename = optarg;
			break;
		case '?':
			fprintf(stderr, "unknown opt: \"%c\"\n", c);
			return (EXIT_FAILURE);
			break;
		default:
			abort();
			break;
		}
	}

	if (filename == NULL) {
		usage(argv[0]);
	}

	kexe_load(filename);

	free_globals();

	return (EXIT_SUCCESS);
}
