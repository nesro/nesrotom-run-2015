/* lexan.cpp */

#include "lexan.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
	LETTER, NUMBER, WHITE_SPACE, NEW_LINE, END, NO_TYPE
} InputCharType;

const char *symbTable[43] = { "IDENT", "NUMB", "PLUS", "MINUS", "TIMES",
		"DIVIDE", "EQ", "NEQ", "LT", "GT", "LTE", "GTE", "LPAR", "RPAR",
		"ASSIGN", "COMMA", "SEMICOLON", "kwVAR", "kwCONST", "LCURLY", "RCURLY",
		"kwIF", "kwTHEN", "kwELSE", "kwWHILE", "kwDO", "kwWRITE", "kwREAD",
		"EOI", "ERR" /* <nesro> */, "kwCASE", "kwOF", "DOT", "DOUBLE_DOT",
		"COLON", "DASH", "kwINTEGER", "kwRECORD", "LBRAC", "RBRAC", "kwFOR", "kwTO", "kwDOWNTO" }; //symbol names in the same order as in LexSymbolType

static int character; // vstupni znak
static InputCharType input; // vstupni symbol

void readInput(void) {
	character = getChar();
	if ((character >= 'A' && character <= 'Z')
			|| (character >= 'a' && character <= 'z'))
		input = LETTER;
	else if (character >= '0' && character <= '9')
		input = NUMBER;
	else if (character == EOF)
		input = END;
	/*else if (character == '\n')
		input = NEW_LINE;*/
	else if (character <= ' ')
		input = WHITE_SPACE;
	else
		input = NO_TYPE;
}

const struct {
	char* slovo;
	LexSymbolType symb;
} keyWordTable[] = {
//
		{ "var", kwVAR }, //
		{ "const", kwCONST }, //
//		{ "begin", kwBEGIN }, //
//		{ "end", kwEND }, //
		{ "if", kwIF }, //
		{ "then", kwTHEN }, //
		{ "else", kwELSE }, //
		{ "while", kwWHILE }, //
		{ "do", kwDO }, //
		{ "write", kwWRITE }, //
		{ "read", kwREAD }, //
		{ "case", kwCASE }, //
		{ "of", kwOF }, //
		{ "Integer", kwINTEGER }, //
		{ "Record", kwRECORD }, //
		{ "for", kwFOR },
		{ "to", kwTO },
		{ "downto", kwDOWNTO },
		{ NULL, (LexSymbolType) 0 } //
};

LexSymbolType keyWord(char* id) {
	int i = 0;
	while (keyWordTable[i].slovo)
		if (strcmp(id, keyWordTable[i].slovo) == 0)
			return keyWordTable[i].symb;
		else
			i++;
	return IDENT;
}

void error(char* text) {
	printf("\n%s\n", text);
	exit(1);
}

