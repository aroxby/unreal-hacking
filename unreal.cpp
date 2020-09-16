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

UnrealObjectRef UnrealObjectRef::next() const {
    return UnrealObjectRef::readFromAddress(nextAddr);
}

void UnrealObjectRef::dump() const {
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
    ObjectChain(baseAddress), remainingBytes(0), nextIndex(0), nextWriteAddr(baseAddress) { }

void WritableObjectChain::appendObject(const UnrealObjectRef &obj) {
    size_t totalBytes = sizeof(obj.index) + sizeof(obj.data) + strlen(obj.name) + 7;

    if (totalBytes > remainingBytes) {
        stringstream msg;
        msg << totalBytes << " bytes required but only " << remainingBytes << " are available!";
        throw ChainOverflowError(msg.str());
    } else {
        memcpy(nextWriteAddr, &nextIndex, sizeof(nextIndex));
        nextWriteAddr = increasePointer(nextWriteAddr, sizeof(nextIndex));
        strcpy((char *)nextWriteAddr, obj.name);
        nextWriteAddr = increasePointer(nextWriteAddr, strlen(obj.name));
        memcpy(nextWriteAddr, "\0\0\0\0\0\0", 7);
        nextWriteAddr = increasePointer(nextWriteAddr, 7);
        memcpy(nextWriteAddr, &obj.data, sizeof(obj.data));
        nextWriteAddr = increasePointer(nextWriteAddr, sizeof(obj.data));
        nextIndex += 2;
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
