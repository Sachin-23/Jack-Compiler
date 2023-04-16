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
      std::string tab;
      for (uint64_t i = 0; i < depth; ++i) {
        tab += "  "; 
      }
      outFile << tab;
    }

    void printError(std::string token) {
      std::cerr << "Syntax Error at " << tokenizer.curLine() 
                << ", found token: '" << currentToken 
                << "', looking for: '" << token << "'" << std::endl;
      exit(1);
    }

    void printXMLToken(std::string token) {
      printTab();
      switch(tokenizer.tokenType()) {
        case tokenType::KEYWORD:  
          outFile << "<keyword> " << token << " </keyword>" << std::endl;
          break; 
        case tokenType::SYMBOL: 
          outFile << "<symbol> " << token << " </symbol>" << std::endl;
          break; 
        case tokenType::IDENTIFIER: 
          outFile << "<identifier> " << token << " </identifier>" << std::endl;
          break; 
        case tokenType::INT_CONST: 
          outFile << "<intCONST> " << token << " </intCONST>" << std::endl;
          break; 
        case tokenType::STR_CONST: 
          outFile << "<stringCONST> " << token << " </stringCONST>" << std::endl;
          break; 
      }
    }

    void eat(std::string token) {
      if (token == currentToken) {
        printXMLToken(token);
      }
      else {
        printError(token);
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
      while (currentToken == "static" || currentToken == "field")
        compileClassVarDec();   
      while (currentToken == "constructor" || currentToken == "function" 
              || currentToken == "method")
        compileSubroutine();   
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
        printError("static|field");
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        printError("varName");
      while (currentToken == ",") {
        eat(",");
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          eat(currentToken);
        else
          printError("varName");
      }
      eat(";");
      --depth;
      printTab();
      outFile << "</classVarDec>\n";
    }

    /* Why should I implement this is in the particular*/
    /* check this */
    void compileType() {
      // fileName should be the className
      if (currentToken == "int")
        eat("int");
      else if (currentToken == "char")
        eat("char");
      else if (currentToken == "boolean")
        eat("boolean");
      else if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        eat("int|char|boolean|className");
    }

    /* Why should I implement this is in the particular*/
    /* check this */
    void compileType(std::string eType) {
      // fileName should be the className
      if (currentToken == "int")
        eat("int");
      else if (currentToken == "char")
        eat("char");
      else if (currentToken == "boolean")
        eat("boolean");
      else if (currentToken == eType)
        eat(currentToken);
      else if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        printError("int|char|boolean|className" + eType);
    }

    void compileSubroutine() {
      printTab();
      outFile << "<subroutineDec>\n";
      ++depth;
      if (currentToken == "constructor") {
        eat("constructor");
        if (currentToken == fileName)
          eat(currentToken);
        else
          printError(fileName);
      }
      else if (currentToken == "function") {
        eat("function");
        // And other class name too will come here.
        compileType("void");
      }
      else if (currentToken == "method") {
        eat("method");
        // And other class name too will come here.
        compileType("void");
      }
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        printError("subroutineName");
      eat("(");
      compileParameterList();
      eat(")");
      compileSubroutineBody(); 
      --depth;
      printTab();
      outFile << "</subroutineDec>\n";
    }

    void compileParameterList() {
      printTab();
      outFile << "<parameterList>\n";
      ++depth;
      if (currentToken != ")") {
        compileType();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          eat(currentToken);
        else
          printError("varName");
        while (currentToken == ",") {
          eat(",");
          compileType();
          if (tokenizer.tokenType() == tokenType::IDENTIFIER)
            eat(currentToken);
          else
            printError("varName");
        }
      }
      --depth;
      printTab();
      outFile << "</parameterList>\n";
    }

    void compileSubroutineBody() {
      printTab();
      outFile << "<subroutineBody>\n";
      ++depth;
      eat("{");
      while (currentToken == "var")
        compileVarDec();
      compileStatements();
      eat("}");
      --depth;
      printTab();
      outFile << "</subroutineBody>\n";
    }

    void compileVarDec() {
      printTab();
      outFile << "<varDec>\n" ;
      ++depth;
      eat("var");
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        printError("varName");
      while (currentToken == ",") {
        eat(",");
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          eat(currentToken);
        else
          printError("varName");
      }
      eat(";");
      --depth;
      printTab();
      outFile << "</varDec>\n" ;
    } 

    void compileStatements() {
      printTab();
      outFile << "<statements>\n" ;
      ++depth;
      while (currentToken == "let" || currentToken == "do"
              || currentToken == "while" || currentToken == "return" 
              || currentToken == "if") {
        if (currentToken == "let") {
          compileLet(); 
        }
        else if (currentToken == "if") {
          compileIf();
        }
        else if (currentToken == "while") {
          compileWhile();
        }
        else if (currentToken == "do") {
          compileDo();
        }
        else if (currentToken == "return") {
          compileReturn();
        }
        else {
          eat("let|if|while|do|return");
        }
      }
      --depth;
      printTab();
      outFile << "</statements>\n";
    }

    void compileLet() {
      printTab();  
      outFile << "<letStatement>\n";
      ++depth;
      eat("let");
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        printError("varName");
      if (currentToken == "[") {
        eat("[");
        compileExpression();
        eat("]");
      } 
      eat("=");
      compileExpression();
      eat(";");
      --depth;
      printTab();
      outFile << "</letStatement>\n";
    }

    void compileIf() {
      printTab();  
      outFile << "<ifStatement>\n";
      ++depth;
      eat("if");
      eat("(");
      compileExpression();
      eat(")");
      eat("{");
      compileStatements();
      eat("}");
      if (currentToken == "else") {
        eat("else");
        eat("{");
        compileStatements();
        eat("}");
      }
      --depth;
      printTab();
      outFile << "</ifStatement>\n";
    }

    void compileWhile() {
      printTab();  
      outFile << "<whileStatement>\n";
      ++depth;
      compileExpression();
      eat("{");
      compileStatements();
      eat("}");
      --depth;
      printTab();
      outFile << "</whileStatement>\n";
    }

    void compileDo() {
      printTab();  
      outFile << "<doStatement>\n";
      ++depth;
      eat("do");
      compileSubroutineCall();
      eat(";");
      --depth;
      printTab();
      outFile << "</doStatement>\n";
    }

    // remove this
    void compileSubroutineCall() {
      /* How to know if subroutineName is used ? */
      /* subroutine doesn't have . operator */
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        eat(currentToken);
      else
        printError("subroutineName|className|varName");
      if (currentToken == ".") {
        eat(".");
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          eat(currentToken);
        else
          printError("subroutineName");
      }
      eat("(");
      compileExpressionList();
      eat(")");
    }


    void compileReturn() {
      printTab();  
      outFile << "<returnStatement>\n";
      ++depth;
      eat("return");
      if (currentToken != ";")
        compileExpression();
      eat(";");
      --depth;
      printTab();
      outFile << "</returnStatement>\n";
    }

    void compileExpressionList() {
      printTab();  
      outFile << "<expressionList>\n";
      ++depth;
      if (currentToken != ")") {
        compileExpression(); 
        while (currentToken == ",")  {
          eat(",");
          compileExpression(); 
        }
      }
      --depth;
      printTab();
      outFile << "</expressionList>\n";
    }

    void compileExpression() {
      printTab();  
      outFile << "<expression>\n";
      ++depth;
      compileTerm();
      --depth;
      printTab();
      outFile << "</expression>\n";
    }

    void compileTerm() {
      printTab();  
      outFile << "<term>\n";
      ++depth;
      eat(currentToken);
      --depth;
      printTab();
      outFile << "</term>\n";
    }
};
