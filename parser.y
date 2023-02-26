%{ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "scanType.h"
  //#include "symbolTable.cpp"
#include "util.h"
#include "parser.tab.h" 
#include "semantic.h"
#include "ourgetopt.h"
#include "ourgetopt.c"
#include "yyerror.h"

#ifdef CPLUSPLUS
  extern int yylex();
#endif
 
  extern FILE *yyin;
  extern int yydebug;

#define YYERROR_VERBOSE


  //  void yyerror(const char *msg)
  // {
    //    printf("ERRORla(PARSER): %s\n", msg);
  // }
  //// any C/C++ functions you want here that might be used in grammar actions below
  //// any C/C++ globals you want here that might be used in grammar actions below

extern OpKind setOp(TokenData*);
extern  astNode * addDel(declKind , TokenData * );
extern  astNode * addStmt(stmtKind , TokenData * );
extern  astNode * addExp(expKind , TokenData * );
extern ExpType setType(TokenData *);
extern void printTree(astNode *);
extern int numErrors;
extern int goffset;
extern void hindSight(astNode *, SymbolTable *);
extern void initErrorProcessing();
 extern void codeGen(astNode *, astNode *);
//int  numErrors = 0;
 
astNode * syntaxTree;
 FILE * code;
 int nodeCount = 0;
 // int Scope = 0;
 //
%}

//// your %union statement
%union {
  TokenData *tokenData;
  struct ASTNode  *node;
}
//// your %token statements defining token classes

%token <tokenData> ID NUMCONST CHARCONST STRINGCONST BOOLCONST WARNCHAR
%token <tokenData> COMMENTS KEYWORD ERROR OPERATOR CHAR INT BOOL STATIC VOID 
%token <tokenData> IF THEN ELSE DO RETURN BREAK WHILE
%token <tokenData> OR AND NOT TO BY ASS PLUS Minus MUL DIV PER QUE
%token <tokenData> EQ GREATEQ LESSEQ NOTEQ LEQ NEQ GEQ GREAT LESS MAX MIN DEC leftSquareBrac rightSquareBrac 
%token <tokenData> INC ADDASS SUBASS MULASS DIVASS ASGN WCHAR COLON COMMA FOR SEMI leftBrac rightBrac leftCurlBrac rightCurlBrac
 //%token <node> typeSpec
 //%token <tokenData> typeSpec

%type <node> program constant call factor
%type <node> declList decl 
%type <node> varDecl funDecl scopedVarDecl localDecls
%type <tokenData> typeSpec 
%type <tokenData> unaryop relop mulop sumop
%type <node> varDeclList argList paramList paramTypeList paramIdList stmtList
%type <node> varDeclInit 
%type <node> varDeclId paramId iterRange
%type <node> params stmt args exp expStmt simpleExp compoundStmt matched unmatched returnStmt breakStmt
%type <node> mutable andExp immutable unaryRelExp relExp sumExp mulExp unaryExp


%%

program        : declList{
  //printf("getting to syntax tree point\n");
  syntaxTree = $1;
  $$ = $1;
 }
;

declList       : declList decl{
  astNode *t = $1;
  //printf("getting to set decl list siblingpoint\n");
  addSibling($1, $2);
  //printf("got out from new\n");    
 }
|decl {
  //printf("getting to set decl list point\n");
  $$ = $1;
 }
;

decl           : varDecl{
  //printf("getting to set decl point\n");
  $$ = $1;
 }
|funDecl{
  //printf("getting to func decl\n");
  $$ = $1;
 }
|error{
  $$=NULL;

 }
;

/*2**************************************************************/
varDecl        : typeSpec varDeclList SEMI{
  astNode * t = $2;
  if(t != NULL){
    //    printf("t is not NULL\n");
  t->expType = setType($1);
  while( t->sibling != NULL){
    //printf("stuck1!\n");                                                                                                                                                                                
    t->expType = setType($1);
    t = t-> sibling;
    t->expType = setType($1);
  }
  }
  else{
    //printf("t is NULL\n");
  }
  //printf("alalsfs %d\n", $1->tokenclass);
  /*
  while( t->sibling != NULL){
    //printf("stuck1!\n");
    t->expType = setType($1);
    t = t-> sibling;
    t->expType = setType($1);
    }*/
  //printf("I got out!\n");
  $$ = $2;
  yyerrok;
 }
|error varDeclList SEMI{
  $$= NULL;
  yyerrok;
 }
|typeSpec error SEMI{
  $$=NULL;
  //printf("vardecl wrong\n");
  yyerrok;
 }
;

scopedVarDecl  : STATIC typeSpec varDeclList SEMI{

  astNode * t = $3;
  t->expType = setType($2);
  //printf("declList 124\n");
  t->isStatic = true;
  while( t->sibling != NULL){
    //printf("stuck2!\n");
    t->expType = setType($2);
    t->isStatic = true;
    t = t-> sibling;
    t-> expType = setType($2);
  }
  //  printDecl($3);
  $$ = $3;
  yyerrok;
 }
