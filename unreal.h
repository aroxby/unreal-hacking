#ifndef _INC_UNREAL_H_
#define _INC_UNREAL_H_

#include <memory>
#include "exceptions.h"

EXCEPTIONTYPE(ChainOverflowError);

class UnrealObjectRef {
public:
    static UnrealObjectRef readFromAddress(const void *addr);
    UnrealObjectRef(unsigned long index, const char *name, const void *data);
    size_t totalLength() const;
    std::ostream &dump(std::ostream &os) const;

    const static UnrealObjectRef null;

    const unsigned long index;
    const char * const name;
    const void * const data;
};

std::ostream &operator<<(std::ostream &os, const UnrealObjectRef &obj);

class ObjectChain {
public:
    class Iterator {
    public:
        Iterator(const void *readAddress);
        UnrealObjectRef operator*();
        // FIXME: Possible read overflow
        void operator++();
        bool operator!=(const Iterator &other);

    private:
        const void *readAddress;
    };

    ObjectChain(const void *baseAddress);
    const void *getBaseAddress() const;
    Iterator begin() const;
    Iterator end() const;

private:
    const void *head;
};

class WritableObjectChain : public ObjectChain {
public:
    static WritableObjectChain allocateChain(size_t bytes);

    WritableObjectChain(void *baseAddress);
    void appendObject(const UnrealObjectRef &obj);

private:
    std::unique_ptr<unsigned char> allocated;
    size_t remainingBytes;
    unsigned long nextIndex;
    void *nextWriteAddr;
};

std::unique_ptr<WritableObjectChain> createUnrealData();

#endif//_INC_UNREAL_H_
