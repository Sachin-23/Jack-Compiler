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
    std::string functionName;
    // Tokenizer object
    JackTokenizer tokenizer;
    // VMWriter object
    VMWriter vmWriter;
    uint64_t ifCount = 0;
    uint64_t whileCount = 0;
    bool isVoid = false;

    // function method or constructor ?
    keyWord funcType;

    SymbolTable classTable;
    SymbolTable subTable;
    uint64_t classVarCount = 0;
    uint64_t subVarCount = 0; 

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
      fileName = fileName.substr(fileName.find_last_of("/") + 1
          , fileName.length());
    }

    ~CompilationEngine() {
    }

    void compileClass() {
      eat("class");
      eat(fileName);
      eat("{");
      while (currentToken == "static" || currentToken == "field")
        compileClassVarDec();   
      while (currentToken == "constructor" || currentToken == "function" 
              || currentToken == "method") {
        compileSubroutine();   
        // subtable is cleared
        subTable.reset();
        subVarCount = 0;
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
          ++classVarCount;
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
        if (kindOf == kind::FIELD)
          ++classVarCount;
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
      functionName = fileName;
      switch (tokenizer.keyWord()) {
        case keyWord::CONSTRUCTOR:
          funcType = keyWord::CONSTRUCTOR;
          advance(); 
          eat(fileName);
          break;
        // Note precedence of cases matters
        case keyWord::METHOD:
          funcType = keyWord::METHOD;
          advance();
          if (compileType() || (tokenizer.keyWord() == keyWord::VOID && (isVoid=true)))
            advance();
          else
            printError("void|int|char|boolean|className");
          break;
        case keyWord::FUNCTION:
          funcType = keyWord::FUNCTION;
          advance();
          // Weird Syntax ?
          if (compileType() || (tokenizer.keyWord() == keyWord::VOID && (isVoid=true)))
            advance();
          else
            printError("void|int|char|boolean|className");
          break;
        default: printError("subroutineDec");
      }
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        functionName += "." + currentToken;
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
          advance();
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
      while (tokenizer.keyWord() == keyWord::VAR) {
        compileVarDec();
      }
      switch (funcType) {
        case keyWord::CONSTRUCTOR: 
          vmWriter.writeFunction(functionName, 0);
          vmWriter.writePush(segment::CONSTANT, classVarCount);
          vmWriter.writeCall("Memory.alloc", 1);
          vmWriter.writePop(segment::POINTER, 0);
          break;
        case keyWord::METHOD:
          vmWriter.writeFunction(functionName, subVarCount);
          vmWriter.writePush(segment::ARGUMENT, 0);
          vmWriter.writePop(segment::POINTER, 0);
          break;
        case keyWord::FUNCTION:
          vmWriter.writeFunction(functionName, subVarCount);
          break;
      }
      // I'll know the variable count in the function.
      /*
       * if constructor  
       * function filename.new 0 <- local variables
       * if method
       * push this <- current object
       * function filename.new 0 <- local variables
       * push argument 0 <- current object
       * pop pointer 0 <- set the current object
       *
       */
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
        ++subVarCount;
        subTable.define(currentToken, type, kind::VAR); 
        advance();
      }
      else
        printError("varName");

      while (currentToken == ",") {
        advance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          ++subVarCount;
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
    // let x = 2;
    // find x in subtable if yes

    void compileLet() {
      std::string identifier;
      int64_t index = -1;
      enum::kind kindOf;
      enum::segment segmentType;
      bool isArray = false;
      eat("let");
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        identifier = currentToken;
        advance();
      }
      else
        printError("varName");

      if (subTable.in(identifier)) {
        kindOf = subTable.kindOf(identifier);
        index = subTable.indexOf(identifier);
      }
      else if (classTable.in(identifier)) {
        kindOf = classTable.kindOf(identifier);
        index = classTable.indexOf(identifier);
      }
      else {
        printError("From let: LHS ");
      } 
      switch (kindOf) {
        case kind::STATIC: segmentType = segment::STATIC;
          break;
        case kind::FIELD: segmentType = segment::THIS;
          break;
        case kind::ARG: segmentType = segment::ARGUMENT; 
          break;
        case kind::VAR: segmentType = segment::LOCAL;
          break;
      }
      if (currentToken == "[") {
        // calculate the address
        vmWriter.writePush(segmentType, index);
        advance();
        compileExpression();
        vmWriter.writeArithmetic(command::ADD);
        eat("]");
      } 
      eat("=");
      compileExpression();
      // if LHS is an arr[exp1]
      if (isArray) {
        vmWriter.writePop(segment::TEMP, 0);
        vmWriter.writePop(segment::POINTER, 1);
        vmWriter.writePush(segment::TEMP, 0);
        vmWriter.writePop(segment::THAT, 0);
      } 
      else {
      // Normal variable
        vmWriter.writePop(segmentType, index);
      }
      eat(";");
    }
    /*
     * if (exp) {
     * }
     *
     * exp
     * not
     * if l1
     * statements
     * l1
     *
     * exp
     * not
     * if l1
     * statement
     * goto l2
     * l1
     * else - statement
     * l2
     *
     */

    void compileIf() {
      uint64_t count = ifCount++;
      eat("if");
      eat("(");
      compileExpression();
      vmWriter.writeArithmetic(command::NOT); 
      vmWriter.writeIf("IF_" + std::to_string(count));
      eat(")");
      eat("{");
      compileStatements();
      eat("}");
      if (currentToken == "else") {
        ++count;
        ++ifCount;
        vmWriter.writeGoto("IF_" + std::to_string(count));
        vmWriter.writeLabel("IF_" + std::to_string(count - 1));
        advance();
        eat("{");
        compileStatements();
        eat("}");
      }
      vmWriter.writeLabel("IF_" + std::to_string(count));
    }

    void compileWhile() {
      uint64_t count = whileCount;
      whileCount += 2;
			eat("while");
			eat("(");
      vmWriter.writeLabel("WHILE_" + std::to_string(count));
			compileExpression();
      vmWriter.writeArithmetic(command::NOT); 
      vmWriter.writeIf("WHILE_" + std::to_string(count + 1));
			eat(")");
      eat("{");
      compileStatements();
      vmWriter.writeGoto("WHILE_" + std::to_string(count));
      vmWriter.writeLabel("WHILE_" + std::to_string(count + 1));
      eat("}");
    }

    void compileDo() {
      eat("do");
      std::string functionName;  
      enum::segment segType;
      uint64_t nArgs = 0;
			if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        if (subTable.in(currentToken)) {
          functionName += subTable.typeOf(currentToken);
          segType = subTable.typeOf(currentToken);
        }
        else if (classTable.in(currentToken)) {
          functionName += classTable.typeOf(currentToken);
        }
        else {
          functionName += currentToken;
        }
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
      else {
        // if directly calling self methods
        functionName = fileName + "." + functionName;
      }
      eat("(");
      nArgs = compileExpressionList();
      eat(")");
      vmWriter.writePush(segment::POINTER, 0); 
      vmWriter.writeCall(functionName, nArgs);
      vmWriter.writePop(segment::TEMP, 0); 
      // push this it's the calling function 
      // responsibility to use to or not
      eat(";");
    }

    void compileReturn() {
      if (isVoid)
        vmWriter.writePush(segment::CONSTANT, 0);
      eat("return");
      if (currentToken != ";")
        compileExpression();
      eat(";");
      vmWriter.writeReturn();
    }

    uint64_t compileExpressionList() {
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
          case '!': vmWriter.writeArithmetic(command::NOT); 
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
      enum::segment segmentType; 
      int64_t index = -1;
      enum::kind kindOf;
      std::string identifier;
      uint64_t nArgs = 0;
			switch (tokenizer.tokenType()) {
        case tokenType::KEYWORD:
          std::cout << currentToken << std::endl; 
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
              vmWriter.writePush(segment::THIS, 0);
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
          identifier = currentToken;
          advance();
          // find if the identifier is
          // -> subroutine
          // -> array
          // -> variable
          //vmWriter.writePush(segmentType, 10);
          if (tokenizer.tokenType() != tokenType::SYMBOL) {
            if (subTable.in(identifier)) {
              kindOf = subTable.kindOf(identifier);
              index = subTable.indexOf(identifier);
            }
            else if (classTable.in(identifier)) {
              kindOf = classTable.kindOf(identifier);
              index = classTable.indexOf(identifier);
            }
            else {
              printError("Error in Array");
            }
            vmWriter.writePush(segmentType, index);
            return;
          }
          switch (tokenizer.symbol()) {
            case '[':
              advance();
              if (subTable.in(identifier)) {
                kindOf = subTable.kindOf(identifier);
                index = subTable.indexOf(identifier);
              }
              else if (classTable.in(identifier)) {
                kindOf = classTable.kindOf(identifier);
                index = classTable.indexOf(identifier);
              }
              else {
                printError("Error in Array");
              }
              vmWriter.writePush(segmentType, index);
              compileExpression();
              vmWriter.writeArithmetic(command::ADD);
              eat("]");
              break;
            // Code reability vs optimization ?
            case '.':
              advance();
              if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
                identifier += "." + currentToken;
                advance();
                eat("(");
                nArgs = compileExpressionList();
                eat(")");
                vmWriter.writeCall(identifier, nArgs);
              }
              break;
            case '(': 
              advance();
              compileExpressionList(); 
              eat(")");
              break;
            default: break; 
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
          break;
        default: printError("Error in term");
			}
    }
};