| typeSpec varDeclList SEMI{
  astNode *t = $2;
  //printf("declList 136\n");
  //printf("OOOOOOOOOOO type %s\n", $1->tokenstr);
  if(t!= NULL){
  t-> expType = setType($1);
  while( t->sibling != NULL){
    //printf("stuck3!\n");
    t-> expType = setType($1);
    t->isStatic = 0;
    t = t->sibling;
    t->expType = setType($1);
  }
  }
  $$=$2;
  yyerrok;
 }
;

varDeclList    : varDeclList COMMA varDeclInit
{
  astNode *t = $1;
  if(t != NULL){

    while(t->sibling != NULL){
      //printf("stuck4!\n");
      t= t->sibling;
    }
    t->sibling = $3;
    $$ = $1;
  }
  else{
    $$ = $3;
  }
  yyerrok;
  //$$->isUsed = false;
}
| varDeclInit{
  //printf("passing ID 167\n");
  //printDecl($$);
  $$ = $1;
 }
|varDeclList COMMA error{
  $$= NULL;
 }
|error{
  $$=NULL;
  //  printf("error here\n");
 }
;

varDeclInit    : varDeclId{
  $$ = $1;
  //printf("passing ID 173\n");
 }
| varDeclId COLON simpleExp{
  $$ = addExp(InitK, $2);
  if($1!= NULL)
  $1->child[0] = $3;
  $$=$1;
  /*
  $$->nodekind=expK;
  $$->subkind.exp=InitK;
  $1->child[0]->nodekind=expK;
  $1->child[0]->subkind.exp=InitK;
  */
  if($1!= NULL)
  $$->isInitialized =true;
  nodeCount++;
  
 }
|error COLON simpleExp{
  $$=NULL;
  yyerrok;
 }
;

varDeclId      : ID{
  $$ = addDecl(varK, $1);
  //printf("This ID should be a CCCCCAAAATTTT 184 the id is %s\n", $1->tokenstr);
  //printDecl($$);  
  $$->id = strdup($1->tokenstr);
  $$->isUsed = false;
  nodeCount++;
 
 }
| ID leftSquareBrac NUMCONST rightSquareBrac{
  $$ = addDecl(varK, $1);
  $$->id = strdup($1->tokenstr);
  $$->isArray = true;//the varaible is an array
  nodeCount++;
  $$->size = atoi(strdup($3->tokenstr));
  $$->isUsed = false;
  }
| ID leftSquareBrac error{
  $$=NULL;
 }
| error rightSquareBrac{
  $$=NULL;
  yyerrok;
 }
;

typeSpec       : INT{
  $$ = $1;
  //printf("The type spec is %s \n", $1->tokenstr);
 }
| BOOL{
  $$ = $1;
  }
| CHAR{
  $$ = $1;
  }
;

/*3****************************************************************/
//
funDecl        : typeSpec ID leftBrac params rightBrac compoundStmt{
  //  printf("getting to the func point\n");
  $$ = addDecl(funcK, $2);
  $$->expType = setType($1);
  //  $2->expType = setType($1);
  $$ -> id = strdup($2 -> tokenstr);
  $$->hasReturn=false;
  $$->returnV=false;
  $$ -> child[0] = $4;
  $$ -> child[1] = $6;
  nodeCount++;
 }
| ID leftBrac params rightBrac compoundStmt{
  //printf("funct decl\n");
  $$ = addDecl(funcK, $1);
  $$->expType = Void;
  $$ ->id = strdup($1 -> tokenstr);
  $$ ->child[0] = $3;
  $$ ->child[1] = $5;
  nodeCount++;
  }
|typeSpec error{
  //printf("if you dont' match I die\n");
  $$=NULL;
 }
|typeSpec ID leftBrac error{
  $$=NULL;
  //printf("matching error here\n");
 }
|ID leftBrac error{
  $$=NULL;
  //printf("matching error 3\n");
 }
|ID leftBrac params rightBrac error{
  $$=NULL;
  //printf("matching error la here\n");
 }
;
//
params          : paramList{
  $$ = $1;
  //printf("paramlist passed\n");
 }
| %empty{
  //printf("param empty\n");
  $$= NULL;
 }
;

paramList       : paramList SEMI paramTypeList{
  astNode *t = $1;
  if( t != NULL){
    while(t->sibling != NULL){
      t = t->sibling;
    }
    t->sibling = $3;
    $$= $1;
  }else{
    $$ = $3;
  }
 }
|paramTypeList{
  $$ = $1;
 }
|paramList SEMI error{
  $$=NULL;
 }
|error{
  $$=NULL;
 }
;

paramTypeList   : typeSpec paramIdList{
  astNode *t = $2;
  if($2 != NULL){
  t-> expType = setType($1);
  //printf("param list\n");
  //printDecl(t);
  while( t->sibling != NULL){
    t->expType = setType($1);
    t= t-> sibling;
    t->expType = setType($1);
  }
  }
  $$= $2;
 }
|typeSpec error{
  $$=NULL;
  //printf("things wrong at param\n");
 }
;

