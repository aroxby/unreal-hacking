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

#endif//_INC_EXCEPTIONS_H_
