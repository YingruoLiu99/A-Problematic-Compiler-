#include <stdlib.h>
#include <stdio.h>
#include "scanType.h"
//#include "parser.tab.h"                                                                                                                                                                                
#include "util.h"
#include "symbolTable.h"


void semanticAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter);
void hingSight(astNode * tree, SymbolTable * symbolTable);
void hindSightDecl(astNode * tree, SymbolTable * symbolTable);
void hindSightStmt(astNode * tree, SymbolTable * symbolTable);
void hindSightExp(astNode * tree, SymbolTable * symbolTable);
void stmtAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter);
void declAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter);
void expAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree,  bool Enter);
void andorCheck(astNode * tree, SymbolTable * symboTable);
//void semanticAnalysis(astNode * tree, SymbolTable * symbolTable, bool insertIO);
//char * setPrint(ExpType stuff);
char * getID(astNode *);
//void andorcheck(astNode *);

char * convExpType(ExpType);
char * getType(astNode *);
//astNode * genIO();
ExpType checkType(astNode *);
//char * setMem();