paramIdList     : paramIdList COMMA paramId{
  astNode *t = $1;
  //printf("in param list\n");
  if(t!= NULL){
    while(t->sibling != NULL){
      t= t->sibling;
    }
    t->sibling = $3;
    $$ = $1; 
  }
  else{
    $$ = $1;
  }
  yyerrok;
 }
| paramId{
  $$ = $1;
  //printf("ParamID 289\n");
 }
|paramIdList COMMA error{
  $$=NULL;
 }
|error{
  $$=NULL;
 }
;

paramId         : ID{
  $$ = addDecl(paramK, $1);
  $$->isUsed = false;
  //printf("IDDDDD 287\n");
  //printDecl($$);
  //printf("IDDDDD fini289\n");
  $$-> id = strdup($1->tokenstr);
  nodeCount++;
 }
| ID leftSquareBrac rightSquareBrac{
  $$ = addDecl(paramK, $1);
  $$-> id = strdup($1->tokenstr);
  $$->isArray=true;
  nodeCount++;
  }
;

/*****************************************************************/
/*
stmt           : matched{
  $$ = $1;  }
| unmatched{
  $$ = $1;
  }
;
*/
compoundStmt   : leftCurlBrac localDecls stmtList rightCurlBrac{
  $$ = addStmt (CompoundK, $1);
  //printf("compound stmt\n");
  $$ -> child[0] = $2;
  $$ -> child[1] = $3;
  if($3 == NULL){
    //printf("XIAXIAXIAXIA\n");
  }else{
    //printf("STMT NOT NULLLLLL]n");
    //printTree($3);
    //printf("done with counpound\n");
  }
  //printf("done with counpound\n");  
  nodeCount++;
  yyerrok;
 }
;
localDecls     : localDecls scopedVarDecl{
  astNode *t = $1;

  if( t!= NULL){
    while(t->sibling != NULL){
      t = t->sibling;
    }
    t->sibling = $2;
    $$ = $1;
    //printf("LALALALALALALA t is not NULL\n");                                                                                                                                                           
  }
  else{
    //printf("NULLLLLLLL\n");                                                                                                                                                                             
    $$ = $2;
    //printStmt($2);                                                                                                                                                                                      
    //printf("NULLLLLLLL fini\n");                                                                                                                                                                        
  }

  /*
  astNode *t = $2;
  t-> expType = setNodeType($2);
  addSibling($1,$2);
  /*
  while( t->sibling != NULL){
    t->expType = setNodeType($2);
    t= t-> sibling;
    t->expType = setNodeType($2);
  }
  
  $$= $2;*/
 }
| %empty{
  //printf("local decls empty\n");
  $$ = NULL;
 }
;

stmtList       : stmtList stmt{
  astNode *t = $1;

  if( t!= NULL){
    while(t->sibling != NULL){
      t = t->sibling;
    }
    t->sibling = $2;
    $$ = $1;
    //printf("LALALALALALALA t is not NULL\n");
  }
  else{
    //printf("NULLLLLLLL\n");
    $$ = $2;
    //printStmt($2);
    //printf("NULLLLLLLL fini\n");
  }

  nodeCount++;
 }
| %empty{
  //printf("SEEETTTING NULLL\n");
  $$ = NULL;
 }
;

stmt           : matched{
  $$ = $1;
  //printf("MAAAATCHED\n");
  //printStmt($$);
  //printf("MAAAAAATCHfini\n");
 }
| unmatched{
  $$ = $1;
  //printf("UNMAAAATCHED\n");
  //printStmt($$);
  //printf("UNMAAAATCHED fini\n");
  }
;
//fix
matched        : WHILE simpleExp DO matched {
  $$=addStmt(WhileK, $1);
  //printf("WHILELLLLLLLLLL \n");
  $$->child[0]=$2;
  $$->child[1]=$4;
  //printf("WHILELLLLLLLLLL fini\n");
  nodeCount++;
 }
| IF simpleExp THEN matched ELSE matched{
  //printf("IFFFFFFFFFFFla]\n");
  $$=addStmt(IfK, $1);
  //printStmt($$);
  $$->child[0] = $2;
  $$->child[1]=$4;
  $$->child[2]=$6;
  //printf("IFFFFFFFFFFFla]fini\n");
 }
|FOR ID ASGN iterRange DO matched{
  //printf("FOOOOOOOOORRRRRRR match\n");
  $$=addStmt(IterK, $1);
  astNode *t = addDecl(varK, $2);
  t->isUsed=false;
  t->id = $2->tokenstr;
  $$->child[0] = t;
  // astNode *t2 = addExp(idK, $1);
  //t2->attr.id = $4->tokenstr;
  $$->child[1]=$4;
  $$->child[2]=$6;
  t->expType=Int;
  nodeCount++;
  //printf("FOOOOOOOOORRRRRRR match fini\n");
 }
| returnStmt{
  $$ = $1;
  }
| breakStmt{
  $$ = $1;
  }
| expStmt{
  $$ = $1;
  //printf("EEXXXXPPPPPPPP\n");
  //printExp($$);
  //printf("EEXXXXPPPPPPPPfini\n");

 }
|compoundStmt{
  //printf("BACK TO COMPOUND\n");

  $$ = $1;
 }
