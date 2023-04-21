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

      std::cout << "VMWriter: " << path << std::endl;

      outFile.open(fileName + ".tvm");

      if (!outFile) 
        throw std::runtime_error(std::string("Failed to open file: ") + fileName + ".xml");
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
        case segment::CONSTANT: _segment = "CONSTANT";
          break;
        case segment::ARGUMENT: _segment = "ARGUMENT";
          break;
        case segment::LOCAL: _segment = "LOCAL";
          break;
        case segment::STATIC: _segment = "STATIC";
          break;
        case segment::THIS: _segment = "THIS";
          break;
        case segment::THAT: _segment = "THAT";
          break;
        case segment::POINTER: _segment = "POINTER";
          break;
        case segment::TEMP: _segment = "TEMP";
          break;
        default: printError();
      }
      outFile << "push " << _segment << " " << index << std::endl;;
    }

    void writePop(enum::segment seg, uint64_t index) {
      std::string _segment;
      switch (seg) {
        case segment::CONSTANT: _segment = "CONSTANT";
          break;
        case segment::ARGUMENT: _segment = "ARGUMENT";
          break;
        case segment::LOCAL: _segment = "LOCAL";
          break;
        case segment::STATIC: _segment = "STATIC";
          break;
        case segment::THIS: _segment = "THIS";
          break;
        case segment::THAT: _segment = "THAT";
          break;
        case segment::POINTER: _segment = "POINTER";
          break;
        case segment::TEMP: _segment = "TEMP";
          break;
        default: printError();
      }
      outFile << "pop" << _segment << " " << index << std::endl;;
    }

    void writeArithemetic(enum::command _command) {
      switch (_command) {
        case command::ADD: outFile << "add";
        case command::SUB: outFile << "sub";
        case command::NEG: outFile << "neg";
        case command::EQ: outFile << "eq";
        case command::GT: outFile << "gt";
        case command::LT: outFile << "lt";
        case command::AND: outFile << "and";
        case command::OR: outFile << "or";
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
      outFile << "if" << label << std::endl;
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

