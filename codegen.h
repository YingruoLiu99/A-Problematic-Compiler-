#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include "treeUtils.h"
#include "scanType.h"
#include "emitcode.h"
#include "util.h"
#include "parser.tab.h"

typedef struct FuncList{

  char *id;
  char *location;
  int memLocation;
  FuncList *next;

}funcList;

typedef struct VarList{
  char * id;
  int memLocation;
  char * location;
  VarList * next;
} varList;


void codeGenG(std::string, void * ptr);
void codeGen(astNode * node1, astNode * node2);
void codeGenIO(astNode * node);
astNode * generateIOTree();
void expGen(astNode * node);
void stmtGen(astNode * node);
void declGen(astNode * node);
void countSize(astNode * tree);

