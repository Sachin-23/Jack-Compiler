#include <iostream>
#include <map>

enum tokenTypes {
  KEYWORD 
  , SYMBOL
  , IDENTIFIER
  , INT_CONST
  , STR_CONST
};

enum keyWords {
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

std::map<std::string, enum::keyWords> keyWordTypes = {
 {"class",        keyWords::CLASS} 
 , {"method"      , keyWords::METHOD}
 , {"function"    , keyWords::FUNCTION}
 , {"constructor" , keyWords::CONSTRUCTOR}
 , {"int"         , keyWords::INT}
 , {"boolean"     , keyWords::BOOLEAN}
 , {"char"        , keyWords::CHAR}
 , {"void"        , keyWords::VOID}
 , {"var"         , keyWords::VAR}
 , {"static"      , keyWords::STATIC}
 , {"field"       , keyWords::FIELD}
 , {"let"         , keyWords::LET}
 , {"do"          , keyWords::DO}
 , {"if"          , keyWords::IF}
 , {"else"        , keyWords::ELSE}
 , {"while"       , keyWords::WHILE}
 , {"return"      , keyWords::RETURN}
 , {"true"        , keyWords::TRUE}
 , {"false"       , keyWords::FALSE}
 , {"null"        , keyWords::NONE} 
 , {"this"        , keyWords::THIS}
};

