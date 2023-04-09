#include <iostream>
#include <fstream>

#include "JackTokens.hh"

class JackTokenizer {
  private:
    std::ifstream inFile;
    std::string token;
    uint8_t c; // temp character

    bool isSymbol(char c) {
      return c == '(' || c == ')' 
          || c == '{' || c == '}'
          || c == '[' || c == ']'
          || c == '.' || c == ',' || c == ';' 
          || c == '+' || c == '-' || c == '*' || c == '/'
          || c == '&' || c == '|' 
          || c == '<' || c == '>' 
          || c == '='
          || c == '~';
    }

  public:
    // path to the jack file
    JackTokenizer(std::string path) {
      inFile.open(path, std::ios::in);

      if (!inFile) 
        throw std::runtime_error(std::string("Failed to open file: " + path));
    }

    // Read till spaces or symbol   
    bool hasMoreTokens() {
      bool inComment = false;
      while (!inFile.eof()) {
        // Priority Removal of white spaces and comment then tokenization
        // Comment
        // if c == ' ' skip
        // if c == '/' then check if next char is '*' or // 
          // if * then skip till "*/"
          // if // skip the whole line
        // if c is Symbol then c = symbol
        c = inFile.get();
        if (c == ' ') continue;
        else if (isSymbol(c)) {
           
        }
      }
      return false;
    }

    ~JackTokenizer() {
      inFile.close(); 
    }
};
