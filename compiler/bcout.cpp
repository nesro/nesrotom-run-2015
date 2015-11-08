#include "bcout.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

bcout_t *bcout_g;

/******************************************************************************/
#if DEBUG

void bco_debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
}

void bco_print_const(bcout_t *bco, uint8_t idx) {
	constant_item_t* item = (constant_item_t*) (bco->const_table + idx);
	switch (item->type) {
	case KEK_NIL:
		bco_debug("nil");
		break;
	case KEK_INT:
		bco_debug("%d", item->ci.value);
		break;
	case KEK_STR:
		bco_debug("\"%s\"", item->cs.string);
		break;
	case KEK_SYM:
		bco_debug("%s", item->cs.string);
		break;
	case KEK_ARR:
		bco_debug("array [length: %d, elems: %d", item->ca.length, item->ca.elems[0]);
		for (int i = 1; i < item->ca.length; ++i) {
			bco_debug(", %d", item->ca.elems[i]);
		}
		bco_debug("]");
		break;
	}
}

#else

void bco_debug(const char *format, ...) {
}

void bco_print_const(bcout_t *bco, uint8_t idx) {
}

#endif
/******************************************************************************/

const char *op_str[] = { "UNDEF", "BOP", "UNM", "LD", "ST", "IFNJ", "JU", "WRT",
		"RD", "DUP", "SWAP", "NOT", "STOP", "RET", "CALL", "CALLS", "CALLE",
		"PUSH_C", "PUSH_ARG", "PUSH_LOC", "PUSH_IV", "PUSH_CV", "PUSH_IVE",
		"PUSH_CVE", "PUSH_SELF", "CLASSREF", "PUSHA_ARG", "PUSHA_LOC",
		"PUSHA_IV", "PUSHA_CV", "PUSHA_IVE", "PUSHA_CVE", "IDX", "IDXA", "NEW" };

bcout_t *bcout_init() {
	bcout_t *bco;

	bco = (bcout_t *) malloc(sizeof(bcout_t));
	assert(bco);

	/* alocate array for bytecode */
	bco->bc_arr_size = DEFAULT_BUFFER_SIZE;
	bco->bc_arr_cnt = 0;
	bco->bc_arr = (uint8_t *) malloc(bco->bc_arr_size * sizeof(uint8_t));
	assert(bco->bc_arr);

	bco->bc_lab = (uint8_t *) malloc(bco->bc_arr_size * sizeof(uint8_t));
	assert(bco->bc_lab);

	/* alocate array for bytecode */
	bco->const_table_size = DEFAULT_BUFFER_SIZE;
	bco->const_table_cnt = 0;
	bco->const_table = (uint8_t *) calloc(bco->const_table_size,
			sizeof(uint8_t *));
	assert(bco->const_table);

	/* array of pointers to const_table */
	bco->items_size = 4096;
	bco->items_cnt = 0;
	bco->items = (constant_item_t **) malloc(
			bco->items_size * sizeof(constant_item_t *));
	assert(bco->items);

	/* Create constant singletons */
	bco_nil(bco);

	return (bco);
}

void bcout_free(bcout_t *bco) {
	free(bco->items);
	free(bco);
}

/******************************************************************************/
/* bytecode utils */

/* check, if we can add 'size' to the array. if not, realloc */
void bc_arr_realloc(bcout_t *bco, size_t size) {
	if (bco->bc_arr_size < bco->bc_arr_cnt + size) {
		bco->bc_arr_size *= 2;
		bco->bc_arr = (uint8_t *) realloc(bco->bc_arr,
				bco->bc_arr_size * sizeof(uint8_t *));
		assert(bco->bc_arr);
		bco->bc_lab = (uint8_t *) realloc(bco->bc_lab,
				bco->bc_arr_size * sizeof(uint8_t *));
		assert(bco->bc_lab);
	}
}

/* write an uint8 to the bc_arr */
uint32_t bco_w8(bcout_t *bco, uint8_t uint8) {
	bc_arr_realloc(bco, sizeof(uint8_t));
	bco->bc_lab[bco->bc_arr_cnt] = NIL;
	bco->bc_arr[bco->bc_arr_cnt++] = uint8;

	return (bco->bc_arr_cnt - 1);
}

