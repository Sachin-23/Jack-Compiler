#include <iostream>
#include <map>

enum lexElements {
  KEYWORD 
  , SYMBOL
  , INT_CONST
  , STR_CONST
};

std::map<std::string, enum lexElements> TOKENS = {
  // All the keywords
  {"class", lexElements::KEYWORD} 
  , {"constructor", lexElements::KEYWORD} 
  // All the symbols
  , {"{", lexElements::SYMBOL}
  , {"}", lexElements::SYMBOL}
};
