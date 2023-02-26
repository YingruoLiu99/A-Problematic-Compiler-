#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanType.h"
#include "parser.tab.h"
#include "semantic.h"
#include "util.h"
#include "symbolTable.h"
//#include "cgen.h"
//#include "code.h"

int numErrors =0;
int numWarnings=0;
int Scope=0;
int foffset = -1;
//int prefoffset;
int goffset = 0;
int sizeL = 0;
int loopFlag = 0;//iteration loops                                                                                                                                                                       

SymbolTable *symbolTable = new SymbolTable();
     
int Compound = 0;//compound statement inside funcbool hasReturn = false;
bool assignParent=false;
bool iterChild = false;
bool hasReturn = false;
char * funcParent = NULL;
//bool loopFlag = false;
void semanticAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter){
  //void newS;
  while(tree != NULL){

    switch(tree->nodekind){

    case declK:
      declAnalysis(tree, symbolTable, IOTree, Enter);
      //Scope=newS;
      break;
    case stmtK:
      stmtAnalysis(tree, symbolTable, IOTree, Enter);
      //Scope=newS;
      break;
    case expK:
      expAnalysis(tree, symbolTable, IOTree, Enter);
      break;
    default:
      printf("unrecognized nodekind\n");
      break;

    }
    tree = tree->sibling;
  }
  //  return Scope;
}

void hindSight(astNode * tree, SymbolTable * symbolTable){

  while(tree != NULL){

    switch(tree->nodekind){

    case declK:
      hindSightDecl(tree, symbolTable);
      break;
    case stmtK:
      hindSightStmt(tree, symbolTable);
      break;
    case expK:
      hindSightExp(tree, symbolTable);
      break;
    default:
      printf("Unrecognized nodekind after annotation\n");
      break;
    }

    tree = tree->sibling;
  }

}

void hindSightDecl(astNode * tree, SymbolTable * symbolTable){

  switch(tree->subkind.decl){
  case varK:
    break;
  case funcK:
    if(tree != NULL){
    void * tempTcc = NULL;
    astNode * nodeTcc;
    tempTcc = symbolTable->lookup(tree->id);
    nodeTcc = static_cast <astNode *>(tempTcc);

    if(nodeTcc != NULL){
      if(nodeTcc->isUsed != true){
	if(strcmp(tree->id, "main")==0){

	}else{

	  numWarnings++;
	  printf("WARNING(%d): The function '%s' seems not to be used.\n", tree->lineno, tree->id);
	}
      }
    }
        
    if(tree->hasReturn != true){
      if(tree->expType != Void){
      numWarnings++;
      printf("WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n", tree->lineno, convExpType(tree->expType),tree->id);
      }
    }else{
      if(tree->expType == Void){
	if(tree->returnV==true){
	numErrors++;
	printf("ERROR(%d): Function '%s' at line %d is expecting no return value but return has a value.\n",nodeTcc->returnL, tree->id, tree->lineno);  
	       }
      }else{
	
	if(tree->returnV==false){
	  numErrors++;
	  // printf("reruen no value\n");
     	  printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but return has no value.\n", nodeTcc->returnL, tree->id, tree->lineno, convExpType(tree->expType));

	}else{
	  if(tree->returnType != tree->expType){

	    numErrors++;
	    printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but returns type %s.\n", nodeTcc->returnL, tree->id, tree->lineno, convExpType(tree->expType), convExpType(nodeTcc->returnType));
	  }	  

	}

      }

    }
    }
    hindSight(tree->child[0], symbolTable);
    hindSight(tree->child[1], symbolTable);
    break;
  case paramK:
   
    
    if(tree->isUsed != true){

      //   numWarnings++;
      //printf("WARNING(%d): The parameter '%s' seems not to be used.\n", tree->lineno, tree->id);

    }
    break;
  default:
    break;
  }
}

void hindSightStmt(astNode * tree, SymbolTable * symbolTable){

  switch(tree->subkind.stmt){
  case IfK:
    hindSight(tree->child[0], symbolTable);
    hindSight(tree->child[1], symbolTable);
    hindSight(tree->child[2], symbolTable);
    break;
  case WhileK:
    hindSight(tree->child[0], symbolTable);
    hindSight(tree->child[1], symbolTable);
    hindSight(tree->child[2], symbolTable);
    break;
  case IterK:
    hindSight(tree->child[0], symbolTable);
    hindSight(tree->child[1], symbolTable);
    hindSight(tree->child[2], symbolTable);
    break;
  case CompoundK:
    hindSight(tree->child[0], symbolTable);
    hindSight(tree->child[1], symbolTable);
    break;
  case ReturnK:
    hindSight(tree->child[0], symbolTable);

    break;
  default:
    break;

  }

}

