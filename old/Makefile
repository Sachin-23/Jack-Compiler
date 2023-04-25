CC=g++
CFLAGS= -std=c++11 -Wall -Wextra -O0

all: build

build: JackCompiler.cc JackTokenizer.hh JackTokens.hh SymbolTable.hh
	$(CC) $(CFLAGS) JackCompiler.cc -o JackCompiler

submit: 
	zip -R project10 Makefile *.cc *.hh lang.txt

clean:
	rm -r JackAnalyzer *.dSYM project10.zip

