#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>

#include "JackTokenizer.hh"
#include "VMWriter.hh"
#include "SymbolTable.hh"

class CompilationEngine {
  private: 
    std::string fileName;
    std::string currentToken;

    SymbolTable subTable, classTable;
    JackTokenizer tokenizer;
    VMWriter vmWriter;

    uint64_t ifCount = 0;
    uint64_t whileCount = 0;
    keyWord funcType;
    std::string funcName;

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
      if (tokenizer.hasMoreTokens()) {
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
      tokenizer.init(path);
      vmWriter.init(path);

      advance();

      // Remove file extension
      fileName = path.substr(0, path.find_last_of("."));

      // Get the file name only
      fileName = fileName.substr(fileName.find_last_of("/")+1, fileName.length()); 
    }

    ~CompilationEngine() { }

    void compileClass() {
      eat("class");
      eat(fileName);
      eat("{");
      while (currentToken == "static" || currentToken == "field")
        compileClassVarDec();   
      while (currentToken == "constructor" || currentToken == "function" 
              || currentToken == "method") {
        compileSubroutine();   
        subTable.reset();
        ifCount = 0;
        whileCount = 0;
      }
      classTable.reset();
      eat("}");
    }

    void compileClassVarDec() {
      kind kindOf;
      std::string typeOf;
      if (currentToken == "static") {
        kindOf = kind::STATIC;
        advance();
      }
      else if (currentToken == "field") { 
        kindOf = kind::FIELD;
        advance();
      }
      else
        printError("static|field");
      typeOf = currentToken;
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        classTable.define(currentToken, typeOf, kindOf);
        advance();
      }
      else
        printError("varName");
      while (currentToken == ",") {
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          classTable.define(currentToken, typeOf, kindOf);
          advance();
        }
        else
          printError("varName");
      }
      eat(";");
    }

    /* Why should I implement this is in the particular*/
    /* check this */
    void compileType() {
      // fileName should be the className
      if (currentToken == "int")
        advance();
      else if (currentToken == "char")
        advance();
      else if (currentToken == "boolean")
        advance();
      else if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        advance();
      else
        printError("int|char|boolean|className");
    }

    /* Why should I implement this is in the particular*/
    /* check this */
    void compileType(std::string eType) {
      // fileName should be the className
      if (currentToken == "int")
        advance();
      else if (currentToken == "char")
        advance();
      else if (currentToken == "boolean")
        advance();
      else if (currentToken == eType)
        advance();
      else if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        advance();
      else
        printError("int|char|boolean|className|" + eType);
    }

    void compileSubroutine() {
      if (currentToken == "constructor") {
        funcType = keyWord::CONSTRUCTOR;
        advance();
        if (currentToken == fileName) {
          advance();
        }
        else
          printError(fileName);
      }
      else if (currentToken == "function") {
        funcType = keyWord::FUNCTION;
        advance();
        // And other class name too will come here.
        compileType("void");
      }
      else if (currentToken == "method") {
        funcType = keyWord::METHOD;
        advance();
        // And other class name too will come here.
        compileType("void");
        subTable.define("this", fileName, kind::ARG);
      }
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        funcName = "." + currentToken;
        advance();
      }
      else
        printError("subroutineName");
      eat("(");
      compileParameterList();
      eat(")");
      compileSubroutineBody(); 
    }

    void compileParameterList() {
      std::string typeOf;
      if (currentToken != ")") {
        typeOf = currentToken ; 
        compileType();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          subTable.define(currentToken, typeOf, kind::ARG);
          advance();
        }
        else
          printError("varName");
        while (currentToken == ",") {
          advance();
          compileType();
          if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
            subTable.define(currentToken, typeOf, kind::ARG);
            advance();
          }
          else
            printError("varName");
        }
      }
    }

    void compileSubroutineBody() {
      eat("{");
      while (currentToken == "var")
        compileVarDec();
      switch (funcType) {
        case keyWord::CONSTRUCTOR:
          vmWriter.writeFunction(fileName + funcName, subTable.varCount(kind::VAR));
          vmWriter.writePush(segment::CONSTANT, classTable.varCount(kind::FIELD));
          vmWriter.writeCall("Memory.alloc", 1);
          vmWriter.writePop(segment::POINTER, 0) ;
          break;
        case keyWord::FUNCTION:
          vmWriter.writeFunction(fileName + funcName, subTable.varCount(kind::VAR));
          break;
        case keyWord::METHOD:
          vmWriter.writeFunction(fileName + funcName, subTable.varCount(kind::VAR));
          vmWriter.writePush(segment::ARGUMENT, 0);
          vmWriter.writePop(segment::POINTER, 0) ;
          break;
        default: printError("constructor|function|method");
      }
      compileStatements();
      eat("}");
    }

    void compileVarDec() {
      std::string typeOf;
      eat("var");
      typeOf = currentToken;
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        subTable.define(currentToken, typeOf, kind::VAR);
        advance();
      }
      else
        printError("varName");
      while (currentToken == ",") {
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          subTable.define(currentToken, typeOf, kind::VAR);
          advance();
        }
        else
          printError("varName");
      }
      eat(";");
    } 

    void compileStatements() {
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
          printError("let|if|while|do|return");
        }
      }
    }

    void compileLet() {
      eat("let");
      std::string identifier;
      enum::segment segmentType;
      enum::kind kindOf;
      uint64_t index;
      bool isArr = false;
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        identifier = currentToken;
        if (subTable.contains(currentToken)) {
          index = subTable.indexOf(currentToken);
          kindOf = subTable.kindOf(currentToken);
        }
        else if (classTable.contains(currentToken)) {
          index = classTable.indexOf(currentToken);
          kindOf = classTable.kindOf(currentToken);
        }
        else {
          printError("Declared the variable before using it : " + identifier);
        }
        switch (kindOf) {
          case kind::VAR: segmentType = segment::LOCAL;
            break;
          case kind::STATIC: segmentType = segment::STATIC;
            break;
          case kind::FIELD: segmentType = segment::THIS;
            break;
          case kind::ARG: segmentType = segment::ARGUMENT;
            break;
        }
        advance();
      }
      else
        printError("varName");
      if (currentToken == "[") {
        isArr = true;
        advance();
        compileExpression();
        vmWriter.writePush(segmentType, index);
        vmWriter.writeArithmetic(command::ADD);
        eat("]");
      }
      eat("=");
      compileExpression();
      if (isArr) {
        vmWriter.writePop(segment::TEMP, 0);
        vmWriter.writePop(segment::POINTER, 1);  
        vmWriter.writePush(segment::TEMP, 0);
        vmWriter.writePop(segment::THAT, 0);
      }
      else {
        vmWriter.writePop(segmentType, index);
      }
      eat(";");
    }

    void compileIf() {
      uint64_t count = ifCount++;
      eat("if");
      eat("(");
      compileExpression();
      vmWriter.writeIf("IF_TRUE" + std::to_string(count));
      vmWriter.writeGoto("IF_FALSE" + std::to_string(count));
      vmWriter.writeLabel("IF_TRUE" + std::to_string(count));
      eat(")");
      eat("{");
      compileStatements();
      eat("}");
      if (currentToken == "else") {
        vmWriter.writeGoto("IF_END" + std::to_string(count));
        vmWriter.writeLabel("IF_FALSE" + std::to_string(count));
        advance();
        eat("{");
        compileStatements();
        eat("}");
        vmWriter.writeLabel("IF_END" + std::to_string(count));
      }
      else {
        vmWriter.writeLabel("IF_FALSE" + std::to_string(count));
      }
    }

    void compileWhile() {
      uint64_t count = whileCount++;
      vmWriter.writeLabel("WHILE_EXP" + std::to_string(count));
			eat("while");
			eat("(");
			compileExpression();
      vmWriter.writeArithmetic(command::NOT);
      vmWriter.writeIf("WHILE_END" + std::to_string(count));
			eat(")");
      eat("{");
      compileStatements();
      eat("}");
      vmWriter.writeGoto("WHILE_EXP" + std::to_string(count));
      vmWriter.writeLabel("WHILE_END" + std::to_string(count));
    }

    void compileDo() {
      eat("do");
      std::string identifier;
      uint64_t nArgs = 0;
      bool isMethod = false;
      segment segmentType;
      kind kindOf;
      uint64_t index;
			if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        if (subTable.contains(currentToken)) {
          isMethod = true;
          identifier = subTable.typeOf(currentToken);
          kindOf = subTable.kindOf(currentToken);
          index = subTable.indexOf(currentToken);
        }
        else if (classTable.contains(currentToken)) {
          isMethod = true;
          identifier = classTable.typeOf(currentToken);
          kindOf = classTable.kindOf(currentToken);
          index = classTable.indexOf(currentToken);
        }
        else {
          identifier = currentToken;
        }
        advance();
      }
      else
        printError("subroutineName|className|varName");

      switch (kindOf) {
        case kind::VAR: segmentType = segment::LOCAL;
          break;
        case kind::STATIC: segmentType = segment::STATIC;
          break;
        case kind::FIELD: segmentType = segment::THIS;
          break;
        case kind::ARG: segmentType = segment::ARGUMENT;
          break;
      }

      if (currentToken == ".") {
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          identifier += "." + currentToken;
          advance();
        }
        else
          printError("subroutineName");
      }
      else {
        // if call method of same class
        isMethod = true;
        // Push the current object
        segmentType = segment::POINTER;
        index = 0;
        identifier = fileName + "." + identifier;
      }
      eat("(");
      if (isMethod) {
        ++nArgs;
        vmWriter.writePush(segmentType, index);
      }
      nArgs += compileExpressionList();
      eat(")");
      eat(";");
      vmWriter.writeCall(identifier, nArgs);
      vmWriter.writePop(segment::TEMP, 0);
       
    }

    void compileReturn() {
      eat("return");
      if (currentToken != ";")
        compileExpression();
      else 
        vmWriter.writePush(segment::CONSTANT, 0);
      vmWriter.writeReturn();
      eat(";");
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
          case '*': vmWriter.writeCall("Math.multiply", 2); 
              break;
          case '/': vmWriter.writeCall("Math.divide", 2); 
              break;
          case '&': vmWriter.writeArithmetic(command::AND); 
              break;
          case '|': vmWriter.writeArithmetic(command::OR); 
              break;
          case '!': vmWriter.writeArithmetic(command::NEG); 
              break;
          case '<': vmWriter.writeArithmetic(command::LT); 
              break;
          case '>': vmWriter.writeArithmetic(command::GT); 
              break;
          case '=': vmWriter.writeArithmetic(command::EQ); 
              break;
        }
      }
    }

    void compileTerm() {
      std::string identifier;
      enum::segment segmentType;
      enum::kind kindOf;
      uint64_t index;
      bool isVar = false;
      uint64_t nArgs = 0;
			switch (tokenizer.tokenType()) {
				case tokenType::KEYWORD: 
          switch (tokenizer.keyWord()) {
            case keyWord::TRUE:
              vmWriter.writePush(segment::CONSTANT, 0);
              vmWriter.writeArithmetic(command::NOT);
              break;
            case keyWord::FALSE:
            case keyWord::NONE:
              vmWriter.writePush(segment::CONSTANT, 0);
              break;
            case keyWord::THIS:
              vmWriter.writePush(segment::POINTER, 0);
              break;
            default: printError("true|false|null|this");
          }
          advance();
          break;
				case tokenType::INT_CONST: 
          vmWriter.writePush(segment::CONSTANT, tokenizer.intVal());
          advance();
          break;
				case tokenType::STR_CONST: 
          vmWriter.writePush(segment::CONSTANT, currentToken.length()); 
          vmWriter.writeCall("String.new", 1);
          for (char c: currentToken) {
            vmWriter.writePush(segment::CONSTANT, (int)c);
            vmWriter.writeCall("String.appendChar", 2);
          }
          advance();
					break;	
				case tokenType::IDENTIFIER:
          if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
            if (subTable.contains(currentToken)) {
              isVar = true;
              index = subTable.indexOf(currentToken);
              kindOf = subTable.kindOf(currentToken);
              identifier = subTable.typeOf(currentToken);
            }
            else if (classTable.contains(currentToken)) {
              isVar = true;
              index = classTable.indexOf(currentToken);
              kindOf = classTable.kindOf(currentToken);
              identifier = classTable.typeOf(currentToken);
            }
            else {
              identifier = currentToken;
            }
            advance();
          }
          // If identifier is a variable 
          if (isVar) {
            switch (kindOf) {
              case kind::VAR: segmentType = segment::LOCAL;
                break;
              case kind::STATIC: segmentType = segment::STATIC;
                break;
              case kind::FIELD: segmentType = segment::THIS;
                break;
              case kind::ARG: segmentType = segment::ARGUMENT;
                break;
            }
          }
					if (currentToken == "[") {
						advance();
						compileExpression();
            vmWriter.writePush(segmentType, index);
            vmWriter.writeArithmetic(command::ADD);
            vmWriter.writePop(segment::POINTER, 1);
            vmWriter.writePush(segment::THAT, 0);
						eat("]");
					}
					else if (currentToken == "(") {
            identifier = fileName + "." + identifier;
						advance();
            vmWriter.writePush(segment::POINTER, 0);
	  				nArgs = compileExpressionList() + 1; 
            vmWriter.writeCall(identifier, nArgs);
						eat(")");
					}
					else if (currentToken == ".") {
            advance();
            if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
              identifier += "." + currentToken;
            } 
            advance();
						eat("(");
	  				nArgs = compileExpressionList(); 
            if (isVar) {
              ++nArgs;
              vmWriter.writePush(segmentType, index);
            }
            vmWriter.writeCall(identifier, nArgs);
						eat(")");
					}
          else {
            // Normal variable;
            vmWriter.writePush(segmentType, index);
          }
					break;
				case tokenType::SYMBOL:
          switch (tokenizer.symbol()) {
             case '(':
              advance();
              compileExpression();
              eat(")");
              break;
            case '~':
              advance();
              compileTerm();
              vmWriter.writeArithmetic(command::NOT); 
              break;
            case '-':
              advance();
              compileTerm();
              vmWriter.writeArithmetic(command::NEG); 
              break;
          }
			}
    }
};
