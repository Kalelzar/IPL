#ifndef CL_SYNTAX_HIGHLIGHTER_LEXER
#define CL_SYNTAX_HIGHLIGHTER_LEXER

#include "Token.hpp"

namespace cl {

class Lexer {

private:
  char *buffer = nullptr;
  size_t bufferSize = 0;

  STLString file;

  uint32_t line = 0;
  uint32_t column = 0;

  uint32_t point = 0;
  uint32_t mark = 0;
  bool markp = true;

  void enableMark();
  void disableMark();
  void switchMarkAndPoint();

  bool endp();

  char next();
  char peek();
  char previous();
  char forward(uint32_t offset) {
    if (point + offset >= bufferSize + 1) {
      emitError("Unexpected EOF");
    }
    return buffer[point + offset];
  }

  void emitError(const STLString &errorMessage);

  void emitToken(const TokenType type, const STLString &&lexeme);

  bool whitespacep();
  bool swallowWhitespace();

  /**
   *
   *
   * The permitted characters in a symbol name are
   * <span>
   * ~!@$%^&*_+=-0987654321qwertyuiop[]{}POIUYTR
   *  EWQasdfghjkl:LKJHGFDSAzxcvbnm./?><MNBVCXZ\|
   * </span>
   *
   * @return true if a symbol was successfully read
   */
  bool emitSymbol();

  /**
   * Check if the char at point can be part of a symbol name
   *
   * The permitted characters in a symbol name are
   * <span>
   * ~!@$%^&*_+=-0987654321qwertyuiop[]{}POIUYTR
   *  EWQasdfghjkl:LKJHGFDSAzxcvbnm./?><MNBVCXZ\|
   * </span>
   *
   * It is actually easier to just check for the inverse.
   */
  bool symbolp();

  /**
   * Returns the list of characters that
   * can only exist inside a symbol (or a string)
   */
  bool strictSymbolP();

  /**
   * Check if the char at point is a 'special' char
   * as in it means something special to the language and
   * can't appear in any other lexical constructs without being escaped (except
   *inside strings).
   *
   * In Common Lisp those are:
   *
   * ;       - Beginning of comment
   * '       - Reader macro for (quote ...)
   *\\`      - Reader macro
   * ( and ) - Delimit Expressions
   * ,       - Needs to be inside backquote (\`)
   * "       - Delimits a string
   */
  bool specialp(); // DONE

  bool numericp(); // DONE

  bool emitIntegral();      // DONE
  bool emitFloatingPoint(); // DONE
  bool emitString();        // DONE

  bool emitReaderMacro(); // DONE
  bool emitSharpMacro();  // DONE

  bool emitVector();    // DONE
  bool emitBitVector(); // DONE

  bool emitCharacter();           // DONE
  bool emitBinaryIntegral();      // DONE
  bool emitOctalIntegral();       // DONE
  bool emitHexadecimalIntegral(); // DONE
  bool emitNRadixIntegral();      // DONE
  bool emitComplexNumber();       // DONE

  bool emitArray();        // DONE
  bool emitStructure();    // DONE
  bool emitPath();         // DONE
  bool emitLabel();        // DONE
  bool emitReferToLabel(); // DONE

  bool emitReadTimeTestPlus();  // DONE
  bool emitReadTimeTestMinus(); // DONE

  bool emitLeftParentheses();  // DONE
  bool emitRightParentheses(); // DONE

  bool emitQuote();                // DONE
  bool emitBackquote();            // DONE
  bool emitComma();                // DONE
  bool emitCommaAtSign();          // DONE
  bool emitCommaDot();             // DONE
  bool emitFunctionAbbreviation(); // DONE
  bool emitUninternedSymbol();     // DONE
  bool emitReadTimeEvaluation();   // DONE
  bool emitReadTimeError();        // DONE

  bool emitDot();     // DONE
  bool emitComment(); // DONE

  bool emitEof();

  typedef STLVector<Token> Result;

  Result result;

public:
  Result &lex(const STLString &filePath);
};

} // namespace cl

#endif // CL_SYNTAX_HIGHLIGHTER_LEXER
