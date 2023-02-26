BIN  = parser
NAME = c-
CC   = g++
CFLAGS = -DCPLUSPLUS -g     # for use with C++ if file ext is .c

SRCS = $(BIN).y $(BIN).l 
HDRS = scanType.h 
OBJS = lex.yy.o $(BIN).tab.o util.o symbolTable.o semantic.o yyerror.o emitcode.o codegen.o
LIBS = -lm 

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

util.o: util.cpp util.h
	$(CC) $(CFLAGS) -c util.cpp

symbolTable.o: symbolTable.cpp symbolTable.h
	$(CC) $(CFLAGS) -c symbolTable.cpp

semantic.o: semantic.cpp semantic.h
	$(CC) $(CFLAGS) -c semantic.cpp

yyerror.o: yyerror.cpp yyerror.h
	$(CC) $(CFLAGS) -c yyerror.cpp

emitcode.o: emitcode.cpp emitcode.h
	$(CC) $(CFLAGS) -c emitcode.cpp

codegen.o: codegen.cpp codegen.h
	$(CC) $(CFLAGS) -c codegen.cpp

lex.yy.c: $(BIN).l $(BIN).tab.h
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c: $(BIN).y
	bison -v -t -d $(BIN).y

all:    
	touch $(SRCS)
	make

clean:
	rm -f $(OBJS) $(BIN) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).tar $(BIN).output *~

tar:
	tar -cvf $(BIN).tar $(SRCS) $(HDRS) util.h util.cpp ourgetopt.h ourgetopt.c semantic.h semantic.cpp symbolTable.h symbolTable.cpp yyerror.h yyerror.cpp codegen.h codegen.cpp emitcode.h emitcode.cpp makefile
	ls -l $(BIN).tar
