#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "emitcode.h"
#include "codegen.h"
#include "semantic.h"
#include "util.h"
#include "parser.tab.h"

extern int litLoc;
extern int emitLoc;
extern int goffset;
int Main = 0;

extern SymbolTable * symbolTable;
funcList * funcs = (funcList *) malloc(sizeof(funcList));
funcList * vars = (funcList *) malloc(sizeof(funcList));
int tOffset = 0;
/*
astNode * generateIOTree(){

  astNode * dummy;
  astNode *input;
  astNode *output;
  astNode *input2;
  astNode *output2;
  astNode *input3;
  astNode *output3;
  astNode *outnl;
  astNode *iDummy;
  astNode *bDummy;
  astNode *cDummy;

  dummy = (tokenData *) malloc (sizeof(tokenData));
  dummy = linenum = -1;

  input = addDecl(funcK, dummy);
  input ->expType = Int;

  }*/
void codeGenIO(astNode * io){

  //printf("in IO\n");
  while(io != NULL && io->id != NULL){
    //    printf("in IO loop\n");
    emitComment( (char *) " ");
    emitComment( (char *) "** ** ** ** ** ** ** **");
    emitComment( (char *) "FUNCTION", io->id);

    io->offset = emitSkip(0);

    emitRM( (char *) "ST", AC, -1, FP, (char *) "Store return address");

    
    if (strcmp(io->id,"outnl")==0){
      emitRO( (char *) "OUTNL", AC, AC, AC, (char *) "Output a new line");
    }
    else if( strcmp(io->id,"input")==0){
      emitRO( (char *) "IN", RT, RT, RT, (char *) "Grab int input");
    }
    else if( strcmp(io->id, "output")==0){
      emitRM( (char *) "LD", AC, -2, FP, (char *) "Load parameter");
      emitRO( (char *) "OUT", AC, AC, AC, (char *) "Output integer");
    }
    else if( strcmp(io->id, "inputb")==0){
      emitRO( (char *) "INB", RT, RT, RT, (char *) "Grab bool input");
    }
    else if( strcmp(io->id, "outputb")==0){
      emitRM( (char *) "LD", AC, -2, FP, (char *) "Load parameter");
      emitRO( (char *) "OUTB", AC, AC, AC, (char *) "Output bool");
    }
    else if( strcmp(io->id, "inputc")==0){
      emitRO( (char *) "INC", RT, RT, RT, (char *) "Grab char input");
    }
    else if( strcmp(io->id, "outputc")==0){
      emitRM( (char *) "LD", AC, -2, FP, (char *) "Load parameter");
      emitRO( (char *) "OUTC", AC, AC, AC, (char *) "Output char");
    }
    else{
      emitComment( (char *) "ERROR io tree not recognized");
    }
   
    emitRM( (char *) "LD", AC, -1, FP, (char *) "Load return address");
    emitRM( (char *) "LD", FP, 0, FP, (char *) "Adjust fp");
    emitGoto(0, AC, (char *) "Return");
    emitComment( (char *) "END FUNCTION", io->id);
    io = io->sibling;

  }
  emitComment ( (char *) " ** ** ** ** ** ** **");

}

void mainLoop(astNode * tree){

  while(tree != NULL){

    switch(tree->nodekind){
    case declK:
      declGen(tree);
      break;
    case stmtK:
      stmtGen(tree);
      break;
    case expK:
      expGen(tree);
      break;
    default:
      printf("something weird here\n");
      break;

    }
    tree= tree->sibling;

  }


}



void codeGen( astNode * tree, astNode *io){
  //node1 for parsing, node2 for IO

  int Init;
  Init = emitSkip(1);
  codeGenIO(io);
  mainLoop(tree);
  backPatchAJumpToHere((char *) "JMP", PC, Init, (char *) "Jump to init [backpatch]");

  emitComment( (char *) "INIT");
  //emitRM( (char *) "LD", 0, 0, 0, (char *) "Set the Global Pointer");
  emitRM( (char *) "LDA", FP, goffset, GP, (char *)"set first frame at end of global");
  emitRM( (char *) "ST", FP, 0, FP, (char *) "Store old fp (point to self)");

  emitComment( (char *) "INIT GLOBALS AND STATICS");
  //////////////////////////////////////
  symbolTable->applyToAllGlobal(codeGenG);
  emitComment( (char *) "END INIT GLOBALS AND STATICS");
  emitRM((char *)"LDA", AC, 1, PC, (char *)"Return address in ac");
  emitRMAbs((char*)"JMP", PC, Main, (char *)"Jump to main");
  emitRO((char *)"HALT", 0, 0, 0, (char *)"DONE!");
  emitComment((char *)"END INIT");
}

