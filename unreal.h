#ifndef _INC_UNREAL_H_
#define _INC_UNREAL_H_

#include <memory>
#include "exceptions.h"

EXCEPTIONTYPE(ChainOverflowError);

class UnrealObjectRef {
public:
    static UnrealObjectRef readFromAddress(const void *addr);
    UnrealObjectRef(unsigned long index, const char *name, const void *data, const void *nextAddr);
    UnrealObjectRef next();
    void dump();

    // TODO: These objects should be immutable, then we wouldn't need 'friend's
    unsigned long index;
    const char *name;
    const void *data;

private:
    const void *nextAddr;
    friend class WritableObjectChain;
};

class ObjectChain {
public:
    ObjectChain(const void *baseAddress);
    UnrealObjectRef first() const;

private:
    const void *head;
};

class WritableObjectChain : public ObjectChain {
public:
    static WritableObjectChain allocateChain(size_t bytes);

    WritableObjectChain(void *baseAddress);
    void appendObject(const UnrealObjectRef &obj);

private:
    size_t remainingBytes;
    UnrealObjectRef tail;
    std::unique_ptr<unsigned char> allocated;
};

void *createUnrealData();

#endif//_INC_UNREAL_H_