|IF error{
  $$=NULL;
 }
|IF error ELSE matched{

  $$=NULL;
  yyerrok;
 }
|IF error THEN matched ELSE matched{
  $$=NULL;
  yyerrok;
 }
|WHILE error DO matched{
  $$= NULL;
  yyerrok;
 }
|WHILE error{
  $$=NULL;
 }
|FOR ID ASGN error DO matched{

  $$=NULL;
  yyerrok;
 }
|FOR error{
  $$=NULL;
 }
;

unmatched      : IF simpleExp THEN unmatched{
  //printf("IFFFFFFFFFFFlaunmatched]\n");
  $$=addStmt(IfK, $1);
  $$->child[0]=$2;
  $$->child[1]=$4;
  nodeCount++;
  //printf("IFFFFFFFFFFFlaunmatched]fini\n");
 }
| IF simpleExp THEN matched{
  //printf("IFFFFFFFFFFFlaun unununmatched]999\n");
  $$=addStmt(IfK, $1);
  $$->child[0]=$2;
  
    $$->child[1]=$4;
 
     //printExp($4);
  nodeCount++;
  // printf("IFFFFFFFFFFFlaun unununmatched] 999fini\n");
 }
| IF simpleExp THEN matched ELSE unmatched{
  
  //printf("if passed to here\n");
  $$ = addStmt(IfK, $1);
  $$->child[0]=$2;
  $$->child[1]=$4;
  $$->child[2]=$6;
  nodeCount++;
 }
|WHILE simpleExp DO unmatched{
  $$=addStmt(WhileK, $1);
  $$->child[0]=$2;
  $$->child[1]=$4;
  nodeCount++;
 }
|FOR ID ASGN iterRange DO unmatched{
  //printf("FOOOOOOOOORRRRRRR unmatch\n");
  $$=addStmt(IterK, $1);
  astNode * t = addDecl(varK, $2);
  t->isUsed=false;
  t->id = $2->tokenstr;
  t->expType=Int;
  $$->child[0]=t;
  //astNode *t2 = addExp(varK, $1);
  //t2->attr.id = $4;
  $$->child[1]=$4;
  $$->child[2]=$6;
  nodeCount++;
  //printf("FOOOOOOOOORRRRRRR fini unmatched\n");
 }
|IF error THEN stmt{
  $$=NULL;
  yyerrok;
 }
|IF error THEN matched ELSE unmatched{
  $$=NULL;
  yyerrok;
 }
;

//
iterRange      : simpleExp TO simpleExp{
  $$ = addStmt(rangeK, $2);
  $$->child[0] = $1;
  $$->child[1] = $3;
 }
| simpleExp TO simpleExp BY simpleExp{
  $$ = addStmt(rangeK, $2);
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->child[2]=$5;
 }
|simpleExp TO error{
  $$= NULL;
 }
|error BY error{
  $$=NULL;
  yyerrok;
 }
|simpleExp TO simpleExp BY error{
  $$=NULL;
 }
;

//
returnStmt     : RETURN SEMI{

  $$ = addStmt(ReturnK, $1);
  nodeCount++;

 }
| RETURN exp SEMI{
  $$ = addStmt(ReturnK, $1);
  //printf("trying to have a RETURN child\n");
  //printExp($2);
  //getting NULL
  //printf("REturn finished\n");
  $$ -> child[0] = $2;
  nodeCount++;
  yyerrok;
 }
|RETURN error SEMI{
  $$=NULL;
  yyerrok;
 }
;

breakStmt      : BREAK SEMI{
  $$ = addStmt(BreakK, $1);
  nodeCount++;
 }
;

expStmt        : exp SEMI{
  //unsure                                                                                                                                                                                                
  //  $$ = addExp(idK, $2);
  //$$ -> child[0] = $1;
  //nodeCount++;
  //printf("EEEXXXPPPP\n");
  //printExp($1);
  $$ = $1;
  //printf("EEEXXXPPPP fini\n");
 }
| SEMI{
  //unsure                                                                                                                                                                                              

  /*
    printf("EEEXXXPPPPSEMi\n");
  printf("lalala %s",$1->tokenstr);
  $$ = $1;
  printf("EEEXXXPPPP SEMifini\n");
  $$ = addExp(opK,$1);
  $$->attr.op = setOp($1);
  printExp($$);

  */
   $$=NULL;
  }
|error SEMI{
  $$=NULL;
  yyerrok;
 }
;
/*
compoundStmt   : leftCurlBrac localDecls stmtList rightCurlBrac{
  $$ = addStmt (CompoundK, $1);
  $$ -> child[0] = $2;
  $$ -> child[1] = $3;
  printf("HJKSBFJKABSJK %s\n", $3->attr.id);
  printDecl($3);
  nodeCount++;
 }
;
*/
/*****************************************************************/