uint32_t bco_w8_labeled(bcout_t *bco, uint8_t uint8, label_t lab) {
	bc_arr_realloc(bco, sizeof(uint8_t));
	bco->bc_lab[bco->bc_arr_cnt] = lab;
	bco->bc_arr[bco->bc_arr_cnt++] = uint8;

	return (bco->bc_arr_cnt - 1);
}

/* write an uint16 (as two uint8) to the bc_arr */
uint32_t bco_w16(bcout_t *bco, uint16_t uint16) {
	bc_arr_realloc(bco, sizeof(uint16_t));

	*(uint16_t*) &bco->bc_lab[bco->bc_arr_cnt] = NIL;
	*(uint16_t*) &bco->bc_arr[bco->bc_arr_cnt] = uint16;
	bco->bc_arr_cnt += 2;
	return (bco->bc_arr_cnt - 2);
}

/* write an uint32 (as four uint8) to the bc_arr */
uint32_t bco_w32(bcout_t *bco, uint32_t uint32) {
	bc_arr_realloc(bco, sizeof(uint32_t));

	*(uint32_t*) &bco->bc_lab[bco->bc_arr_cnt] = NIL;
	*(uint32_t*) &bco->bc_arr[bco->bc_arr_cnt] = uint32;
	bco->bc_arr_cnt += 4;
	return (bco->bc_arr_cnt - 4);
}

/* FIXME: the name of this method is probably not good */
uint32_t bco_w0(bcout_t *bco, bc_t bc) {
	uint32_t ip = bco_w8(bco, bc);

	bco_debug("%3d: %s\n", ip, op_str[bc]);
	return (ip);
}

uint32_t bco_wb1(bcout_t *bco, bc_t bc, uint8_t arg) {
	uint32_t ip;

	ip = bco_w8(bco, bc);
	(void) bco_w8(bco, arg);

	bco_debug("%3d: %s %u ", ip, op_str[bc], arg);
	bco_debug("(");
	bco_print_const(bco, arg);
	bco_debug(")\n");
	return (ip);
}

uint32_t bco_ww1(bcout_t *bco, bc_t bc, uint16_t arg) {
	uint32_t ip;

	ip = bco_w8(bco, bc);
	(void) bco_w16(bco, arg);

	bco_debug("%3d: %s %u ", ip, op_str[bc], arg);
	bco_debug("(");
	bco_print_const(bco, arg);
	bco_debug(")\n");
	return (ip);
}

uint32_t bco_ww1_labeled(bcout_t *bco, bc_t bc, uint16_t arg, label_t lab) {
	uint32_t ip;

	ip = bco_w8_labeled(bco, bc, lab);
	(void) bco_w16(bco, arg);

	bco_debug("%3d: %s %u ", ip, op_str[bc], arg);
	bco_debug("(");
	bco_print_const(bco, arg);
	bco_debug(")\n");
	return (ip);
}

uint32_t bco_ww2(bcout_t *bco, bc_t bc, uint16_t arg0, uint16_t arg1) {
	uint32_t ip;

	ip = bco_w8(bco, bc);
	(void) bco_w16(bco, arg0);
	(void) bco_w16(bco, arg1);

	bco_debug("%3d: %s %u %u ", ip, op_str[bc], arg0, arg1);
	bco_debug("(");
	bco_print_const(bco, arg0);
	bco_debug(" ");
	bco_print_const(bco, arg1);
	bco_debug(")\n");
	return (ip);
}

/******************************************************************************/
/* constat table utils */

void bcout_items_add(bcout_t *bco, constant_item_t *i) {
	if (bco->items_size <= bco->items_cnt) {
		bco->items_size *= 2;
		bco->items = (constant_item_t **) realloc(bco->items,
				bco->items_size * sizeof(constant_item_t *));
		assert(bco->items);
	}
	bco->items[bco->items_cnt++] = i;
}

