/* tabsym.h */

#ifndef TABSYM_H
#define TABSYM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

enum DruhId {
	Nedef, IdProm, IdConstNum, IdConstStr
};

enum Scope {
	//SC_GLOBAL, // Only class names are global - you can globally reference a static class member
	SC_LOCAL,
	SC_ARG,
	SC_INSTANCE, // instance variable
	SC_CLASS // class static variable
};

class CRecord {
public:
	char *m_Ident;
	int m_Val;
	CRecord *m_Next;

	CRecord(char *ident);
	CRecord(char *ident, CRecord *next);
	/* TODO: destructor */
};

struct ClassEnv;

// At runtime, all symbols should be in arrays instead of linked lists. We want to have random access by addr indices.
struct PrvekTab {
	char *ident;
	DruhId druh;
	Scope sc;
	int addr; // address within method/class (even for consts)
	//int prvni, posledni;
	uint32_t const_ptr; // index to constant pool (in case of ConstNum, ConstStr) - replaced by pointer at runtime

	// value - for compiler use only (const inlining)
	union {
		int num;
		char * str;
	} val;

	//CRecord *record;
	PrvekTab *dalsi;
	//PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r);

	// variable / constant int
	PrvekTab(char *i, DruhId d, Scope s, int a, PrvekTab *n);
	// const int
	PrvekTab(char *i, DruhId d, Scope s, int a, int v, PrvekTab *n);
	// const str
	PrvekTab(char *i, DruhId d, Scope s, int a, const char * v, PrvekTab *n);
	//PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n);
	~PrvekTab();
};

struct MethodEnv {
	char * methodName;
	bool isStatic;
	PrvekTab * args; // method arguments
	PrvekTab * syms; // method vars and consts
	MethodEnv * next;
	int arg_addr_next; // Arguments address counter
	int local_addr_next; // Local vars address counter
	unsigned int bc_entrypoint; // Bytecode address

	MethodEnv(char * name, bool sttc, MethodEnv * n = NULL);
	~MethodEnv();
};

struct ClassEnv {
	char * className;
	PrvekTab * syms; // class consts and vars
	MethodEnv * methods;
	MethodEnv * constructor;
	MethodEnv * static_init;
	ClassEnv * next;
	ClassEnv * parent;
	int class_addr_next; // Static members address counter
	int obj_addr_next; // Instance members address counter

	ClassEnv(char * name, ClassEnv * par, ClassEnv * n);
	~ClassEnv();
};

// Special class environment for searching through all classes
#if defined(__LP64__)
#define CLASS_ANY (ClassEnv*)0xFFFFFFFFFFFFFFFFULL
#else
#define CLASS_ANY (ClassEnv*)0xFFFFFFFF
#endif

struct Env {
	ClassEnv * clsEnv;
	MethodEnv * mthEnv;
	bool self;
};

extern ClassEnv *TabClass;


ClassEnv * deklClass(char *, char * = NULL);
MethodEnv * deklMethod(char *, bool constructor = false, bool isStatic = false, ClassEnv * cls = NULL);
// const int
void deklKonst(char *, int, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
// const string
void deklKonst(char *, char *, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, bool arg = false, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);

ClassEnv * hledejClass(char *);
MethodEnv * hledejMethod(char *, ClassEnv *, bool = true);
PrvekTab * hledejMember(char *, ClassEnv *, MethodEnv *, bool = true);

PrvekTab * adrSym(char*, ClassEnv *, MethodEnv *);
PrvekTab * adrProm(char*, ClassEnv *, MethodEnv *);
int prvniIdxProm(char *, ClassEnv *, MethodEnv *);

void symCleanup();

#endif
