#ifndef CL_SYNTAX_HIGHLIGHTER_TOKEN
#define CL_SYNTAX_HIGHLIGHTER_TOKEN

#include "stl.hpp"
#include <cstring>

namespace cl {

/**
 * Common Lisp is a bit special in that there really aren't
 * any reserved keywords as in other languages. Yes, even things like
 * 'defun' and 'defvar' are technically symbols from the 'common-lisp' package.
 *
 * I ~could~ have a token for each exported symbol
 * but there happen to be 978 according to
 * ANSI INCITS 226-1994 specification.
 *
 * That is frankly unrealistic and counterproductive seeing as
 * they are meant to be used as ordinary Common Lisp symbols.
 *
 * However as this is meant to be a syntax highlighter it might be
 * fruitful to treat some commonly used symbols specially.
 *
 * Regardless they are all going to be treated as any other
 * identifier by the lexer.
 */
enum TokenType {

  // Basic types
  Integral,      // DONE
  FloatingPoint, // DONE
  String,        // DONE
  Symbol,        // DONE

  Vector,    // SharpLeftParen DONE
  BitVector, // SharpStar DONE

  SharpBackslash, // Character DONE
  SharpB,         // DONE
  SharpO,         // DONE
  SharpX,         // DONE
  SharpR,         // DONE
  SharpC,         // DONE

  SharpA, // DONE

  SharpS, // DONE
  SharpP, // DONE
  // Parentheses

  LeftParentheses,  // DONE
  RightParentheses, // DONE

  // Symbol manipulation

  Quote,       // DONE
  Backquote,   // DONE
  Comma,       // DONE
  CommaAtSign, // DONE
  CommaDot,    // DONE
  SharpQuote,  // DONE
  SharpColon,  // DONE
  SharpDot,    // DONE

  // Misc

  Dot,             // DONE
  Comment,         // DONE
  SharpEqual,      // DONE
  SharpSharp,      // DONE
  SharpPlus,       // DONE
  SharpMinus,      // DONE
  SharpRightParen, // DONE
  Whitespace,
  Newline,
  Error,
  Eof

};

STLString tokenTypeToString(cl::TokenType);
void genTokenTypeStyle();

struct Token {
  const TokenType type;
  const uint32_t line;
  const uint32_t column;
  const STLString lexeme;

  Token(TokenType type, uint32_t line, uint32_t column,
        const STLString &&lexeme)
      : type(type), line(line), column(column), lexeme(lexeme) {}
};

class LexError : public STLException {
public:
  LexError(uint32_t line, uint32_t column, STLString file, STLString context,
           STLString message);

  LexError(const LexError &other);

  ~LexError();

  LexError &operator=(const LexError &other);

  const char *what() const noexcept override { return exceptionMessage; }

private:
  const char *format = "Encountered error at %s:%d:%d: %s\n---> %s";

  void copy(const LexError &other);

  void initMessage(const char *message);

  void initMessage(uint32_t line, uint32_t column, STLString file,
                   STLString context, STLString message);

  char *exceptionMessage = nullptr;
};

} // namespace cl
#endif // CL_SYNTAX_HIGHLIGHTER_TOKEN