void declGen(astNode * tree){

  switch(tree->subkind.decl){

  case varK:
    if(tree-> isArray == true){
      emitRM( (char*) "LDC", AC, tree->size -1, AC, (char*) "Load size of array", tree->id);
      //      printf("%d\n", tree->offset);
      emitRM( (char*) "ST", AC, tree->offset, FP, (char *) "Saving size of array", tree->id);
      tOffset += tree->offset;

    }else{
      if(tree->child[0] != NULL){
	tOffset--;
	mainLoop(tree->child[0]);
	emitRM( (char*) "ST",AC, tree->offset, GP, (char *) "Store Global vars", tree->id);

      }else{
	tOffset --;
      }
    }
    break;
  case funcK:
    countSize(tree);
    tOffset = (-1) * (tree->size);
    emitComment( (char*)"FUNCTION", tree->id);
    if( !strcmp(tree->id, "main")){

	Main = emitSkip(0);
      }

      tree->memLoc = emitSkip(0);
      emitRM( (char*) "ST", AC, -1, FP, (char *) "Store return address");
      mainLoop(tree->child[0]);
      mainLoop(tree->child[1]);

      emitComment( (char *) "Add standard closing in case there is no return statement");
      emitRM( (char *) "LDC", RT, 0, AC3, (char *) "set return value to 0");
      emitRM( (char *) "LD", AC, -1, FP, (char *) "Load return address");
      emitRM( (char *) "LD", FP, 0, FP, (char *) "adjust fp");
      emitRM( (char *) "LDA", PC, 0, AC, (char *) "return");

      emitComment( (char *) "END FUNCTION", tree->id);

      if(funcs->id == NULL){

	funcs->id = tree->id;
	funcs->memLocation = tree->memLoc;

      }else{

	funcList * temp = (funcList *)malloc(sizeof(funcList));
	temp = funcs;
	funcList * temp2 = (funcList*) malloc(sizeof(funcList));
	temp2->id=strdup(tree->id);
	temp2->memLocation=tree->memLoc;
	while(temp->next != NULL){

	  temp=temp->next;
	}
	temp->next = temp2;
      }
    break;
    case paramK:
      break;
    default:
      printf("Code gen decl \n");
      break;
  }

}

