#include "Lexer.hpp"
#include <fstream>
#include <iostream>

void cl::Lexer::enableMark() {
  mark = point;
  markp = true;
}

void cl::Lexer::disableMark() { markp = false; }

void cl::Lexer::switchMarkAndPoint() {
  uint32_t temporary = point;
  point = mark;
  mark = temporary;
}

bool cl::Lexer::endp() { return buffer[point] == '\0'; }

char cl::Lexer::next() {

  if (endp())
    emitError("Unexpected EOF while grabbing next character.");
  column++;
  return buffer[point++];
}

char cl::Lexer::peek() { return buffer[point]; }

char cl::Lexer::previous() {
  if (point == 0)
    emitError("Unexpected SOF.");
  return buffer[point - 1];
}

bool cl::Lexer::whitespacep() {
  char upcoming = peek();
  return upcoming == ' ' || upcoming == '\t' || upcoming == '\n' ||
         upcoming == '\r' || isblank(upcoming);
}

bool cl::Lexer::emitReaderMacro() {
  if (endp())
    return false;

  enableMark();

  switch (peek()) {
  case '#':
    return emitSharpMacro();
  }

  disableMark();
  return false;
}

bool cl::Lexer::emitSharpMacro() {
  if (!markp)
    enableMark();
  if (endp() && peek() != '#')
    return false;

  next();

  if (endp()) {
    emitError("Unexpected EOF while parsing a reader macro.");
  }

  switch (peek()) {
  case '\\':
    return emitCharacter();
  case '\'':
    return emitFunctionAbbreviation();
  case ':':
    return emitUninternedSymbol();
  case '.':
    return emitReadTimeEvaluation();
  case 'B':
    return emitBinaryIntegral();
  case 'O':
    return emitOctalIntegral();
  case 'X':
    return emitHexadecimalIntegral();
  case 'C':
    return emitComplexNumber();
  case 'S':
    return emitStructure();
  case 'P':
    return emitPath();
  case ')':
    return emitReadTimeError();
  }

  while (!endp() && isdigit(peek())) {
    next();
  }

  if (endp()) {
    emitError("Unexpected EOF while parsing a reader macro.");
  }

  // Reader macros with optional numeric argument.

  switch (peek()) {
  case '(':
    return emitVector();
  case '*':
    return emitBitVector();
  case 'R':
    return emitNRadixIntegral();
  case 'A':
    return emitArray();
  case '=':
    return emitLabel();
  case '#':
    return emitReferToLabel();
  case '+':
    return emitReadTimeTestPlus();
  case '-':
    return emitReadTimeTestMinus();
  }

  if (mark != point) {
    emitError("Unexpected character while parsing reader macro.");
  }

  disableMark();
  return false;
}

