#include <iostream>
#include <fstream>

#include "../JackTokens.hh"

class JackTokenizer {
  private:
    std::ifstream inFile;
    char c;
    std::string token;
    enum::tokenType tType;
    uint64_t line = 1;

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

		bool isChar(char c) {
	    return (c >= 32 && c <= 126);
		}

  public: 
    JackTokenizer() { }
    JackTokenizer(std::string path) {
      init(path);
    }

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
              while ((c=inFile.get()) != EOF && c != '\n') 
                ;
              ++line;
            }
            else if (inFile.peek() == '*') {
              while ((c=inFile.get()) != EOF && !(c == '*' && inFile.peek() == '/'))
                if (c == '\n') ++line;
              inFile.get();
            }
            else 
              return true; // If '/' is an operator
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
				while (c != '"') {
					c = inFile.get();	
					if (isChar(c=inFile.get()) && (c != '"' || token.back() == '/'))
						token += c;
				}
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
      return c;
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


int main(int argc, char *argv[]) {
  JackTokenizer tokenizer;
  tokenizer.init(argv[1]);

  std::string currentToken;

  while (tokenizer.hasMoreTokens()) {

    currentToken = tokenizer.advance();

    switch(tokenizer.tokenType()) {

      case tokenType::KEYWORD:  
        std::cout << "<keyword> " << currentToken << " </keyword>";
        break; 
      case tokenType::SYMBOL: 
        std::cout << "<symbol> " << currentToken << " </symbol>";
        break; 
      case tokenType::IDENTIFIER: 
        std::cout << "<identifier> " << currentToken << " </identifier>";
        break; 
      case tokenType::INT_CONST: 
        std::cout << "<intCONST> " << currentToken<< " </intCONST>";
        break; 
      case tokenType::STR_CONST: 
        std::cout << "<stringCONST> " << currentToken << " </stringCONST>";
        break; 
    }
    std::cout << std::endl;
  }      
  return 0;
}