void expGen(astNode * tree){

  funcList *temp1 = (funcList *)malloc(sizeof(funcList));
  funcList *temp2 = (funcList *)malloc(sizeof(funcList));

  switch(tree->subkind.exp){
  case InitK:
    if(tree->child[0]!= NULL){
      mainLoop(tree->child[0]);
    }
    break;

  case idK:
    if(tree->child[0] != NULL){

      expGen(tree->child[0]);
      if( tree->scope != 0){
	if(tree->subkind.decl == paramK){
	  emitRM( (char *) "LD", AC3, tree->offset, FP, (char *) "idk param array");
	}else{
	  emitRM( (char *) "LDA", AC3, tree->offset, FP, (char *) "idk local node array");
	}

      }else if(tree->scope ==0){
	emitRM( (char *) "LDA", AC3, tree->offset, GP, (char *) "idk global node");
      }

      emitRO( (char *) "SUB", AC3, AC3, AC, (char *) "offset value count");
      emitRM( (char *) "LD", AC, 0, AC3,(char *) "Load array element");
    }else if( tree->isArray == 1){

      if(tree->scope != 0){

	if( tree->subkind.decl == paramK){
	  emitRM( (char *) "LD", AC3, tree->offset, FP, (char *) "Load address of base of array", tree->child[0]->id);
	}else{
	  //	  printf("%s", tree->child[0]->id);
	  if(tree->child[0]!=NULL)
	  emitRM( (char *) "LDA", AC3, tree->offset, FP, (char *) "Load address of base of array", tree->child[0]->id);
	}

      }	else if (tree->child[0]->scope ==0){
	emitRM( (char *) "LDA", AC3, tree->offset, GP, (char *) "Load address of base of array", tree->child[0]->id);
      }
    }else{
      if(tree->scope == 0){
	if(tree->child[0]!= NULL)
	emitRM( (char *) "LD", AC, tree->offset, GP, (char *) "load from lhs variable ", tree->child[0]->id);
	else
	  emitRM( (char *) "LD", AC, tree->offset, GP, (char *) "load from idk ");
      }else if( tree->scope != 0){
	if(tree->child[0]!= NULL)
	emitRM( (char *) "LD", AC, tree->offset, FP, (char *) "load from lhs variable ", tree->child[0]->id);
	else
	  emitRM( (char *) "LD", AC, tree->offset, GP, (char *) "load from idk ");
      }else{


      }
      /////////////////////////////////
    }
    break;
  case CallK:{
    emitComment( (char *) "EXPRESSION");
    emitComment( (char *) "CALL", tree->id);
    emitRM( (char *) "ST", FP, tOffset, FP, (char *) "Store fp");

    int pretOffset = tOffset;
    tOffset --;
    tOffset --;
    if(tree->child[0] != NULL){

      astNode * temp = (astNode *) malloc (sizeof (astNode));
      temp = tree->child[0];

      while( temp != NULL){

	emitComment( (char *) "Param");
	mainLoop(temp);
	emitRM( (char *) "ST", AC, tOffset, FP, (char *) "Push parameter");
	char * temp2 = tree->id;
	emitComment( (char *)"Param end", temp2);
	temp= temp->sibling;
	tOffset--;
      }
    }

    emitComment( (char *) "Param end", tree->id);
    emitRM( (char *) "LDA", FP, pretOffset, FP, (char *) "Load address of new frame");
    emitRM( ( char *) "LDA", AC, 1, PC, (char *) "Return address in AC");

    temp1 = funcs;


    while( temp1 != NULL && temp1->id != NULL){

      if( !strcmp(tree->id, "This")){
	////////////////////////////////
	tree->memLoc = temp1->memLocation;

      }
      temp1 = temp1->next;
    }
    emitRMAbs( (char *) "JMP", PC, tree->memLoc, (char *)"CALL", tree->id);
    emitRM( (char *)"LDA", AC, 0, RT, (char *) "Save in ab");
    emitComment( (char *)"CALL END", tree->id);
    tOffset= pretOffset;
  }
    break;
  case opK:
    mainLoop(tree->child[0]);
    if(tree->value == 1){

    }else{
      emitRM( (char *) "ST", AC, tOffset--, FP, (char*)"OPK 1");
    }
    if(tree->child[1] != NULL){
      mainLoop(tree->child[1]);
    }
    emitRM( (char *)"LD", AC, ++tOffset, FP, (char *) "Load address of base of array", tree->id);

    if(tree->attr.op == PLUSOp){
        emitRO( (char *) "ADD", AC, AC1, AC, (char *) "OP");
    }
    if(tree->attr.op == MULOp){
        if(tree->child[0] != NULL){
	if(tree->child[0]->scope != 0){

	  if(tree->child[0]->subkind.decl == paramK){
	    emitRM( (char *)"LD", AC3, tree->child[0]->offset, FP, (char *)"param idk");
	  }else{
	    emitRM( (char *)"LDA", AC3, tree->child[0]->offset, FP, (char *)"local array idk");
	  }


	}else if (tree->child[0]->scope==0){
	  emitRM( (char*)"LDA", AC3, tree->child[0]->offset, FP, (char*) "local array idk");
	  }
	  emitRM( (char *) "LD", AC, 1, AC, (char *)"global array idk");
	}else{
	  emitRO( (char*) "MUL", AC, AC1, AC, (char *)"Op *");
	}

    }
    if(tree->attr.op == MinusOp){
        if(tree->child[1] == NULL){

	emitRM( (char *)"LDC", AC1, 0, AC1, (char *) "LOADING MINUS");
	emitRO( (char *)"SUB", AC, AC1, AC, (char *) "Op -");
        }else{
  	emitRO( (char *)"SUB", AC, AC1, AC, (char *)"Op -");
        }
    }
    if(tree->attr.op == DIVOp){
	emitRO( (char *)"DIV", AC, AC1, AC, (char *)"Op /");
    }
    if(tree->attr.op == EQOp){
	emitRO((char *)"TEQ", AC, AC1,AC, (char *)"Op :=");
    }
    if(tree->attr.op == ANDOp){
	emitRO((char *)"AND", AC, AC1,AC, (char *)"Op and");
    }
    if(tree->attr.op == OROp){
	emitRO((char *)"OR", AC, AC1,AC, (char *)"Op or");
    }
    if(tree->attr.op == LESSEQOp){
	emitRO((char *)"TLE", AC, AC1,AC, (char *)"Op <=");
    }
    if(tree->attr.op == LESSOp){
	emitRO((char *)"TLT", AC, AC1,AC, (char *)"Op <");
    }
    if(tree->attr.op == GREATEQOp){
	emitRO((char *)"TGE", AC, AC1,AC, (char *)"Op >=");
    }
    if(tree->attr.op == GREATOp){
	emitRO((char *)"TGT", AC, AC1,AC, (char *)"Op >");
    }
    if(tree->attr.op == PEROp){
	emitRO((char *)"DIV", AC2, AC1,AC, (char *)"Op %");
	emitRO((char *)"MUL", AC2, AC1, AC2, (char *)"");
	emitRO((char *)"SUB", AC, AC1, AC2, (char *)"");
    }
    if(tree->attr.op == NOTOp){
	mainLoop(tree->child[0]);
	emitRM( (char *) "LDC", AC1, 1, AC1, (char *) "");
	emitRO( (char *) "XOR", AC, AC, AC1, (char *) "OP |");
    }
    if(tree->attr.op == leftSquareBracOp){

	if(tree->child[0] != NULL){
	  expGen(tree->child[0]);
	  emitRO( (char *) "SUB", AC3, AC3, AC, (char *) "Compute offset of value");
	  emitRM( (char *) "LD", AC, 0, AC3, (char *) "Load array element");
	}else if(tree->isArray == true){
	  ////
          if( tree->child[0]->scope != 0){
              if(tree->child[0]->subkind.decl == paramK){
                emitRM( (char *)"LD", AC3, tree->child[0]->offset, FP, (char *)"Load address of base of array", tree->child[0]->id);
              }else{

                emitRM( (char *)"LDA", AC3, tree->child[0]->offset, FP, (char *)"Load address of base of array", tree->child[0]->id);
              }
            }
	  else if (tree->child[0]->scope ==0){
	    emitRM( (char *) "LDA", AC3, tree->child[0]->offset, GP, (char *) "Load address of base of array", tree->child[0]->id);
	      }

	}
	else{
	 
	  if(tree->scope ==0){
	    emitRM( (char *)"IP", AC, tree->offset, GP, (char *)"Load from lbrack");
	  }
	  if(tree->scope !=0){
	    emitRM( (char *)"LD", AC, tree->offset, FP, (char *)"Load from lbrack");
	  } 

	}	  	
    }

    break;

    
    case boolConstK:
    emitComment( (char *) "EXPRESSION");
    emitRM( (char *) "LDC", AC, tree->attr.boolVal, AC3, (char *)"Load Boolean constant");    
    break;

    case ConstantK:
      emitComment( (char *) "EXPRESSION");

      if(tree->isBool == 1){
	emitRM( (char *) "LDC", AC,tree->attr.boolVal, AC3, (char *) "Load boolean cons");
      }else{

	emitRM( (char *) "LDC", AC, tree->nvalue, AC3, (char *) "Load integer constant");
      }
    break;

    case charConstantK:
      emitComment( (char *) "EXPRESSION");
      char tempc;
      tempc = tree->attr.cvalue[1];
      int tempn;
      tempn = (int) tempc;
      emitRM( (char *) "LDC", AC, tempc, AC3, (char *) "Load char constant");
      break;    
    case AssignK:
      emitComment((char*)"EXPRESSION AssignK");
      if(tree->child[0]->subkind.exp == idK && tree->child[0]->child[0] != NULL){
	expGen(tree->child[0]->child[0]);
	emitRM( (char *)"ST", AC, tOffset--, FP, (char *)"Push index");
      }

      if(tree->attr.op == EQOp){
	expGen(tree->child[1]);
      }
      if(tree->attr.op = INCOp){
	mainLoop(tree->child[0]);
      }
      /*
      if(tree->child[0]->scope ==0){

	emitRM( (char *) "ST", AC, tree->child[0]->offset, GP, (char *)"Store variable", tree->child[0]->id);
      }else{
	emitRM( (char *) "ST", AC, tree->child[0]->offset, FP, (char *)"Store variable", tree->child[0]->id);
      }
      */
      if(tree->attr.op == DECOp){
	emitRM( (char *) "LDA", AC, -1, AC, (char *) "decrement value of", tree->child[0]->id);
      }
      if(tree->attr.op == INCOp){
	emitRM( (char *) "LDA", AC, 1, AC, (char *) "increment value of", tree->child[0]->id);
      }
      if(tree->attr.op == ADDASSOp){
	emitRO((char *) "ADD",AC, AC, AC1, (char *) "OP +=");
      }
      if(tree->attr.op == SUBASSOp){
	emitRO((char *) "SUB", AC, AC, AC1, (char *) "OP -=");
      }
      if(tree->attr.op == MULASSOp){
	emitRO((char *) "MUL", AC, AC, AC1, (char *) "OP *=");
      }
      if(tree->attr.op == DIVASSOp){
	emitRO( (char *) "DIV", AC, AC, AC1, (char *) "OP /=");
      }
      if(tree->attr.op == ASSOp){


      }

      if(tree->child[0]->scope ==0){

        emitRM( (char *) "ST", AC, tree->child[0]->offset, GP, (char *)"Store variable", tree->child[0]->id);
      }else{
        emitRM( (char *) "ST", AC, tree->child[0]->offset, FP, (char *)"Store variable", tree->child[0]->id);
      }
      if( tree->child[0]->subkind.exp == idK && tree->child[0] ->child[0] != NULL){

	emitRM( (char *) "LD", AC1, tOffset +1, FP, (char *) "Array index");

	if( tree->child[0]->scope != 0){

	    if(tree->child[0]->subkind.decl == paramK){
	      emitRM( (char *) "LD", AC3, tree->child[0]->offset, FP, (char *) "Local address of base of array", tree->child[0]->id);
	    }else{
	      emitRM( (char *) "LDA", AC3, tree->child[0]->offset, FP, (char *) "Load address of base of arrat", tree->child[0]->id);
	    }

	  }else{
	    if(tree->child[0]->scope ==0){
		emitRM( (char *) "LDA", AC3, tree->child[0]->offset, GP, (char *) "Load address of base of array", tree->child[0]->id);
	      }
	  }
	 
      }
    break;
}

}