exp            : mutable ASGN exp{
  //printf("getting to assignLLLLLLLLL problem might be here\n");
  $$ = addExp(AssignK, $2);
  //printf("Line %d Token: ID Value: %s\n", $2->linenum, $2->tokenstr);
  $$ -> attr.op = setOp($2);
  //printf("getting to assignLLLLLLLLL\n");
  //printf("what's in exp \n");
  //printExp($$);
  //$$->expType =Int;
  $$->isInitialized = true;
  //printf("finished ASSSGGGIn\n");
  $$ -> child[0] = $1;
  $$ -> child[1] = $3;
  nodeCount++;

 }
| mutable ADDASS exp{
  $$ = addExp(AssignK, $2);
  $$ -> child[0] = $1;
  $$ -> child[1] = $3;
  $$->expType =Int;
  $$->child[0]->expType = Int;
  $$->child[1]->expType =Int;
  $$->isInitialized = true;
  $$ -> attr.op = setOp($2);
  nodeCount++;

  }
| mutable SUBASS exp{
  $$ = addExp(AssignK, $2);
  $$ -> child[0] = $1;
  $$ ->child[1] = $3;
  $$->expType =Int;
  $$->isInitialized = true;
  $$ ->attr.op = setOp($2);
  $$->expType =Int;
  $$->child[0]->expType =Int;
  $$->child[1]->expType =Int;
  nodeCount++;

  }
| mutable MULASS exp{
  $$ = addExp(AssignK, $2);
  $$ -> child[0] = $1;
  $$ ->child[1] = $3;
  $$->expType =Int;
  $$->isInitialized = true;
  $$ ->attr.op = setOp($2);
  $$->expType =Int;
  $$->child[0]->expType =Int;
  $$->child[1]->expType =Int;
  nodeCount++;

  }
| mutable DIVASS exp{
  $$ = addExp(AssignK, $2);
  $$ -> child[0] = $1;
  $$ ->child[1] = $3;
  $$->expType =Int;
  $$->isInitialized = true;
  $$ ->attr.op = setOp($2);
  $$->expType =Int;
  $$->child[0]->expType =Int;
  $$->child[1]->expType =Int;
  nodeCount++;

  }
| mutable DEC{
  $$ = addExp(AssignK, $2);
  $$ -> child[0] = $1;
  $$->expType =Int;
  $$->isInitialized = true;
  $$ ->attr.op = setOp($2);
  $$->expType =Int;
  //$$->child[0]->expType =Int;
  nodeCount++;

  }
| mutable INC{
  $$ = addExp(AssignK, $2);
  $$ -> child[0] = $1;
  $$->expType =Int;
  $$->isInitialized = true;
  $$ ->attr.op = setOp($2);
  $$->expType =Int;
  //$$->child[0]->expType =Int;
  nodeCount++;

  }
| simpleExp{
  //printf("at Chat const11\n");
  $$ = $1;
  //printf("PASSING to mutable10\n");
  //printExp($1);

  }
|error DIVASS exp{
  $$=NULL;
  yyerrok;
 }
|error ASGN exp{
  $$=NULL;
  yyerrok;
 }
|error ADDASS exp{
  $$=NULL;
  yyerrok;
 }
|error SUBASS exp{
  $$=NULL;
  yyerrok;
 }
|error MULASS exp{
  $$=NULL;
  yyerrok;
 }
|mutable ASGN error{
 $$= NULL;
 }
|mutable DIVASS error{
  $$= NULL;
 }
|mutable ADDASS error{
  $$=NULL;
 }
|mutable MULASS error{
  $$= NULL;
 }
|mutable SUBASS error{
  $$=NULL;
 }
|error INC{
  $$=NULL;
  yyerrok;
 }
|error DEC{
  $$=NULL;
  yyerrok;
 }
;


simpleExp      : simpleExp OR andExp{
  $$ = addExp(opK, $2);
  $$->child[0]=$1;
  $$->child[1]=$3;
  $$->attr.op = setOp($2);
  $$->expType = Bool;
  if($1 != NULL)
  $$->child[0]->expType =Bool;
  if($3 != NULL)
  $$->child[1]->expType = Bool;
  //
 }
| andExp{
  //printf("at Chat const10\n");
  //printf("PASSING to mutable9\n");
  $$=$1;
 }
|simpleExp OR error{
  $$=NULL;
 }
;
andExp         : andExp AND unaryRelExp{
  $$ = addExp(opK, $2);
  $$->child[0]=$1;
  $$->child[1]=$3;
  $$->expType = Bool;
  if($1 != NULL)
  $$->child[0]->expType =Bool;
  if($3 != NULL)
  $$->child[1]->expType = Bool;
  $$->attr.op = setOp($2);
  nodeCount++;
  // 
 }
|unaryRelExp{
  //printf("at Chat const9\n");
  //printf("PASSING to mutable8\n");
  $$ = $1;
 }
|andExp AND error{
  $$=NULL;
 }
;

unaryRelExp    : NOT unaryRelExp{
  $$ = addExp(opK, $1);
  $$->child[0]=$2;
  $$->expType =Bool;
  //$$->child[0]->expType = Bool;
  $$->attr.op = setOp($1);
  nodeCount++;

 }
|relExp{
  //printf("at Chat const8\n");
  //printf("PASSING to mutable7\n");
  $$=$1;
 }
