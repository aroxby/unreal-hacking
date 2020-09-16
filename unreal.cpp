#include <iostream>
#include <sstream>
#include <cstring>
#include "memory.h"
#include "unreal.h"
using namespace std;

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
    cout << '('
        << int(index) << ", "
        << (void*)(name) << '(' << name << "), "
        << data << ", "
        << nextAddr
        << ")\n";
}

ObjectChain::ObjectChain(const void *baseAddress) : head(baseAddress) {
}

UnrealObjectRef ObjectChain::first() const {
    return UnrealObjectRef::readFromAddress(head);
}

const void *ObjectChain::getBaseAddress() const {
    return head;
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
        msg << totalBytes << " bytes required but only " << remainingBytes << " are available!";
        throw ChainOverflowError(msg.str());
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

unique_ptr<WritableObjectChain> createUnrealData() {
    WritableObjectChain localChain = WritableObjectChain::allocateChain(1024);
    unique_ptr<WritableObjectChain> chain(new WritableObjectChain(move(localChain)));
    UnrealObjectRef none(0, "None", nullptr, nullptr);
    chain.get()->appendObject(none);
    return chain;
}
