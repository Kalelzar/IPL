#ifndef CL_SYNTAX_HIGHLIGHTER_STL
#define CL_SYNTAX_HIGHLIGHTER_STL

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

template <typename Type> using STLVector = std::vector<Type>;

using STLString = std::string;

template <typename Ptr> using STLSharedPointer = std::shared_ptr<Ptr>;

template <typename Ptr, class... Args>
inline STLSharedPointer<Ptr> makeSTLSharedPointer(Args&&... args){
  return std::make_shared<Ptr, Args...>(args ...);
}

using STLException = std::exception;
using STLRuntimeError = std::runtime_error;


#endif // CL_SYNTAX_HIGHLIGHTER_STL
