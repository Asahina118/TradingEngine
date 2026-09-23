#pragma once

#include <string>
#include <fstream>
#include <cstdio>

// #include "types.h"
#include "macros.h"
#include "lf_queue.h"
#include "thread_utils.h"
#include "time_utils.h"
#include "constants.h"

namespace Common
{
    // constexpr size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024;
    enum class LogType : int8_t
    {
        CHAR = 0,
        INT = 1, LONG_INT = 2, LONG_LONG_INT = 3,
        UNSIGNED = 4, UNSIGNED_LONG = 5, UNSIGNED_LONG_LONG = 6,
        FLOAT = 7, DOUBLE = 8
    };

    struct LogElement
    {
        LogType type_ = LogType::CHAR;
        union
        {
            char c;
            int i; long l; long long ll;
            unsigned u; unsigned long ul; unsigned long long ull;
            float f; double d;
        } u_;
    };

    // this class will launch a logger thread in the thread pool but the resource ownership is managed in this Logger class
    class Logger final
    {
        const std::string fileName_;
        std::ofstream file_;
        LFQueue<LogElement> queue_;
        std::atomic<bool> running_{true};
        std::thread* loggerThread_ = nullptr;

        void flushQueue() noexcept
        {
            while (running_) 
            {
                for (auto next = queue_.get(); queue_.size() && next; next = queue_.get()) {
                    switch (next->type_) {
                        case LogType::CHAR: 
                            file_ << next->u_.c;
                            break;
                        
                        case LogType::INT:
                            file_ << next->u_.i;
                            break;

                        case LogType::LONG_INT:
                            file_ << next->u_.l;
                            break;

                        case LogType::LONG_LONG_INT:
                            file_ << next->u_.ll;
                            break;

                        case LogType::UNSIGNED:
                            file_ << next->u_.u;
                            break;

                        case LogType::UNSIGNED_LONG:
                            file_ << next->u_.ul;
                            break;

                        case LogType::UNSIGNED_LONG_LONG:
                            file_ << next->u_.ull;
                            break;

                        case LogType::FLOAT:
                            file_ << next->u_.f;
                            break;

                        case LogType::DOUBLE:
                            file_ << next->u_.d;
                            break;
                    }
                    queue_.updateReadNext();
                    next = queue_.get();
                }

                using namespace std::literals::chrono_literals;
                std::this_thread::sleep_for(1ms);
            }
        }

    public:
        explicit Logger(const std::string& fileName) : fileName_(fileName), queue_(LOG_QUEUE_SIZE)
        {
            file_.open(fileName);
            ASSERT(file_.is_open(), "Could not open log file: " + fileName);

            loggerThread_ = createAndStartThread(-1, "Common/Logger", [this]() { flushQueue(); });
            ASSERT(loggerThread_ != nullptr, "Failed to start Logger Thread");
        }

        ~Logger()
        {
            std::cerr << "Flushing and closing Logger for " << fileName_ << std::endl;

            // logger Thread is still running in the thread pool doing the flushQueue() constantly, so we will wait for this flushing to clean up the queue_ before joining the loggerThread_
            while (queue_.size()) {
                using namespace std::literals::chrono_literals;
                std::this_thread::sleep_for(1s);
            }
            running_ = false;
            loggerThread_->join();

            file_.close();
        }

        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger& operator=(Logger&&) = delete;

        void pushValue(const LogElement& logElement) noexcept
        {
            *(queue_.getNextWritePtr()) = logElement;
            queue_.updateWriteNext();
        }

        void pushValue(const char val) noexcept
        {
            pushValue(LogElement{LogType::CHAR, {.c = val}});
        }

        void pushValue(const char* val) noexcept
        {
            // here we assume val points to a string with '\0', which has ASCII 0, thats why we can use *val itself to stop the while loop below 
            while (*val) {
                pushValue(*val);
                val++;
            }
        }

        void pushValue(const std::string& val) noexcept
        {
            pushValue(val.c_str());
        }

        void pushValue(const int val) noexcept
        {
            pushValue(LogElement{LogType::INT, {.i = val}});
        }

        void pushValue(const long val) noexcept
        {
            pushValue(LogElement{LogType::LONG_INT, {.l = val}});
        }

        void pushValue(const long long val) noexcept
        {
            pushValue(LogElement{LogType::LONG_LONG_INT, {.ll = val}});
        }

        void pushValue(const unsigned val) noexcept
        {
            pushValue(LogElement{LogType::UNSIGNED, {.u = val}});
        }

        void pushValue(const unsigned long val) noexcept
        {
            pushValue(LogElement{LogType::UNSIGNED_LONG, {.ul = val}});
        }

        void pushValue(const unsigned long long val) noexcept
        {
            pushValue(LogElement{LogType::UNSIGNED_LONG_LONG, {.ull = val}});
        }

        void pushValue(const float val) noexcept
        {
            pushValue(LogElement{LogType::FLOAT, {.f = val}});
        }

        void pushValue(const double val) noexcept
        {
            pushValue(LogElement{LogType::DOUBLE, {.d = val}});
        }

        template <typename T, typename... A>
        void log(const char* s, const T& val, A... args)
        {
            while (*s)
            {
                if (*s == '%') {
                    if (UNLIKELY(*(s+1) == '%')) {
                        s++;
                    }
                    else {
                        pushValue(val);
                        log(s+1, args...);
                        return;
                    }
                }
                pushValue(*s++);
            }
            FATAL("Extra arguments provided to log()");
        }

        void log(const char* s) noexcept
        {
            while (*s) {
                if (*s == '%') {
                    if (UNLIKELY(*(s+1) == '%')) {
                        s++;
                    } else {
                        FATAL("missing arguments to log()");
                    }
                }
            }
            pushValue(*s++);
        }
    };
}