void hindSightExp(astNode * tree, SymbolTable * symbolTable){

  switch(tree->subkind.exp){
  case opK:

    hindSight(tree->child[0], symbolTable);
    hindSight(tree->child[1], symbolTable);
    break;
  case CallK:
    hindSight(tree->child[0], symbolTable);
  default:
    break;

  }

} 
void declAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter){

  switch(tree->subkind.decl){

  case varK:
    if(symbolTable->lookup(tree->id) == NULL){
      tree->scope = Scope;
      //foffset--;
      tree->memLoc=foffset;
      if(tree->isStatic == true && Scope != 0){
	tree->refType = LocalStatic;
	tree->location = strdup("Local");
	tree->memLoc = goffset;
	if(tree->isArray == true){
	  int T = tree->size +1;
	  goffset-=T;

	}else{

	  goffset--;
	}
      }
      //foffset--;
      //printf("foffset %d\n", foffset);
      if(tree->isArray != true){
      tree->size=1;
      if(Scope == 0){
	tree->refType = Global;
	tree->location = strdup("Global");
	tree->memLoc = goffset;
	goffset--;
      }else{
	foffset--;

      }
      }else{

	//if it's an array
	//printf("Scope is %d %d\n", Scope, tree->lineno);
	if(tree->isStatic != true){
	int sizeTemp= tree->size;
	tree->size = sizeTemp+1;
	tree->memLoc = foffset-1;
	foffset -= tree->size;
	}
	if(Scope==0){
	  tree->refType = Global;
	  tree->location = strdup("Global");
	  tree->memLoc = goffset-1;
	  goffset = goffset-tree->size;
	  //goffset--;
	}
	else{
	  //tree->memLoc = foffset-1;

	}
	if(tree->child[0] != NULL){
	  int len;
	  len = strlen(tree->child[0]->id);
	  len--;
	  tree->child[0]->refType = Global;
	  tree->location = strdup("Global");
	  //here the length is the size of the constant
	  tree->child[0]->size = len;
	  tree->child[0]->memLoc=goffset-1;
	  if(Scope ==0){
	    tree->refType =Global;
	    tree->location = strdup("Global");
	    tree->child[0]->location = strdup("Global");
	    tree->child[0]->refType = Global;
	    tree->child[0]->memLoc=goffset-1;
	    tree->memLoc=goffset-1-len;
	    goffset = goffset-tree->size;
	    goffset = goffset -tree->child[0]->size;

	  }
	  else{

	    goffset = goffset-tree->child[0]->size;
	  }
	}

      }

      if(Scope == 0){
	symbolTable->insertGlobal(tree->id, tree);
	symbolTable->insert(tree->id, tree);
      }else{
      symbolTable->insert(tree->id, tree);
      }
    }
    else{
      void * temp1 = NULL;
      astNode * node1;
      astNode *node2;
      temp1 = symbolTable->lookup(tree->id);
      node1 = static_cast <astNode *>(temp1);
      tree->scope = Scope;
      if(node1->nodekind == declK){

	if(node1->subkind.decl == funcK){

	  if(node1->scope == Scope){
	  numErrors++;
	  printf("ERROR(%d): Cannot use function '%s' as a variable.\n", tree->lineno, tree->id);
	  }
	}else{
	  //printf("node %d  tree %d \n", node1->scope, Scope);
	  //printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", tree->lineno, tree->id, node1->lineno);
	  if(node1->scope == Scope){
      printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", tree->lineno, tree->id, node1->lineno);
      numErrors++;
	  }else{
	    if(Scope==0){
	      symbolTable->insertGlobal(tree->id, tree);
	      symbolTable->insert(tree->id, tree);
	    }else{
	    symbolTable->insert(tree->id, tree);
	    }
	  }
	}
      }
      
    }

    if(tree->isInitialized == true){
      if(tree->child[0] != NULL){
	//this commented out block is my food for thoughts

	
	void * temp1 = NULL;
	astNode * node1;
	temp1 = symbolTable->lookup(tree->child[0]->id);
	node1 = static_cast <astNode *>(temp1);
	int constFlag=0;

	//1 for is constant 0 not constant
	if(node1 != NULL){
	  
          numErrors++;
          printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", tree->lineno, tree->id);

	}else{
	  if(tree->child[0]->nodekind == expK){

	    if(tree->child[0]->subkind.exp == charConstantK)
	      constFlag=1;
	    if(tree->child[0]->subkind.exp == boolConstK)
	      constFlag=1;
	    if(tree->child[0]->subkind.exp == stringConstantK)
	      constFlag=1;
	    if(tree->child[0]->subkind.exp == ConstantK)
	      constFlag=1;
	    if(constFlag == 0){
	      numErrors++;
	      printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", tree->lineno, tree->id);

	    }

	  }

	}

	if(constFlag == 1){
	  if(tree->expType != tree->child[0]->expType){
	    numErrors++;
	    printf("ERROR(%d): Initializer for variable '%s' of type %s is of type %s.\n", tree->lineno, tree->id, convExpType(tree->expType), convExpType(tree->child[0]->expType));

	  }

	}


      }
    }
    tree->scope = Scope;
    symbolTable-> insert( tree-> id, tree);
    break;
  case funcK:
    if(tree!= NULL){

      void * tempTcc = NULL;
      astNode * nodeTcc;
      tempTcc = symbolTable->lookup(tree->id);
      nodeTcc = static_cast <astNode *>(tempTcc);
      bool countFlag = false;
      astNode * IO;
      if(nodeTcc!= NULL){
	countFlag =true;
	}
      if(countFlag == true){
	numErrors++;
	printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", tree->lineno, tree->id, nodeTcc->lineno);
      }

    }
    funcParent = tree->id;
    //printf("the fucntion here is %s\n", tree->id);
    Scope++;
    foffset =-1;
    tree->refType = Global;
    tree->location = strdup("Global");
    tree->memLoc=0;
    tree->scope = Scope;
    // printf("the scope number is %d\n", Scope);
    if(symbolTable->insert( tree->id, tree)){
      //Scope++;
	symbolTable->enter(tree->id);
	semanticAnalysis(tree->child[0], symbolTable, IOTree, false);
	tree->size = foffset-1;
	semanticAnalysis(tree->child[1], symbolTable, IOTree, false);
	symbolTable->leave();
      }
 
    sizeL =0;
    /*
    semanticAnalysis(tree->child[0], symbolTable);
    semanticAnalysis(tree->child[1], symbolTable);
    /*
    if(tree->child[1] != NULL){

      if(tree->child[1]->child[1] != NULL){
	//	printf("hi\n");
	if(tree->child[1]->child[1]->child[0] != NULL){
	  if(tree->expType){
	    if(convExpType(tree->expType) != getType(tree->child[1]->child[1]->child[0])){
	      printf("id is %s\n",tree->child[1]->child[1]->child[0]->id );
	      //	      printExp(tree->child[1]->child[1]->child[0]);
	      printf("ERROR(%d): Cannot return an array %s.\n", tree->child[1]->child[1]->child[0]->lineno, convExpType(tree->child[1]->child[1]->child[0]->expType));
	    }
	       
				    }
	  // printf("em\n");
	}

      }
    }
    
    */

    /*
    //this part of the code checks for parameter use for functions

    if(tree->child[0] != NULL){
    astNode * ptr= tree->child[0];
    astNode * ptr2=tree->child[1]->child[1];
    astNode * ptr3=ptr2;
    while(ptr != NULL){

      if(ptr2 != NULL){

	ptr3=ptr2;
	while(ptr3!= NULL){

	  if(ptr3->nodekind == expK){
	    if(ptr3->subkind.exp == AssignK){
	      if(ptr3->child[0] != NULL){
		if(ptr3->child[0]->id == ptr->id){

		  tree->child[0]->isUsed = true;

		}

	      }

	    }

	  } 
	  ptr3=ptr3->sibling;
	}

      }

      ptr=ptr->sibling;
    }
    }
    */
    if(tree->child[1] != NULL){
      if(tree->child[1]->child[1] != NULL){
	astNode * temp = tree->child[1]->child[1];
	while(temp != NULL){
	  if( temp->nodekind == stmtK){
       	    if(temp->subkind.stmt == ReturnK){
	      if(temp->child[0] != NULL){
      		void * temp1 = NULL;
		astNode * node1;
		temp1 = symbolTable->lookup(getID(temp->child[0]));
		node1 = static_cast <astNode *>(temp1);
		if(node1 != NULL){
		if(node1->expType != tree->expType){
		  numErrors++;
		  printf("ERROR(%d): Cannot return %s.\n", node1->lineno,convExpType(node1->expType));

		}
		}
		else{

		  temp->expType = tree->expType;
		  //set the type for return so that we can check

		}

	      }  


	    }
	  }

	  temp = temp->sibling;
	}
      }
    }
    if(strcmp(tree->id, "main")==0){
      if(tree->child[0] != NULL){
	printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n");
	numErrors++;
      }
    }
    /*
    void * tempTcc = NULL;
    astNode * nodeTcc;
    tempTcc = symbolTable->lookup(tree->id);
    nodeTcc = static_cast <astNode *>(tempTcc);

    if(nodeTcc!= NULL){
      numErrors++;
      printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", tree->lineno, tree->id, nodeTcc->lineno);

      }*/
    break;
  case paramK:
      if(symbolTable->lookup(tree->id) == NULL){
	tree->scope = Scope;
	tree->refType = Parameter;
	tree->size =1;
	tree->memLoc = foffset-1;
	foffset--;
	if(symbolTable->insert(tree->id, tree)){
	  //printf("insertion success\n");

	}
      }else{
	void * temp1 = NULL;
	astNode * node1;
	tree->refType = Parameter;
	temp1 = symbolTable->lookup(tree->id);
	node1 = static_cast <astNode *>(temp1);
	tree->scope = Scope;
	//printf("node %d tree %d Scope %d\n", node1->scope, tree->scope, Scope);
	if(node1->scope == tree->scope){
	numErrors++;
	printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", tree->lineno, tree->id, node1->lineno);
	}else{
	  tree->scope = Scope;
	  symbolTable->insert(tree->id, tree);
	}
      }
     
      
  break;    
  default:
    printf("Unrecognized decl kind\n");
   break;

 }
  //return Scope;
}

void stmtAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter){

  switch(tree->subkind.stmt){

  case NullK:
    break;
  case ElifK:
    break;
  case IfK:
    ExpType temp;
    void * temp1;
    semanticAnalysis(tree->child[0], symbolTable, IOTree, Enter);
    semanticAnalysis(tree->child[1], symbolTable, IOTree, Enter);
    semanticAnalysis(tree->child[2], symbolTable, IOTree, Enter);

    if(tree->child[0]->id != NULL){
      tree->child[0]->expType=Bool;
      temp1 = symbolTable->lookup( tree->child[0]->id);
      if(temp1 == NULL){
	//numErrors++;
	//printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->id);
	//printf("here\n");
      }
      else{
	void * temp;
	temp = symbolTable->lookup (tree->child[0]->id);
	astNode *node;
	node = static_cast <astNode *>(temp);
	tree->child[0]->expType = node->expType;
        //printf("The type being called is %s\n", convExpType(node->expType));                                                                                                                     
	//printf("there\n");
	//printf("the child 1 is %s\n", tree->child[0]->id);
      }


    }
    else{
      temp1 = symbolTable->lookup(getID(tree->child[0]));
    }
    if(tree->child[1] != NULL){

      if(tree->child[1]->nodekind == expK && tree->child[1]->subkind.exp == idK){   
     if(symbolTable->lookup(tree->child[1]->id) == NULL){
       numErrors++;
       printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[1]->id);
      }
      else{
	void * temp;
	temp = symbolTable->lookup (tree->child[1]->id);
	astNode *node;
	node = static_cast <astNode *>(temp);
	tree->child[1]->expType = node->expType;
	// printf("The type being called is %s\n", convExpType(node->expType));                                                                                                                     
	//printf("WARNING(%d): Variable '%s' may be the child 1 .\n", tree->lineno, tree->child[0]->id);
      }

    }
    }

    if(tree->child[2] != NULL){
      if(tree->child[2]->nodekind == expK && tree->child[2]->subkind.exp == idK){
	if(symbolTable->lookup(tree->child[2]->id) == NULL){
	 
          numErrors++;
	  printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[2]->id);
	}
	else{
	  void * temp;
	  temp = symbolTable->lookup (tree->child[2]->id);
	  astNode *node;
	  node = static_cast <astNode *>(temp);
	  tree->child[2]->expType = node->expType;
	  // printf("The type being called is %s\n", convExpType(node->expType));                                                                                                                           
	  //printf("WARNING(%d): Variable '%s' may be the child 1 .\n", tree->lineno, tree->child[0]->id);                                                                                                  
	}

      }
    }
    astNode * ifTemp;
    ifTemp = static_cast<astNode *>(temp1);

    if(tree->isInitialized ==true){


    }
    else{
      if(tree->child[0]!= NULL){

	if(tree->child[0]->subkind.exp == idK){
	  //numWarnings++;
	  //printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", tree->lineno, tree->child[0]->id);
	}
      }


    }  
    
    if(ifTemp != NULL){
    if(ifTemp ->expType != Bool){
      numErrors++;
      printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", tree->lineno, getType(ifTemp));
    }
    }
    else{

    }
    break;
  case WhileK:
    semanticAnalysis(tree->child[0], symbolTable, IOTree, Enter);
    Scope++;
    tree->scope = Scope;
    symbolTable->enter("WhileK");
    //printf("in while case\n");
    loopFlag++;
    void * temp0;
    temp0 = symbolTable->lookup(getID(tree->child[0]));
    astNode * whileTemp;
    whileTemp = static_cast<astNode *>(temp0);
    
    if(whileTemp == NULL){

      //      printf("while is NULL\n");
    }else{
    
    if(whileTemp ->expType != Bool){
      //
      printf("ERROR(%d): Expecting Boolean test condition in while statement but got type %s.\n", tree->lineno, getType(whileTemp));
      numErrors++;
      }

      }
    semanticAnalysis(tree->child[1], symbolTable, IOTree, Enter);
    if(tree->child[2]!= NULL){
      semanticAnalysis(tree->child[2], symbolTable, IOTree, Enter);
    }
    loopFlag--;
    symbolTable->leave();
    //printf("left while case\n");
    break;
  case IterK:
    iterChild=true;
    Scope++;
    tree->refType=None;
    tree->scope = Scope;
    symbolTable -> enter("For");
    loopFlag++;
    tree->size = -3;
    semanticAnalysis(tree->child[0], symbolTable, IOTree, Enter);
    semanticAnalysis(tree->child[1], symbolTable, IOTree, Enter);
    //    printf("got iter\n");

    
    if(tree->child[0] != NULL){
    if(tree->child[0]->expType != Int){
      numErrors++;
      printf("ERROR(%d): Expecting integer in range for loop statement but got type  %s.\n", tree->lineno, tree->child[0]->expType);
    }
    }
    semanticAnalysis(tree->child[2], symbolTable, IOTree,  Enter);
    symbolTable->leave();
    //printf("out of iter\n");
    break;
  case CompoundK:
    if(Enter == true){
    Scope++;
    tree->scope = Scope;
    symbolTable->enter("Compound");
    tree->refType = None;
    //tree->size =-3;
    
    semanticAnalysis(tree->child[0], symbolTable, IOTree,  true);
    semanticAnalysis(tree->child[1], symbolTable, IOTree, true);
    tree->size = foffset;
    symbolTable->leave();

    }
    else{
      tree->refType = None;
      //tree->size=-foffset;
      foffset--;
      //foffset--;
      semanticAnalysis(tree->child[0], symbolTable, IOTree,  true);
      semanticAnalysis(tree->child[1], symbolTable, IOTree, true);
      tree->size=foffset;
    }
    break;
  case ReturnK:
    if(tree->child[0] != NULL){
      semanticAnalysis(tree->child[0], symbolTable, IOTree,  Enter);
      void * temp1 = NULL;
      astNode * node1;
      temp1 = symbolTable->lookup(getID(tree->child[0]));
      node1 = static_cast <astNode *>(temp1);

      if(node1 != NULL){

	if(node1->expType != tree->expType){

	  // numErrors++;
	  if(node1->isArray == true){
	    numErrors++;
	    printf("ERROR(%d): Cannot return an array.\n", tree->lineno);
	  }
	}

      }

    }
    if(funcParent != NULL){
      void * temp1 = NULL;
      astNode * node1;
      temp1 = symbolTable->lookup(funcParent);
      node1 = static_cast <astNode *>(temp1);
      if(node1 != NULL){
	node1->hasReturn=true;
	node1->returnL=tree->lineno;
	if(tree->child[0]!= NULL){
	  node1->returnV=true;
	  node1->returnType=tree->child[0]->expType;
	}
      }

    }

    break;
  case BreakK:
    if(loopFlag == 0){
      numErrors++;
      printf("ERROR(%d): Cannot have a break statement outside of loop.\n", tree->lineno);
      }
    break;
  case rangeK:
    void * temp9;
    temp9 = symbolTable->lookup(getID(tree->child[0]));
    astNode * loopTemp;
    loopTemp = static_cast<astNode *>(temp0);
    ExpType temp7;
    temp7 = checkType(tree->child[1]);

    if(tree != NULL){

      void * temp1=NULL;
      void * temp2=NULL;
      void * temp3=NULL;
      astNode * node1;
      astNode * node2;
      astNode * node3;
      if(tree->child[0]!= NULL)
      temp1 = symbolTable->lookup(getID(tree->child[0]));
      if(tree->child[1]!= NULL) 
      temp2=symbolTable->lookup(getID(tree->child[1]));
      if(tree->child[2]!= NULL)
      temp3= symbolTable->lookup(getID(tree->child[2]));
      node1=static_cast<astNode *>(temp1);
      node2=static_cast<astNode *>(temp2);
      node3=static_cast<astNode *>(temp3);

      if(node1!= NULL){
	tree->child[0]->memLoc = node1->memLoc;
	tree->child[0]->size = node1->size;
	if(node1->isArray == true){
	  numErrors++;
	  printf("ERROR(%d): Cannot use array in position 1 in range of for statement.\n", tree->lineno);
	}

	if(node1->expType != Int){
	  numErrors++;
	  printf("ERROR(%d): Expecting type int in position 1 in range of for statement but got type %s.\n", tree->lineno, convExpType(node1->expType));
	}
      }else{
	if(tree->child[0]!= NULL){
        if(tree->child[0]->expType != Int){
          numErrors++;
          printf("ERROR(%d): Expecting type int in position 1 in range of for statement but got type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));
        }

	}
      }
      if(node2!= NULL){
        tree->child[1]->memLoc = node2->memLoc;
        tree->child[1]->size = node2->size;
        if(node2->isArray == true){
          numErrors++;
          printf("ERROR(%d): Cannot use array in position 2 in range of for statement.\n", tree->lineno);
        }
        if(node2->expType != Int){
          numErrors++;
          printf("ERROR(%d): Expecting type int in position 2 in range of for statement but got type %s.\n", tree->lineno, convExpType(node2->expType));
        }
      }else{
        if(tree->child[1]!= NULL){
	  if(tree->child[1]->expType != Int){
	    numErrors++;
	    printf("ERROR(%d): Expecting type int in position 2 in range of for statement but got type %s.\n", tree->lineno, convExpType(tree->child[1]->expType));
	  }

	}

      }
      if(node3!= NULL){
        tree->child[2]->memLoc = node3->memLoc;
        tree->child[2]->size = node3->size;
        if(node3->isArray == true){
          numErrors++;
          printf("ERROR(%d): Cannot use array in position 3 in range of for statement.\n", tree->lineno);
        }

        if(node3->expType != Int){
          numErrors++;
          printf("ERROR(%d): Expecting type int in position 3 in range of for statement but got type %s.\n", tree->lineno, convExpType(node3->expType));
        }
      }else{
        if(tree->child[2]!= NULL){
          if(tree->child[2]->expType != Int){
            numErrors++;
            printf("ERROR(%d): Expecting type int in position 3 in range of for statement but got type %s.\n", tree->lineno, convExpType(tree->child[2]->expType));
          }

        }



      }


    }
    /*
    if(loopTemp != NULL){
      printf("got range\n");
    if(loopTemp->expType != Int){
      numErrors++;
      printf("ERROR(%d): Expecting integer in range for loop statement but got type %s.\n", tree->lineno, getType(loopTemp));
    }

    }
    /*
    temp9=symbolTable->lookup(tree->child[2]->id);
    loopTemp=static_cast<astNode*>(temp9);
    if(loopTemp != NULL){
      printf("got range2\n");
    if(loopTemp->expType != Int){
      numErrors++;
      printf("ERROR(%d): Expecting integer in range for loop statement but got type %s.\n", tree->lineno, getType(loopTemp));
    }
    }
    printf("out of range\n");
    */
    break;
  default:
    printf("Unrecognized stmt node analysis\n");
    break;
  }


}

