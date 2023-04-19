CC=g++
CFLAGS= -std=c++11 -Wall -Wextra -O0

all: build

build: JackAnalyzer.cc JackTokenizer.hh JackTokens.hh 
	$(CC) $(CFLAGS) JackAnalyzer.cc -o JackAnalyzer

submit: 
	zip -R project10 Makefile *.cc *.hh lang.txt

clean:
	rm -r JackAnalyzer *.dSYM project10.zip

