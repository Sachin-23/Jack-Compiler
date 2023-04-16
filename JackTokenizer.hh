#include <iostream>
#include <fstream>

#include "JackTokens.hh"

class JackTokenizer {
  private:
    std::ifstream inFile;
    char c;
    std::string token;
    enum::tokenType tType;
    uint64_t line = 0;

    // There is no '/' operator
    bool isSymbol(char c) {
      return c == '(' || c == ')' 
          || c == '{' || c == '}'
          || c == '[' || c == ']'
          || c == '+' || c == '-' || c == '*'
          || c == '~' || c == '&' || c == '|' 
          || c == '<' || c == '>' || c == '='
          || c == '.' || c == ',' || c == ';';
    }

    bool isAlpha(char c) {
      return (c >= 'a' && c <= 'z') 
              || (c >= 'A' && c <= 'Z') 
              || c == '_';
    }

    bool isDigit(char c) {
      return (c >= '0' && c <= '9');
    }

    bool isAlphaNumeric(char c) {
      return isAlpha(c) || isDigit(c);
    }

  public: 
    void init(std::string path) {
      inFile.open(path);
      if (!inFile) {
        throw std::runtime_error(std::string("Failed to open file: " + path));
      }

    }
    ~JackTokenizer() {
      inFile.close(); 
    }
    
    bool hasMoreTokens() {
      while ((c = inFile.get()) != EOF) {
        switch(c) {
          case '/': 
            // Skip comments
            if (inFile.peek() == '/') {
              while ((c=inFile.get()) != EOF 
                && c != '\n')
                ;
            }
            else if (inFile.peek() == '*') {
              while ((c=inFile.get()) != EOF 
                && !(c == '*' && inFile.peek() == '/'))
                ;
              inFile.get();
            }
            else 
              // if it is a '/' operator
              return true;
            break;
          case ' ':
          case '\r':
          case '\t':
            // Skip blank spaces
            break; 
          case '\n':
            ++line;
            break;
          default: return true;
        }
      }
      return false;
    }

    std::string advance() {
      token.clear();
      if (isSymbol(c)) {
        token = c; 
        tType = tokenType::SYMBOL;
      }
      else if (c == '"') {
        while ((c=inFile.get()) && isAlphaNumeric(c))
          token += c;
        tType = tokenType::STR_CONST;
      }
      else if (isAlpha(c)) {
        token += c; 
        // Longest maximal munch
        while (isAlphaNumeric(inFile.peek()) && (c=inFile.get()))
          token += c;
        // NULL pointer is returned when key doesn't exists
        if (keyWords.find(token) != keyWords.end()) {
          tType = tokenType::KEYWORD;
        }
        else
          tType = tokenType::IDENTIFIER;
      }
      else if (isDigit(c)) {
        token += c; 
        while (isDigit(inFile.peek()) && (c=inFile.get()))
          token += c;
        tType = tokenType::INT_CONST;
      }
      return token;
    }

     
    // return token type   
    enum::tokenType tokenType() {
      return tType;
    }

    enum::keyWord keyWord() {
      return keyWords[token];
    }

    // return symbol if tokentype is symbol
    char symbol() {
      return token[0];
    }

    // return symbol if tokentype is symbol
    std::string identifier() {
      return token;
    }

    // return identifier if tokentype is identifier
    int16_t intVal() {
      /* check the int range */
      return std::stoi(token);
    }

    // return symbol if tokentype is symbol
    std::string stringVal() {
      return token;
    }

    // For debugging purposes
    uint64_t curLine() {
      return line;
    }
};

