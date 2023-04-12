#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>

#include "JackTokenizer.hh"

class CompilationEngine {
  private: 
    std::string fileName;
    std::ofstream outFile;
    JackTokenizer tokenizer;
    std::string currentToken;
    uint64_t depth = 0;

    void printTab() {
      std::string tab = "\t";
      for (uint64_t i = 0; i < depth-1; i++) {
        tab += tab; 
      }
      outFile << tab;
    }

    void printXMLToken(std::string token) {
      printTab();
      switch(tokenizer.tokenType()) {
        case tokenTypes::KEYWORD:  
          outFile << "<keyword>\t" << token << "\t</keyword>" << std::endl;
          break; 
        case tokenTypes::SYMBOL: 
          outFile << "<symbol>\t" << token << "\t</symbol>" << std::endl;
          break; 
        case tokenTypes::IDENTIFIER: 
          outFile << "<identifier>\t" << token << "\t</identifire>" << std::endl;
          break; 
        case tokenTypes::INT_CONST: 
          outFile << "<intCONST>\t" << token << "\t</intCONST>" << std::endl;
          break; 
        case tokenTypes::STR_CONST: 
          outFile << "<stringCONST>\t" << token << "\t</stringCONST>" << std::endl;
          break; 
      }
    }

    void eat(std::string token) {
      if (token == currentToken) {
        printXMLToken(token);
      }
      else {
        std::cerr << "Syntax Error at " << tokenizer.curLine() 
                  << ", found token: '" << currentToken 
                  << "', looking for: '" << token << "'" << std::endl;
        exit(1);
      }
      if (tokenizer.hasMoreTokens()) {
        currentToken = tokenizer.advance();
      }
    }
     
  public:
    CompilationEngine(std::string path) {
      tokenizer.init(path);

      // check if file contains any tokens
      if (!tokenizer.hasMoreTokens()) {
        std::cerr << "Empty file" << std::endl;
        exit(1);
      }
       
      // Get the current token
      currentToken = tokenizer.advance(); 

      // Remove file extension
      fileName = path.substr(0, path.find_last_of("."));

      std::cout << "Path " << path << " --> " << fileName << ".vm" << std::endl; 
      
      outFile.open(fileName + ".vm");

      if (!outFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + fileName + ".vm");

      // Get the file name only
      fileName = fileName.substr(fileName.find_last_of("/")+1, fileName.length()); 
    }
    ~CompilationEngine() {
      outFile.close();
    }

    void compileClass() {
      outFile << "<class>\n";
      ++depth;
      eat("class");
      eat(fileName);
      eat("{");
      compileClassVarDec();   
      eat("}");
      --depth;
      outFile << "</class>\n";
    }

    void compileClassVarDec() {
      printTab();
      outFile << "<classVarDec>\n";
      ++depth;
      if (currentToken == "static") 
        eat("static");
      else if (currentToken == "field") 
        eat("field");
      else
        eat("static|field");
      --depth;
      outFile << "</classVarDec>\n";
    }

    void compileTerm() {
      if (currentToken == "int")
        eat("int");
      else if (currentToken == "char")
        eat("char");
      else if (currentToken == "boolean")
        eat("boolean");
      // fileName should be the className
      else if (currentToken == fileName)
        eat(fileName);
      else
        eat("int|char|boolean|" + fileName);
    }


    /*
      compileSubroutine
      compileParameterList
      compileSubroutineBody
      compileVarDec
      compileStatements
      compileLet
      compileIf
      compileWhile
      compileDo
      compileReturn
      compileExpression
      compileExpressionList
    */
};