void *ct_malloc(bcout_t *bco, size_t obj_size) {
	void *ptr; /* pointer to the beginning of the allocated data */
	size_t oldsize;

	ptr = bco->const_table + bco->const_table_cnt;

	if (bco->const_table_size <= bco->const_table_cnt + obj_size) {
		bco_debug("ct_malloc: reallocing\n");
		oldsize = bco->const_table_size;
		bco->const_table_size *= 2;
		bco->const_table = (uint8_t*) realloc(bco->const_table,
				bco->const_table_size);
		assert(bco->const_table);
		memset(bco->const_table + oldsize, 0, bco->const_table_size - oldsize);
	}

	bco->const_table_cnt += obj_size;

	return (ptr);
}

/******************************************************************************/

uint32_t bco_nil(bcout_t *bco) {
	constant_nil_t *cn;
	static int singleton = 0;
	static uint32_t ret;

	if (singleton) {
		return (ret);
	} else {
		cn = (constant_nil_t *) ct_malloc(bco, sizeof(constant_nil_t));
		cn->type = KEK_NIL;
		ret = ((uint8_t *) cn - bco->const_table);
		singleton = 1;
	}

	//bcout_items_add(bco, (constant_item_t *) cn);

	return (ret);
}

/* create an integer, save it into the constant table and create
 * a pointer in the items array
 * usage:
 *
 * int i;
 * i = bco_int(bco, 666);
 * constant_int_t *j = (constant_int_t *)(bco->const_table + i));
 *
 */
uint32_t bco_int(bcout_t *bco, int v) {
	constant_int_t *ci;
	int found;

	found = bco_find_int(bco, v);
	if (-1 < found) {
		return (found);
	}

	ci = (constant_int_t *) ct_malloc(bco, sizeof(constant_int_t));
	ci->type = KEK_INT;
	ci->value = v;

	bcout_items_add(bco, (constant_item_t *) ci);

	return ((uint8_t *) ci - bco->const_table);
}

uint32_t bco_str(bcout_t *bco, const char *str) {
	constant_string_t *cs;
	size_t len;
	int found;

	found = bco_find_str(bco, str);
	if (-1 < found) {
		return (found);
	}

	len = strlen(str);

	/* round up to the multiple of 4 */
	len = (len + 3) & ~3;

	cs = (constant_string_t *) ct_malloc(bco, sizeof(constant_string_t) + len);
	cs->type = KEK_STR;
	cs->length = len;
	strcpy(cs->string, str);

	bcout_items_add(bco, (constant_item_t *) cs);

	return ((uint8_t *) cs - bco->const_table);
}

uint32_t bco_sym(bcout_t *bco, const char *str) {
	constant_string_t *cs;
	size_t len;
	int found;

	found = bco_find_sym(bco, str);
	if (-1 < found) {
		return (found);
	}

	len = strlen(str);

	cs = (constant_string_t *) ct_malloc(bco, sizeof(constant_string_t) + len);
	cs->type = KEK_SYM;
	cs->length = len;
	strcpy(cs->string, str);

	bcout_items_add(bco, (constant_item_t *) cs);

	return ((uint8_t *) cs - bco->const_table);
}

uint32_t bco_arr(bcout_t *bco, size_t len) {
	constant_array_t *ca;

	 // Size of the allocated memory is already a multiple of four in this case.
	ca = (constant_array_t *) ct_malloc(bco, sizeof(constant_array_t) + (len - 1) * sizeof(uint32_t));
	ca->type = KEK_ARR;
	ca->length = len;

	return ((uint8_t *) ca - bco->const_table);
}

void bco_arr_set_idx(bcout_t *bco, uint32_t arr, size_t idx, uint32_t elem){
	constant_array_t *ca = (constant_array_t *)(bco->const_table + arr);
	ca->elems[idx] = elem;
}

size_t bco_get_ip(bcout_t *bco) {
	return bco->bc_arr_cnt;
}

void bco_fix_forward_jmpw(bcout_t *bco, size_t idx) {
	*(uint16_t*) &bco->bc_arr[idx + 1] = (uint16_t) idx;
}

void bco_resolve_break(bcout_t *bco, size_t a1, size_t a2) {
	for (size_t i = a1; i < a2; ++i) {
		if (bco->bc_lab[i] == BRK) {
			bco_fix_forward_jmpw(bco, i);
			bco->bc_lab[i] = NIL;
		}
	}
}

