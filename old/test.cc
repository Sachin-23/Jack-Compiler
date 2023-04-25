#include <iostream>

#include "JackTokenizer.hh"

int main(int argc, char *argv[]) {
  JackTokenizer tokenizer(argv[1]); 
  while (tokenizer.hasMoreTokens()) {
    std::cout << tokenizer.advance() << std::endl;;
  }
  return 0;
}

