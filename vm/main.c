/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "loader.h"
#include "vm.h"

uint32_t debug_level_g = 0;

void usage(const char *progname) {
	printf("Usage:\n");
	printf("%s -d [s|sf|l|b] -f [filename]\n", progname);
	exit(EXIT_FAILURE);
}

void free_globals() {
	uint32_t i;

	if (classes_g != NULL) {
		for (i = 0; i < classes_cnt_g; i++) {
			class_free(&classes_g[i]);
		}
	}

	free(bc_arr_g);
	free(const_table_g);

#ifdef DEBUG
	/* free the buffer inside */
	(void) kek_obj_print(NULL);
#endif
}

void debug_add(char *level) {
	if (strcmp(level, "s") == 0 || strcmp(level, "stack") == 0) {
		debug_level_g |= DBG_STACK;
	} else if (strcmp(level, "sf") == 0 || strcmp(level, "stack_full") == 0) {
		debug_level_g |= DBG_STACK_FULL;
	} else if (strcmp(level, "l") == 0 || strcmp(level, "loading") == 0) {
		debug_level_g |= DBG_LOADING;
	} else if (strcmp(level, "b") == 0 || strcmp(level, "bc") == 0) {
		debug_level_g |= DBG_BC;
	} else if (strcmp(level, "v") == 0 || strcmp(level, "v") == 0) {
		debug_level_g |= DBG_VM;
	} else if (strcmp(level, "a") == 0 || strcmp(level, "all") == 0) {
		debug_level_g |= DBG_ALL;
	}
}

int main(int argc, char *argv[]) {
	int c;
	char *filename = NULL;

	while ((c = getopt(argc, argv, "d:")) != -1) {
		switch (c) {
		case 'd':
			debug_add(optarg);
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

	if (optind == argc) {
		usage(argv[0]);
	}

	filename = argv[optind];

	if (!kexe_load(filename)) {
		return (EXIT_FAILURE);
	}

	vm_init_builtin_classes();
	vm_init_parent_pointers();
	vm_init_const_table_elems();

	stack_init();
	// Call static initializers of all classes
	// or we could do some sort of lazy loading.
	vm_call_class_initializers();
	vm_call_main(argc - optind, argv + optind);

	free_globals();

	return (EXIT_SUCCESS);
}
