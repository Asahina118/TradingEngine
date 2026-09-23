#pragma once

#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>

#include <sys/syscall.h>

namespace Common
{

    // called inside the thread body
    inline bool setThreadId(int coreId) noexcept
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(coreId, &cpuset);

        return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0);
    }

    template <typename Func, typename... Args>
    inline std::thread* createAndStartThread(
        int coreId,
        const std::string& name,
        Func&& func,
        Args&&... args
    ) noexcept
    {
        std::atomic<bool> running(false), failed(false);

        auto threadBody = [&]
        {
            if (coreId >= 0 && !setThreadId(coreId)) {
                std::cout << "Failed to set core affinity for " << name << " " << pthread_self() << " to " << coreId << std::endl;
                failed = true;
                return;
            }

            std::cout << "Set core affinity for " << name << " with id: " << pthread_self() << ", to " << coreId << std::endl;

            running = true;
            func(std::forward<Args>(args)...);
        };

        std::thread* t = new std::thread(threadBody);

        while (!running && !failed) {
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1s);
        }

        if (failed) {
            t->join();
            delete t;
            t = nullptr;
        }

        return t;
    }
}