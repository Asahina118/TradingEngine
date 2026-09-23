#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "macros.h"

namespace Common
{
    template <typename T>
    class MemPool final 
    {
        struct ObjectBlock
        {
            T object_;
            bool isFree_ = true;
        };

        std::vector<ObjectBlock> store_;
        size_t nextFreeIndex_ = 0;

        void updateNextFreeIndex() noexcept
        {
            size_t initIndex = nextFreeIndex_;
            while (!store_[nextFreeIndex_].isFree_) {
                nextFreeIndex_++;
                if (UNLIKELY(nextFreeIndex_ == store_.size())) {
                    nextFreeIndex_ = 0;
                }
                if (UNLIKELY(nextFreeIndex_ == initIndex)) {
                    ASSERT(initIndex != nextFreeIndex_, "Memory pool is out of space");
                }
            }
        }
    
    public:
        explicit MemPool(size_t size) : store_(size, {T(), true}) 
        {
            // check the first object is T
            ASSERT(
                reinterpret_cast<const ObjectBlock*>(&(store_[0].object_)) == &(store_[0]),
                "T object should be the first member of ObjectBlock."
            );
        }

        template <typename... Args>
        T* allocate(Args... args) noexcept
        {
            ObjectBlock* objBlock = &(store_[nextFreeIndex_]);
            ASSERT(objBlock->isFree_, "Object Block is not free at index: " + std::to_string(nextFreeIndex_));
            objBlock->isFree_ = false;

            updateNextFreeIndex();

            return new(&(objBlock->object_)) T(args...);
        }

        void deallocate(T* ptr) noexcept
        {
            size_t index = reinterpret_cast<const ObjectBlock*>(ptr) - &store_[0];
            ASSERT(index < store_.size(), "Pointer location is outside of the memory pool");
            ASSERT(!store_[index].isFree_, "Deallocating a free memory location.");
            reinterpret_cast<ObjectBlock*>(ptr)->isFree_ = true;
        }

        MemPool() = delete;

        MemPool(const MemPool& other) = delete;
        MemPool(MemPool&& other) = delete;
        MemPool& operator=(const MemPool& ohter) = delete;
        MemPool& operator=(MemPool&& other) = delete;
    };
}