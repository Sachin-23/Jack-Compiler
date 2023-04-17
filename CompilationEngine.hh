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
          outFile << "<symbol> " << currentToken << " </symbol>";
          break; 
        case tokenType::IDENTIFIER: 
          outFile << "<identifier> " << currentToken << " </identifier>";
          break; 
        case tokenType::INT_CONST: 
          outFile << "<intCONST> " << currentToken<< " </intCONST>";
          break; 
        case tokenType::STR_CONST: 
          outFile << "<stringCONST> " << currentToken << " </stringCONST>";
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
        printAndAdvance();
      else if (currentToken == "field") 
        printAndAdvance();
      else
        printError("static|field");
      compileType();
      if (tokenizer.tokenType() == tokenType::IDENTIFIER)
        printAndAdvance();
      else
        printError("varName");
      while (currentToken == ",") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          printAndAdvance();
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
        printError("int|char|boolean|className" + eType);
    }

    void compileSubroutine() {
      printTab();
      outFile << "<subroutineDec>\n";
      ++depth;
      if (currentToken == "constructor") {
        printAndAdvance();
        if (currentToken == fileName)
          printAndAdvance();
        else
          printError(fileName);
      }
      else if (currentToken == "function") {
        printAndAdvance();
        // And other class name too will come here.
        compileType("void");
      }
      else if (currentToken == "method") {
        printAndAdvance();
        // And other class name too will come here.
        compileType("void");
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
      if (currentToken != ")") {
        compileType();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          printAndAdvance();
        else
          printError("varName");
        while (currentToken == ",") {
          printAndAdvance();
          compileType();
          if (tokenizer.tokenType() == tokenType::IDENTIFIER)
            printAndAdvance();
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
        printAndAdvance();
      else
        printError("varName");
      while (currentToken == ",") {
        printAndAdvance();
        if (tokenizer.tokenType() == tokenType::IDENTIFIER)
          printAndAdvance();
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
				case tokenType::INT_CONST: 
				case tokenType::STR_CONST: 
				case tokenType::KEYWORD: 
					printAndAdvance();
					break;	
				case tokenType::IDENTIFIER:
					printAndAdvance();
					if (currentToken == "[") {
						printAndAdvance();
						compileExpressionList();
						eat("]");
					}
					else if (currentToken == "(") {
						printAndAdvance();
	  				compileExpressionList(); 
						eat(")");
					}
					else if (currentToken == ".") {
						printAndAdvance();
						if (tokenizer.tokenType() == tokenType::IDENTIFIER)
							printAndAdvance();
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
