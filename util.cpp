#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "scanType.h"
#include "util.h"
#include "parser.tab.h"
#include "semantic.h"

extern int numErrors;

astNode * addDecl(declKind kind, TokenData * token){
  astNode * t = (astNode *) malloc(sizeof(astNode));
  int i;
  for( i = 0; i<MAXCHILDREN; i++){
    t->child[i] = NULL;
  }
  t->sibling = NULL;
  t-> nodekind = declK;
  t->subkind.decl = kind;
  t->lineno = token->linenum;
  t->id = token -> tokenstr;
  //  t->attr.name = token -> tokenstr;
  
  t->attr.stringConsto = token -> tokenstr;
  //t->isInitializer = false;
  t->isBool = 0;
  t->expType = Void;
  return t;
};

astNode * addStmt(stmtKind kind, TokenData * token){
  astNode * t = (astNode *) malloc(sizeof(astNode));
  int i;
  for( i = 0; i<MAXCHILDREN; i++){
    t->child[i] = NULL;
  }
  t->sibling = NULL;
  t->nodekind = stmtK;
  t->id = token -> tokenstr;
  //t->attr.name = token -> tokenstr;
  t->subkind.stmt= kind;
  t->attr.stringConsto = token -> tokenstr;
  //t->isInitializer = false;
  t->lineno= token->linenum;
  t->isBool = 0;
  return t;
};


astNode * addExp(expKind kind, TokenData * token){
  astNode * t = (astNode *) malloc(sizeof(astNode));
  int i;
  for( i = 0; i<MAXCHILDREN; i++){
    t->child[i] = NULL;
  }
  t->sibling = NULL;
  t->nodekind = expK;
  t->subkind.exp=kind;
  t->attr.stringConsto = token->tokenstr;
  t->lineno = token->linenum;
  t->attr.cvalue = token->tokenstr;
  //t->isInitializer = false;
  //t->attr.name = token->tokenstr;
  t->id = token->tokenstr; 
  //printf("In the addExp attrname %s XXXXXXXXXX\n", t->attr.name);
 //  t->attr.id = token->tokenstr;
 
  //printf("Preparing to set up op\n");
  //  t->attr.op = setOp(token);
  //printExp(t);
  //printf("finishi problem op\n");
  t->isBool = 0;
  t->expType = Undefined;
  //printf("finishi the function\n");
  return t;
}; 
/*
void setType(TreeNode *t, ExpType type, bool isStatic)
{
  while (t) {
    t->expType = type;
    t->isStatic = isStatic;

    t = t->sibling;
  }
}
*/
astNode *addSibling(astNode *t, astNode *s)
{
  if (s==NULL && numErrors ==0) {
    printf("ERROR(SYSTEM): never add a NULL to a sibling list.\n");
    exit(1);
  }
  if (t!=NULL) { 
    astNode *tmp;
    tmp = t;
    while (tmp->sibling!=NULL) tmp = tmp->sibling;
    tmp->sibling = s; 
    return t;
  }
  return s;
}
char * convRef(RefType ref){
  char * type = strdup("undefined ref type");
  if(ref == Global)
    type = strdup("Global");
  if(ref == Local)
    type = strdup("Local");
  if(ref == None)
    type = strdup("None");
  if(ref == LocalStatic)
    type = strdup("LocalStatic");
  if(ref == Parameter)
    type = strdup("Parameter");
  return type;


}
void printStmt(astNode * node, bool M){
  //  printf("getting to stmt lala al\n");
  switch(node-> subkind.stmt){
  case ElifK:
    printf("Elsif [line: %d]\n", node->lineno);
    break;
  case IfK:
    printf("If [line: %d]\n", node->lineno);
    break;  
  case WhileK:
    printf("While [line: %d]\n", node->lineno);
    break;
  case CompoundK:
    if(M!= true){
    printf("Compound [line: %d]\n", node->lineno);
    }
    else{
      printf("Compound ");
      printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
      printf("[line: %d]\n", node->lineno);
    }
    break;
  case IterK:
    if(M != true){
      printf("For [line: %d]\n", node->lineno);
    }
    else{
      printf("For [mem: %s loc: %d size: %d] [line: %d]\n", convRef(node->refType), node->memLoc, node->size, node->lineno);

    }
    break;
  case ReturnK:
    printf("Return [line: %d]\n", node->lineno);
    break;
  case BreakK:
    printf("Break [line: %d]\n", node->lineno);
    break;
  case rangeK:
    printf("Range [line: %d]\n", node->lineno);
    break;
  default:
    printf("Unknown stmt\n");
    break;
    
  }
}

