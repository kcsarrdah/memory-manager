#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <memory>

class MemoryPool {
private:
    struct Block {
        size_t size;
        bool used;
        char* data;
        Block* next;
        size_t alignment;

        Block(size_t s, char* d, size_t align = sizeof(void*))
            : size(s), used(false), data(d), next(nullptr), alignment(align) {}
    };

    char* pool;
    size_t totalSize;
    size_t maxSize;
    Block* firstBlock;
    std::vector<void*> allocated;
    mutable std::mutex mtx;  // Changed to mutable
    std::vector<std::unique_ptr<char[]>> additionalPools;
    std::vector<Block*> additionalBlocks;

    void defragment();
    char* alignPointer(char* ptr, size_t alignment);
    bool growPool(size_t requestedSize);

public:
    explicit MemoryPool(size_t size, size_t maxPoolSize = 0);
    ~MemoryPool();

    void* allocate(size_t size, size_t alignment = sizeof(void*));
    void deallocate(void* ptr);

    size_t getUsedSize() const;
    bool isEmpty() const { return getUsedSize() == 0; }
    size_t getFreeSize() const { return totalSize - getUsedSize(); }
    size_t getMaxSize() const { return maxSize; }

    // Disable copy operations
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
};

#endif