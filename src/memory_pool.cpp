#include "../include/memory_pool.h"
#include <algorithm>
#include <iostream>

MemoryPool::MemoryPool(size_t initialSize, size_t maxPoolSize)
    : totalSize(initialSize), maxSize(maxPoolSize == 0 ? initialSize : maxPoolSize) {
    if (initialSize == 0) {
        throw std::invalid_argument("Pool size cannot be zero");
    }
    if (maxSize < initialSize) {
        throw std::invalid_argument("Max size cannot be less than initial size");
    }

    pool = new char[initialSize];
    firstBlock = new Block(initialSize, pool);
}


MemoryPool::~MemoryPool() {
    std::lock_guard<std::mutex> lock(mtx);
    Block* current = firstBlock;
    while (current) {
        Block* next = current->next;
        delete current;
        current = next;
    }
    delete[] pool;
}


char* MemoryPool::alignPointer(char* ptr, size_t alignment) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t aligned = (addr + (alignment - 1)) & ~(alignment - 1);
    return reinterpret_cast<char*>(aligned);
}

void* MemoryPool::allocate(size_t size, size_t alignment) {
    if (size == 0) return nullptr;

    std::cout << "Attempting to allocate " << size << " bytes" << std::endl;
    std::lock_guard<std::mutex> lock(mtx);

    // First attempt: try to find a suitable block
    Block* current = firstBlock;
    while (current) {
        if (!current->used) {
            char* alignedData = alignPointer(current->data, alignment);
            size_t alignmentAdjustment = alignedData - current->data;
            size_t totalRequired = size + alignmentAdjustment;

            if (current->size >= totalRequired) {
                if (current->size > totalRequired + sizeof(Block)) {
                    Block* newBlock = new Block(
                        current->size - totalRequired,
                        current->data + totalRequired,
                        alignment
                    );
                    newBlock->next = current->next;
                    current->next = newBlock;
                    current->size = totalRequired;
                }

                current->used = true;
                current->alignment = alignment;
                allocated.push_back(alignedData);
                std::cout << "Allocation successful" << std::endl;
                return alignedData;
            }
        }
        current = current->next;
    }

    // If we get here, no suitable block was found. Try growing.
    if (size <= maxSize - totalSize) {
        if (growPool(size * 2)) {
            // After growing, try one more time to allocate
            current = firstBlock;
            while (current) {
                if (!current->used) {
                    char* alignedData = alignPointer(current->data, alignment);
                    size_t alignmentAdjustment = alignedData - current->data;
                    size_t totalRequired = size + alignmentAdjustment;

                    if (current->size >= totalRequired) {
                        if (current->size > totalRequired + sizeof(Block)) {
                            Block* newBlock = new Block(
                                current->size - totalRequired,
                                current->data + totalRequired,
                                alignment
                            );
                            newBlock->next = current->next;
                            current->next = newBlock;
                            current->size = totalRequired;
                        }

                        current->used = true;
                        current->alignment = alignment;
                        allocated.push_back(alignedData);
                        return alignedData;
                    }
                }
                current = current->next;
            }
        }
    }

    // If we get here, allocation failed
    std::cout << "Allocation failed" << std::endl;
    return nullptr;
}

bool MemoryPool::growPool(size_t requestedSize) {
    // No mutex here since this is called from allocate which already has the lock
    std::cout << "Growing pool by " << requestedSize << " bytes" << std::endl;

    size_t newPoolSize = std::min(requestedSize, maxSize - totalSize);
    if (newPoolSize == 0) {
        std::cout << "Cannot grow pool further" << std::endl;
        return false;
    }

    try {
        auto newPool = std::make_unique<char[]>(newPoolSize);
        auto newBlock = new Block(newPoolSize, newPool.get());

        // Add to the end of the block list
        Block* current = firstBlock;
        while (current->next) {
            current = current->next;
        }
        current->next = newBlock;

        additionalPools.push_back(std::move(newPool));
        additionalBlocks.push_back(newBlock);

        totalSize += newPoolSize;
        std::cout << "Pool grown successfully" << std::endl;
        return true;
    } catch (...) {
        std::cout << "Growth failed" << std::endl;
        return false;
    }
}

void MemoryPool::deallocate(void* ptr) {
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(mtx);
    Block* current = firstBlock;
    while (current) {
        char* alignedData = alignPointer(current->data, current->alignment);
        if (alignedData == ptr) {
            current->used = false;
            auto it = std::find(allocated.begin(), allocated.end(), ptr);
            if (it != allocated.end()) {
                allocated.erase(it);
            }
            defragment();
            return;
        }
        current = current->next;
    }
}

void MemoryPool::defragment() {
    // No mutex here since this is called from functions that already have the lock
    Block* current = firstBlock;
    while (current && current->next) {
        if (!current->used && !current->next->used) {
            Block* temp = current->next;
            current->size += temp->size;
            current->next = temp->next;
            delete temp;
        } else {
            current = current->next;
        }
    }
}
size_t MemoryPool::getUsedSize() const {
    std::lock_guard<std::mutex> lock(mtx);
    size_t used = 0;
    Block* current = firstBlock;
    while (current) {
        if (current->used) used += current->size;
        current = current->next;
    }
    return used;
}