#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>

#include "JackTokenizer.hh"
#include "SymbolTable.hh"
#include "VMWriter.hh"

class CompilationEngine {
  private: 
    std::string fileName;
    std::string currentToken;
    // Tokenizer object
    JackTokenizer tokenizer;
    // VMWriter object
    VMWriter vmWriter;

    SymbolTable classTable;
    SymbolTable subTable;
    uint64_t classVarCount = 0;

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

    void advance() {
      if (tokenizer.hasMoreTokens())  {
        // Get the current token
        currentToken = tokenizer.advance(); 
      }
    }
     
    void eat(std::string token) {
      if (token != currentToken) {
        printError(token);
      }
      advance();
    }

  public:
    CompilationEngine(std::string path) {
      // Initialize tokenizer
      tokenizer.init(path);

      advance();

      // Initialize vmwriter
      vmWriter.init(path);

      // Remove file extension
      fileName = path.substr(0, path.find_last_of("."));

      // Get the file name only
      fileName = fileName.substr(fileName.find_last_of("/")+1, fileName.length()); 
    }

    ~CompilationEngine() {
    }

    void compileClass() {
      eat("class");
      eat(fileName);
      eat("{");
      while (currentToken == "static" || currentToken == "field") {
        compileClassVarDec();   
        ++classVarCount;
      }
      while (currentToken == "constructor" || currentToken == "function" 
              || currentToken == "method") {
        compileSubroutine();   
        // subtable is cleared
        subTable.reset();
      }
      eat("}");
      // class table is cleared 
      classTable.reset();
    }

