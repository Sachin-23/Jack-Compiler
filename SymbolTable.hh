#include <iostream>
#include <map>
#include <vector>

class SymbolTable {
  private:
     uint64_t staticCount = 0;
     uint64_t fieldCount = 0;
     uint64_t _varCount = 0;
     uint64_t argCount = 0;
     std::map< std::string, 
     std::tuple<std::string, enum::kind, uint64_t> > table; 

  public:
    
    void define(std::string name, std::string type, enum::kind akind) {
      uint64_t index;
      switch (akind) {
        case kind::STATIC: index = staticCount++; 
          break;
        case kind::FIELD: index = fieldCount++;
          break;
        case kind::VAR: index = _varCount++; 
          break;
        case kind::ARG: index = argCount++;
          break;
      }
      table[name] = {type, akind, index};
    }
    
    uint64_t varCount(enum::kind kind) {
      switch (kind) {
        case kind::STATIC: return staticCount; 
        case kind::FIELD: return fieldCount; 
        case kind::VAR: return _varCount; 
        case kind::ARG: return argCount; 
      }
    } 

    enum::kind kindOf(std::string name) {
      return std::get<1>(table[name]);
    }

    std::string typeOf(std::string name) {
      return std::get<0>(table[name]);
    }

    uint64_t indexOf(std::string name) {
     return std::get<2>(table[name]);
    }

    bool contains(std::string name) {
      return table.count(name) > 0;
    }

    void reset() {
      table.clear();
      staticCount = 0; 
      fieldCount = 0;
      _varCount = 0;
      argCount = 0;
    }
};

