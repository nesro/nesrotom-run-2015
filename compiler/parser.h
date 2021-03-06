/* parser.h */

#ifndef PARSER_H
#define PARSER_H

#include "strom.h"
#include "parser.h"
#include "lexan.h"
#include "tabsym.h"

enum Context {
	C_NIL, C_CYCLE
};

Prog *Program();
StatmList * Dekl(Env, bool);
StatmList * DeklKonst(Env, bool, bool = false);
void ZbDeklKonst(Env, bool, bool, int);
StatmList * DeklProm(Env, bool);
StatmList * ZbDeklProm(Env, bool);
StatmList *SlozPrikaz(Env env, Context ctxt = C_NIL);
StatmList *ZbPrikazu(Env env, Context ctxt = C_NIL);
Statm *Prikaz(Env env, Context ctxt = C_NIL);
ArgList * Args(Env env);
ArgList * ZbArgs(Env env);
Expr * Ident(Env, Env, bool);
Expr * ZbIdent(Env, Env, bool, bool&);
Statm *AssignmentOrCall(Env);
Statm * Assignment(Env env, Var * lvalue = NULL);
Statm *CastElse(Env env, Context ctxt = C_NIL);
void ZbFor(Env env, char id[MAX_IDENT_LEN], Expr * offset, Expr ** cond, Statm ** counter, Statm ** body);
Expr *Podminka(Env);
Operator RelOp();
Expr *Vyraz(Env);
Expr *ZbVyrazu(Env, Expr*);
Expr * LOrTerm(Env);
Expr * ZbLOrTermu(Env, Expr*);
Expr * XorTerm(Env);
Expr * ZbXorTermu(Env, Expr*);
Expr * LAndTerm(Env);
Expr * ZbLAndTermu(Env, Expr*);
Expr * BOrTerm(Env);
Expr * ZbBOrTermu(Env, Expr*);
Expr * BAndTerm(Env);
Expr * ZbBAndTermu(Env, Expr*);
Expr * RelOpTerm(Env);
Expr * ZbRelOpTermu(Env, Expr*);
Expr * ShiftTerm(Env);
Expr * ZbShiftTermu(Env, Expr*);
Expr *Term(Env);
Expr *ZbTermu(Env, Expr*);
Expr *Faktor(Env);
Expr * ConstructorCall(Env env);
Expr * Pole(Env);
ArgList * Elems(Env env);
ArgList * ZbElems(Env env);
ArgList * ArrayOffset(Env env);

Expr *VarOrConst(char*, ArgList * offset, Env env);
ClassList * SeznamTrid();
ClassList * ZbTrid();
Class * Trida();
StatmList * SeznamMetod(Env env);
StatmList * ZbMetod(Env env);
Statm * ClenTridy(Env env);
Statm * Metoda(Env env, bool isStatic);

static LexicalSymbol Symb;

Prog* Program();
int initParser(char*);

void Srovnani(LexSymbolType);
void Srovnani_IDENT(char *);

Expr * RecordFaktor(char *);
//CRecord * Record();
//CRecord * ZbRecord();
bool Typ(Env env, char *, bool);
void TypVar(char *);
void TypRec();

CaseBlock * ntCASE_BODY(Env env);
CaseBlockScope * ntCASE_SCOPE(Env env);
Numb * ntCASE_RANGE(Env env);
CaseBlockScope * ntCASE_SCOPE_NEXT(Env env);

#endif