void printExp(astNode * node, bool state, bool M){
  char * printStuff;
  
  switch(node->attr.op){
  case leftSquareBracOp:
     printStuff = strdup("[");
    //    * printStuff = "[";
    break;
  case rightSquareBracOp:
    printStuff = strdup("]");
    break;
  case PLUSOp:
    printStuff =strdup("+");
    break;
  case EQOp:
    printStuff =strdup("==");
    break;
  case ASSOp:
    printStuff =strdup(":=");
    break;
  case ASSOp2:
    printStuff =strdup("=");
    break;
  case ANDOp:
    printStuff =strdup("and");
    break;
  case MULOp:
    printStuff =strdup("*");
    break;
  case MinusOp:
    printStuff =strdup("-");
    break;
  case DIVOp:
    printStuff =strdup("/");
    break;
  case INCOp:
    printStuff =strdup("++");
    break;
  case DECOp:
    printStuff =strdup("--");
    break;
  case OROp:
    printStuff =strdup("or");
    break;
  case NOTOp:
    printStuff =strdup("not");
    break;
  case NOTEQOp:
    printStuff =strdup("!=");
    break;
  case ADDASSOp:
    printStuff =strdup("+=");
    break;
  case SUBASSOp:
    printStuff =strdup("-=");
    break;
  case MULASSOp:
    printStuff =strdup("*=");
    break;
  case DIVASSOp:
    printStuff =strdup("/=");
    break;
  case NEQOp:
    printStuff = strdup("><");
    break;
  case LESSEQOp:
    printStuff =strdup("<=");
    break;
  case GREATEQOp:
    printStuff =strdup(">=");
    break;
  case GREATOp:
    printStuff =strdup(">");
    break;
  case LESSOp:
    printStuff =strdup("<");
    break;
  case PEROp:
    printStuff =strdup("%");
    break;
  case chsignOp:
    printStuff = strdup("chsign");
    break;
  case sizeofOp:
    printStuff = strdup("sizeof");
    break;
  case QUEOp:
    printStuff =strdup("?");
    break;
  case SEMIOp:
    printStuff = strdup(";");
     break;
  default:
    printStuff = strdup("Unknown");
  break;
  }

  switch(node->subkind.exp){
  case opK:
    //    printf("It's case op\n");
    printf("Op: %s ", printStuff);
    if(state == true){
      printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
    }else{
      printf("[line: %d]\n", node->lineno);
    }

    break;
  case ConstantK:
    if(node->isBool ==1){

      if(node -> attr.boolVal == 1){
      printf("Const false ");
      if(state == true){
	printf("of type bool [line: %d]\n", node ->lineno);
      }else{
	printf("[line: %d]\n", node->lineno);
      }

      }
      if(node-> attr.boolVal != 1){
	printf("Const true ");
	if(state == true){
	  printf("of type bool [line: %d]\n", node ->lineno);
	}else{
	  printf("[line: %d]\n", node->lineno);
	}

      }

    }
    else{
      printf("Const %d ", node->nvalue);
      if(state == true){
        printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }else{
        printf("[line: %d]\n", node->lineno);
      }


    }

    break;
  case idK:
    printf("Id: %s ", node->id);
    if(state == true){
      if(M!= true){
	if(node->expType == Undefined){
	  printf("of %s [line: %d]\n", convExpType(node->expType),node ->lineno);
	}else{
	  printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
	} 
     }
      else{

	if(node->expType == Undefined){
	  printf("of %s ", convExpType(node->expType));
	  printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	  printf("[line: %d]\n", node->lineno);
	}else{
	  if(node->isStatic != true){
	  printf("of type %s ", convExpType(node->expType));
	  }
	  else{
	    printf("of static type %s ", convExpType(node->expType));
	  }
	  printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	  printf("[line: %d]\n", node->lineno);
	}
      }

    }else{
      printf("[line: %d]\n", node->lineno);
    }


    break;
  case InitK:
    printf("Init: %s ", node->id);
    if(state == true){

      if(node->expType == Undefined){
        printf("of %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }else{
	printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }
    }else{
      printf("[line: %d]\n", node->lineno);
    }

  case AssignK:
    //    printf("case assgin\n");
    printf("Assign: %s ", printStuff);
    if(state == true){
      printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
    }else{
      printf("[line: %d]\n", node->lineno);
    }
    break;
  case CallK:
    printf("Call: %s ", node->id);
    if(state == true){



      printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
    }else{
      printf("[line: %d]\n", node->lineno);
    }
    break;
  case charConstantK:
    //printf("I'm trying to print\n");
    if(node->attr.cvalue[1]== '\\' && node -> attr.cvalue[2] == 'n'){
      printf("Const \'\n\' ");
      if(state == true){
	printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }else{
	printf("[line: %d]\n", node->lineno);
      }
    }
    else if(node -> attr.cvalue[1]=='\\' && node-> attr.cvalue[2] == '0'){
      printf("Const \' \' ");
      if(state == true){
	printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }else{
	printf("[line: %d]\n", node->lineno);
      }
    }
    else if(node->attr.cvalue[1] == '\\'){
      printf("Const \'%c\' ", node->attr.cvalue[2]);
      if(state == true){
	printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }else{
	printf("[line: %d]\n", node->lineno);
      }
    }
    else{
      printf("Const \'%c\' ", node->attr.cvalue[1]);
      if(state == true){
	printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }else{
	printf("[line: %d]\n", node->lineno);
      }
    }
    break;
  case stringConstantK:
    printf("Const %s ", node->id);
    if(state == true){

      if(M!= true){
      printf("of type %s [line: %d]\n", convExpType(node->expType),node ->lineno);
      }
      else{
	printf("of array of type %s ", convExpType(node->expType));
	printf("[mem: %s loc: %d size: %d]", convRef(node->refType), node->memLoc, node->size);
	printf(" [line: %d]\n", node->lineno);

      }
    }else{
      printf("[line: %d]\n", node->lineno);
    }
    break;
  default:
    printf("Unknown exp\n");
    break;
  }

}
void printDecl(astNode * node, bool state, bool M){
  ExpType expressionType;
  const char * temp = "la";
  switch(node->expType){
  case Void:
    temp = "type void";
    break;
  case Int:
    temp = "type int";
    break;
  case Bool:
    temp="type bool";
    break;
  case Char:
    temp="type char";
    break;
  case Undefined:
    temp = "type undefined";
    break;
  default:
    break;    
  }

  expressionType = node->expType;

  switch (node -> subkind.decl){

  case varK:
  if(node->isArray == 1){
    if(temp == "type void"){

      if(state != true){
      printf("Var: %s of array undefined type [line: %d]\n", node->id, node->lineno);
      }else{

	if(M != true){
	printf("Var: %s of array of type undefined type [line: %d]\n", node->id, node->lineno);
	}
	else{
	  if(node->isStatic != true){
	  printf("Var: %s of array of type undefined type ", node->id);
	  }
	  else{
	    printf("Var: %s of static array of type undefined type ", node->id);
	  }
	  printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	  printf("[line: %d]\n", node->lineno);
	}
      }

    }else{
      if(node->isStatic == 1){
	if(state != true){
	printf("Var: %s of array of static %s [line: %d]\n", node->id, temp, node->lineno);
	}
	else{
	  if(M != true){
	  printf("Var: %s is array of %s [line: %d]\n", node->id,temp, node->lineno);
	  }
	  else{

	    printf("Var: %s of array of type undefined type ", node->id);
	    printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	    printf("[line: %d]\n", node->lineno);
	  }
	}
      }else{
	if(state != true){
      printf("Var: %s of array of %s [line: %d]\n", node->id, temp, node->lineno);
	}
	else{
	  if(M!= true){
	  printf("Var: %s is array of %s [line: %d]\n", node->id,temp, node->lineno);
	  }
	  else{

	    printf("Var: %s of array of type undefined type ", node->id);
	    printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	    printf("[line: %d]\n", node->lineno);
	  }
	}
      }
    }
  }
  else{
    if(temp == "type void"){
      printf("Var: %s is undefined type [line: %d]\n", node->id, node->lineno);
    }
    else{
      if(node->isStatic == 1){

	if(state != true){
	  printf("Var: %s of static %s [line: %d]\n", node->id, temp, node->lineno);}
	else{
	  if( M!= true){
	  printf("Var: %s of %s [line: %d]\n", node->id, temp, node->lineno);
	  }
	  else{
	    printf("Var: %s of static %s ", node->id, temp);
	    printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	    printf("[line: %d]\n", node->lineno);
	  }
	}
      }else{

	if(M!= true){
      printf("Var: %s of %s [line: %d]\n", node->id, temp, node->lineno);
	}
	else{
	  printf("Var: %s of %s ", node->id, temp);
	  printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	  printf("[line: %d]\n", node->lineno);
	}
      }
    }
  }
  break;
  case funcK:
    if(M != true){
    printf("Func: %s returns %s [line: %d]\n", node->id, temp, node->lineno);
    }
    else{
      printf("Func: %s returns %s ", node->id, temp);
      printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
      printf("[line: %d]\n", node->lineno);

    }
    break;
  case paramK:
    if(node-> isArray){
      if(state != true){
      printf("Parm: %s of array of %s [line: %d]\n", node->id, temp, node->lineno);
      }
      else{
	if(M != true){
	printf("Parm: %s is array of %s [line: %d]\n", node->id, temp, node->lineno);
	}
	else{
	  printf("Parm: %s of array of %s ", node->id, temp);
	  printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	  printf("[line: %d]\n", node->lineno);

	}
      }
    }
    else{
      if(M!= true){
      printf("Parm: %s of %s [line: %d]\n", node->id, temp, node->lineno);
      }
      else{
	printf("Parm: %s of %s ", node->id, temp);
	printf("[mem: %s loc: %d size: %d] ", convRef(node->refType), node->memLoc, node->size);
	printf("[line: %d]\n", node->lineno);

      }
    }
    break;
  default:
    break;
  }
}

static void printIndent(){
  int i;
  for (i=0; i<indent; i++){
    printf(". ", i);
  }
}

void printTree(astNode * tree, bool state, bool M){
  int i;
  int siblingCount = 1;
  //  printf("here1\n");
  if(tree == NULL){
    printf("TREE NULL\n");
  }

  while(tree != NULL){
    switch(tree->nodekind){
    case declK:
      //printf("case decl\n");
      printDecl(tree, state, M);
      break;
    case stmtK:
      // printf("case stmt\n");
      printStmt(tree, M);
      break;
    case expK:
      //printf("case exp\n");
      printExp(tree, state, M);
      break;
    default:
      printf("UNK nodekind\n");
      break;
    }

    for( i=0; i< MAXCHILDREN; i++){
      if(tree->child[i] !=NULL){
	indent++;
	printIndent();
	printf("Child: %d ", i);
	//     			printf("might be i\n");
			
	//			printf("%d\n", tree->child[i]->lineno);
	printTree(tree->child[i], state, M);
	//	printf("might not be i\n");
      }
      //  printf("this is i %d", i);
    }

    // printf("EMMM\n");
    tree = tree->sibling;
    if(tree != NULL){
      printIndent();
      printf("Sibling: %d ", siblingCount);
      // printf("might be sibling\n");
      siblingCount++;
    }
  }

  indent--;
}

ExpType setType(TokenData * token){

  //  printf("getting into set type function\n");
   switch(token->tokenclass){
   case CHAR:
     // printf("type char in function\n");
      return Char;
      break;
   case INT:
     //  printf("type int in function\n");
     return Int;
     break;
   case VOID:
     return Void;
     break;
   case BOOL:
     return Bool;
     break;
   default:
     return Undefined;
     break;  
   }
}

ExpType setNodeType(astNode * tree){

  switch(tree->expType){
  case Char:
    return Char;
  case Void:
    return Void;
  case Int:
    return Int;
  default:
    return Undefined;
  }
}

OpKind setOp(TokenData * token){

  switch(token->tokenclass){
  case leftSquareBrac:
    return leftSquareBracOp;
    break;
  case rightSquareBrac:
    return rightSquareBracOp;
    break;
  case PLUS:
    return PLUSOp;
    break;
  case EQ:
    return EQOp;
    break;
  case ASGN:
    return ASSOp;
    break;
  case ASS:
    return ASSOp2;
  case AND:
    return ANDOp;
    break;
  case MUL:
    return MULOp;
    break;
  case Minus:
    return MinusOp;
    break;
  case DIV:
    return DIVOp;
    break;
  case INC:
    return INCOp;
    break;
  case DEC:
    return DECOp;
    break;
  case OR:
    return OROp;
    break;
  case NEQ:
    return NEQOp;
    break;
  case NOT:
    return NOTOp;
    break;
  case NOTEQ:
    return NOTEQOp;
    break;
  case ADDASS:
    return ADDASSOp;
    break;
  case SUBASS:
    return SUBASSOp;
    break;
  case MULASS:
    return MULASSOp;
    break;
  case DIVASS:
    return DIVASSOp;
    break;
  case LEQ:
    return LESSEQOp;
    break;
  case GEQ:
    return GREATEQOp;
    break;
  case GREAT:
    return GREATOp;
    break;
  case LESS:
    return LESSOp;
    break;
  case PER:
    return PEROp;
    break;
  case QUE:
    return QUEOp;
    break;
  case SEMI:
    return SEMIOp;
  default:

    break;
  }


}


astNode *genIO(){

  TokenData * dummy;
  astNode * idummy;
  astNode * bdummy;
  astNode * cdummy;
  astNode * input;
  astNode * output;
  astNode * inputb;
  astNode * outputb;
  astNode * inputc;
  astNode * outputc;
  astNode * outnl;

  dummy =(TokenData *)malloc(sizeof(TokenData));
  dummy->linenum = -1;

  input = addDecl(funcK, dummy);
  input->expType = Int;
  input->id=strdup("input");

  inputb = addDecl(funcK, dummy);
  inputb->expType =Bool;
  inputb->id = strdup("inputb");

  input->sibling = inputb;
  inputc = addDecl(funcK, dummy);
  inputc->expType=Char;
  inputc->id = strdup("inputc");
  inputb->sibling=inputc;


  output = addDecl(funcK, dummy);
  output->expType=Void;
  output->id=strdup("output");

  idummy = addDecl(paramK, dummy);
  idummy->expType = Int;
  idummy->id = strdup("dummy");
  output->child[0] =idummy;
  inputc->sibling = output;

  outputb = addDecl(funcK, dummy);
  outputb->expType = Void;
  outputb->id = strdup("outputb");
  bdummy = addDecl(paramK, dummy);
  bdummy->expType = Bool;
  bdummy->id = strdup("dummy");
  outputb->child[0]=bdummy;
  output->sibling = outputb;

  outputc = addDecl(funcK, dummy);
  outputc->expType = Void;
  outputc->id = strdup("outputc");

  cdummy = addDecl(paramK, dummy);
  cdummy->id = strdup("dummy");
  cdummy->expType=Char;
  outputc->child[0] = cdummy;
  outputb->sibling = outputc;

  outnl= addDecl(funcK, dummy);
  outnl->expType =Void;
  outnl->id = strdup("outnl");
  outputc->sibling = outnl;

  return input;

}

