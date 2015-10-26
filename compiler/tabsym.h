/* tabsym.h */

#ifndef TABSYM_H
#define TABSYM_H

#include <stdio.h>
#include <stdlib.h>

enum DruhId {
	Nedef, IdProm, IdKonst
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

struct PrvekTab {
	char *ident;
	DruhId druh;
	int hodn; // TODO: this will be a pointer (var can also be an obj ref)
	bool pole;
	bool isStatic;
	int prvni, posledni;

	//CRecord *record;
	PrvekTab *dalsi;
	//PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r);

	// Variable
	PrvekTab(char *i, DruhId d, int h, PrvekTab *n);
	// Static array
	PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n);
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
};

struct ClassEnv {
	char * className;
	PrvekTab * syms; // class consts and vars
	MethodEnv * methods;
	MethodEnv * constructor;
	ClassEnv * next;
	ClassEnv * parent;
	int class_addr_next; // Static members address counter
	int obj_addr_next; // Instance members address counter
};

struct Env {
	ClassEnv * clsEnv;
	MethodEnv * mthEnv;
};

static ClassEnv * TabClass;


static PrvekTab * TabSym; // TODO: remove this
static int volna_adr;

ClassEnv * deklClass(char *, char * = NULL);
MethodEnv * deklMethod(char *, bool constructor = false, bool isStatic = false, ClassEnv * cls = NULL);
void deklKonst(char *, int, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, bool arg = false, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, int, int, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
//void deklRecord(char *, CRecord *);

PrvekTab *hledejId(char *);

int adrProm(char*);
int prvniIdxProm(char *id);
DruhId idPromKonst(char*, int*);

void symCleanup();

#endif