|NOT error{
  $$=NULL;
 }
;

relExp         : sumExp relop sumExp{

  //code to fix
  $$ = addExp(opK,$2);
  nodeCount++;
  $$->child[0] = $1;
  //  $$->child[1] = $2;
  $$->child[1] = $3;
  $$->expType=Bool;
  $$->attr.op= setOp($2);
 }
|sumExp{
  //printf("at Chat const7\n");
  //printf("PASSING to mutable6\n");
  $$ = $1;
 }
;

relop          : LEQ{
  $$=$1;
 }
|LESS{
  $$=$1;
 }
|GREAT{
  $$=$1;
  //$$->expType = Bool;
 }
|GEQ{
  $$=$1;
 }
|ASS{
  $$ = $1;
 }
|NEQ{
  $$=$1;
 }
;

sumExp      : sumExp sumop mulExp{
 
  $$ = addExp(opK, $2);
  nodeCount++;
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op=setOp($2);
  $$->expType =Int;
  if($1 != NULL)
  $$->child[0]->expType =Int;
  if($3 != NULL)
  $$->child[1]->expType =Int;
} 

|mulExp{
  //printf("at Chat const6\n");
  //printf("PASSING to mutable5\n");
  $$ = $1;
}
|sumExp sumop error{
  $$=NULL;
 }
;

sumop       : PLUS{
  //+
  $$ = $1;
}
| Minus{
  //-
  $$ = $1;
  //  $$->attr.op = chsignOp;
  }
;

mulExp         : mulExp mulop unaryExp{
  $$ = addExp(opK, $2);
  nodeCount++;
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op = setOp($2);
  $$->expType=Int;
  $$->child[0]->expType =Int;
  $$->child[1]->expType =Int;
}
| unaryExp{
  //printf("at Chat const5\n");
  //printf("PASSING to mutable4\n");
  $$ = $1;
}
|mulExp mulop error{
  $$=NULL;
 }
;

mulop          : MUL{
  //*
  $$=$1;
}
| DIV{
  //  /
  $$=$1;
}

| PER{
  //%
  $$=$1;
  }
;

/*
mulExp         : mulExp mulop unaryExp{
  $$ = addExp(opK, $2);
  nodeCount++;
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op=setOp($2);
}
| unaryExp{
  //printf("at Chat const4\n");
  //printf("PASSING to mutable3\n");
  $$=$1;
  }
;

mulop          : MUL{
  $$ = $1;
 }
| DIV{
  $$ = $1;
  }
| PER{
  $$= $1;
  }
;
*/
unaryExp       : unaryop unaryExp{
  $$ = addExp(opK, $1);
  $$ -> attr.op = setOp($1);
  if($$->attr.op == MinusOp){
    $$->attr.op = chsignOp;
  }
  else if($$->attr.op == MULOp){

    $$->attr.op = sizeofOp;
  }
  $$->expType = Int;
  $$ -> child[0] = $2;
  nodeCount++;

  // $$->expType =Int;
 
 }
| factor{
  //printf("at Chat const3\n");
  //printf("PASSING to mutable2\n");
  $$ = $1;
 }
|unaryop error{
  $$=NULL;
 }
;

unaryop        : Minus{
  $$ = $1;
 }
| MUL{
  $$ = $1;
  }
| QUE{
  $$ = $1;
  }
;

factor         : immutable{
  //printf("at Chat const2\n");
  $$ = $1;
 }
| mutable{
  //printf("PASSING to mutable2\n");
  //printExp($1);
  $$ = $1;

  }
;

mutable        : ID{
  $$ = addExp(idK, $1);
  //printf("this is the ID %s\n", $1->tokenstr);
  //printf("getting to the ID node\n");
  if($$ == NULL){
    //printf("THE mutable is NULL OH\n");
  }else{
    //printf("ok it's not NULL \n");
  }
  //printf("PASSING to mutable1\n");
  //printExp($$);
  $$-> id = strdup($1->tokenstr);
  //printf("this is the ID %s\n", $$-> attr.id);
  nodeCount++;
 }
| mutable leftSquareBrac exp rightSquareBrac{
  //printf("PASSING to mutable2\n");  
  $$ = addExp(opK, $2);
  $$ -> attr.op = setOp($2);
  $$ -> child[0] = $1;
  $$-> id = strdup($2->tokenstr);
  $$ -> child[1] = $3;
  nodeCount++;
  if($3!= NULL)
  $$ -> child[1]->isInitialized = false;
  $$-> isArray = true;
 }
;

immutable      : leftBrac exp rightBrac{
  /*
  $$ = addExp(CallK, $1);
  $$ -> id = strdup( $1->tokenstr);
  $$ -> child[0] = $2;
  nodeCount++;
  */

  $$ =$2;
  yyerrok;
 }
| call{
  $$ = $1;
 }
|constant{

  //printf("at Chat const1\n");
  $$ = $1;
 }
|leftBrac error{
  $$=NULL;
 }
;

call           : ID leftBrac args rightBrac{
  $$ = addExp(CallK, $1);
  $$-> id = strdup($1->tokenstr);
  $$-> child[0] = $3;
  nodeCount++;
 }
