#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>

class MemoryPool {
private:
    char* pool;
    size_t totalSize;
    size_t used;  // Track how much memory is used

public:
    explicit MemoryPool(size_t size);
    ~MemoryPool();

    void* allocate(size_t size);
    void deallocate(void* ptr);
    size_t getUsedSize() const { return used; }
};

#endif