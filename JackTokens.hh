#include <iostream>
#include <map>

enum class tokenType {
  KEYWORD 
  , SYMBOL
  , IDENTIFIER
  , INT_CONST
  , STR_CONST
};

enum class keyWord {
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

enum class kind {
  VAR
  , STATIC
  , FIELD
  , ARG
};

enum class segment {
  CONSTANT
  , ARGUMENT 
  , LOCAL
  , STATIC
  , THIS
  , THAT
  , POINTER
  , TEMP
};

enum class command {
  ADD
  , SUB
  , NEG
  , EQ
  , GT
  , LT
  , AND
  , OR
  , NOT
};

const std::map<std::string, enum::keyWord> keyWords = {
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

const std::map<enum::kind, std::string> kindName = {
  {kind::VAR, "VAR"}
  , {kind::FIELD, "FIELD"}
  , {kind::ARG, "ARG"}
  , {kind::STATIC, "STATIC"}
};

const std::map<enum::segment, std::string> segName = {
  {segment::CONSTANT, "CONSTANT"}
  , {segment::ARGUMENT, "ARGUMENT"}
  , {segment::LOCAL, "LOCAL"}
  , {segment::STATIC, "STATIC"}
  , {segment::THIS, "THIS"}
  , {segment::THAT, "THAT"}
  , {segment::POINTER, "POINTER"}
  , {segment::TEMP, "TEMP"}
};
