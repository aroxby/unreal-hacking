#include <iostream>
#include <sstream>
#include <cstring>
#include "memory.h"
#include "unreal.h"
using namespace std;

const UnrealObjectRef UnrealObjectRef::null(0, nullptr, nullptr);

UnrealObjectRef UnrealObjectRef::readFromAddress(const void *addr) {
    const unsigned long *nextIndex = (const unsigned long *)addr;
    const char *nextName = (const char *)(nextIndex + 1);
    const void **nextData = (const void **)(nextName + strlen(nextName) + 8);

    return UnrealObjectRef(*nextIndex, nextName, *nextData);
}

UnrealObjectRef::UnrealObjectRef(unsigned long index, const char *name, const void *data)
    : index(index), name(name), data(data) {
}

size_t UnrealObjectRef::totalLength() const {
    size_t length = sizeof(index) + sizeof(data) + strlen(name) + 8;
    return length;
}

ostream &UnrealObjectRef::dump(ostream &os) const {
    return os << '('
        << int(index) << ", "
        << (void*)(name) << '(' << name << "), "
        << data
        << ')';
}

ostream &operator<<(ostream &os, const UnrealObjectRef &obj) {
    return obj.dump(os);
}

ObjectChain::Iterator::Iterator(const void *readAddress) : readAddress(readAddress) {
}

UnrealObjectRef ObjectChain::Iterator::operator*() {
    if(readAddress) {
        return UnrealObjectRef::readFromAddress(readAddress);
    } else {
        return UnrealObjectRef::null;
    }
}

void ObjectChain::Iterator::operator++() {
    UnrealObjectRef oldRef = **this;
    unsigned long oldIndex = oldRef.index;
    size_t offset = oldRef.totalLength();

    readAddress = increasePointer(readAddress, offset);
    // FIXME: Possible read overflow
    UnrealObjectRef newRef = **this;
    if(newRef.index != oldIndex + 2) {
        readAddress = nullptr;
    }
}

bool ObjectChain::Iterator::operator!=(const ObjectChain::Iterator &other) {
    return readAddress != other.readAddress;
}

ObjectChain::ObjectChain(const void *baseAddress) : head(baseAddress) {
}

const void *ObjectChain::getBaseAddress() const {
    return head;
}

ObjectChain::Iterator ObjectChain::begin() const {
    return Iterator(head);
}

ObjectChain::Iterator ObjectChain::end() const {
    return Iterator(nullptr);
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
    size_t totalBytes = sizeof(obj.index) + sizeof(obj.data) + strlen(obj.name) + 8;

    if (totalBytes > remainingBytes) {
        stringstream msg;
        msg << totalBytes << " bytes required but only " << remainingBytes << " are available!";
        throw ChainOverflowError(msg.str());
    } else {
        memcpy(nextWriteAddr, &nextIndex, sizeof(nextIndex));
        nextWriteAddr = increasePointer(nextWriteAddr, sizeof(nextIndex));
        strcpy((char *)nextWriteAddr, obj.name);
        nextWriteAddr = increasePointer(nextWriteAddr, strlen(obj.name));
        memcpy(nextWriteAddr, "\0\0\0\0\0\0\0", 8);
        nextWriteAddr = increasePointer(nextWriteAddr, 8);
        memcpy(nextWriteAddr, &obj.data, sizeof(obj.data));
        nextWriteAddr = increasePointer(nextWriteAddr, sizeof(obj.data));
        nextIndex += 2;
        remainingBytes -= totalBytes;
    }
}

unique_ptr<WritableObjectChain> createUnrealData() {
    WritableObjectChain localChain = WritableObjectChain::allocateChain(1024);
    unique_ptr<WritableObjectChain> chain(new WritableObjectChain(move(localChain)));
    UnrealObjectRef none(0, "None", nullptr);
    UnrealObjectRef prop(0, "IntProperty", nullptr);
    chain.get()->appendObject(none);
    chain.get()->appendObject(prop);
    return chain;
}