void stmtGen(astNode * tree){
 int temp1;
  int temp2;
  int temp3;
  int currentLoc, breakLoc, skipLoc;

  switch(tree->subkind.stmt){

  case WhileK:
    emitComment( (char *) "WHILE");
    currentLoc = emitSkip(0);
    expGen(tree->child[0]);

    emitRM( (char *) "JNZ", AC, 1, PC, (char *) "Jumping to while");
    emitComment ( (char *) "DO");
    skipLoc = breakLoc;
    breakLoc= emitSkip(1);
    mainLoop(tree->child[1]);
    emitGotoAbs(currentLoc, (char*) "go to beginning of loop");

    /////
    backPatchAJumpToHere(breakLoc, (char *) "Jump past loop [backpatch]");
    breakLoc = skipLoc;
    emitComment((char*)"END WHILE");
    break;
  case IfK:
    emitComment( (char*) "If");
    ////////////
    break;
  case CompoundK:
    emitComment((char*)"COMPOUND");
    tree->size = tOffset;
    emitComment( (char *)"TOFF set:", tOffset);
    mainLoop(tree->child[0]);
    emitComment((char*)"Compound body");
    mainLoop(tree->child[1]);
    emitComment( (char *)"TOFF set:", tOffset);
    tOffset = tree->size;
    emitComment((char*)"END COMPOUND");
    break;
  case ReturnK:
    emitComment((char*)"RETURN");
    if(tree->child[0] != NULL){
      mainLoop(tree->child[0]);
    }
    emitRM( (char *)"LD", AC, -1, FP, (char *)"Load return address");
    emitRM( (char *)"LD", FP, 0, FP, (char *) "Adjust FP");
    emitRM( (char *)"JMP", PC, 0, AC, (char *) "Return");
    break;
  case IterK:
    mainLoop(tree->child[0]);
    mainLoop(tree->child[1]);
    mainLoop(tree->child[2]);
    break;
  case BreakK:
    emitGotoAbs(breakLoc, (char *)"Break me!");
    break;
  }

}




void codeGenG(std:: string str1, void * ptr){

  astNode * temp1 = (astNode *) malloc(sizeof(astNode));
  temp1 = static_cast<astNode *> (ptr);

  if (temp1 == NULL || temp1 ->ioTree == 1 || temp1 -> subkind.decl == funcK){
    return;
  }

  if(temp1 ->isArray == 1){

    if(temp1 ->scope ==0){

	emitRM( (char *) "LDC", AC, temp1 ->size, AC, (char *) "load size", temp1->id);
	emitRM( (char *) "ST", AC, temp1 ->offset +1, GP, (char *) "Save size of global array", temp1 ->id);
      }
  }
  else if(temp1->child[0] != NULL){

    mainLoop(temp1 ->child[0]);
    emitRM( (char *) "ST", AC, temp1 ->offset, GP, (char *)"Store Global variable", temp1 ->id);
  }
      
}

void countSize(astNode * tree){
  int i = 0;
  astNode * p = (astNode *)malloc(sizeof(astNode *));
  p = tree;
  if(tree->child[0] != NULL){
    p = p->child[0];
    i++;
    while(p->sibling != NULL){

      i++;
      p=p->sibling;

    }
  }
  tree->size = i+2;

}
