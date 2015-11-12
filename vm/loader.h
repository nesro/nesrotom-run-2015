/*
 * loader.h
 *
 *  Created on: Nov 7, 2015
 *      Author: n
 */

#ifndef VM_LOADER_H_
#define VM_LOADER_H_

#include "vm.h"

int kexe_load(const char *filename);
void class_free(class_t *class);

#endif /* VM_LOADER_H_ */