|error leftBrac{
  $$=NULL;
  yyerrok; 
}
;

args           : argList{
  $$ = $1;
 }
| %empty{
  $$ = NULL;
  }
;

argList         : argList COMMA exp{
  $$ = addExp(AssignK, $2);
  astNode *t = $1;
  while( t->sibling!= NULL){
    t= t->sibling;
  }

  t->sibling = $3;
  $$ = $1;

  nodeCount++;
  yyerrok;
 }
| exp{
  $$ = $1;
 }
|argList COMMA error{
  $$=NULL;
 }
;

constant       : NUMCONST{
  //printf("reada numconst\n");
  //  syntaxTree = $$;
  //printf("PASSING to numconst\n");
  //printf("reada numconst\n");
  $$ = addExp(ConstantK, $1);
  //printf("The num CCCCCCOONST is %d\n", $1->nvalue);
  $$-> nvalue = $1 -> nvalue;
  $$-> expType = Int;
  nodeCount++;
 }
| CHARCONST{
  char * temp = strdup($1->tokenstr);//duplicate it but we don't mess with that                                                                                                                           
  char * temp2 = temp;
  /*
  if( temp[1] == '\\' ){

    switch(temp[2]){
    case 'n':
      temp2[0] = '\n';
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum,temp2[0] ,$1->tokenstr);
      break;
    case '0':
      temp2[0] = '\0';
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp2[0],$1->tokenstr);
      break;
    case '^':
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, 94,$1->tokenstr);
      break;
    case '@':
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, 64,$1->tokenstr);
      break;
    case '\"':
      temp2[0] = '\"';
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp2[0],$1->tokenstr);
      break;
    case '\\':
      temp2[0] = '\\';
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp2[0],$1->tokenstr);
      break;
    case '\'':
      temp2[0] = '\'';
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp2[0],$1->tokenstr);
      break;
    default:
      temp2[0] = temp[2];
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp2[0],$1->tokenstr);
      break;
    }
  }
  else{
    printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp[1],$1->tokenstr);
  }
  */
  //printf("at Chat const %s\n", $1->tokenstr);
  $$ = addExp(charConstantK, $1);
  $$ -> attr.cvalue = $1 -> tokenstr;
  nodeCount++;
  $$-> expType = Char;
  }
| STRINGCONST{
  char * temp = strdup($1->tokenstr);  //to be modified                                                                                                                                                   
  char * temp2 = strdup($1->tokenstr); //temp2 has the original string                                                                                                                                    
  int len = strlen($1->tokenstr);
  int i = 1; // i makes sure it goes through the whole string                                                                                                                                             
  int index = 1; //index is used to manipulate temp string                                                                                                                                                
  int j = 1;//j is used to assign value to temp string                                                                                                                                                    
  int b = 1;
  int la = 0;
  for(index = i ; i<len-1; i++, index++){

    if(temp2[i] == '\\'){
      switch (temp2[i+1])
        {
        case 'n':
          temp[index] = '\n';
          break;
        case '\"':
          temp[index] = '\"';
          break;
        case '\0':
          temp[index] = '\0';
          break;
        case '^':
          temp[index] = '^';
          break;
        case '\'':
          temp[index] = '\'';
          break;
        case '\\':
          temp[index] = '\\';
          break;

        }
      if(97<= temp2[i+1] <=122 || 65<= temp2[i+1] <=90)
        {
          if(temp[i+1]!=110){//if not \n for which we handled before                                                                                                                                      
            temp[index] = temp2[i+1];}
        }

      //move                                                                                                                                                                                              
      for(j = index+1,b=i+2; b<len; j++, b++){

        temp[j] = temp2[b];

      }

      //clean the "unused" part of temp                                                                                                                                                                   
      for(; j<len; j++){
        temp[j]='\0';
      }
      i++;
    }

  }

  //  printf("at string const %s\n", $1->tokenstr);  
  $$ = addExp(stringConstantK, $1);
  $$-> attr.stringConsto = $1 -> tokenstr;
  $$->isArray = true;
  $$->expType = Char;
  //printExp($$);
  nodeCount++;
  }
| BOOLCONST{
  //syntaxTree = $$;
  $$ = addExp(ConstantK, $1);
  char * temp;
  temp = strdup($1->tokenstr);
  $$->isBool = 1;
  if( strcmp(temp, "true")){
    // if bool is true
    $$-> attr. boolVal = 1;
  }else{
    $$->attr.boolVal = 0;
  }
  $$->expType =Bool;
  nodeCount++;
  }
| ERROR{
  numErrors++;
  printf("ERROR(%d): Invalid or misplaced input character: \'%c\'. Character Ignored. \n", $1->linenum, $1->tokenstr[0]);
  }
| WARNCHAR{
  char * temp8 = strdup($1->tokenstr);
  printf("WARNING(%d): ", $1->linenum);
  printf("character is %d characters long and not a single character", strlen(temp8)-2);
  printf(": '%s'. ", temp8);
  printf("The first char will be used.");

  printf("\n");
  //printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", $1->linenum, temp8[1],temp8);
  temp8 = NULL;
  }