    void compileClassVarDec() {
      std::string type;
      enum::kind kindOf;  

      switch (tokenizer.keyWord()) {
        case keyWord::STATIC:
          kindOf = kind::STATIC;
          break;
        case keyWord::FIELD:
          kindOf = kind::FIELD;
          break;
        default: 
          printError("static|field");
      }
      advance();

      // Unstructured
      if (compileType()) {
        type = currentToken; 
        advance();
      }
      else
        printError("int|char|boolean|className");

      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        classTable.define(currentToken, type, kindOf);
        advance();
      }
      else
        printError("varName");
      while (currentToken == ",") {
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          classTable.define(currentToken, type, kindOf);
          advance();
        }
        else
          printError("varName");
      }
      eat(";");
    }

    bool compileType() {
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        return true;
      }
      if (tokenizer.tokenType() == tokenType::KEYWORD) {
        switch (tokenizer.keyWord()) {
          case keyWord::INT:  
          case keyWord::CHAR: 
          case keyWord::BOOLEAN: 
            return true;
          default: break;
        }
      }
      return false;
    }

    void compileSubroutine() {
      std::string functionName = fileName;
      bool isMethod = false;
      switch (tokenizer.keyWord()) {
        case keyWord::CONSTRUCTOR:
          advance(); 
          eat(fileName);
          break;
        // Note precedence of cases matters
        case keyWord::METHOD:
          isMethod = true; 
        case keyWord::FUNCTION:
          advance();
          // Weird Syntax ?
          if (compileType() || tokenizer.keyWord() == keyWord::VOID)
            advance();
          else
            printError("void|int|char|boolean|className");
        default: break;
      }
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        functionName += "." + currentToken;
        advance();
      }
      else
        printError("subroutineName");
      vmWriter.writeFunction(functionName, classVarCount);
      eat("(");
      compileParameterList();
      eat(")");
      compileSubroutineBody(); 
    }

    void compileParameterList() {
      std::string type;
      if (currentToken != ")") {
        if (compileType()) {
          type = currentToken;
          advance();
        }
        else
          printError("int|char|boolean|className");

        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          subTable.define(currentToken, type, kind::ARG);
          advance();
        }
        else
          printError("varName");
        while (currentToken == ",") {
          tokenizer.advance();
          if (compileType()) {
            advance();
          }
          else
            printError("int|char|boolean|className");
          if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
            subTable.define(currentToken, type, kind::ARG);
            advance();
          }
          else
            printError("varName");
        }
      }
    }
    void compileSubroutineBody() {
      eat("{");
      while (tokenizer.keyWord() == keyWord::VAR)
        compileVarDec();
      compileStatements();
      eat("}");
    }

    void compileVarDec() {
      eat("var");
      std::string type;

      if (compileType()) {
        type = currentToken; 
        advance(); 
      }
      else
        printError("int|char|boolean|className");

      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        subTable.define(currentToken, type, kind::VAR); 
        advance();
      }
      else
        printError("varName");

      while (currentToken == ",") {
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          subTable.define(currentToken, type, kind::VAR); 
          advance();
        }
        else
          printError("varName");
      }

      eat(";");
    } 

    void compileStatements() {
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
    }

    void compileLet() {
      eat("let");
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        advance();
      else
        printError("varName");
      if (currentToken == "[") {
        advance();
        compileExpression();
        eat("]");
      } 
      eat("=");
      compileExpression();
      eat(";");
    }

    void compileIf() {
      eat("if");
      eat("(");
      compileExpression();
      eat(")");
      eat("{");
      compileStatements();
      eat("}");
      if (currentToken == "else") {
        advance();
        eat("{");
        compileStatements();
        eat("}");
      }
    }

    void compileWhile() {
			eat("while");
			eat("(");
			compileExpression();
			eat(")");
      eat("{");
      compileStatements();
      eat("}");
    }

    void compileDo() {
      eat("do");
      std::string functionName;  
			if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        functionName += currentToken;
        advance();
      }
      else
        printError("subroutineName|className|varName");

      if (currentToken == ".") {
        functionName += currentToken;
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          functionName += currentToken;
          advance();
        }
        else
          printError("subroutineName");
      }
      eat("(");
      compileExpressionList();
      eat(")");
      vmWriter.writeCall(functionName, 0);
      vmWriter.writePop(segment::TEMP, 0); 
      eat(";");
    }

    void compileReturn() {
      eat("return");
      if (currentToken != ";")
        compileExpression();
      eat(";");
      vmWriter.writeReturn();
    }

    int compileExpressionList() {
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
      return exprs;
    }

    void compileExpression() {
      char op;
      compileTerm();
      if (isOp()) {
        op = tokenizer.symbol();
        advance();
        compileTerm();
        switch(op) {
          case '+': vmWriter.writeArithmetic(command::ADD);   
              break;
          case '-': vmWriter.writeArithmetic(command::SUB); 
              break;
          case '*': vmWriter.writeCall("Math.Multiply", 2); 
              break;
          case '/': vmWriter.writeCall("Math.Divide", 2); 
              break;
          case '&': vmWriter.writeArithmetic(command::AND); 
              break;
          case '|': vmWriter.writeArithmetic(command::NOT); 
              break;
          case '!': vmWriter.writeArithmetic(command::NEG); 
              break;
          case '<': vmWriter.writeArithmetic(command::GT); 
              break;
          case '>': vmWriter.writeArithmetic(command::LT); 
              break;
          case '=': vmWriter.writeArithmetic(command::EQ); 
              break;
        }
      }
    }

    void compileTerm() {
			switch (tokenizer.tokenType()) {
        // Notice there is no break
        case tokenType::KEYWORD:
          switch (tokenizer.keyWord()) {
            case keyWord::TRUE:
            case keyWord::FALSE:
            case keyWord::NONE:
            case keyWord::THIS:
              break;
            default: 
              printError("true|false|null|this");
          }
          advance();
          break;
				case tokenType::INT_CONST: 
          vmWriter.writePush(segment::CONSTANT, tokenizer.intVal());
          advance();
          break;
				case tokenType::STR_CONST: 
					advance();
					break;	
				case tokenType::IDENTIFIER:
					advance();
          switch (tokenizer.symbol()) {
            case '[':  
              advance();
              compileExpression();
              eat("]");
              break;
            // Code reability vs optimization ?
            case '.':
              advance();
              if (tokenizer.tokenType() == tokenType::IDENTIFIER)
                advance();
              // eat("(");
              // compileExpressionList();
              // eat(")");
            case '(': 
              advance();
              compileExpressionList(); 
              eat(")");
              break;
            default: break; 
          }
					break;
				case tokenType::SYMBOL:
					if (currentToken == "(") {
						advance();
						compileExpression();
						eat(")");
					}
					else if (currentToken == "~" || currentToken == "-") {
						advance();
						compileTerm();
					}
          break;
        default: printError("Error in term");
			}
    }
};
