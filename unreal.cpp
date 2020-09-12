#include <memory>
#include <sstream>
#include <cstdio>
#include <cstring>
#include "exceptions.h"
#include "memory.h"
using namespace std;

class UnrealObjectRef {
public:
    static UnrealObjectRef readFromAddress(const void *addr);
    UnrealObjectRef(unsigned long index, const char *name, const void *data, const void *nextAddr);
    UnrealObjectRef next();
    void dump();

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
    unique_ptr<unsigned char> allocated;
};

UnrealObjectRef UnrealObjectRef::readFromAddress(const void *addr) {
    const unsigned long *nextIndex = (const unsigned long *)addr;
    const char *nextName = (const char *)(nextIndex + 1);
    const void **nextData = (const void **)(nextName + strlen(nextName) + 7);
    const void *nextNext = (const void *)(nextData + 1);

    return UnrealObjectRef(*nextIndex, nextName, *nextData, nextNext);
}

UnrealObjectRef::UnrealObjectRef(
    unsigned long index, const char *name, const void *data, const void *nextAddr
) : index(index), name(name), data(data), nextAddr(nextAddr) { }

UnrealObjectRef UnrealObjectRef::next() {
    return UnrealObjectRef::readFromAddress(nextAddr);
}

void UnrealObjectRef::dump() {
    printf("%i, %p(%s), %p, %p\n", int(index), name, name, data, nextAddr);
}

ObjectChain::ObjectChain(const void *baseAddress) : head(baseAddress) {
}

UnrealObjectRef ObjectChain::first() const {
    return UnrealObjectRef::readFromAddress(head);
}

WritableObjectChain WritableObjectChain::allocateChain(size_t bytes) {
    unique_ptr<unsigned char> allocated(new unsigned char[bytes]);
    WritableObjectChain chain(allocated.get());
    chain.allocated = move(allocated);
    chain.remainingBytes = bytes;
    return chain;
}

WritableObjectChain::WritableObjectChain(void *baseAddress) :
    ObjectChain(baseAddress), tail(-2, nullptr, nullptr, baseAddress), remainingBytes(0) { }

void WritableObjectChain::appendObject(const UnrealObjectRef &obj) {
    size_t totalBytes = sizeof(obj.index) + sizeof(obj.data) + strlen(obj.name) + 7;

    if (totalBytes > remainingBytes) {
        stringstream msg;
        msg << totalBytes << " require but only " << remainingBytes << " are available!";
        throw SimpleException(msg.str());
    } else {
        unsigned long nextIndex = tail.index + 2;
        void *nextWrite = (void *)tail.nextAddr;

        memcpy(nextWrite, &nextIndex, sizeof(nextIndex));
        nextWrite = increasePointer(nextWrite, sizeof(nextIndex));
        strcpy((char *)nextWrite, obj.name);
        nextWrite = increasePointer(nextWrite, strlen(obj.name));
        memcpy(nextWrite, "\0\0\0\0\0\0", 7);
        nextWrite = increasePointer(nextWrite, 7);
        memcpy(nextWrite, &obj.data, sizeof(obj.data));
        nextWrite = increasePointer(nextWrite, sizeof(obj.data));
        tail.index = nextIndex;
        tail.name = obj.name;
        tail.data = obj.data;
        tail.nextAddr = nextWrite;
        remainingBytes -= totalBytes;
    }
}

void *createUnrealData() {
    WritableObjectChain localChain = WritableObjectChain::allocateChain(1024);
    // FIXME: How to manage the life cycle of this.  Should it live forever?
    WritableObjectChain *chain = new WritableObjectChain(move(localChain));
    UnrealObjectRef none(0, "None", nullptr, nullptr);
    chain->appendObject(none);
    void *chainStartAddress = *(void **)chain;  //Hack for private data member
    return chainStartAddress;
}