;

%%

//// any functions for main here

int main(int argc, char *argv[]) 
{
  yydebug = 0;
  int fileCount=1;
  int c;
  //int debug;
  int print =0;
  int symbolDebug = 0;
  int printAnnotatedSyntaxTree = 0;
  int hh = 0;
  int line = 1;
  int Mem = 0;
  int noOp = 1;//1 for no op
  extern int numErrors;
  //  int numErrors;
  extern int numWarnings;
  extern int optind;
  extern int Scope;
  extern char *optarg;
  extern SymbolTable *symbolTable;
  while(1){
    while((c = ourGetopt(argc, argv, (char *) "dDpPhM"))!= EOF){
      switch(c){
      case 'p'://if we have -p in the command line
	//printf("read!");
	print=1;
	noOp = 0;
	break;
      case 'd':
	yydebug = 1;
	noOp = 0;
	break;
      case 'D':
	noOp = 0;
	symbolDebug = 1;
	break;
      case 'h':
	noOp = 0;
	hh = 1;
	break;
      case 'M':
	noOp = 0;
	Mem =1;
	break;
      case 'P':
	noOp =0;
	printAnnotatedSyntaxTree = 1;
	break;

      }
    }

    fileCount++;
    //printf("optind %d\n", optind);
  if(optind < argc){

    if(yyin = fopen(argv[optind], "r")){

      //file open successfully                                                                                                                                                                           
      char * name = argv[optind];
      int len = strlen(name);
      //printf("len %d\n", len);
      int i;
      int end = len-2;
      for(i=0; i<len; i++){
	if (i == end){
	  name[i] = 't';
	}
	if(i == end +1){

	  name[i]='m';
	}

      }
      //      printf("name for file %s\n", name);
      code=fopen(name, "w");
      //printf("opened success\n");     
      break;
    }
    else{
      //fail to open file                                                                                                                                                                             
      printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", argv[optind]);
      numErrors++;
      printf("Number of warnings: %d\n", numWarnings);
      printf("Number of errors: %d\n", numErrors);
      exit(1);
    }

     optind++;
  }else{
    //printf("la");
    break;
  }


  }
  /*
  ////  some of your stuff here
  if(argc>1){

    if(yyin = fopen(argv[1], "r")){
      if(print == 1)
	printTree(syntaxTree);	    
  //file open successfully
	}
	else{
	  //fail to open file
	  printf("Error: failed to open \'%s\'\n", argv[1]);
	  exit(1);
	}
  }
  /*
  if(argc > 2){

    if(yyin=fopen(argv[optind], "r")){

      yyparse();
      if(print==1){
	printTree(syntaxTree);
      }
    }

  }
 //  yydebug = 1;
  //  yyparse();
  */
  //  yydebug=1;
  //printf("print flag %d\n", print);
  /*
  if(hh = 1){
    printf("Usage: c- [options] [sourceFile]\n");
    printf("options:\n");
    printf("-d      - turn on parser debugging\n");
    printf("-D      - turn on symbol table debugging\n");
    printf("-h      - this usage message\n");
    printf("-p      - print the abstract syntax tree\n");
    printf("-P      - print the abstract syntax tree plus type information\n");
  }
  */
  initErrorProcessing();
  yyparse();

  astNode *IOTree=genIO();
  //  code=fopen("testfile.tm", "w");
  if (numErrors==0) {
    // -p, false stands for no annotation
    if (print==1) printTree(syntaxTree, false, false); // only types in declarations

    //SymbolTable *symbolTable = new SymbolTable();

   
    //-D

    if (symbolDebug == 1){
      symbolTable->debug(true);
    }
    bool insertIO=false;
    //astNode *IOTree=genIO();
    //printTree(IOTree, false);
        semanticAnalysis(syntaxTree, symbolTable, IOTree, true);   // semantic analysis (may have errors)
    hindSight(syntaxTree, symbolTable);
    if(symbolTable->lookupGlobal("main") == NULL){

      //      printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n");                                                                                                                                       
      //numErrors++;                                                                                                                                                                                
    }
    // -P
   
    if (printAnnotatedSyntaxTree==1) {

      //       semanticAnalysis(syntaxTree, symbolTable);
      if(numErrors ==0) 
	printTree(syntaxTree, true, false);
      

      // printTree(syntaxTree, true);
    }

    if(Mem == 1){

      if(numErrors ==0){
	printTree(syntaxTree, true, true);
	printf("Offset for end of global space: %d\n", goffset);

      }
    }
      //      printTree(syntaxTree, true);  // all types
    // code generation will go here

      }

  if(noOp == 1){
    codeGen(syntaxTree, IOTree);


  }
  char * cc;
  cc = strdup("Drop a comment\n");
  //  fprintf(code, "* %s\n", cc);

  //        printTree(syntaxTree, true);
  // report the number of errors and warnings
  printf("Number of warnings: %d\n", numWarnings);
  printf("Number of errors: %d\n", numErrors);
  //printf("print flag %d\n", print);

  return 0;
}