void expAnalysis(astNode * tree, SymbolTable * symbolTable, astNode * IOTree, bool Enter){

  switch(tree->subkind.exp){
  case InitK:
    printf("in init node\n");

    break;

  case opK:
    semanticAnalysis(tree->child[0], symbolTable, IOTree,  Enter);
    if(tree->child[1] != NULL){
      semanticAnalysis(tree->child[1], symbolTable, IOTree, Enter);
    }

    if(tree->attr.op == PLUSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      tree->expType=Int;
      if(tree->child[0] != NULL){
	temp1 = symbolTable->lookup(getID(tree->child[0]));
	node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

	temp2 = symbolTable->lookup(getID(tree->child[1]));
	node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

	if(node1->isArray == true){
	  printf("ERROR(%d): The operation '+' does not work with arrays. \n", tree->lineno);
	  numErrors++;
	}else{
	  if(node2 != NULL && node2->isArray == true){
	    printf("ERROR(%d): The operation '+' does not work with arrays. \n", tree->lineno);
	    numErrors++;
	  }
	}
      }
      if(node1 != NULL){

	if(node1 ->expType != Int){

	  printf("ERROR(%d): '+' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
	}

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '+' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      } 
    }
    /*
    if(tree->attr.op == leftSquareBracOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      //printf("in left square %d\n", tree->lineno);
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
        tree->child[0]->expType =node1->expType;
	//tree->expType=node1->expType;
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
        tree->child[1]->expType =node2->expType;
      }
      //tree->expType=node1->expType;
      }*/
    if(tree->attr.op == GREATOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      tree->expType = Bool;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);

	if(node1 != NULL){
	tree->child[0]->expType =node1->expType;
	}
      } 
      if(tree->child[1] != NULL){

	temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
	//tree->child[1]->expType =Int;
	if(node2 != NULL){
	  tree->child[1]->expType = node2->expType;
	}
      }

      if(node1 != NULL && node2 != NULL){
	if(tree->child[0]->expType != tree->child[1]->expType){

	  numErrors++;
	  printf("ERROR(%d): '>' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType)); 
	}

      }
    }
    if(tree->attr.op == NEQOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      //printf("NEQ %d\n",tree->lineno);
      tree->expType=Bool;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);

        if(node1 != NULL){
	  tree->child[0]->expType =node1->expType;
        }
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
        //tree->child[1]->expType =Int;                                                                                                                                                                   
        if(node2 != NULL){
          tree->child[1]->expType = node2->expType;
        }
      }

      if(node1 != NULL && node2 != NULL){
	if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): '><' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType));
        }

      }
      tree->expType = Bool;
    }

    if(tree->attr.op == LESSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
	if(node1 != NULL){
	  tree->child[0]->expType =node1->expType;
        }

      } 
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
        //tree->child[1]->expType =Int;
        if(node2 != NULL){
          tree->child[1]->expType = node2->expType;
	}
      }
      if(node1 != NULL && node2 != NULL){
        if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): '<' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType));
        }

      }

    }
    if(tree->attr.op == LESSEQOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
        //tree->child[0]->expType =Int;
        if(node1 != NULL){
          tree->child[0]->expType =node1->expType;
        }
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
        //tree->child[1]->expType =Int;
        if(node2 != NULL){
          tree->child[1]->expType = node2->expType;
	}
      }

      if(node1 != NULL && node2 != NULL){
        if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): '<=' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType));
        }

      }
    } 
    if(tree->attr.op == GREATEQOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
        //tree->child[0]->expType =Int;
        if(node1 != NULL){
          tree->child[0]->expType =node1->expType;
	}


      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
	//        tree->child[1]->expType =Int;
        if(node2 != NULL){
          tree->child[1]->expType =node2->expType;
	}


      }

      if(node1 != NULL && node2 != NULL){
	if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): '>=' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType));
        }

      }

      tree->expType=Bool;
    }

    if(tree->attr.op == PEROp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '%' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '%' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '%' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '%' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }

    /*
    if(tree->attr.op == NEQOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
        tree->child[0]->expType =node1->expType;

      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
        tree->child[1]->expType =node2->expType;
      }
      //semanticAnalysis(tree->child[0], symbolTable);
      //semanticAnalysis(tree->child[1], symbolTable);
      }*/
    if(tree->attr.op == MULOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '*' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '*' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '*' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '*' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }

    if(tree->attr.op == DIVOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '/' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '/' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '/' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '/' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }

    if(tree->attr.op == MinusOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode * node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '-' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray ==true){
            printf("ERROR(%d): The operation '-' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
	}
      
      }

      if(node1 != NULL){
        if(node1 ->expType != Int){
          printf("ERROR(%d): '-' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }
      }

      if(node2 != NULL){
        if(node2 ->expType != Int){
          printf("ERROR(%d): '-' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }
      }
    }

    if(tree->attr.op == ANDOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode * node2;
      //printf("in and\n");
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
        //tree->child[0]->expType =Int;                                                                                                                                                                   
        if(node1 != NULL){
          tree->child[0]->expType =node1->expType;
        }
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
        //tree->child[1]->expType =Int;                                                                                                                                                                   
        if(node2 != NULL){
          tree->child[1]->expType = node2->expType;
        }
      }

      if(node1 != NULL && node2 != NULL){
        if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): 'and' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType));
        }
	/*
	if(tree->child[0]->expType != Bool){
	  numErrors++;
	  printf("ERROR(%d): 'and' requires operands of type bool but lhs is type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));
	}
	if(tree->child[1]->expType != Bool){
	  numErrors++;
          printf("ERROR(%d): 'and' requires operands of type bool but rhs is type %s.\n", tree->lineno,convExpType(tree->child[1]->expType));

	  }*/

      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation 'and' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray ==true){
            printf("ERROR(%d): The operation 'and' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(tree->child[0]!= NULL){
        if(tree->child[0]->expType != Bool){
          numErrors++;
          printf("ERROR(%d): 'and' requires operands of type bool but lhs is type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));
        }

      }

      if(tree->child[1]!= NULL){

        if(tree->child[1]->expType != Bool){
          numErrors++;
          printf("ERROR(%d): 'and' requires operands of type bool but rhs is type %s.\n", tree->lineno,convExpType(tree->child[1]->expType));

        }

      }

    }
    if(tree->attr.op == OROp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode * node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
	node1 = static_cast <astNode *>(temp1);
        //tree->child[0]->expType =Int;                                                                                                                                                                  \
                                                                                                                                                                                                          
	if(node1 != NULL){
          tree->child[0]->expType =node1->expType;
	}
      }
      if(tree->child[1] != NULL){

	temp2 = symbolTable->lookup(getID(tree->child[1]));
	node2 = static_cast <astNode *>(temp2);
	//tree->child[1]->expType =Int;                                                                                                                                                                  \
                                                                                                                                                                                                          
	if(node2 != NULL){
          tree->child[1]->expType = node2->expType;
	}
      }

      if(node1 != NULL && node2 != NULL){
	if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): 'or' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType))\
	    ;
        }

	if(node1->expType != Bool){

	  numErrors++;
	  printf("ERROR(%d): 'or' requires operands of type bool but lhs is type %s.\n", tree->lineno, convExpType(node1->expType));
	}
	if(node2->expType != Bool){

          numErrors++;
          printf("ERROR(%d): 'or' requires operands of type bool but rhs is type %s.\n", tree->lineno, convExpType(node1->expType));
	}
      }
    }
    if(tree->attr.op == leftSquareBracOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      //int sizeTemp= tree->size;
      //      printf("temp size %d\n", sizeTemp);
      //tree->size = sizeTemp+1;


      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
	if(node1!= NULL){
	tree->child[0]->expType = node1->expType;
	tree->expType =node1->expType;
	}
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
      }

      if(node1 != NULL){

	if(node1->isArray == false){
	  //changed here, double check!!!
	if(tree->child[0] != NULL){
	  numErrors++;
	  printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, node1->id);
	}
	else{
	  //printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, tree->child[0] ->id);
	}
	//numErrors++;
	}
      
      }
      else{

	if(tree->child[0]!= NULL){
	  if(tree->isArray ==false){
	  printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno,tree->child[0]->id); 
	       numErrors++;
	  }else{
	    // int sizeTemp= tree->size;
	    //printf("temp size %d\n", sizeTemp);
	    //tree->size = sizeTemp+1;

	  }

          }
      }
      /*
      if(node2 == NULL || node2->isArray == true){
	if(node2 != NULL){
	  printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, node2->id);
	}else{
	  printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineno, tree->child[0]->id);
	}
	numErrors++;
	}*/

    }
    
    if(tree->attr.op == leftSquareBracOp){

      if(tree->child[0]->nodekind == expK && tree->child[0]->subkind.exp == idK){
	if(tree->child[1]->nodekind == expK && tree->child[1]->subkind.exp == idK){

	  void * indexCheck = symbolTable->lookup(tree->child[1]->id);
	  astNode * indexCheck1 = static_cast<astNode *>(indexCheck);
	  if(indexCheck1!= NULL){
	  if(indexCheck1->expType != Int){
	    numErrors++;
	    char * stuff = convExpType(indexCheck1->expType);
	    //printf("would go wrong %s\n", stuff);
	    printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", tree->lineno, tree->child[0]->id,stuff);
	  }
	  else{
	    //then we log the int that indexes the array

	  }
	  }

	}
      }

      }

    if(tree->attr.op == DECOp){

      printf("in --\n");
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
       if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
        if(node1!= NULL){
          tree->child[0]->expType=node1->expType;
        }
	semanticAnalysis(tree->child[0], symbolTable, IOTree, Enter);
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
      }
      if(temp2 == NULL){
        if(node1 != NULL && node1->isArray == true){
          printf("ERROR(%d): The operation '--' does not work with arrays.\n", tree->lineno);
          numErrors++;
        }
      }
      if(node1 != NULL){
        if(tree->child[0]->expType != Bool){
	  numErrors++;
          printf("ERROR(%d): Unary '--' requires an operand of type bool but was given type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));

        }

      }else{
	//printf("-- NULL\n");
      }
      // tree->child[0]->expType=Int; 
   }
    if(tree->attr.op == NOTOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
	if(node1!= NULL){
	  tree->child[0]->expType=node1->expType;
	}
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
      }
      if(temp2 == NULL){
	if(node1 != NULL && node1->isArray == true){
	  printf("ERROR(%d): The operation 'not' does not work with arrays.\n", tree->lineno);
	  numErrors++;
	}
      }
      if(node1 != NULL){
	if(tree->child[0]->expType != Bool){
	  numErrors++;
	  printf("ERROR(%d): Unary 'not' requires an operand of type bool but was given type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));

	}

      }
    }

    if(tree->attr.op == sizeofOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
        if(node1!= NULL){
          tree->child[0]->expType=node1->expType;
        }
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
      }
      if(temp2 == NULL){
        if(node1 != NULL && node1->isArray == true){
	  //     printf("ERROR(%d): The operation 'not' does not work with arrays.\n", tree->lineno);
	  // numErrors++;
        }else{
	  numErrors++;
	  printf("ERROR(%d): The operation 'sizeof' only works with arrays.\n", tree->lineno);
	}
      }

      /*
      if(node1 != NULL){
        if(tree->child[0]->expType != Bool){
          numErrors++;
          printf("ERROR(%d): Unary 'not' requires an operand of type bool but was given type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));

        }

      }
      */

    }
    if(tree->attr.op == chsignOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
        if(node1!= NULL){
          tree->child[0]->expType=node1->expType;
        }
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
      }
      
      if(temp2 == NULL){
	if(node1 != NULL && node1->isArray == true){
          printf("ERROR(%d): The operation 'chsign' does not work with arrays.\n", tree->lineno);
          numErrors++;
        }
	}
      if(node1 != NULL){
        if(tree->child[0]->expType != Int){
          numErrors++;
	  printf("ERROR(%d): Unary 'chsign' requires an operand of type int but was given type %s.\n", tree->lineno, convExpType(tree->child[0]->expType));

        }

      }
    }
    if(tree->attr.op == QUEOp){
      void * temp1;
      temp1 = symbolTable->lookup(getID(tree->child[0]));
      astNode * node1;
      node1 = static_cast <astNode *>(temp1);
      if(node1 != NULL && checkType(node1) != Int){
	printf("ERROR(%d): Unary '?' requires an operand of type int but was given type %s. \n", tree->lineno, getType(node1));
	numErrors++;
      }
      if(node1 != NULL && node1->isArray == true){
	printf("ERROR(%d): The operation '?' does not work with arrays. \n", tree->lineno);
	numErrors++;
      }
    }
    /*
    if(tree->attr.op == INCOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode * node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
	tree->child[0]->expType =Int;
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
	tree->child[1]->expType = Int;
      }

      if(node1 != NULL && node1->isArray == true){
	printf("ERROR(%d): The operation '++' does not work with arrays. \n", tree->lineno);
	numErrors++;
      }
      }*/
    if(tree->attr.op == EQOp){
      if(tree->child[0]->nodekind == expK && tree->child[0] ->subkind.exp != AssignK){
	if(tree->child[1]->nodekind == expK && tree->child[1] ->subkind.exp != AssignK){

	  if(tree-> child[0] ->expType != tree->child[1] ->expType){

	    void * temp1;
	    void * temp2;
	    temp1 = symbolTable->lookup(getID(tree->child[0]));
	    temp2 = symbolTable->lookup(getID(tree->child[1]));

	    astNode * node1;
	    astNode * node2;

	    node1 = static_cast<astNode *>(temp1);
	    node2 = static_cast<astNode *>(temp2);

	    if(node1 != NULL && node2 != NULL){

	      if(node1->expType != node2->expType){

		printf("a bunch of error messages here\n");
	      }
	    }
	  }

	}


      }
    }

    if(tree->attr.op == ASSOp2){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
	if(tree->child[0]->nodekind == expK && tree->child[0]->subkind.exp == stringConstantK){
	  tree->child[0]->refType = Global;
	  tree->location = strdup("Global");
	  tree->child[0]->size = strlen(tree->child[0]->id)-1;
	  tree->child[0]->memLoc = goffset-1;
	  goffset-=tree->child[0]->size;
	}
        if(node1 != NULL){
          tree->child[0]->expType = node1->expType;
          tree->expType =node1->expType;
        }else{

	  //numErrors++;
	  //printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[0]);
	}
      }
      if(tree->child[1] != NULL){
        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);

        if(tree->child[1]->nodekind == expK && tree->child[1]->subkind.exp == stringConstantK){
          tree->child[1]->refType = Global;
	  tree->location = strdup("Global");
          tree->child[1]->size = strlen(tree->child[1]->id)-1;
	  tree->child[1]->memLoc = goffset-1;
          goffset-=tree->child[1]->size;
        }
        if(node2 != NULL){
          tree->child[1]->expType =node2->expType;
        }else{
	  //numErrors++;
	  //printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[1]);
	}
      }
      if(node1 != NULL && node2 != NULL){
        if(tree->child[0]->expType != tree->child[1]->expType){

          numErrors++;
          printf("ERROR(%d): '=' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno,convExpType(tree->child[0]->expType), convExpType(tree->child[1]->expType));
        }

      }

    }


    if(tree->attr.op == ANDOp || tree->attr.op == OROp){
      //andorCheck(tree, symbolTable);
    }

    if(tree->attr.op == EQOp || tree->attr.op == ASSOp){

      void * temp1;
      temp1 = symbolTable ->lookup(getID(tree->child[0]));
      astNode * node1;
      node1 = static_cast<astNode *>(temp1);

      void * temp2;
      temp2 = symbolTable ->lookup(getID(tree->child[1]));
      astNode * node2;
      node2 = static_cast<astNode *>(temp2);

      if(node1 != NULL && node2 != NULL){
	if(node1->isArray == true && node2 -> isArray == false && tree->child[0]->child[1] == NULL && tree->child[1]->child[1] == NULL){

	  numErrors++;
	  printf("ERROR(%d): ':=' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", tree->lineno);

	}
      }

      if(node1!= NULL && node2!= NULL){

	if(node1->isArray == false && node2->isArray == true && tree->child[0]->child[1] == NULL && tree->child[1]->child[1] == NULL){

          numErrors++;
          printf("ERROR(%d): ':=' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n", tree->lineno);

	}

      }
    }

    if(tree->attr.op == EQOp || tree->attr.op == ASSOp2){

      void * temp1;
      temp1 = symbolTable ->lookup(getID(tree->child[0]));
      astNode * node1;
      node1 = static_cast<astNode *>(temp1);

      void * temp2;
      temp2 = symbolTable ->lookup(getID(tree->child[1]));
      astNode * node2;
      node2 = static_cast<astNode *>(temp2);

      if(node1 != NULL && node2 != NULL){
        if(node1->isArray == true && node2 -> isArray == false && tree->child[0]->child[1] == NULL && tree->child[1]->child[1] == NULL){

          numErrors++;
          printf("ERROR(%d): '=' requires both operands be arrays or not but the lhs is an array and rhs is not an array.\n", tree->lineno);

        }
      }

      if(node1!= NULL && node2!= NULL){

        if(node1->isArray == false && node2->isArray == true && tree->child[0]->child[1] == NULL && tree->child[1]->child[1] == NULL){

          numErrors++;
          printf("ERROR(%d): '=' requires both operands be arrays or not but the lhs is not an array and rhs is an array.\n", tree->lineno);

        }

      }
    }
    break;
  case ConstantK:
    tree->scope = Scope;
    symbolTable->insert(tree->id, tree);
    break;
  case idK:
    if(symbolTable->lookup(tree->id) != NULL){
      tree->scope = Scope;
      void * temp;
      if(Scope !=0 || tree->isStatic == true){
	tree->location = strdup("Local");
	tree->refType = LocalStatic;
      }
      tree->memLoc=foffset;
      temp = symbolTable->lookup (tree->id);
      astNode *node;
      node = static_cast <astNode *>(temp);
      tree->expType=node->expType;
      tree->refType = node->refType;
      tree->memLoc = node->memLoc;
      tree->size =node->size;
      tree->isArray = node->isArray;
      //printf("The things %d %s\n",tree->lineno, tree->id);  
    if( node -> nodekind == declK){
	if(node->subkind.decl == funcK){
	  printf("ERROR(%d): Cannot use function '%s' as a variable.\n", tree->lineno, tree->id);
	  numErrors++;
	  //look up and found the variable is a function
	}
	else{
	  //looked it up and found the original is declaration
	  //but it's not a function

	  //so we have to check the info here
	  //to see if the id is used properly here
	  if(node->isInitialized != true){
	    //  printf("tree %d node %d\n", tree->scope, node->scope);
	    if(tree->scope == node->scope|| node->scope==0){
	      //either they are in the same scope or the declaration node is a global
	      if(node-> subkind.decl != paramK){
		//we can ignore the parameters for initialization
		numWarnings++;
		printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", tree->lineno, tree->id);
	      }
	    }
	  }

	}
      } else{
      //look up and found the variable but the original is not declaration
      /*
	numErrors++;
	printf("ERROR(%d): Symbol '%s' is already declared at %d.\n",tree->lineno, node->id,node->lineno);
      */


      }

    }else{
      
      printf("ERROR(%d): Symbol '%s' is not declared. \n", tree->lineno, tree->id);
      numErrors++;
    }

    break;
  case CallK:
    if(symbolTable->lookup(tree->id) == NULL){
      astNode *IO=IOTree;
      bool countFlag = true;
      IO=IOTree;                                                                                                                                                                                
      while(IO != NULL){                                                                                                                                                                                
	//	printf("in IO\n");                                                                                                                                                                              
	if(tree->child[0]== NULL){                                                                                                                                                                      
	  //                                                                                                                                                          
	  //for IO functions that have no parameters                                   
	  if(strcmp(IO->id, tree->id)==0){                                                                                                                                                              
	    countFlag=false;                                                                                                                                                                            
	    tree->expType=IO->expType;
	  }                                                                                                                                                                                                                                                                                                                                                                                                       
	}                                                                                                                                                                                               
	else{                                                                                                                                                                                           
	  if(strcmp(IO->id, tree->id)==0){                                                                                                                                                              
	    
	    void * temp;
	    temp = symbolTable->lookup (tree->child[0]->id);
	    //child is the parameter
	    astNode *node;
	    node = static_cast <astNode *>(temp);
	    if(strcmp(tree->child[0]->id,"[")==0 || strcmp(tree->child[0]->id, ":=")==0){
	      //if an array look up the array name
	      temp = symbolTable->lookup (tree->child[0]->child[0]->id);
	      node = static_cast <astNode *>(temp);
	      if(tree->child[0]->child[0]!= NULL){
		//	printf("%d %s\n", tree->lineno, tree->child[0]->child[0]->id);
	      }
	      //printf("%d id %s exptype %s\n", tree->lineno, node->id, node->expType);
	    }
	    if(node!= NULL){
	      //printf("%d id %s exptype %s\n", tree->lineno, node->id, convExpType(node->expType));
	    if(IO->child[0]->expType==node->expType){                                                                                                                                        
	      countFlag=false;                                                                                                                                                                          
	      //and by the way we annotate the tree
	      tree->expType=IO->expType;
	    }               
	    }else{
	      if(IO->child[0]->expType == tree->child[0]->expType){
		countFlag=false;
		tree->expType = IO->expType;
	      }
	    }                                                                                                                                                                            
	  }                                                                                                                                                                                             
	}       


	IO = IO->sibling;                                                                                                                                                                               
      }                    
      if(countFlag==true){
      numErrors++;
      printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->id);
      }
    }
    else{
      //tree->isUsed=true;
      void * temp;
      temp = symbolTable->lookup (tree->id);
      astNode *node;
      node = static_cast <astNode *>(temp);
      tree->expType = node->expType;
      if(node->nodekind ==declK){
	if(node->subkind.decl != funcK){

	  numErrors++;
	  printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", tree->lineno, tree->id);
	}else{
	  node->isUsed=true;
	  //printf("The content is changed to %d\n", node->isUsed);
	}

      }

      //first we count the number of siblings in the call                                                                                                                                                 

      int Ccount=0;
      astNode *Ctemp = tree->child[0];
      //printf("tree id is %s\n", tree->child[0]->id);
      while(Ctemp != NULL){

        Ccount++;
        Ctemp=Ctemp->sibling;

      }
      //then we count the number of siblings in the declaration of the function                                                                                                                           
      Ctemp = node->child[0];
      int Dcount=0;
      while(Ctemp != NULL){

        Dcount++;
        Ctemp = Ctemp->sibling;

      }

      if(Ccount > Dcount){
        numErrors++;
        printf("ERROR(%d): Too many parameters passed for the function '%s' declared on line %d.\n", tree->lineno, tree->id, node->lineno);

      }
      if(Ccount < Dcount){
	numErrors++;
        printf("ERROR(%d): Too few parameters passed for the function '%s' declared on line %d.\n", tree->lineno, tree->id, node->lineno);

      }
      //printf("C is %d D is %d\n", Ccount, Dcount);   
   //      printf("The type being called is %s\n", convExpType(node->expType));
    }
   
    if(tree->child[0] != NULL){
      if(symbolTable->lookup(tree->child[0]->id) != NULL){

	if(tree->child[0]->isInitialized != true){
	  // numWarnings++;
	  //printf("WARNING(%d): Variable '%s' may be uninitialized when used here\n", tree->lineno, tree->child[0]->id);


	}

      }

    }
    semanticAnalysis(tree->child[0], symbolTable, IOTree,  Enter);
    break;
  case AssignK:
    if(tree->attr.op == ASSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;


      astNode *tempR = tree;
      //expType Type;
      astNode *tempR3 = tree;
      if(tree->child[0]!= NULL){
      void * temp = NULL;
      astNode * node;
      tree->child[0]->isInitialized = true;
      temp = symbolTable->lookup(tree->child[0]->id);
      node = static_cast <astNode *>(temp);

      if(node!= NULL){
	node->isInitialized =true;
	tree->child[0]->size = node->size;
	tree->child[0]->memLoc = node->memLoc;
	tree->child[0]->refType = node->refType;
	//set anything on the left side to be initialized
      }
      }
      if(strcmp(tree->child[0]->id, "[")== 0){
	void * temp = NULL;
	astNode * node;
	temp = symbolTable->lookup(tree->child[0]->child[0]->id);
	node = static_cast <astNode *>(temp);

	if(node!= NULL){
	  node->isInitialized =true;
	}

      }else{
      if(symbolTable->lookup(tree->child[0]->id) == NULL){
	numErrors++;
	printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[0]->id);
	return;
      }
      }
      while(tempR->child[1] != NULL){

	if(tempR->child[1]->attr.op != ASSOp){
	  if(tempR->child[0] != NULL){
       	    temp1 = symbolTable->lookup(getID(tempR->child[0]));
	    node1 = static_cast <astNode *>(temp1);

	    if(node1 != NULL){
	      tempR->child[0]->expType = node1->expType;
	      tempR->expType =node1->expType;
	      tempR3 = tempR;
	    }else{
	      if(tempR->child[1]->nodekind == expK){
		//numErrors++;
		//printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tempR->child[0]->id);
	      }
	    }
	    
	  }
	  if(tempR->child[1] != NULL){

	    temp2 = symbolTable->lookup(getID(tempR->child[1]));
	    node2 = static_cast <astNode *>(temp2);
	    if(node2 != NULL){
	      tempR->child[1]->expType =node2->expType;
	    }
	    else{
              //numErrors++;
              //printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno,tempR->child[1]->id);
	    }
	    semanticAnalysis(tempR->child[1]->child[0], symbolTable, IOTree, Enter);
	  }

	}

	tempR=tempR->child[1];

      }
      //the first while is to set the type of the last variable of the string of :=
      
      tempR=tree;
      
      while(tempR->child[1]!= NULL){
	if(tempR->child[0]!= NULL){

	  //printf("the %d id is %s\n", tempR->lineno, tempR->child[0]->id);
	
	  if( strcmp(tempR->child[0]->id, "[")==0){

	    void * temp;
	    astNode * node;
	    temp = symbolTable->lookup(tempR->child[0]->child[0]->id);
	    node = static_cast<astNode *>(temp);

	    if(node!= NULL){
	      node -> isInitialized=true;
	    }
	    tempR->child[0]->isArray =true;
	    semanticAnalysis(tempR->child[0], symbolTable, IOTree,  Enter);
	   
	    }

	  if(tempR->child[1]->attr.op != ASSOp){
	    semanticAnalysis(tempR->child[1], symbolTable, IOTree, Enter);

	  }
	  tempR->expType = tempR->child[0]->expType;
	  // if(tempR->child[0]->attr.op == leftSquareBracOp){
	  //semanticAnalysis(tempR->child[0], symbolTable); 
	  //}
	}
	tempR=tempR->child[1];
	}
      //the third while is to set all the types of the whole series of the :=
      tempR=tree;
      
      while(tempR->child[1] != NULL){
	if(tempR->child[1]->attr.op == ASSOp){
          if(tempR->child[0] != NULL){
            temp1 = symbolTable->lookup(getID(tempR->child[0]));
            node1 = static_cast <astNode *>(temp1);

            if(node1 != NULL){
              tempR->child[0]->expType = node1->expType;
              tempR->expType =node1->expType;
              tempR3 = tempR;
	    }
          }
        }

	tempR=tempR->child[1];
      }
      

      if(tree->child[0]!= NULL){
	if(tree->child[0]->attr.op == leftSquareBracOp){
	  //semanticAnalysis(tree->child[0], symbolTable, IOTree, Enter);
	  //printf("check for %d[\n", tree->lineno);
	}
      }

      if(tree->child[1]!=NULL){
	if(tree->attr.op!=ASSOp){
	  temp2 =symbolTable->lookup(getID(tree->child[1]));
	  node2 =static_cast <astNode *>(temp2);
          tree->child[1]->expType = node2->expType;

	  if(tree->child[0]!= NULL){
	    temp1 = symbolTable->lookup(getID(tree->child[0]));
	    node1 =static_cast <astNode *>(temp1);
	    tree->expType=node1->expType;
	    tree->child[0]->expType=node2->expType;
	  }

	}
      }

      // if(tree->child[0]->nodekind == expK){

      //if(tree->child[0]->subkind.exp == idK){
      temp1 = symbolTable->lookup(getID(tree->child[0]));
      node1 = static_cast <astNode *>(temp1);
      
      if(node1 == NULL){

	if(strcmp(tree->child[0]->id,"[")==1){
	numErrors++;
	printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[0]->id);
	//tree->child[0]->child[0]->isArray =true;
	}
	else{
	  tree->child[0]->child[0]->isArray =true;
	}
      }
      //	}
      // }


      // if(tree->child[0]->nodekind == expK){

      //  if(tree->child[0]->subkind.exp == idK){
      temp2 = symbolTable->lookup(getID(tree->child[1]));
      node2 = static_cast <astNode *>(temp2);

      if(node2 == NULL){
	if(strcmp(tree->child[1]->id,"[")==1){
        numErrors++;
        printf("ERROR(%d): Symbol '%s' is not declared.\n", tree->lineno, tree->child[1]->id);
	tree->child[1]->child[0]->isArray = true;
	}else{
	  if(tree->child[1]->child[0] != NULL){
	  tree->child[1]->child[0]->isArray = true;
	  }
	}
	// }
	//	}
      }else{




      }


    }

    if(tree->attr.op == INCOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode * node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
        tree->child[0]->expType =Int;
	semanticAnalysis(tree->child[0], symbolTable, IOTree,  Enter);
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
        tree->child[1]->expType = Int;
      }

      if(node1 != NULL && node1->isArray == true){
        printf("ERROR(%d): The operation '++' does not work with arrays. \n", tree->lineno);
        numErrors++;
      }
    }
    if(tree->attr.op == DECOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode * node2;
      if(tree->child[0]!= NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 =static_cast <astNode *>(temp1);
        tree->child[0]->expType =Int;
        semanticAnalysis(tree->child[0], symbolTable, IOTree,  Enter);
      }
      if(tree->child[1] != NULL){

        temp2 =symbolTable->lookup(getID(tree->child[1]));
        node2 =static_cast <astNode *>(temp2);
        tree->child[1]->expType = Int;
      }

      if(node1 != NULL && node1->isArray == true){
        printf("ERROR(%d): The operation '--' does not work with arrays. \n", tree->lineno);
        numErrors++;
      }
    }
    if(tree->attr.op == ADDASSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }



      if(node1 != NULL){
	tree->child[0]->memLoc = node1->memLoc;
	tree->child[0]->size = node1->size;
        if(node1->isArray == true){
          printf("ERROR(%d): The operation '+=' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '+=' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '+=' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '+=' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }

    if(tree->attr.op == ASSOp){

      void * temp1;
      temp1 = symbolTable ->lookup(getID(tree->child[0]));
      astNode * node1;
      node1 = static_cast<astNode *>(temp1);

      void * temp2;
      temp2 = symbolTable ->lookup(getID(tree->child[1]));
      astNode * node2;
      node2 = static_cast<astNode *>(temp2);

      if(node1 != NULL && node2 != NULL){
	if(node1->isArray == true && node2 -> isArray == false && tree->child[0]->child[1] == NULL && tree->child[1]->child[1] == NULL){

          numErrors++;
          printf("ERROR(%d): ':=' requires both operands be array or not but the lhs is an array and rhs is not an array.\n", tree->lineno);

        }
      }

      if(node1!= NULL && node2!= NULL){

        if(node1->isArray == false && node2->isArray == true && tree->child[0]->child[1] == NULL && tree->child[1]->child[1] == NULL){

          numErrors++;
          printf("ERROR(%d): ':=' requires both operands be array or not but the lhs is not an array and rhs is an array.\n", tree->lineno);

        }

      } 
      astNode *tempR = tree;
      while(tempR->child[1]!= NULL){

	if(tempR->child[1]->attr.op != ASSOp){

	  if(tempR->child[0]!= NULL && tempR->child[1] != NULL){
	    //printf("Im %d %s here in while\n", tempR->lineno, tempR->child[1]->id);
	    //printExp(tempR->child[1],true);
	    void * temp1;
	    temp1 = symbolTable ->lookup(getID(tempR->child[0]));
	    astNode * node1;
	    node1 = static_cast<astNode *>(temp1);

	    if(node1 != NULL){
	      if(node1->nodekind == declK){
	       if(node1-> subkind.decl == funcK){
		 //numErrors++;
		 //printf("ERROR(%d): Cannot use function '%s' as a variable.\n", tempR->lineno, node1->id);
		 tempR = tempR->child[1];
		 continue;
	      }
	      //printf("%s\n", node1->id);
	      }
	    }
	    
	    if(tempR->child[0]->expType != tempR->child[1]->expType){
	      printf("ERROR(%d): ':=' requires operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno, convExpType(tempR->child[0]->expType), convExpType(tempR->child[1]->expType));
	      numErrors++;
	    }
	  }
	  

	}

        tempR = tempR->child[1];
      }

      /*
      printf("Im %d %s here\n", tree->lineno, tree->child[1]->id);
      if(node1 != NULL && node2 != NULL){
	printf("Im %d here\n", tree->lineno);
	if(node1 -> expType != node2 ->expType){
	  printf("ERROR(%d): ':=' requires both operands of the same type but lhs is type %s and rhs is type %s.\n", tree->lineno, convExpType(node1->expType), convExpType(node2->expType));
	}
      }
      else{

      }*/
    }
    if(tree->attr.op == MULASSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '*=' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '*=' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '*=' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '*=' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }

    if(tree->attr.op == SUBASSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '-=' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '-=' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '-=' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '-=' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }
    if(tree->attr.op == DIVASSOp){
      void * temp1 = NULL;
      void * temp2 = NULL;
      astNode * node1;
      astNode *node2;
      if(tree->child[0] != NULL){
        temp1 = symbolTable->lookup(getID(tree->child[0]));
        node1 = static_cast <astNode *>(temp1);
      }
      if(tree->child[1] != NULL){

        temp2 = symbolTable->lookup(getID(tree->child[1]));
        node2 = static_cast <astNode *>(temp2);
      }
      if(node1 != NULL){

        if(node1->isArray == true){
          printf("ERROR(%d): The operation '/=' does not work with arrays. \n", tree->lineno);
          numErrors++;
        }else{
          if(node2 != NULL && node2->isArray == true){
            printf("ERROR(%d): The operation '/=' does not work with arrays. \n", tree->lineno);
            numErrors++;
          }
        }
      }
      if(node1 != NULL){

        if(node1 ->expType != Int){

          printf("ERROR(%d): '/=' requires operands of type int but lhs is of type %s.\n", tree->lineno, convExpType(node1->expType));
        }

      }
      if(node2 != NULL){

        if(node2 ->expType != Int){

          printf("ERROR(%d): '/=' requires operands of type int but rhs is of type %s.\n", tree->lineno, convExpType(node2->expType));
        }

      }
    }

    break;
  case boolConstK:
    if(symbolTable->lookup(tree->id)){
      tree->scope = Scope;
      symbolTable->insert(tree->id, tree);
    }
    break;
  case charConstantK:
    if(!symbolTable->lookup(tree->id)){
      tree->scope = Scope;
      symbolTable->insert(tree->id, tree);
    }
    break;
  }

}

char * getID(astNode * tree){
  
  while(tree->id == NULL){
    tree = tree->child[0];
  }
  return tree->id;
}

ExpType checkType(astNode * tree){

  while(tree->expType == Void){

    if(tree->child[0] != NULL){
      tree =  tree->child[0];
    }else{
      break;
    }
  }

  return tree->expType;
}

char * getType(astNode *tree){

  char * type;

  switch(tree->expType){
  case Void:
    type = strdup("void");
    break;
  case Int:
    type = strdup("int");
    break;
  case Bool:
    type = strdup("bool");
    break;
  case Char:
    type = strdup("char");
    break;
  default:
    type = strdup("cant get type");
    break;
  }

  return type;
}

char * convExpType(ExpType expType){
  char * type = strdup("undefined type");
  if(expType == Void)
    type = strdup("void");
  if(expType == Int)
    type = strdup("int");
  if(expType == Bool)
    type = strdup("bool");
  if(expType == Char)
    type = strdup("char");

  return type;
 }

void andorCheck(astNode * tree, SymbolTable * st){


}
