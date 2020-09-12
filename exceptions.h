#ifndef _INC_EXCEPTIONS_H_
#define _INC_EXCEPTIONS_H_

#include <exception>
#include <string>

class SimpleException : public std::exception {
public:
    SimpleException(const std::string &what): msg(what) {}
    virtual const char *what() const noexcept override { return msg.c_str(); }
private:
    const std::string msg;
};

#define EXCEPTIONTYPE(TYPENAME) \
class TYPENAME : public SimpleException { \
    public: TYPENAME(const std::string &what) : SimpleException(what) {} \
};

#endif//_INC_EXCEPTIONS_H_
