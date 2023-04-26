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
    std::ofstream outFile;

    SymbolTable subTable, classTable;
    JackTokenizer tokenizer;
    VMWriter vmWriter;

    uint64_t depth = 0;
    uint64_t nVars = 0;
    uint64_t fieldCount = 0;
    keyWord funcType;
    std::string funcName;

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
      switch(tokenizer.tokenType()) {
        case tokenType::KEYWORD:  
          outFile << "<keyword> " << currentToken << " </keyword>";
          break; 
        case tokenType::SYMBOL: 
          outFile << "<symbol> " << tokenizer.stringVal() << " </symbol>";
          break; 
        case tokenType::IDENTIFIER: 
          outFile << "<identifier> " << currentToken << " </identifier>";
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
      vmWriter.init(path);

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
        compileSubroutine();   
        subTable.reset();
        nVars = 0;
      }
      classTable.reset();
      eat("}");
      --depth;
      outFile << "</class>\n";
    }

    void compileClassVarDec() {
      kind kindOf;
      std::string typeOf;
      printTab();
      outFile << "<classVarDec>\n";
      ++depth;
      if (currentToken == "static") {
        kindOf = kind::STATIC;
        printAndAdvance();
      }
      else if (currentToken == "field") { 
        kindOf = kind::FIELD;
        printAndAdvance();
      }
      else
        printError("static|field");
      typeOf = currentToken;
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        classTable.define(currentToken, typeOf, kindOf);
        if (kindOf == kind::FIELD)
          ++fieldCount;
        printTab();
        outFile << "<identifier typeOf='" << typeOf
          << "' indexOf='" << classTable.indexOf(currentToken)
          << "' kindOf='" << kindName.at(classTable.kindOf(currentToken))
          << "'>" << currentToken
          << "</identifier>" << std::endl; 
          if (tokenizer.hasMoreTokens()) {
            currentToken = tokenizer.advance();
          }
      }
      else
        printError("varName");
      while (currentToken == ",") {
        printAndAdvance();
        if (kindOf == kind::FIELD)
          ++fieldCount;
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          classTable.define(currentToken, typeOf, kindOf);
          printTab();
          outFile << "<identifier typeOf='" << typeOf
            << "' indexOf='" << classTable.indexOf(currentToken)
            << "' kindOf='" << kindName.at(classTable.kindOf(currentToken))
            << "'>" << currentToken
            << "</identifier>" << std::endl; 
          if (tokenizer.hasMoreTokens()) {
            currentToken = tokenizer.advance();
          }
        }
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
        printAndAdvance();
      else if (currentToken == "char")
        printAndAdvance();
      else if (currentToken == "boolean")
        printAndAdvance();
      else if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        printAndAdvance();
      else
        printError("int|char|boolean|className");
    }

    /* Why should I implement this is in the particular*/
    /* check this */
    void compileType(std::string eType) {
      // fileName should be the className
      if (currentToken == "int")
        printAndAdvance();
      else if (currentToken == "char")
        printAndAdvance();
      else if (currentToken == "boolean")
        printAndAdvance();
      else if (currentToken == eType)
        printAndAdvance();
      else if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        printAndAdvance();
      else
        printError("int|char|boolean|className|" + eType);
    }

    void compileSubroutine() {
      printTab();
      outFile << "<subroutineDec>\n";
      ++depth;
      if (currentToken == "constructor") {
        funcType = keyWord::CONSTRUCTOR;
        printAndAdvance();
        if (currentToken == fileName) {
          printAndAdvance();
        }
        else
          printError(fileName);
      }
      else if (currentToken == "function") {
        funcType = keyWord::FUNCTION;
        printAndAdvance();
        // And other class name too will come here.
        compileType("void");
      }
      else if (currentToken == "method") {
        funcType = keyWord::METHOD;
        printAndAdvance();
        // And other class name too will come here.
        compileType("void");
      }
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        funcName = "." + currentToken;
        printAndAdvance();
      }
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
      std::string typeOf;
      printTab();
      outFile << "<parameterList>\n";
      ++depth;
      if (currentToken != ")") {
        typeOf = currentToken ; 
        compileType();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          printTab();
          subTable.define(currentToken, typeOf, kind::ARG);
          outFile << "<identifier typeOf='" << typeOf
            << "' indexOf='" << subTable.indexOf(currentToken)
            << "' kindOf='" << kindName.at(subTable.kindOf(currentToken))
            << "'>" << currentToken
            << "</identifier>" << std::endl; 
          if (tokenizer.hasMoreTokens()) {
            currentToken = tokenizer.advance();
          }
        }
        else
          printError("varName");
        while (currentToken == ",") {
          printAndAdvance();
          compileType();
          if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
            printTab();
            subTable.define(currentToken, typeOf, kind::ARG);
            outFile << "<identifier typeOf='" << typeOf
              << "' indexOf='" << subTable.indexOf(currentToken)
              << "' kindOf='" << kindName.at(subTable.kindOf(currentToken))
              << "'>" << currentToken
              << "</identifier>" << std::endl; 
            if (tokenizer.hasMoreTokens()) {
              currentToken = tokenizer.advance();
            }
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
      while (currentToken == "var")
        compileVarDec();
      switch (funcType) {
        case keyWord::CONSTRUCTOR:
          vmWriter.writeFunction(fileName + funcName, nVars);
          vmWriter.writePush(segment::CONSTANT, fieldCount);
          vmWriter.writeCall("Memory.alloc", 1);
          vmWriter.writePop(segment::POINTER, 0) ;
          break;
        case keyWord::FUNCTION:
          vmWriter.writeFunction(fileName + funcName, nVars);
          break;
        case keyWord::METHOD:
          vmWriter.writeFunction(fileName + funcName, nVars);
          vmWriter.writePush(segment::ARGUMENT, 0);
          vmWriter.writePop(segment::POINTER, 0) ;
          break;
        default: printError("constructor|function|method");
      }
      compileStatements();
      eat("}");
      --depth;
      printTab();
      outFile << "</subroutineBody>\n";
    }

    void compileVarDec() {
      std::string typeOf;
      printTab();
      outFile << "<varDec>\n" ;
      ++depth;
      eat("var");
      typeOf = currentToken;
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        printTab();
        ++nVars;
        subTable.define(currentToken, typeOf, kind::VAR);
        outFile << "<identifier typeOf='" << typeOf
          << "' indexOf='" << subTable.indexOf(currentToken)
          << "' kindOf='" << kindName.at(subTable.kindOf(currentToken))
          << "'>" << currentToken
          << "</identifier>" << std::endl; 
        if (tokenizer.hasMoreTokens()) {
          currentToken = tokenizer.advance();
        }
      }
      else
        printError("varName");
      while (currentToken == ",") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          printTab();
          ++nVars;
          subTable.define(currentToken, typeOf, kind::VAR);
          outFile << "<identifier typeOf='" << typeOf
            << "' indexOf='" << subTable.indexOf(currentToken)
            << "' kindOf='" << kindName.at(subTable.kindOf(currentToken))
            << "'>" << currentToken
            << "</identifier>" << std::endl; 
          if (tokenizer.hasMoreTokens()) {
            currentToken = tokenizer.advance();
          }
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
      --depth;
      printTab();
      outFile << "</statements>\n";
    }

    void compileLet() {
      printTab();  
      outFile << "<letStatement>\n";
      ++depth;
      eat("let");
      printTab();
      std::string identifier;
      enum::segment segmentType;
      enum::kind kindOf;
      uint64_t index;
      if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        identifier = currentToken;
        if (subTable.in(currentToken)) {
          subTable.typeOf(currentToken);
          index = subTable.indexOf(currentToken);
          kindOf = subTable.kindOf(currentToken);
        }
        else if (classTable.in(currentToken)) {
          subTable.typeOf(currentToken);
          index = classTable.indexOf(currentToken);
          kindOf = classTable.kindOf(currentToken);
        }
        else {
          printError("Found declare the variable: " + identifier);
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
        if (tokenizer.hasMoreTokens()) {
          currentToken = tokenizer.advance();
        }
      }
      else
        printError("varName");
      if (currentToken == "[") {
        outFile << "<identifier index='" << index
          << "' segmentType='" << segName.at(segmentType) 
          << "'>" << identifier << "</identifier>" << std::endl;
        printAndAdvance();
        compileExpression();
        eat("]");
      }
      else {
        outFile << "<identifier index='" << index
          << "' segmentType='" << segName.at(segmentType) 
          << "'>" << identifier << "</identifier>" << std::endl;
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
      std::string identifier;
			if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
        if (subTable.in(currentToken)) {
          identifier = subTable.typeOf(currentToken);
        }
        else if (classTable.in(currentToken)) {
          identifier = classTable.typeOf(currentToken);
        }
        else {
          identifier = currentToken;
        }
        if (tokenizer.hasMoreTokens()) {
          currentToken = tokenizer.advance();
        } 
      }
      else
        printError("subroutineName|className|varName");
      if (currentToken == ".") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
          identifier += "." + currentToken;
          if (tokenizer.hasMoreTokens()) {
            currentToken = tokenizer.advance();
          }
        }
        else
          printError("subroutineName");
      }
      printTab();  
      outFile << "<identifier>" << identifier << "</identifier>" << std::endl;
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
      std::string identifier;
      enum::segment segmentType;
      enum::kind kindOf;
      uint64_t index;
			switch (tokenizer.tokenType()) {
				case tokenType::INT_CONST: 
				case tokenType::STR_CONST: 
				case tokenType::KEYWORD: 
					printAndAdvance();
					break;	
				case tokenType::IDENTIFIER:
          /*
          if (subTable.in(currentToken)) {
            outFile << "<identifier typeOf='" << subTable.typeOf(currentToken)
              << "' indexOf='" << subTable.indexOf(currentToken)
              << "' kindOf='" << kindName.at(subTable.kindOf(currentToken))
              << "'>" << currentToken
              << "</identifier>" << std::endl; 
          }
          else if (classTable.in(currentToken)) {
            outFile << "<identifier typeOf='" << classTable.typeOf(currentToken)
              << "' indexOf='" << classTable.indexOf(currentToken)
              << "' kindOf='" << kindName.at(classTable.kindOf(currentToken))
              << "'>" << currentToken
              << "</identifier>" << std::endl; 
          }
          else {
            outFile << "<identifier typeOf='subroutine'>"<< currentToken
              << "</identifier>" << std::endl; 
          }
          */
          if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
            identifier = currentToken;
            if (subTable.in(currentToken)) {
              subTable.typeOf(currentToken);
              index = subTable.indexOf(currentToken);
              kindOf = subTable.kindOf(currentToken);
            }
            else if (classTable.in(currentToken)) {
              subTable.typeOf(currentToken);
              index = classTable.indexOf(currentToken);
              kindOf = classTable.kindOf(currentToken);
            }
            else {
              identifier = currentToken;
            }
            if (tokenizer.hasMoreTokens()) {
              currentToken = tokenizer.advance();
            }
          }
					if (currentToken == "[") {
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
            printTab();
            outFile << "<identifier index='" << index
              << "' segmentType='" << segName.at(segmentType) 
              << "'>" << identifier << "</identifier>" << std::endl;
						printAndAdvance();
						compileExpression();
						eat("]");
					}
					else if (currentToken == "(") {
            identifier = fileName + "." + identifier;
            printTab();
            outFile << "<identifier subroutine>" << identifier << "</identifier>" << std::endl;
            if (tokenizer.hasMoreTokens()) {
              currentToken = tokenizer.advance();
            }
	  				compileExpressionList(); 
						eat(")");
					}
					else if (currentToken == ".") {
            if (tokenizer.hasMoreTokens()) {
              currentToken = tokenizer.advance();
            }
            if (tokenizer.tokenType() == tokenType::IDENTIFIER) {
              identifier += "." + currentToken;
              printTab();
              outFile << "<identifier subroutine>" << identifier << "</identifier>" << std::endl;
            } 
            if (tokenizer.hasMoreTokens()) {
              currentToken = tokenizer.advance();
            }
						eat("(");
	  				compileExpressionList(); 
						eat(")");
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
			}
      --depth;
      printTab();
      outFile << "</term>\n";
    }
};
