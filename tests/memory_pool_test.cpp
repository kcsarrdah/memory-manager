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

// Alignment Tests
TEST(MemoryPoolTest, ProperAlignment) {
   MemoryPool pool(1024);
   void* ptr = pool.allocate(128, 16);  // 16-byte alignment
   ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % 16, 0);
}

// Fragmentation Tests
TEST(MemoryPoolTest, HandlesFragmentation) {
   MemoryPool pool(1024);
   void* ptr1 = pool.allocate(128);
   void* ptr2 = pool.allocate(128);
   void* ptr3 = pool.allocate(128);

   // Create fragmentation by deallocating middle block
   pool.deallocate(ptr2);

   // Should be able to allocate in the fragmented space
   void* ptr4 = pool.allocate(128);
   ASSERT_NE(ptr4, nullptr);
}

// Edge Cases
TEST(MemoryPoolTest, ZeroSizeAllocation) {
   MemoryPool pool(1024);
   void* ptr = pool.allocate(0);
   ASSERT_EQ(ptr, nullptr);
}

TEST(MemoryPoolTest, NullPtrDeallocation) {
   MemoryPool pool(1024);
   ASSERT_NO_THROW(pool.deallocate(nullptr));
}

TEST(MemoryPoolTest, ExactSizeAllocation) {
   MemoryPool pool(1024);
   void* ptr = pool.allocate(1024);
   ASSERT_NE(ptr, nullptr);
}

// Error Handling
TEST(MemoryPoolTest, ZeroSizePoolCreation) {
   ASSERT_THROW(MemoryPool pool(0), std::invalid_argument);
}

TEST(MemoryPoolTest, MultipleDeallocations) {
   MemoryPool pool(1024);
   void* ptr = pool.allocate(128);
   pool.deallocate(ptr);
   ASSERT_NO_THROW(pool.deallocate(ptr));  // Second deallocation should be safe
}

// Memory Reuse
TEST(MemoryPoolTest, MemoryReuse) {
   MemoryPool pool(1024);
   void* ptr1 = pool.allocate(128);
   pool.deallocate(ptr1);
   void* ptr2 = pool.allocate(128);
   EXPECT_EQ(ptr1, ptr2);
}

// Stress Test
TEST(MemoryPoolTest, StressTest) {
   MemoryPool pool(1024);
   std::vector<void*> ptrs;

   // Allocate many small blocks
   for(int i = 0; i < 8; i++) {
       void* ptr = pool.allocate(64);
       ASSERT_NE(ptr, nullptr);
       ptrs.push_back(ptr);
   }

   // Deallocate every other block
   for(size_t i = 0; i < ptrs.size(); i += 2) {
       pool.deallocate(ptrs[i]);
   }

   // Should be able to allocate a larger block after defragmentation
   void* large_ptr = pool.allocate(256);
   ASSERT_NE(large_ptr, nullptr);
}