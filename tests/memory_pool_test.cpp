#include <gtest/gtest.h>
#include "memory_pool.h"

TEST(MemoryPoolTest, CanAllocate) {
    MemoryPool pool(1024);
    void* ptr = pool.allocate(128);
    ASSERT_NE(ptr, nullptr);
}

TEST(MemoryPoolTest, AllocationTooLarge) {
    MemoryPool pool(1024);
    void* ptr = pool.allocate(2048);  // Try to allocate more than pool size
    ASSERT_EQ(ptr, nullptr);  // Should fail and return nullptr
}