CC=clang++
CFLAGS= -std=c++11 -Wall -Wextra
#-O0

all: build

build: JackAnalyzer.cc JackTokenizer.hh JackTokens.hh 
	$(CC) $(CFLAGS) JackAnalyzer.cc -o JackAnalyzer

submit: 
	zip -R project08.zip *.cc *.hh

clean:
	rm -r JackAnalyzer *.dSYM

