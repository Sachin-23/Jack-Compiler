#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>

#include "JackTokenizer.hh"
#include "SymbolTable.hh"

class CompilationEngine {
  private: 
    std::string fileName;
    std::ofstream outFile;

    JackTokenizer tokenizer;

    std::string currentToken;

    // false if in class, else in subroutine
    bool subLevel=false;
    uint64_t depth = 0;

    SymbolTable classTable;
    SymbolTable subTable;

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

    bool isOp() {
      return currentToken == "+" || currentToken == "-"
              || currentToken == "*" || currentToken == "/"
              || currentToken == "&" || currentToken == "|"
              || currentToken == "<" || currentToken == ">"
              || currentToken == "=";
    }

    void printXMLToken() {
      printTab();
      SymbolTable curTable;
      switch(tokenizer.tokenType()) {
        case tokenType::KEYWORD:  
          outFile << "<keyword> " << currentToken << " </keyword>";
          break; 
        case tokenType::SYMBOL: 
          outFile << "<symbol> " << tokenizer.stringVal() << " </symbol>";
          break; 
        case tokenType::IDENTIFIER: 
          if (subLevel) {
            curTable = subTable;
          }
          else {
            curTable = classTable;
          } 
          outFile << "<identifier type='" << curTable.typeOf(currentToken);
          outFile << "' kind='"; 
          switch(curTable.kindOf(currentToken)) {
            case kind::STATIC: outFile << "static";
              break;
            case kind::FIELD: outFile << "field";
              break;
            case kind::VAR: outFile << "var";
              break;
            case kind::ARG: outFile << "arg";
              break;
          }
          outFile << "' index='" << curTable.indexOf(currentToken) << "'>"
                  << currentToken << "</identifier>";
          break; 
        case tokenType::INT_CONST: 
          outFile << "<integerConstant> " << currentToken<< " </integerConstant>";
          break; 
        case tokenType::STR_CONST: 
          outFile << "<stringConstant> " << currentToken << " </stringConstant>";
          break; 
      }
      outFile << std::endl;
    }

    void printAndAdvance() {
      printXMLToken();
      if (tokenizer.hasMoreTokens()) {
        currentToken = tokenizer.advance();
      }
    }

    void eat(std::string token) {
      if (token == currentToken) {
        printXMLToken();
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

      std::cout << "Path " << path << " --> " << fileName << ".xml" << std::endl; 
      
      outFile.open(fileName + ".xml");

      if (!outFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + fileName + ".xml");

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
              || currentToken == "method") {
        // Compiling in subroutine level
        subLevel = true;
        compileSubroutine();   
        subTable.reset();
        // subtable is cleared
      }
      // Now at class level
      subLevel = false;
      eat("}");
      // class table is cleared 
      classTable.reset();
      --depth;
      outFile << "</class>\n";
    }

