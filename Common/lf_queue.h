#pragma once

#include <iostream>
#include <vector>
#include <atomic>

#include <source_location>

namespace Common
{
    template <typename T>
    class LFQueue final
    {
    public:
        LFQueue(size_t numElements) : store_(numElements, T()), nextWriteIndex_(0), nextReadIndex_(0), numElements_(0) {}

        LFQueue(const LFQueue& other) = delete;
        LFQueue(LFQueue&& other) = delete;
        LFQueue& operator=(const LFQueue& other) = delete;
        LFQueue& operator=(LFQueue&& other) = delete;

        bool push(T data)
        {
            if (numElements_.load(std::memory_order_acquire) == store_.size()) {
                std::cout << "queue is full" << std::endl;
                return false;
            }

            store_[nextWriteIndex_.load(std::memory_order_relaxed)] = data;

            return true;
        }

        void updateWriteNext()
        {
            size_t writeIdx = nextWriteIndex_.load(std::memory_order_acquire);
            nextWriteIndex_.store((writeIdx + 1) % store_.size(), std::memory_order_release);
            numElements_++;
        }

        T* get()
        {
            auto location = std::source_location::current();
            if (numElements_.load(std::memory_order_acquire) == 0) {
                std::cerr << "No elements left to read: " << location.function_name() << " at line " << location.line() << std::endl;
                return nullptr;
            }

            return &store_[nextReadIndex_.load(std::memory_order_relaxed)];
        }

        T* getNextWritePtr() noexcept
        {
            return &store_[nextWriteIndex_.load(std::memory_order_acquire)];
        }

        void updateReadNext()
        {
            size_t readIdx = nextReadIndex_.load(std::memory_order_acquire);
            nextReadIndex_.store((readIdx + 1) % store_.size(), std::memory_order_release);
            numElements_--;
        }

        void print() const
        {
            std::cout << "(";
            for (const T data : store_) {
                std::cout << data << ", ";
            }
            std::cout << ")\n";
        }

        size_t size() const noexcept { return numElements_.load(); }

    private:
        std::vector<T> store_;
        std::atomic<size_t> nextWriteIndex_;
        std::atomic<size_t> nextReadIndex_;

        std::atomic<size_t> numElements_;
    };
}