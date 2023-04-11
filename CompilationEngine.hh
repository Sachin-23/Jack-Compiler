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

    void eat(std::string token) {
      tokenizer.hasMoreTokens(); 
      if (token == tokenizer.getToken) {
        
      }
    }
     
  public:
    CompilationEngine(std::string path) {
      tokenizer.init(path);

      fileName = path.substr(0, path.find_last_of("."))

      std::cout << "Path " << path << " --> " << fileName << std::endl; 
      
      outFile.open(fileName + ".vm");

      if (!outFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + fileName + ".vm");

      std::cout << "Output file: " << fileName << std::endl; 

    }
    ~CompilationEngine() {
      outFile.close();
    }

    void compileClass() {
      eat("class");
      eat(filename);
      eat("{");
      eat("}");
    }
    /*
      compileClassVarDec 
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
      compileTerm
      compileExpressionList
    */
};
