# MemoryPool: A Custom C++ Memory Manager

## Overview
The `MemoryPool` class is a custom memory manager that enables efficient, manually controlled memory allocation and deallocation, tailored for performance-sensitive applications that require fine-grained memory management. This memory pool avoids system-level allocation overhead by managing a pre-allocated block of memory internally.

## Key Features
- **Alignment Control**: Allocations can be aligned to user-specified boundaries, improving cache performance.
- **Defragmentation**: The pool can coalesce adjacent free blocks to reduce fragmentation and increase memory reuse.
- **Custom Allocations**: Unlike standard memory managers, `MemoryPool` allows for fixed-size memory allocation within a fixed total size, enhancing performance in scenarios with repetitive allocation and deallocation patterns.

## How It Works
1. **Pre-allocated Memory Block**: `MemoryPool` allocates a large contiguous block on initialization.
2. **Block Management**: Each allocation is managed as a "Block," with details like size, usage status, and alignment stored.
3. **Alignment Handling**: The `alignPointer` function ensures that allocated blocks meet the specified alignment constraints.
4. **Defragmentation**: When blocks are freed, adjacent free blocks are merged to minimize fragmentation and maximize reuse.

## Installation

### Clone the Repository

```bash
git clone <your_repository_url>
cd memory-pool-library
```

### Using with an IDE (e.g., CLion, Visual Studio Code)
1. Open the project in your IDE.
2. Set up CMake: Ensure CMake is installed and configured in your IDE (most C++ IDEs have built-in CMake support).
3. Add Tests: Tests are provided in the tests folder using Google Test.
4. Build and Run: Build the project, and you can run the tests to verify functionality.

### Using from Command Line

1. **Install CMake and Google Test**:
   * Ensure CMake is installed on your system.
   * Clone and install Google Test (if it’s not available in your environment).
2. **Build the Library**:
```
mkdir build
cd build
cmake ..
make
```
3. **Run Tests:**
`./tests/MemoryPoolTest`

## Usage

### Initialize the MemoryPool:
```
MemoryPool pool(1024);  // 1024 bytes total size
```
### Allocate and Deallocate Memory:
```
void* ptr1 = pool.allocate(128);       // Allocates 128 bytes
void* ptr2 = pool.allocate(64, 16);    // Allocates 64 bytes with 16-byte alignment
pool.deallocate(ptr1);                 // Deallocates 128 bytes
```
### Get Memory Usage:
```
size_t usedSize = pool.getUsedSize();
size_t freeSize = pool.getFreeSize();
```

## Differences from Standard Memory Managers
* **Fixed Size**: The pool has a fixed maximum size, unlike dynamic memory allocation via new or malloc.
* **Custom Defragmentation**: Adjacent blocks are merged when freed, which is typically managed by the OS in standard managers.
* **Manual Control**: Fine-tuned control over block alignment and allocation patterns, which may lead to performance gains in certain use cases.

