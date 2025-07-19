#pragma once

#include <cstddef>
#include <type_traits>
#include <stdexcept>
#include <unordered_set>
#include <mutex>
#include <cassert>
#include <new>

template<typename T, std::size_t BlockCount>
class MemoryPool {
private:
    struct Node { Node* next; };

    static_assert(std::is_nothrow_destructible<T>::value,
                  "MemoryPool requires T to be nothrow-destructible");

    // Ensure each slot can hold either a T or a Node
    static constexpr std::size_t BlockSize  =
        (sizeof(T)   > sizeof(Node) ? sizeof(T)   : sizeof(Node));
    static constexpr std::size_t BlockAlign =
        (alignof(T)  > alignof(Node) ? alignof(T)  : alignof(Node));

    using AlignedBuffer = std::aligned_storage_t<BlockSize, BlockAlign>;

    AlignedBuffer                 buffer[BlockCount];
    Node*                         freeListHead;
    std::unordered_set<void*>     overflowPtrs;
    mutable std::mutex            _mtx;

    // Helper to check if a pointer lies in our static buffer
    bool inPoolRange(void* ptr) const {
        auto start = reinterpret_cast<std::uintptr_t>(&buffer[0]);
        auto end   = start + BlockCount * BlockSize;
        auto pval  = reinterpret_cast<std::uintptr_t>(ptr);
        return (pval >= start && pval < end);
    }

    void initializeFreeList() {
        for (std::size_t i = 0; i < BlockCount - 1; ++i) {
            Node* curr = reinterpret_cast<Node*>(&buffer[i]);
            Node* next = reinterpret_cast<Node*>(&buffer[i + 1]);
            curr->next = next;
        }
        Node* tail = reinterpret_cast<Node*>(&buffer[BlockCount - 1]);
        tail->next = nullptr;
        freeListHead = reinterpret_cast<Node*>(&buffer[0]);
    }

public:
    MemoryPool() {
        initializeFreeList();
    }

    ~MemoryPool() {
        // Clean up any overflow allocations
        for (auto p : overflowPtrs) {
            ::operator delete(p);
        }
        overflowPtrs.clear();

        // Diagnostic: ensure all in-pool blocks were returned
        std::size_t count = 0;
        for (Node* cur = freeListHead; cur; cur = cur->next) {
            ++count;
        }
        assert(count == BlockCount && "MemoryPool destroyed with blocks still in use");
    }

    // Allocate raw storage (thread-safe)
    void* allocate() {
        std::lock_guard<std::mutex> lk(_mtx);
        if (!freeListHead) {
            // Overflow to global heap
            void* p = ::operator new(sizeof(T));
            overflowPtrs.insert(p);
            return p;
        }
        Node* node = freeListHead;
        freeListHead = node->next;
        return node;
    }

    // Return storage (thread-safe)
    void deallocate(void* ptr) noexcept {
        if (!ptr) return;

        std::lock_guard<std::mutex> lk(_mtx);
        if (inPoolRange(ptr)) {
            Node* node = reinterpret_cast<Node*>(ptr);
            node->next = freeListHead;
            freeListHead = node;
        } else {
            auto it = overflowPtrs.find(ptr);
            if (it != overflowPtrs.end()) {
                ::operator delete(ptr);
                overflowPtrs.erase(it);
            } else {
                assert(false && "Attempt to deallocate unknown pointer");
            }
        }
    }

    // Disable copy
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Allow move
    MemoryPool(MemoryPool&&) = default;
    MemoryPool& operator=(MemoryPool&&) = default;
};