bool cl::Lexer::swallowWhitespace() {
  if (endp() || !whitespacep())
    return false;

  enableMark();

  unsigned ogMark = mark;

  while (!endp() && whitespacep()) {
    if (peek() == '\n') {
      if (mark == point) {
        emitToken(TokenType::Whitespace, " ");
      } else {
        emitToken(TokenType::Whitespace,
                  std::move(STLString(buffer + mark, point - mark)));
      }
      enableMark();
      next();
      emitToken(TokenType::Newline,
                std::move(STLString(buffer + mark, point - mark)));
      column = 0;
      line++;
      enableMark();
    } else if (peek() == '\r') {
      if (mark == point) {
        emitToken(TokenType::Whitespace, " ");
      } else {
        emitToken(TokenType::Whitespace,
                  std::move(STLString(buffer + mark, point - mark)));
      }
      enableMark();
      next();
      if (!endp() && peek() == '\n') {
        next();
      }
      emitToken(TokenType::Newline,
                std::move(STLString(buffer + mark, point - mark)));
      enableMark();
      column = 0;
      line++;
    } else {
      next();
    }
  }

  if (mark != point) {
    emitToken(TokenType::Whitespace,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else if (ogMark != point && mark == point) {
    return true;
  }
  return false;
}

bool cl::Lexer::emitEof() {

  if (endp()) {
    emitToken(TokenType::Eof, "");
    return true;
  }
  return false;
}

bool cl::Lexer::emitComment() {

  if (markp) {
    emitError("A comment was found in the middle of another expression.");
  }
  enableMark();

  if (endp()) {
    disableMark();
    return false;
  }

  if (peek() != ';') {
    disableMark();
    return false;
  }

  while (!endp() && peek() == ';')
    next();

  while (!endp() && peek() != '\n' && peek() != '\r') {
    next();
  }

  if (mark != point) {
    emitToken(TokenType::Comment,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::specialp() {
  if (endp())
    return false;
  // Anything can go in if you escape it.
  if (point != 0 && previous() == '\\')
    return true;

  switch (peek()) {
  case ';':
  case '\'':
  case '(':
  case ')':
  case ',':
  case '"':
    return true;
  default:
    return false;
  }
}

bool cl::Lexer::symbolp() {
  if (endp())
    return false;

  bool validSymbol =
      !specialp() || (specialp() && point != 0 && previous() == '\\');

  return validSymbol && !whitespacep();
}

bool cl::Lexer::emitSymbol() {

  // the only character that you can't use in the
  // beginning of a symbol name but can after is '#'
  if ((!markp && point != mark) && !endp() && peek() == '#') {
    disableMark();
    return false;
  }

  if (!markp)
    enableMark();

  while (symbolp()) {
    next();
  }

  if (mark != point) {
    emitToken(TokenType::Symbol,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }

  disableMark();
  return false;
}

bool cl::Lexer::numericp() {
  if (endp())
    return false;
  if (point != 0 && previous() == '\\')
    return false;

  return isdigit(peek());
}

bool cl::Lexer::strictSymbolP() {
  if (endp())
    return false;
  if (peek() == '\\')
    return true;

  if (numericp())
    return false;
  if (whitespacep())
    return false;

  if (peek() == '.')
    return false;

  return symbolp();
}

bool cl::Lexer::emitIntegral() {
  enableMark();

  if (!endp() && peek() == '-') {
    next();
    if (!numericp()) {
      return emitSymbol();
    }
  } else if (!endp() && numericp()) {
    next();
  } else {
    disableMark();
    return false;
  }

  while (!endp() && numericp()) {
    next();
  }

  if (!endp() && peek() == '.') {
    return emitFloatingPoint();
  }

  if (mark == point) {
    disableMark();
    return false;
  }

  if (strictSymbolP()) {
    return emitSymbol();
  }

  emitToken(TokenType::Integral,
            std::move(STLString(buffer + mark, point - mark)));

  disableMark();
  return true;
}

bool cl::Lexer::emitFloatingPoint() {

  if (!markp) {
    enableMark();
  }

  if (peek() == '.') {

    next();
  } else {

    if (mark == point) {

      disableMark();
      return false;
    } else {

      return emitSymbol();
    }
  }

  if (endp() || !isdigit(peek())) {

    if (mark + 1 == point) { // this is just matching a .

      return emitDot();
    }

    if (endp() || whitespacep()) {

      // Oddly enough SBCL says that numbers that end in a dot are integers.
      // No idea if this is compiler specific.
      emitToken(TokenType::Integral,
                std::move(STLString(buffer + mark, point - mark)));
      disableMark();
      return true;
    } else {

      return emitSymbol();
    }
  }

  while (!endp() && isdigit(peek())) {
    next();
  }

  if (strictSymbolP()) {
    return emitSymbol();
  }

  emitToken(TokenType::FloatingPoint,
            std::move(STLString(buffer + mark, point - mark)));

  disableMark();

  return true;
}

bool cl::Lexer::emitDot() {
  if (!markp)
    enableMark();
  if (peek() == '.') {
    next();
    if (emitSymbol()) {
      return true;
    } else {
      disableMark();
      emitToken(TokenType::Dot, ".");
    }
  }

  return false;
}

void cl::Lexer::emitError(const STLString &message) {

  // I genuinely hate the person who wrote this. Wait a moment...

  static const int contextRange = 16;

  unsigned maxJumpForward = 0;

  unsigned startingPoint = point > 0 ? point - 1 : 0;

  while (!endp() && !whitespacep() && peek() != ')' && peek() != '(') {
    next();
    maxJumpForward++;
  }

  STLString context = STLString(buffer + startingPoint, maxJumpForward + 1);

  static const char *format = "<br>(%s)~%s";

  size_t messageSize =
      snprintf(nullptr, 0, format, message.c_str(), context.c_str()) + 1;

  if (messageSize <= 0) {
    throw STLRuntimeError("Error during formatting of error message.");
  }

  char *exceptionMessage_s = new char[messageSize];

  // Why isn't there a std::string format function???
  // Oh there is? In c++20? Why though?
  snprintf(exceptionMessage_s, messageSize, format, message.c_str(),
           context.c_str());

  exceptionMessage_s[messageSize - 1] = '\0';

  STLString exceptionMessage = STLString(exceptionMessage_s, messageSize - 1);

  delete[] exceptionMessage_s;

  emitToken(TokenType::Error, std::move(exceptionMessage));
  disableMark();
}

bool cl::Lexer::emitString() {
  enableMark();

  if (peek() == '"') {
    next();
  } else {
    return false;
  }

  while (!endp() && (peek() != '"' || (point != 0 && previous() == '\\'))) {
    next();
  }

  if (previous() ==
          '\\' // last double quote is escaped so the string is not terminated.
      || peek() != '"') {
    emitError("Unterminated string.");
  }

  next();
  emitToken(TokenType::String,
            std::move(STLString(buffer + mark, point - mark)));
  disableMark();
  return true;
}

bool cl::Lexer::emitLabel() {

  if (!markp)
    return false;

  if (!endp() && peek() == '=') {
    next();
    emitToken(TokenType::SharpEqual,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#=' reader macro");
  }
  return false;
}

bool cl::Lexer::emitPath() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'P') {
    next();
    emitToken(TokenType::SharpP,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#P' reader macro");
  }
  return false;
}

bool cl::Lexer::emitReadTimeError() {

  if (!markp)
    return false;

  if (!endp() && peek() == ')') {
    next();
    // Ordinarily this would signal to the scanner to throw an error.
    // But since this is a syntax highlighter and not a proper
    // CL interpreter it doesn't make sense to just end it here.
    // Though there probably should be some warning that this file
    // doesn't contain valid CL code
    emitToken(TokenType::SharpRightParen,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#)' reader macro");
  }
  return false;
}

bool cl::Lexer::emitComplexNumber() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'C') {
    next();
    emitToken(TokenType::SharpC,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#C' reader macro");
  }
  return false;
}

bool cl::Lexer::emitStructure() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'S') {
    next();
    emitToken(TokenType::SharpS,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#S' reader macro");
  }
  return false;
}

bool cl::Lexer::emitArray() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'A') {
    next();
    emitToken(TokenType::SharpA,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#A' reader macro");
  }
  return false;
}

bool cl::Lexer::emitNRadixIntegral() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'R') {
    next();
    emitToken(TokenType::SharpR,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#R' reader macro");
  }
  return false;
}

bool cl::Lexer::emitOctalIntegral() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'O') {
    next();
    emitToken(TokenType::SharpO,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#O' reader macro");
  }
  return false;
}

bool cl::Lexer::emitBinaryIntegral() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'B') {
    next();
    emitToken(TokenType::SharpB,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#B' reader macro");
  }
  return false;
}

bool cl::Lexer::emitHexadecimalIntegral() {

  if (!markp)
    return false;

  if (!endp() && peek() == 'X') {
    next();
    emitToken(TokenType::SharpX,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#X' reader macro");
  }
  return false;
}

bool cl::Lexer::emitReadTimeEvaluation() {

  if (!markp)
    return false;

  if (!endp() && peek() == '.') {
    next();
    emitToken(TokenType::SharpDot,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#.' reader macro");
  }
  return false;
}

bool cl::Lexer::emitUninternedSymbol() {

  if (!markp)
    return false;

  if (!endp() && peek() == ':') {
    next();
    emitToken(TokenType::SharpColon,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#:' reader macro");
  }
  return false;
}

bool cl::Lexer::emitBitVector() {

  if (!markp)
    return false;

  if (!endp() && peek() == '*') {
    next();
    emitToken(TokenType::BitVector,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#*' reader macro");
  }
  return false;
}

bool cl::Lexer::emitReadTimeTestMinus() {

  if (!markp)
    return false;

  if (!endp() && peek() == '-') {
    next();
    emitToken(TokenType::SharpMinus,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#-' reader macro");
  }
  return false;
}

bool cl::Lexer::emitRightParentheses() {
  enableMark();

  if (!endp() && peek() == ')') {
    next();
    emitToken(TokenType::RightParentheses,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitLeftParentheses() {
  enableMark();

  if (!endp() && peek() == '(') {
    next();
    emitToken(TokenType::LeftParentheses,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitQuote() {
  enableMark();

  if (!endp() && peek() == '\'') {
    next();
    emitToken(TokenType::Quote,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitBackquote() {
  enableMark();

  if (!endp() && peek() == '`') {
    next();
    emitToken(TokenType::Backquote,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitCommaAtSign() {
  enableMark();

  if (!endp() && peek() == '@') {
    next();
    emitToken(TokenType::CommaAtSign,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitCommaDot() {
  enableMark();

  if (!endp() && peek() == '.') {
    next();
    emitToken(TokenType::CommaDot,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitComma() {
  enableMark();

  if (!endp() && peek() == ',') {
    next();
    if (peek() == '@') {
      return emitCommaAtSign();
    } else if (peek() == '.') {
      return emitCommaDot();
    } else {
      emitToken(TokenType::Comma,
                std::move(STLString(buffer + mark, point - mark)));
      disableMark();
      return true;
    }
  }
  disableMark();
  return false;
}

bool cl::Lexer::emitReadTimeTestPlus() {

  if (!markp)
    return false;

  if (!endp() && peek() == '+') {
    next();
    emitToken(TokenType::SharpPlus,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#+' reader macro");
  }
  return false;
}

bool cl::Lexer::emitReferToLabel() {

  if (!markp)
    return false;

  if (!endp() && peek() == '#') {
    next();
    emitToken(TokenType::SharpSharp,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '##' reader macro");
  }
  return false;
}

bool cl::Lexer::emitVector() {

  if (!markp)
    return false;

  if (!endp() && peek() == '(') {
    next();
    emitToken(TokenType::Vector,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#(' reader macro");
  }
  return false;
}

bool cl::Lexer::emitFunctionAbbreviation() {

  if (!markp)
    return false;

  if (!endp() && peek() == '\'') {
    next();

    emitToken(TokenType::SharpQuote,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#'' reader macro");
  }
  return false;
}

bool cl::Lexer::emitCharacter() {

  if (!markp)
    return false;

  if (!endp() && peek() == '\\') {
    next();

    emitToken(TokenType::SharpBackslash,
              std::move(STLString(buffer + mark, point - mark)));
    disableMark();
    return true;
  } else {
    emitError("Incomplete '#\\' reader macro");
  }
  return false;
}

void cl::Lexer::emitToken(const TokenType type, const STLString &&lexeme) {
  result.push_back({type, line, column, std::move(lexeme)});
}

/**
 * Load an entire file into RAM
 * Not really efficient.
 * It should probably read the file in chunks.
 * But it doesn't.
 */
STLString readFile(const STLString &fileName) {
  std::ifstream ifs(fileName.c_str(), std::ios::ate);

  std::ifstream::pos_type fileSize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  STLVector<char> bytes(fileSize);
  ifs.read(bytes.data(), fileSize);

  return STLString(bytes.data(), fileSize);
}

/**
 *
 * Take a Common Lisp <b>source</b> file and return the result of
 * tokenizing it.
 *
 */
cl::Lexer::Result &cl::Lexer::lex(const STLString &filepath) {
  auto filecontents = readFile(filepath);
  buffer = (char *)filecontents.c_str();
  bufferSize = strlen(buffer);
  result = Result();
  file = filepath;
  bool tryAgain=false;
  while (!endp()) {
    // I hate this. And so should you
    disableMark();
    if (!(emitComment() || emitString() || emitLeftParentheses() ||
          emitRightParentheses() || emitQuote() || emitBackquote() ||
          emitIntegral() || emitComma() || emitReaderMacro() || emitDot() ||
          emitSymbol() || swallowWhitespace())) {

      if (endp())
        break;

      if(!tryAgain){
        tryAgain = true;
        continue;
      }else{
        tryAgain = false;
      }
      emitError("Failed to parse any token.");
    }
  }

  if (endp()) {
    emitEof();
  }

  return result;
}
