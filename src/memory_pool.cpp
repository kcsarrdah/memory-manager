#include "../include/memory_pool.h"

MemoryPool::MemoryPool(size_t size) : totalSize(size) {
    pool = new char[size];
}

MemoryPool::~MemoryPool() {
    delete[] pool;
}

void* MemoryPool::allocate(size_t size) {
    // Basic implementation for now
    if (size > totalSize) return nullptr;
    return pool;
}

void MemoryPool::deallocate(void* ptr) {
    // Basic implementation for now
}