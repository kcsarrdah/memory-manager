#include <gtest/gtest.h>
#include "memory_pool.h"
#include <thread>
#include <vector>
#include <atomic>

TEST(MemoryPoolTest, CanAllocate) {
   MemoryPool pool(1024);
   void* ptr = pool.allocate(128);
   ASSERT_NE(ptr, nullptr);
}

TEST(MemoryPoolTest, AllocationTooLarge) {
    std::cout << "Starting AllocationTooLarge test" << std::endl;
    MemoryPool pool(1024);
    std::cout << "Pool created" << std::endl;

    // First try a successful allocation
    void* small_ptr = pool.allocate(512);
    ASSERT_NE(small_ptr, nullptr);
    std::cout << "Small allocation successful" << std::endl;

    // Now try the too-large allocation
    void* ptr = pool.allocate(2048);
    std::cout << "Large allocation attempted" << std::endl;
    ASSERT_EQ(ptr, nullptr);
    std::cout << "Test completed" << std::endl;
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

TEST(MemoryPoolTest, DynamicGrowth) {
    MemoryPool pool(512, 1024);  // Initial 512B, max 1KB

    // First allocation within initial size
    void* ptr1 = pool.allocate(400);
    ASSERT_NE(ptr1, nullptr);

    // Second allocation triggers growth
    void* ptr2 = pool.allocate(400);
    ASSERT_NE(ptr2, nullptr);

    // Third allocation should fail (exceeds max size)
    void* ptr3 = pool.allocate(400);
    ASSERT_EQ(ptr3, nullptr);
}

// Test maximum size constraints
TEST(MemoryPoolTest, MaxSizeConstraint) {
    MemoryPool pool(512, 1024);
    EXPECT_EQ(pool.getMaxSize(), 1024);
    EXPECT_EQ(pool.getFreeSize(), 512);
}

// Test invalid construction parameters
TEST(MemoryPoolTest, InvalidConstruction) {
    EXPECT_THROW(MemoryPool(0, 1024), std::invalid_argument);
    EXPECT_THROW(MemoryPool(1024, 512), std::invalid_argument);
}

// Thread safety tests
TEST(MemoryPoolTest, BasicThreadSafety) {
    MemoryPool pool(1024, 2048);
    std::vector<std::thread> threads;
    std::atomic<int> successCount(0);

    // Create multiple threads that allocate and deallocate
    for(int i = 0; i < 4; i++) {
        threads.emplace_back([&pool, &successCount]() {
            void* ptr = pool.allocate(128);
            if(ptr != nullptr) {
                successCount++;
                pool.deallocate(ptr);
            }
        });
    }

    // Wait for all threads to complete
    for(auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount, 4);
    EXPECT_TRUE(pool.isEmpty());
}

// Stress test with multiple threads
TEST(MemoryPoolTest, ThreadStressTest) {
    MemoryPool pool(1024, 4096);
    std::vector<std::thread> threads;
    std::atomic<int> allocFailures(0);
    std::atomic<int> successfulAllocs(0);

    // Create threads that perform multiple allocations/deallocations
    for(int i = 0; i < 8; i++) {
        threads.emplace_back([&pool, &allocFailures, &successfulAllocs]() {
            std::vector<void*> ptrs;
            for(int j = 0; j < 5; j++) {
                void* ptr = pool.allocate(64);
                if(ptr) {
                    ptrs.push_back(ptr);
                    successfulAllocs++;
                } else {
                    allocFailures++;
                }
            }
            // Deallocate all successful allocations
            for(void* ptr : ptrs) {
                pool.deallocate(ptr);
            }
        });
    }

    // Wait for all threads to complete
    for(auto& thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(pool.isEmpty());
    EXPECT_GT(successfulAllocs, 0);
}

// Test pool growth limits
TEST(MemoryPoolTest, GrowthLimits) {
    MemoryPool pool(256, 512);

    // Fill initial pool
    void* ptr1 = pool.allocate(200);
    ASSERT_NE(ptr1, nullptr);

    // Trigger first growth
    void* ptr2 = pool.allocate(200);
    ASSERT_NE(ptr2, nullptr);

    // This should fail as it would exceed max size
    void* ptr3 = pool.allocate(200);
    ASSERT_EQ(ptr3, nullptr);

    // Cleanup
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
}

// Test memory reuse after growth
TEST(MemoryPoolTest, MemoryReuseAfterGrowth) {
    MemoryPool pool(512, 1024);

    // Initial allocation
    void* ptr1 = pool.allocate(400);
    ASSERT_NE(ptr1, nullptr);

    // Trigger growth
    void* ptr2 = pool.allocate(400);
    ASSERT_NE(ptr2, nullptr);

    // Free first pointer
    pool.deallocate(ptr1);

    // Should reuse the first block
    void* ptr3 = pool.allocate(400);
    ASSERT_NE(ptr3, nullptr);
    EXPECT_EQ(ptr1, ptr3);
}