LexicalSymbol readLexemInternal(void) {
	LexicalSymbol data;
	int delkaId;
	q0: switch (character) {
	case '{':
		data.type = LCURLY;
		readInput();
		return data;
	case '}':
		data.type = RCURLY;
		readInput();
		return data;
	case '+':
		data.type = PLUS;
		readInput();
		return data;
	case '-':
		data.type = MINUS;
		readInput();
		return data;
	case '*':
		data.type = TIMES;
		readInput();
		return data;
	case '/':
		readInput();
		goto q1;
	case '(':
		data.type = LPAR;
		readInput();
		return data;
	case ')':
		data.type = RPAR;
		readInput();
		return data;
	case '=':
		readInput();
		goto q6;
	case '<':
		readInput();
		goto q4;
	case '>':
		readInput();
		goto q5;
	case ':':
		data.type = COLON;
		readInput();
		return data;
	case ',':
		data.type = COMMA;
		readInput();
		return data;
	case ';':
		data.type = SEMICOLON;
		do{ // All following new lines ignored
			readInput();
		} while(input == NEW_LINE || input == WHITE_SPACE);
		return data;
	case '.':
		readInput();

		if (character == '.') {
			data.type = DOUBLE_DOT;
			readInput();
		} else {
			data.type = DOT;
		}
		return data;
	case '[':
		data.type = LBRAC;
		readInput();
		return data;
	case ']':
		data.type = RBRAC;
		readInput();
		return data;
	default:;
	}

	switch (input) {
	case NEW_LINE: // New line is treated as semicolon
		data.type = SEMICOLON;
		do{ // All following new lines ignored
			readInput();
		} while(input == NEW_LINE || input == WHITE_SPACE);
		return data;
	case WHITE_SPACE:
		readInput();
		goto q0;
	case END:
		data.type = EOI;
		return data;
	case LETTER:
		delkaId = 1;
		data.ident[0] = character;
		readInput();
		goto q2;
	case NUMBER: {
		data.number = character - '0';
		data.type = NUMB;

		int zeroTest = data.number;

		readInput();

		if (zeroTest == 0 && input == LETTER && character == 'x') {
			/* read the 'x' symbol */
			readInput();
			goto q7;
		}

		goto q3;
	}
	default:
		data.type = ERR;
		error("Nedovoleny znak.");
		return data;
	}

	q1: //
	switch (character) {
	/*case '}':
		readInput();
		goto q0;*/
	case '/': // One line comment
		do{
			readInput();
			if(input == END){
				error("Neocekavany konec souboru v komentari.");
				data.type = ERR;
				return data;
			}
		} while(input != NEW_LINE);
		readInput();
		goto q0;
	case '*': // Long comment
		readInput();
		goto q1a;
	default:
		data.type = DIVIDE;
		return data;
	}

	q1a: // Long comment
	switch (character) {
	case '*':
		readInput();
		if(character == '/'){
			readInput();
			goto q0;
		}
		goto q1a;
	case EOF:
		data.type = ERR;
		error("Neocekavany konec souboru v komentari.");
		return data;
	default:
		readInput();
		goto q1a;
	}

	q2: //
	switch (input) {
	case LETTER:
	case NUMBER:
		data.ident[delkaId] = character;
		delkaId++;
		readInput();
		goto q2;
	default:
		data.ident[delkaId] = 0;
		data.type = keyWord(data.ident);
		return data;
	}

	q3: //
	switch (input) {
	case NUMBER:
		data.number = 10 * data.number + (character - '0');
		readInput();
		goto q3;
	default:
		return data;
	}

	q4: //
	switch (character) {
	case '=':
		data.type = LTE;
		readInput();
		return data;
	case '>':
		data.type = NEQ;
		readInput();
		return data;
	default:
		break;
	}
	switch (input) {
	default:
		data.type = LT;
		return data;
	}

	q5: //
	switch (character) {
	case '=':
		data.type = GTE;
		readInput();
		return data;
	default:
		break;
	}
	switch (input) {
	default:
		data.type = GT;
		return data;
	}

	q6: //
	switch (character) {
	case '=':
		readInput();
		data.type = EQ;
		return data;
	default:
		data.type = ASSIGN;
		return data;
	}
	/*switch (input) {
	default:
		data.type = ERR;
		error("Ocekava se \'=\'.");
		return data;
	}*/

	q7: //
	switch (input) {
	case NUMBER:
		data.number = 16 * data.number + (character - '0');
		readInput();
		goto q7;
	case LETTER:
		switch (character) {
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			data.number = 16 * data.number + (character - 'a' + 10);
			readInput();
			goto q7;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			data.number = 16 * data.number + (character - 'A' + 10);
			readInput();
			goto q7;
		default:
			fprintf(stderr, "Unknown hexadecimal symbol.\n");
			exit(1);
		}
		break;
	default:
		return data;
	}
}

LexicalSymbol readLexem(void) {
	LexicalSymbol symb = readLexemInternal();
	printf("LEXEM: %s", symbTable[symb.type]);
	if(symb.type == NUMB){
		printf(" %d", symb.number);
	}
	else if(symb.type == IDENT){
		printf(" \"%s\"", symb.ident);
	}
	printf("\n");
	return symb;
}

int initLexan(char *fileName) {
	if (!initInput(fileName))
		return 0;
	readInput();
	return 1;
}
