#pragma once

#include "MemoryPool.hpp"
#include <utility>

template<typename T, size_t Count>
class ObjectPool
{
private:
    MemoryPool<T, Count> _pool;
public:
    // Creating an object in-place, forwarding any of its arguments 
    template<typename... Args>
    T* create(Args&&... args)
    {
        // raw is the address: a void* pointer to the pre-allocated memory. 
        // It points to uninitialized raw memory popped from the freeList
        void* raw = _pool.allocate();
        T* obj = new(raw) T(std::forward<Args>(args)...); // Placement-new constructs the object in that block
                                                           
        return obj;
    }

    // Destroy an object and recycle its storage
    void destroy(T* obj)
    {
        if (!obj) { return; }
        obj->~T();
        _pool.deallocate(static_cast<void*>(obj));
    }
};

// T is essentially the object type that is stored in the blocks of our memory buffer. 
// When we call create on an object pointer, we are allocating a single block for it 
// in our buffer first as a void* that we take from our free list, and then we 
// placement-new that object into that memory block