uint32_t bco_find_int(bcout_t *bco, int v) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i]->type == KEK_INT && bco->items[i]->ci.value == v) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (-1);
}

uint32_t bco_find_str(bcout_t *bco, const char *str) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i]->type == KEK_STR
				&& strcmp(bco->items[i]->cs.string, str) == 0) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (-1);
}

uint32_t bco_find_sym(bcout_t *bco, const char *str) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i]->type == KEK_SYM
				&& strcmp(bco->items[i]->cs.string, str) == 0) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (-1);
}

/******************************************************************************/
/* classes */

/* check, if we can add 'size' to the classout. if not, realloc */
void classout_realloc(classout_wrapp_t *cow, size_t size) {
	if (cow->classout_size < cow->classout_cnt + size) {
		bco_debug("classout_realloc\n");
		cow->classout_size *= 2;
		cow->classout = (uint8_t *) realloc(cow->classout,
				cow->classout_size * sizeof(uint8_t));
		assert(cow->classout);
	}
}

uint8_t classout_w8(classout_wrapp_t *cow, uint8_t uint8) {
	classout_realloc(cow, sizeof(uint8_t));
	cow->classout[cow->classout_cnt] = uint8;
	cow->classout_cnt += 1;

	return (cow->classout_cnt - 1);
}

uint8_t classout_w32(classout_wrapp_t *cow, uint32_t uint32) {
	classout_realloc(cow, sizeof(uint32_t));
	cow->classout[cow->classout_cnt] = uint32;
	cow->classout_cnt += 4;

	return (cow->classout_cnt - 4);
}

/* this write lenght+string */
/* TODO: these functions could return a pointer */
uint8_t classout_wstr(classout_wrapp_t *cow, const char *str) {
	size_t len;

	len = strlen(str) + 1;

	classout_w32(cow, len); /* FIXME? */

	classout_realloc(cow, len * sizeof(char));
	(void) strcpy((char *) &cow->classout[cow->classout_cnt], str);
	cow->classout_cnt += len;

	return (cow->classout_cnt - len);
}

/* write symbol_t */
#if 0
symbol_t s; /* eclipse will show me how it looks like */
#endif
void classout_symbol(classout_wrapp_t *cow, PrvekTab *pt) {
	classout_wstr(cow, pt->ident);
	classout_w8(cow, pt->druh);
	classout_w8(cow, pt->sc);
	classout_w32(cow, pt->addr);
	classout_w32(cow, pt->const_ptr);
}

/* uint32_t cnt;
 * PrvekTab[cnt]
 */
void classout_prvektab_ll(classout_wrapp_t *cow, PrvekTab *ptarg) {
	PrvekTab *pt;
	uint32_t pt_cnt;
	uint8_t pt_cnt_offset;

	pt = ptarg;

	pt_cnt = 0;
	pt_cnt_offset = classout_w32(cow, pt_cnt);
	if (pt != NULL) {
		while (pt->dalsi != NULL) {
			classout_symbol(cow, pt);
			pt = pt->dalsi;
		}
		cow->classout[pt_cnt_offset] = pt_cnt;
	}
}

/* write method_t */
#if 0
method_t m; /* eclipse will show me how it looks like */
#endif
void classout_method(classout_wrapp_t *cow, MethodEnv *me) {
	classout_wstr(cow, me->methodName);
	classout_prvektab_ll(cow, me->args);
	classout_prvektab_ll(cow, me->syms);
	classout_w32(cow, me->bc_entrypoint);
	classout_w8(cow, me->isStatic);
}

