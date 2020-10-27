#include "Token.hpp"

STLString cl::tokenTypeToString(cl::TokenType type) {
  switch (type) {

  case TokenType::Integral:
    return "Integral";
  case TokenType::FloatingPoint:
    return "FloatingPoint";
  case TokenType::String:
    return "String";
  case TokenType::Symbol:
    return "Symbol";
  case TokenType::Vector:
    return "Vector";
  case TokenType::BitVector:
    return "BitVector";
  case TokenType::SharpBackslash:
    return "Character";
  case TokenType::SharpB:
    return "BinaryIntegral";
  case TokenType::SharpO:
    return "OctalIntegral";
  case TokenType::SharpX:
    return "HexIntegral";
  case TokenType::SharpR:
    return "RadixNIntegral";
  case TokenType::SharpC:
    return "SharpC";
  case TokenType::SharpA:
    return "SharpA";
  case TokenType::SharpS:
    return "RMStruct";
  case TokenType::SharpP:
    return "SharpP";
  case TokenType::LeftParentheses:
    return "LParen";
  case TokenType::RightParentheses:
    return "RParen";
  case TokenType::Quote:
    return "Quote";
  case TokenType::Backquote:
    return "Backquote";
  case TokenType::Comma:
    return "Comma";
  case TokenType::CommaAtSign:
    return "CommaAtSign";
  case TokenType::CommaDot:
    return "CommaDot";
  case TokenType::SharpQuote:
    return "SharpQuote";
  case TokenType::SharpColon:
    return "SharpColon";
  case TokenType::SharpDot:
    return "SharpDot";
  case TokenType::Dot:
    return "Dot";
  case TokenType::Comment:
    return "Semicolon";
  case TokenType::SharpEqual:
    return "SharpEqual";
  case TokenType::SharpSharp:
    return "SharpSharp";
  case TokenType::SharpPlus:
    return "SharpPlus";
  case TokenType::SharpMinus:
    return "SharpMinus";
  case TokenType::SharpRightParen:
    return "SharpRightParen";
  case TokenType::Eof:
    return "EOF";
  case TokenType::Whitespace:
    return "Whitespace";
  case TokenType::Newline:
    return "Newline";
  case TokenType::Error:
    return "Error";
  }
  return "ERROR -_-";
}

cl::LexError::LexError(uint32_t line, uint32_t column, STLString file,
                       STLString context, STLString message) {
  initMessage(line, column, file, context, message);
}

cl::LexError::LexError(const LexError &other) {
  if (this != &other) {
    copy(other);
  }
}

cl::LexError::~LexError() {
  if (exceptionMessage)
    delete[] exceptionMessage;
}

cl::LexError &cl::LexError::operator=(const LexError &other) {
  if (this != &other) {
    copy(other);
  }
  return *this;
}

void cl::LexError::copy(const LexError &other) {
  const char* message = other.what();
  initMessage(message);
}

void cl::LexError::initMessage(const char *message) {

  if(message == nullptr){
    exceptionMessage = new char[1];
    exceptionMessage[0] = '\0';
    return;
  }

  size_t exceptionSize = strlen(message);
  exceptionMessage = new char[exceptionSize + 1];
  strncpy(exceptionMessage, message, exceptionSize);
  exceptionMessage[exceptionSize + 1] = '\0';
}

void cl::LexError::initMessage(uint32_t line, uint32_t column, STLString file,
                               STLString context, STLString message) {

  size_t messageSize = snprintf(nullptr, 0, format, file.c_str(), line, column,
                                message.c_str(), context.c_str()) +
                       1;

  if (messageSize <= 0) {
    throw STLRuntimeError("Error during formatting of error message.");
  }

  exceptionMessage = new char[messageSize];

  snprintf(exceptionMessage, messageSize, format, file.c_str(), line, column,
           message.c_str(), context.c_str()) ;
}
