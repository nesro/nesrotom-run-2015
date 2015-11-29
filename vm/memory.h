/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdbool.h>

#include "vm.h"

#define ARR_INIT_SIZE 512

struct _class;

// Integers are primitive objects without methods - they don't need a class pointer
union _kek_obj * alloc_integer(void);
union _kek_obj * alloc_array(struct _class * arr_class);
void alloc_arr_elems(struct _kek_array * arr);
union _kek_obj ** alloc_const_arr_elems(int length);
void realloc_arr_elems(struct _kek_array * arr, int length);
union _kek_obj * alloc_string(struct _class * str_class, int length);
union _kek_obj * alloc_exception(struct _class * expt_class);
union _kek_obj * alloc_udo(struct _class * arr_class);
union _kek_obj * alloc_file(struct _class * file_class);
union _kek_obj * alloc_term(struct _class * term_class);

/******************************************************************************/
/* memory managment */

/* http://jayconrod.com/posts/55/a-tour-of-v8-garbage-collection */
/* http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.63.6386&rep=rep1&type=pdf */

/* from claus */
#define SEGMENT_SIZE (2*1024) /* FIXME TODO 2KB for now */
typedef double data_t; /* data type */
#define OBJ_ALIGN sizeof(double)
#define ALIGNED(n) (((n) + OBJ_ALIGN-1) & ~(OBJ_ALIGN-1))
#define ALIGNED_SIZE_OF(obj) ALIGNED((obj)->h.size)



/* Remember set */
typedef struct _segment_slots_buffer {
	/* TODO */
	kek_obj_t *obj;
	struct _segment_slots_buffer *next;
} segment_slots_buffer_t;

/* pointers from old to new space */
typedef struct _segment_write_barrier {
	/* TODO */
	kek_obj_t *obj;
	struct _segment_slots_buffer *next;
} segment_write_barrier_t;

typedef struct _segment_new {
	int a;
} segment_new_t;

typedef struct _segment_old {
	int a;
} segment_old_t;

typedef union {
	segment_new_t *hdr_new;
	segment_old_t *hdr_old;
} segment_header_t;

typedef struct _segment {
	segment_header_t header;
	size_t size;
	size_t used;
	//segment_slots_buffer_t *slots_buffer;

	void *beginning; /* pointer to the start of the data */
	void *end; /* pointer to the end of the data of this segment */

	struct _segment *next;

	double data[1];
	/* data[size-1] */
} segment_t;


extern segment_t *segments_old_space_g;

/* this will main call to initialiaze everything and then free */
bool mem_init(void);
bool mem_free(void);
segment_t *mem_segment_init(size_t size);

void *mem_obj_malloc(type_t type, class_t *cls, size_t size);
void *mem_obj_calloc(type_t type, class_t *cls, size_t num, size_t size);

/******************************************************************************/
/* obj_table */

typedef enum _obj_state {
	OBJ_UNKNOWN_STATE = 0, //
	OBJ_1ST_GEN_YOUNG, //
	OBJ_2ND_GEN_YOUNG, //
	OBJ_OLD_WHITE, //
	OBJ_OLD_GRAY, //
	OBJ_OLD_BLACK //
} obj_state_t;

typedef struct _obj_table {
	obj_state_t state;
	kek_obj_t *obj_ptr;

	/* array of pointers that points to this obj */
	/* we'll asume that most to most objt will point only one ptr */
	kek_obj_t **ptr;

	uint32_t ptr_arr_cnt;
	uint32_t ptr_arr_size;
	kek_obj_t ***ptr_arr;
} obj_table_t;

extern obj_table_t *obj_table_g;
extern uint32_t obj_table_size_g;
#define REF(obj) (*(obj))
#define OBJ_TABLE_DEFAULT_SIZE 2048
#define OBJ_TABLE_PTR_ARR_DEFAULT_SIZE 256
void obj_table_init(void);
void obj_table_free(void);

/* argument is a pointer to the pointer to the object.
 * we need it for updating when the obj moves in the heap */
uint32_t obj_table_regptr(kek_obj_t **);

/******************************************************************************/
/* gc */

typedef enum _gc_type {
	GC_NONE, //
	GC_NEW, // just new space (cheney only)
	GC_GEN // generational GC. new and old space
} gc_type_t;

#define GC_TYPE_DEFAULT GC_NONE
extern gc_type_t gc_type_g;


typedef struct _gc_obj {
	kek_obj_t *obj;
	size_t size;
	struct _gc_obj *next;
} gc_obj_t;

#define GC_TICKS_DEFAULT 10
extern int gc_ticks_g; /* how often will gc run */
extern gc_obj_t *gc_obj_g;
extern gc_obj_t *gc_obj_root_g;

/* this function will be called from the main loop in vm */
void gc(void);
void gc_init(void);
void gc_free(void);
void gc_delete_all(void);

void gc_rootset(void (*fn)(kek_obj_t **));

/******************************************************************************/
/* cheney */

#define NEW_SEGMENT_SIZE 1024
extern segment_t *segments_from_space_g;
extern segment_t *segments_to_space_g;
extern void *from_space_free_g; /* points to the end of data in from-space */
extern void *alloc_ptr_g;
extern void *scan_ptr_g;

void gc_cheney_init(void);
void gc_cheney_free(void);
void *gc_cheney_malloc(type_t type, class_t *cls, size_t size);
void *gc_cheney_calloc(type_t type, class_t *cls, size_t size);
void gc_cheney_scavenge();

/******************************************************************************/

#endif
