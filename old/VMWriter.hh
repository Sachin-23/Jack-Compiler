#include <iostream>
#include <fstream>

class VMWriter {
  private:
    std::ofstream outFile;
    std::string fileName;

    void printError() {

    }

  public:
    VMWriter() { };

    VMWriter(std::string path) {
      init(path);
    }

    void init(std::string path) {
        
      // Remove file extension
      fileName = path.substr(0, path.find_last_of("."));
      path = fileName + ".vm";

      outFile.open(path);

      std::cout << "VMWriter: " << path << std::endl;

      if (!outFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + path); 
    }

    void close() {
      outFile.close();
    } 

    ~VMWriter() {
      outFile.close();
    }
    

    void writePush(enum::segment seg, uint64_t index) {
      std::string _segment;
      switch (seg) {
        case segment::CONSTANT: _segment = "constant";
          break;
        case segment::ARGUMENT: _segment = "argument";
          break;
        case segment::LOCAL: _segment = "local";
          break;
        case segment::STATIC: _segment = "static";
          break;
        case segment::THIS: _segment = "this";
          break;
        case segment::THAT: _segment = "that";
          break;
        case segment::POINTER: _segment = "pointer";
          break;
        case segment::TEMP: _segment = "temp";
          break;
        default: printError();
      }
      outFile << "push " << _segment << " " << index << std::endl;;
    }

    void writePop(enum::segment seg, uint64_t index) {
      std::string _segment;
      switch (seg) {
        case segment::CONSTANT: _segment = "constant";
          break;
        case segment::ARGUMENT: _segment = "argument";
          break;
        case segment::LOCAL: _segment = "local";
          break;
        case segment::STATIC: _segment = "static";
          break;
        case segment::THIS: _segment = "this";
          break;
        case segment::THAT: _segment = "that";
          break;
        case segment::POINTER: _segment = "pointer";
          break;
        case segment::TEMP: _segment = "temp";
          break;
        default: printError();
      }
      outFile << "pop " << _segment << " " << index << std::endl;;
    }

    void writeArithmetic(enum::command _command) {
      switch (_command) {
        case command::ADD: outFile << "add";
          break;
        case command::SUB: outFile << "sub";
          break;
        case command::NEG: outFile << "neg";
          break;
        case command::EQ: outFile << "eq";
          break;
        case command::GT: outFile << "gt";
          break;
        case command::LT: outFile << "lt";
          break;
        case command::AND: outFile << "and";
          break;
        case command::OR: outFile << "or";
          break;
        case command::NOT: outFile << "not";
      }
      outFile << std::endl;
    }

    void writeLabel(std::string label) {
      outFile << "label " << label << std::endl;
    };

    void writeGoto(std::string label) {
      outFile << "goto " << label << std::endl;
    };

    void writeIf(std::string label) {
      outFile << "if-goto " << label << std::endl;
    };

    void writeCall(std::string name, uint64_t nArgs) {
      outFile << "call " << name << " " << nArgs << std::endl;
    };

    void writeFunction(std::string name, uint64_t nVars) {
      outFile << "function " << name << " " << nVars << std::endl;
    };

    void writeReturn() {
      outFile << "return" << std::endl;
    };
};

