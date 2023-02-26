#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "scanType.h"
#include "parser.tab.h"
//#include "semantic.h"

typedef enum {declK, stmtK, expK} subKind;
typedef enum {varK, funcK, paramK} declKind;
typedef enum {NullK, ElifK, IfK, WhileK, CompoundK, IterK, ReturnK, BreakK, rangeK} stmtKind;
typedef enum {opK, ConstantK, charConstantK,stringConstantK, idK, AssignK, InitK, CallK, boolConstK} expKind;
typedef enum {Void, Int, Bool, Char, Undefined} ExpType;
typedef enum {Local, Global, Static, Parameter, None, LocalStatic} RefType;
typedef enum {leftSquareBracOp, rightSquareBracOp, PLUSOp, EQOp, ASSOp, ANDOp, MULOp, MinusOp, DIVOp, INCOp, DECOp, OROp, NOTOp, NOTEQOp, ADDASSOp, SUBASSOp, MULASSOp, DIVASSOp, LESSEQOp, GREATEQOp, GREATOp, LESSOp, NEQOp,ASSOp2,PEROp, QUEOp, SEMIOp, sizeofOp, chsignOp} OpKind;

#define MAXCHILDREN 3
static int indent = 0;

typedef struct ASTNode{

  // connectivity in the tree
  struct ASTNode *child[MAXCHILDREN];   // children of the node
  struct ASTNode *sibling;              // siblings for the node

  // what kind of node
  int lineno;                            // linenum relevant to this node
  subKind nodekind;                     // type of this node
  int nvalue; //for number value
  int value;
  union                                  // subtype of type
  {
    declKind decl;                     // used when DeclK
    stmtKind stmt;                     // used when StmtK
    expKind exp;                       // used when ExpK
  } subkind;
    
  // extra properties about the node depending on type of the node
  union                                  // relevant data to type -> attr
  {
    OpKind op;                         // type of token (same as in bison)
    int value;                         // used when an integer constant or boolean
    char *cvalue;               // used when a character
    int boolVal;//the value for bool if it's bool
    char *stringConsto;                      // used when a string constant
    char *name;                        // used when IdK
  } attr;                                 

  RefType refType;
  ExpType expType;           // used when ExpK for type checking
  ExpType returnType;     //used only for function nodes
  int Union;//This string stores which union memory space is used
  // 1 cvalue, 2 op, 3 name, 4 stringConsto, 5 value, 6 boolVal
  bool isArray;                          // is this an array
  bool isFunc;
  bool comFunc;
  bool isStatic;                         // is staticly allocated?
  bool isUsed;
  bool isInitialized;
  bool hasReturn;
  bool returnV;//does it have a return value?
  int returnL;//used for functions recording the line of the return statement
  char * id = NULL;;
  char * location;
  int memLoc;
  bool ioTree;
  //  int offset;
  int size;
  long long int offset;
  int isBool;//if it's bool
  int scope;  //this integer assigns an attribute to the scope
  //when entering a new scope this integer self increments by 1
  //and all the local variables within this scope inherit this scope number
  //when leaving a scope it doesn't do anything

}astNode;

astNode * addDecl(declKind kind, TokenData * token);
astNode * addStmt(stmtKind kind, TokenData * token);
astNode * addExp(expKind kind, TokenData * token);
astNode *addSibling(astNode *t, astNode *s);
void printStmt(astNode * node, bool M);
void printExp(astNode * node, bool state, bool M);
void printDecl(astNode * node, bool state, bool M);
static void printIndent();
void printTree(astNode * tree, bool state, bool M);
ExpType setType(TokenData * token);
ExpType setNodeType(astNode * tree);
OpKind setOp(TokenData * token);
char * convRef(RefType);
astNode * genIO();

#endif
