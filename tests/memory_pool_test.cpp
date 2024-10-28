#include <gtest/gtest.h>
#include "memory_pool.h"

TEST(MemoryPoolTest, CanAllocate) {
    MemoryPool pool(1024);
    void* ptr = pool.allocate(128);
    ASSERT_NE(ptr, nullptr);
}

TEST(MemoryPoolTest, AllocationTooLarge) {
    MemoryPool pool(1024);
    void* ptr = pool.allocate(2048);
    ASSERT_EQ(ptr, nullptr);
}

TEST(MemoryPoolTest, TracksUsedSize) {
    MemoryPool pool(1024);
    pool.allocate(128);
    ASSERT_EQ(pool.getUsedSize(), 128);
    pool.allocate(256);
    ASSERT_EQ(pool.getUsedSize(), 384);
}