    void compileClassVarDec() {
      printTab();
      outFile << "<classVarDec>\n";
      ++depth;

      std::string type;
      enum::kind kindOf;  

      switch(tokenizer.keyWord()) {
        case keyWord::STATIC:
          kindOf = kind::STATIC;
          printAndAdvance();
          break;
        case keyWord::FIELD:
          kindOf = kind::FIELD;
          printAndAdvance();
          break;
        default: 
          printError("static|field");
      }

      // Weird Syntax ?
      if (tokenizer.tokenType() == tokenType::IDENTIFIER
          || (tokenizer.tokenType() == tokenType::KEYWORD
          && (tokenizer.keyWord() == keyWord::INT
              || tokenizer.keyWord() == keyWord::CHAR
              || tokenizer.keyWord() == keyWord::BOOLEAN))) {
        type = currentToken; 
        printAndAdvance(); 
      }
      else
        printError("int|char|boolean|className");

      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        classTable.define(currentToken, type, kindOf);
        printAndAdvance();
      }
      else
        printError("varName");
      while (currentToken == ",") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          classTable.define(currentToken, type, kindOf);
          printAndAdvance();
        }
        else
          printError("varName");
      }
      eat(";");
      --depth;
      printTab();
      outFile << "</classVarDec>\n";
    }

    void compileSubroutine() {
      printTab();
      outFile << "<subroutineDec>\n";
      ++depth;
      switch (tokenizer.keyWord()) {
        case keyWord::CONSTRUCTOR:
          printAndAdvance();
          if (currentToken == fileName)
            printAndAdvance();
          else
            printError(fileName);
          break;
        case keyWord::FUNCTION:
        case keyWord::METHOD:
          printAndAdvance();
          // Weird Syntax ?
          if (tokenizer.tokenType() == tokenType::IDENTIFIER
              || (tokenizer.tokenType() == tokenType::KEYWORD
              && (tokenizer.keyWord() == keyWord::INT
                  || tokenizer.keyWord() == keyWord::CHAR
                  || tokenizer.keyWord() == keyWord::VOID
                  || tokenizer.keyWord() == keyWord::BOOLEAN)))
            printAndAdvance(); 
          else
            printError("int|char|boolean|className");
        default: break;
      }
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        printAndAdvance();
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
      std::string type;
      if (currentToken != ")") {

        // Weird Syntax ?
        if (tokenizer.tokenType() == tokenType::IDENTIFIER
            || (tokenizer.tokenType() == tokenType::KEYWORD
            && (tokenizer.keyWord() == keyWord::INT
                || tokenizer.keyWord() == keyWord::CHAR
                || tokenizer.keyWord() == keyWord::BOOLEAN))) {
          type = currentToken;
          printAndAdvance(); 
        }
        else
          printError("int|char|boolean|className");

        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          subTable.define(currentToken, type, kind::ARG);
          printAndAdvance();
        }
        else
          printError("varName");
        while (currentToken == ",") {
          printAndAdvance();
          // Weird Syntax ?
          if (tokenizer.tokenType() == tokenType::IDENTIFIER
              || (tokenizer.tokenType() == tokenType::KEYWORD
              && (tokenizer.keyWord() == keyWord::INT
                  || tokenizer.keyWord() == keyWord::CHAR
                  || tokenizer.keyWord() == keyWord::BOOLEAN))) {
            type = currentToken;
            printAndAdvance(); 
          }
          else
            printError("int|char|boolean|className");
          if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
            subTable.define(currentToken, type, kind::ARG);
            printAndAdvance();
          }
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
      while (tokenizer.keyWord() == keyWord::VAR)
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
      std::string type;
      // Weird Syntax ?
      if (tokenizer.tokenType() == tokenType::IDENTIFIER
          || (tokenizer.tokenType() == tokenType::KEYWORD
          && (tokenizer.keyWord() == keyWord::INT
              || tokenizer.keyWord() == keyWord::CHAR
              || tokenizer.keyWord() == keyWord::BOOLEAN))) {
        type = currentToken; 
        printAndAdvance(); 
      }
      else
        printError("int|char|boolean|className");

      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        subTable.define(currentToken, type, kind::VAR); 
        printAndAdvance();
      }
      else
        printError("varName");
      while (currentToken == ",") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          subTable.define(currentToken, type, kind::VAR); 
          printAndAdvance();
        }
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
      while (tokenizer.tokenType() == tokenType::KEYWORD) {
        switch (tokenizer.keyWord()) {
          case keyWord::LET:
            compileLet(); 
            break;   
          case keyWord::IF:
            compileIf();
            break;   
          case keyWord::WHILE:
            compileWhile();
            break;   
          case keyWord::DO:
            compileDo();
            break;   
          case keyWord::RETURN:
            compileReturn();
            break;   
          default:  printError("let|if|while|do|return"); 
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
        printAndAdvance();
      else
        printError("varName");
      if (currentToken == "[") {
        printAndAdvance();
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
        printAndAdvance();
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
			eat("while");
			eat("(");
			compileExpression();
			eat(")");
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
			if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        printAndAdvance();
      else
        printError("subroutineName|className|varName");
      if (currentToken == ".") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
					printAndAdvance();
        else
          printError("subroutineName");
      }
      eat("(");
      compileExpressionList();
      eat(")");
      eat(";");
      --depth;
      printTab();
      outFile << "</doStatement>\n";
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

    int compileExpressionList() {
      printTab();  
      outFile << "<expressionList>\n";
      ++depth;
      uint64_t exprs = 0;
      if (currentToken != ")") {
        exprs = 1;
        compileExpression(); 
        while (currentToken == ",")  {
          eat(",");
          compileExpression(); 
          ++exprs;
        }
      }
      --depth;
      printTab();
      outFile << "</expressionList>\n";
      return exprs;
    }

    void compileExpression() {
      printTab();  
      outFile << "<expression>\n";
      ++depth;
      compileTerm();
      if (isOp()) {
        printAndAdvance();
        compileTerm();
      }
      --depth;
      printTab();
      outFile << "</expression>\n";
    }

    void compileTerm() {
      printTab();  
      outFile << "<term>\n";
      ++depth;
			switch (tokenizer.tokenType()) {
        // Notice there is no break
        case tokenType::KEYWORD:
          switch(tokenizer.keyWord()) {
            case keyWord::TRUE:
            case keyWord::FALSE:
            case keyWord::NONE:
            case keyWord::THIS:
              break;
            default: 
              printError("true|false|null|this");
          }
				case tokenType::INT_CONST: 
				case tokenType::STR_CONST: 
					printAndAdvance();
					break;	
				case tokenType::IDENTIFIER:
					printAndAdvance();
          switch (tokenizer.symbol()) {
            case '[':  
              printAndAdvance();
              compileExpression();
              eat("]");
              break;
            // Code reability vs performance ?
            case '.':
              printAndAdvance();
              if (tokenizer.tokenType() == tokenType::IDENTIFIER)
                printAndAdvance();
              // eat("(");
              // compileExpressionList(); 
              // eat(")");
            case '(': 
              printAndAdvance();
              compileExpressionList(); 
              eat(")");
              break;
            default: break; 
          }
					break;
				case tokenType::SYMBOL:
					if (currentToken == "(") {
						printAndAdvance();
						compileExpression();
						eat(")");
					}
					else if (currentToken == "~" || currentToken == "-") {
						printAndAdvance();
						compileTerm();
					}
          break;
        default: printError("Error in term");
			}
      --depth;
      printTab();
      outFile << "</term>\n";
    }
};
