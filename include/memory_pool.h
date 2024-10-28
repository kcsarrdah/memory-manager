#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <vector>
#include <stdexcept>

class MemoryPool {
private:
    struct Block {
        size_t size;
        bool used;
        char* data;
        Block* next;
        size_t alignment;  // New: track alignment

        Block(size_t s, char* d, size_t align = sizeof(void*))
            : size(s), used(false), data(d), next(nullptr), alignment(align) {}
    };

    char* pool;
    size_t totalSize;
    Block* firstBlock;
    std::vector<void*> allocated;

    void defragment();
    char* alignPointer(char* ptr, size_t alignment);

public:
    explicit MemoryPool(size_t size);
    ~MemoryPool();

    void* allocate(size_t size, size_t alignment = sizeof(void*));
    void deallocate(void* ptr);

    size_t getUsedSize() const;
    bool isEmpty() const { return getUsedSize() == 0; }
    size_t getFreeSize() const { return totalSize - getUsedSize(); }
};

#endif