/* write class_t */
#if 0
class_t m; /* eclipse will show me how it looks like */
#endif
void classout_class(classout_wrapp_t *cow, ClassEnv *ce) {
	MethodEnv *method;
	uint32_t method_cnt;
	uint8_t method_cnt_ptr;

	PrvekTab *sym;
	uint32_t sym_cnt;
	uint8_t sym_cnt_ptr;

	PrvekTab *sym_instance;
	PrvekTab *sym_static;

	/**************************************************************************/

	classout_wstr(cow, ce->className);

	if (ce->parent != NULL) {
		classout_wstr(cow, ce->parent->className);
	} else {
		classout_wstr(cow, "NOPARENT");
	}

	/* now we need to divide syms */
	ce->syms_instance = NULL;
	sym_instance = ce->syms_instance; /* FIXME assign pointer? */
	ce->syms_static = NULL;
	sym_static = ce->syms_static;
	if (ce->syms != NULL) {
		sym = ce->syms;
		while (sym->dalsi != NULL) {
			switch (sym->sc) {
			case SC_INSTANCE: // instance variable
				if (sym_instance == NULL) {
					sym_instance = sym;
				} else {
					sym_instance->dalsi = sym;
					sym_instance = sym;
				}
				break;
			case SC_CLASS: // class static variable
				if (sym_static == NULL) {
					sym_static = sym;
				} else {
					sym_static->dalsi = sym;
					sym_static = sym;
				}
				break;
			default:
				assert(0);
				break;
			}

			sym = sym->dalsi;
		}
	}

	/* print syms_{static,instance} *******************************************/

	classout_prvektab_ll(cow, ce->syms_static);
	classout_prvektab_ll(cow, ce->syms_instance);

	/**************************************************************************/

	method_cnt = 0;
	method_cnt_ptr = classout_w32(cow, method_cnt);

	if (ce->constructor != NULL) {
		classout_method(cow, ce->constructor);
		method_cnt++;
	} else {
		bco_debug("class \"%s\" has no constructor\n", ce->className);
	}

	if (ce->methods != NULL) { /* TODO: this could be refactored? */
		method = ce->methods;
		while (method->next != NULL) {
			classout_method(cow, method);
			method_cnt++;
			method = method->next;
		}
		cow->classout[method_cnt_ptr] = method_cnt;
	} else {
		bco_debug("class \"%s\" has no methods\n", ce->className);
	}
}

classout_wrapp_t *classout_wrapp_init(ClassEnv *ce) {
	classout_wrapp_t *cow;
	ClassEnv *ceptr;

	cow = (classout_wrapp_t *) malloc(sizeof(classout_wrapp_t));
	assert(cow);
	cow->classout_cnt = 0;
	cow->classout_size = DEFAULT_BUFFER_SIZE;
	/* FIXME: how it's possible that we have "holes" in classout? */
	//cow->classout = (uint8_t *) malloc(cow->classout_size * sizeof(uint8_t));
	cow->classout = (uint8_t *) calloc(cow->classout_size, sizeof(uint8_t));
	assert(cow->classout);

	ceptr = ce;
	if (ceptr != NULL) {
		while (ceptr->next != NULL) {
			classout_class(cow, ceptr);
			cow->classout_cnt++;
			ceptr = ceptr->next;
		}
	} else {
		bco_debug("top_class is NULL\n");
	}

	return (cow);
}

/******************************************************************************/
void bcout_to_file(bcout_t *bcout, ClassEnv *top_class, const char *filename) {
	FILE *f;
	classout_wrapp_t *cow;
	uint32_t kek_magic = 0x42666CEC;

	f = fopen(filename, "w");

	/* write magic */
	bco_debug("fwrite: kek_magic=0x%08x\n", kek_magic);
	fwrite(&kek_magic, sizeof(uint32_t), 1, f);

	/* classes */
	cow = classout_wrapp_init(top_class);
	bco_debug("fwrite: cow->classout_cnt=%d\n", cow->classout_cnt);
	fwrite(&cow->classout_cnt, sizeof(uint32_t), 1, f);
	fwrite(cow->classout, sizeof(uint8_t), cow->classout_cnt, f);
	free(cow->classout); /* TODO: make normal _free */
	free(cow);

	/* constant table */
	bco_debug("fwrite: bcout->const_table_cnt=%d\n", bcout->const_table_cnt);
	fwrite(&bcout->const_table_cnt, sizeof(size_t), 1, f);
	fwrite(bcout->const_table, sizeof(uint8_t), bcout->const_table_cnt, f);

	/* bytecode */
	bco_debug("fwrite: bcout->bc_arr_cnt=%d\n", bcout->bc_arr_cnt);
	fwrite(&bcout->bc_arr_cnt, sizeof(size_t), 1, f);
	fwrite(bcout->bc_arr, sizeof(uint8_t), bcout->bc_arr_cnt, f);

	fclose(f);
}
