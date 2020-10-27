#include "Lexer.hpp"
#include "Token.hpp"
//#include "Parser.hpp"

#include <iostream>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "You need to provide the name of the .lisp"
                 " file to convert to html."
              << std::endl;
    return 1;
  } else if (argc > 3) {
    std::clog << "Ignoring argument 2+" << std::endl;
  }

  char* file = argv[1];

  try {
    STLVector<cl::Token> tokens = cl::Lexer().lex(file);

    unsigned line = 0;
    int depth = -1;
    bool afterOpenParen = false;
    std::cout << "<html>" << std::endl
              << "<head>" << std::endl
              << "<link rel=\"stylesheet\" href=\"syntax.css\">" << std::endl
              << "</head>" << std::endl
              << "<body>" << std::endl;

    std::cout << "<div class=\"filepath\">" << std::endl << "<ul>" << std::endl;

    STLString fileString = STLString(file, strlen(file) + 1);

    while(fileString.size() > 0){
      int next = fileString.find_first_of('/');
      if(next == -1){
        std::cout<<"<li class=\"file\">"
               <<fileString
               <<"</li>"
               <<std::endl;
        break;
      }
      std::cout<<"<li>"
               <<fileString.substr(0, next+1)
               <<"</li>"
               <<std::endl;
      std::cout<<next+1<<" "<<fileString.size();
      fileString = fileString.substr(next+1);
    }

    std::cout << "</ul>"
              << std::endl
              << "</div>"
              << std::endl;


    std::cout << "<div class=\"code\">" << std::endl
              << "<ol id=\"code\">" << std::endl
              << "<li>" << std::endl
              << "<div line=\"1\">" << std::endl;
    for (cl::Token token : tokens) {
      if (line != token.line) {
        std::cout << "</div>" << std::endl
                  << "</li>" << std::endl
                  << "<li>" << std::endl
                  << "<div line=\"" << token.line << "\">" << std::endl;
        line = token.line;
      }
      switch (token.type) {
      case cl::TokenType::Newline:
        std::cout << "<br/>" << std::endl;
        afterOpenParen = false;
        break;
      case cl::TokenType::LeftParentheses:
        depth++;
        afterOpenParen = true;
      case cl::TokenType::RightParentheses:
        std::cout << "<span class=\"parenthesis\" depth=\"";
        if (depth < 0) {
          std::cout << "error";
        } else {
          std::cout << depth % 7;
        }
        std::cout << "\">";
        std::cout << token.lexeme;
        std::cout << "</span>" << std::endl;
        if (token.type == cl::TokenType::RightParentheses) {
          depth--;
          afterOpenParen = false;
        }
        break;
      case cl::TokenType::Symbol:
        std::cout << "<span class=\"";
        if (token.lexeme.c_str()[0] == ':') {
          std::cout << "property";
        } else if (token.lexeme.c_str()[0] == '&') {
          std::cout << "key";
        } else if (afterOpenParen) {
          std::cout << "funcall";
        } else {
          std::cout << "Symbol";
        }
        std::cout << "\">";
        std::cout << token.lexeme;
        std::cout << "</span>" << std::endl;
        afterOpenParen = false;
        break;
      case cl::TokenType::Error:

        std::cout << "<span class=\"error\">";

        std::cout << token.lexeme.substr(token.lexeme.find('~') + 1);
        std::cout << "</span>" << std::endl;
        std::cout << "<span class=\"hidden-error\">";

        std::cout << token.lexeme.substr(0, token.lexeme.find('~'));
        std::cout << "</span>";
        break;
      default:
        std::cout << "<span class=\"" << cl::tokenTypeToString(token.type)
                  << "\">";
        std::cout << token.lexeme;
        std::cout << "</span>" << std::endl;
        afterOpenParen = false;
        break;
      }
    }
    std::cout << "</li>" << std::endl
              << "</ol>" << std::endl
              << "</div>" << std::endl
              << "</body>" << std::endl
              << "</html>" << std::endl;
  } catch (cl::LexError &error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }

  return 0;
}
