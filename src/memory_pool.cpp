#include "../include/memory_pool.h"
#include <algorithm>

MemoryPool::MemoryPool(size_t size) : totalSize(size) {
    if (size == 0) {
        throw std::invalid_argument("Pool size cannot be zero");
    }
    pool = new char[size];
    firstBlock = new Block(size, pool);
}

MemoryPool::~MemoryPool() {
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
                return alignedData;
            }
        }
        current = current->next;
    }

    defragment();
    return nullptr;
}

void MemoryPool::deallocate(void* ptr) {
    if (!ptr) return;

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
    size_t used = 0;
    Block* current = firstBlock;
    while (current) {
        if (current->used) used += current->size;
        current = current->next;
    }
    return used;
}