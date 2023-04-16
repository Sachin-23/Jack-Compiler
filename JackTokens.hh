#include <iostream>
#include <map>

enum tokenType {
  KEYWORD 
  , SYMBOL
  , IDENTIFIER
  , INT_CONST
  , STR_CONST
};

enum keyWord {
  CLASS
  , METHOD
  , FUNCTION
  , CONSTRUCTOR
  , INT
  , BOOLEAN
  , CHAR
  , VOID
  , VAR
  , STATIC
  , FIELD
  , LET
  , DO
  , IF
  , ELSE
  , WHILE
  , RETURN
  , TRUE
  , FALSE
  , NONE // same NULL
  , THIS
};

std::map<std::string, enum::keyWord> keyWords = {
 {"class",        keyWord::CLASS} 
 , {"method"      , keyWord::METHOD}
 , {"function"    , keyWord::FUNCTION}
 , {"constructor" , keyWord::CONSTRUCTOR}
 , {"int"         , keyWord::INT}
 , {"boolean"     , keyWord::BOOLEAN}
 , {"char"        , keyWord::CHAR}
 , {"void"        , keyWord::VOID}
 , {"var"         , keyWord::VAR}
 , {"static"      , keyWord::STATIC}
 , {"field"       , keyWord::FIELD}
 , {"let"         , keyWord::LET}
 , {"do"          , keyWord::DO}
 , {"if"          , keyWord::IF}
 , {"else"        , keyWord::ELSE}
 , {"while"       , keyWord::WHILE}
 , {"return"      , keyWord::RETURN}
 , {"true"        , keyWord::TRUE}
 , {"false"       , keyWord::FALSE}
 , {"null"        , keyWord::NONE} 
 , {"this"        , keyWord::THIS}
};

