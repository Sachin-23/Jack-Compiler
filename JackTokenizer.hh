#include <iostream>
#include <fstream>

#include "JackTokens.hh"

class JackTokenizer {
  private:
    std::ifstream inFile;
    std::string token;
    bool sComment = false, mComment = false;
    char cur;
    enum::keywords keyword;
    enum::tokenTypes tType;

    bool isKeyword(std::string token) {
      return token == "class" || token ==  "method" || token == "function"
             || token == "constructor" || token == "int" || token == "boolean"
             || token == "char" || token == "void" || token == "var" 
             || token == "static" || token == "field" || token == "let" 
             || token == "do" || token == "if" || token == "else"
             || token == "while" || token == "return" || token == "true"
             || token == "false" || token == "null" || token == "this";
    }

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

    bool isINT_CONST(std::string token) {
      for (char c: token) {
        if (c < 48 || c > 58)
          return false;
      }
      return true;
    }
    
    bool isSTRING_CONST(std::string token) {
      for (char c: token) {
        if (c < 33 || c > 126) {
          return false; 
        }
      }
      return true;
    }

    bool isIDENTIFIER(std::string token) {
      return false;
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
      // Clear the contents
      token.clear();
      // if cur has symbol 
      if (isSymbol(cur) && !sComment && !mComment) {
        token += cur;
        cur = '\0';
        return true;
      }
      while (!inFile.eof()) {
        // Get a char from input file
        cur = inFile.get();
        // Skip if single line comment
        if (sComment) {
          // End of single line comment 
          if (cur == '\n') 
            sComment = false;
          continue;
        }
        // Check for single comment
        if (cur == '/' && inFile.peek() == '/') {
          sComment = true; 
          // Check if there was any token before the comment
          if (!token.empty()) {
            return true;
          }
          continue;
        }
        // Skip if multi-line comment
        if (mComment) {
          // End of multi-line comment 
          if (cur == '*' && inFile.get() == '/') 
            mComment = false;
          continue;
        }
        // Start of multi-line comment
        if (cur == '/' && inFile.peek() == '*') {
          mComment = true; 
          // Check if there was any token before the comment
          if (!token.empty()) {
            return true;
          }
          continue;
        }
        // skip new lines 
        if (cur == '\n') {
          continue;
        }
        if (cur == ' ') {
          if (!token.empty()) 
            return true;
          continue;
        }
        if (isSymbol(cur)) {
          if (!token.empty()) 
            return true;
        }
        token += cur;
      }
      return false;
    }

    void advance() {
      if (isKeyword(token)) {
        tType = tokenTypes::KEYWORD;
      }
      else if (isSymbol(token[0])) {
        tType = tokenTypes::SYMBOL; 
      }
      else if (isINT_CONST(token)) {
        tType = tokenTypes::INT_CONST; 
      }
      else if (isSTRING_CONST(token)) {
        tType = tokenTypes::STR_CONST; 
      }
      else if (isIDENTIFIER(token)) {
        tType = tokenTypes::IDENTIFIER; 
      }
    }
     
    std::string tokenType() {
      return "NOT YET IMPLEMENTED";
    }

    std::string getToken() {
      return token;
    }

    ~JackTokenizer() {
      inFile.close(); 
    }
};
