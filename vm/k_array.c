/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "vm.h"
#include "k_array.h"
#include "stack.h"

void init_kek_array_class(void) {
	char name[] = "Array";
	classes_g[classes_cnt_g].name = malloc(strlen(name + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 0;
	// TODO: add native methods such as length(), get_idx(), get_idxa()
	classes_g[classes_cnt_g].methods = NULL;

	classes_g[classes_cnt_g].allocator = alloc_array;

	classes_g[classes_cnt_g].constructor = malloc(sizeof(method_t));
	vm_init_native_method(classes_g[classes_cnt_g].constructor, "Array", 0, false, new_array);

	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

// Constructor of an empty array.
// Can be called from bytecode, so we use our custom stack.
void new_array(void) {
	kek_array_t * arr = (kek_array_t*)THIS;
	arr->h.t = KEK_ARR;
	arr->length = 0;
	arr->elems = alloc_arr_elems(ARR_INIT_SIZE);

	PUSH(NIL); // All kek methods must return something
}