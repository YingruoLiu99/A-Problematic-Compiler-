%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "scanType.h"
#include "parser.tab.h" 
#include "yyerror.h"

int line = 1;
char * lastToken = strdup("last");
static int setValue(int linenum, int tokenClass, char *svalue){

    //create
    yylval.tokenData = new TokenData;

    //fill it up
    yylval.tokenData->linenum = linenum;
    yylval.tokenData->tokenclass = tokenClass;
    yylval.tokenData->tokenstr = strdup(svalue); // duplicate
  
    switch(tokenClass){
    case BOOLCONST:
    if(svalue == "true"){
    yylval.tokenData->nvalue = 1;                                                                                                                             
    }
    else{
     yylval.tokenData->nvalue = 0;
    } 
    break;
    case NUMCONST:
    yylval.tokenData->nvalue = atoi(svalue);        
    break;

    case CHARCONST:
    yylval.tokenData->cvalue = svalue[1];
    break;

    }
   switch(tokenClass){
   case ERROR:

   break;

   default:

   lastToken = strdup(svalue);
   break;

   } 

    return tokenClass;

}

%}

%option noyywrap

letter          [a-zA-Z]
digit           [0-9]
NUMCONST        [0-9]+
ID              [a-zA-Z][a-zA-Z0-9]*
CHARCONST       \'([^\n\\\']|\\.)?\'
WARNCHAR        \'([^\'\n\'\\]|\\.)([^\'\n\\]|\\.)+\'
STRINGCONST     \"([^\"\n]|\\.)*\"

%%

{CHARCONST}       { return setValue(line, CHARCONST, yytext);}                                                                                                                                           
{WARNCHAR}        { return setValue(line, WARNCHAR, yytext);}                                                                                                                                            
"true"            { return setValue(line, BOOLCONST, yytext); }                                                                                                                                          
"false"           { return setValue(line, BOOLCONST, yytext); }                                                                                                                                          
"char"            { return setValue(line, CHAR, yytext);}                                                                                                                                                
"int"             { return setValue(line, INT, yytext);}                                                                                                                                                 
"bool"            { return setValue(line, BOOL, yytext);}                                                                                                                                                
"static"          { return setValue(line, STATIC, yytext);}                                                                                                                                              
"if"              { return setValue(line, IF, yytext);}                                                                                                                                                  
"then"            { return setValue(line, THEN, yytext);}                                                                                                                                                
"else"            { return setValue(line, ELSE, yytext);}                                                                                                                                                
"do"              { return setValue(line, DO, yytext);}                                                                                                                                                  
"return"          { return setValue(line, RETURN, yytext);}                                                                                                                                              
"while"           { return setValue(line, WHILE, yytext);}                                                                                                                                               
"break"           { return setValue(line, BREAK, yytext);}                                                                                                                                               
"or"              { return setValue(line, OR, yytext);}                                                                                                                                                  
"for"             { return setValue(line, FOR, yytext);}                                                                                                                                                 
"and"             { return setValue(line, AND, yytext);}                                                                                                                                                 
"not"             { return setValue(line, NOT, yytext);}                                                                                                                                                 
"to"              { return setValue(line, TO, yytext);}                                                                                                                                                  
"by"              { return setValue(line, BY, yytext);}                                                                                                                                                  
"then"            { return setValue(line, THEN, yytext);}                
{NUMCONST}            { return setValue(line, NUMCONST, yytext);}
{ID}              { return setValue(line, ID, yytext);}
")"               {  return setValue(line, rightBrac, yytext);}                                                                                                                                          
"("               {  return setValue(line, leftBrac, yytext);}                                                                                                                                           
"!="              {  return setValue(line, NOTEQ, yytext);}                                                                                                                                           
"<="              {  return setValue(line, LEQ, yytext);}                                                                                                                                                
"<"               {  return setValue(line, LESS, yytext);}                                                                                                                                           
">="              {  return setValue(line, GEQ, yytext);}                                                                                                                                                
">"               {  return setValue(line, GREAT, yytext);}                                                                                                                                           
"="               {  return setValue(line, ASS, yytext);}                                                                                                                                           
"+="              {  return setValue(line, ADDASS, yytext);}                                                                                                                                             
"-="              {  return setValue(line, SUBASS, yytext);}                                                                                                                                           
"++"              {  return setValue(line, INC, yytext);}                                                                                                                                                
"*="              {  return setValue(line, MULASS, yytext);}                                                                                                                                           
"/="              {  return setValue(line, DIVASS, yytext);}                                                                                                                                           
":>:"             {  return setValue(line, MAX, yytext);}                                                                                                                                           
"><"              {  return setValue(line, NEQ, yytext);}                                                                                                                                                
":="              {  return setValue(line, ASGN, yytext);}                                                                                                                                               
":<:"             {  return setValue(line, MIN, yytext);}                                                                                                                                           
"*"               {  return setValue(line, MUL, yytext);}                                                                                                                                           
"+"               {  return setValue(line, PLUS, yytext);}                                                                                                                                           
"-"               {  return setValue(line, Minus, yytext);}                                                                                                                                           
"/"               {  return setValue(line, DIV, yytext);}                                                                                                                                           
"%"               {  return setValue(line, PER, yytext);}                                                                                                                                           
"["               {  return setValue(line, leftSquareBrac, yytext);}                                                                                                                                     
"]"               {  return setValue(line, rightSquareBrac, yytext);}                                                                                                                                    
"--"              {  return setValue(line, DEC, yytext);}                                                                                                                                                
"?"               {  return setValue(line, QUE, yytext);}                                                                                                                                           
":"               {  return setValue(line, COLON, yytext);}                                                                                                                                           
";"               {  return setValue(line, SEMI, yytext);}                                                                                                                                           
"{"               {  return setValue(line, leftCurlBrac, yytext);}                                                                                                                                       
"}"               {  return setValue(line, rightCurlBrac, yytext);}                                                                                                                                      
","               {  return setValue(line, COMMA, yytext);} 
\n                { line++;}  
[ \t]             ;
\/\/.*            ; 
{STRINGCONST}     { return setValue(line, STRINGCONST, yytext);}
.                 { return setValue(line, ERROR, yytext);}

%%
