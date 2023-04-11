#include <iostream>
#include <fstream>

#include "JackTokens.hh"

class JackTokenizer {
  private:
    std::ifstream inFile;
    std::string token;
    // Track current line - For error reporting
    uint64_t lineCount = 0;
    bool sComment = false, mComment = false;
    char cur;
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
        if (c < 48 || c > 57)
          return false;
      }
      return true;
    }

    bool isSTRING_CONST(std::string token) {
      // STRING_CONST token will have "str" 
      std::string::size_type n = token.size()-1; 
      if (token[0] != '"' || token[n] != '"') return false;
      for(std::string::size_type i = 1; i < n; ++i) {
        // from ascii table
        if (token[i] < 31 || token[i] > 126) {
          return false; 
        }
      }
      return true;
    }

    bool isIDENTIFIER(std::string token) {
      if ((token[0] >= '0' && token[0] <= '9')) return false;
      for(std::string::size_type i = 1; i < token.size(); ++i) {
        if (!((token[i] >= '0' && token[i] <= '9') 
              || (token[i] >= 'a' && token[i] <= 'z') 
              || (token[i] >= 'A' && token[i] <= 'Z') 
              || token[i] == '_'))
         return false; 
      }
      return true;
    }

  public:
    JackTokenizer() {};

    // path to the jack file
    void init(std::string path) {
      if (inFile.is_open()) {
        std::cerr << "There is already a file processing: " << path << std::endl;
        exit(1);
      }
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
          ++lineCount;
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
      else {
        std::cerr << "Error at " << lineCount << " with token: " << token << std::endl;
        exit(1);
      }
    }
     
    // return token type   
    enum::tokenTypes tokenType() {
      return tType;
    }

    enum::keyWords keyWord() {
      return keyWordTypes[token];
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
      return token.substr(1, token.size()-2);
    }

    // For debug purposes
    std::string getToken() {
      return token;
    }

    ~JackTokenizer() {
      inFile.close(); 
    }
};
