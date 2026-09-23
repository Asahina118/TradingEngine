#include "chapter 4/lf_queue.h"
#include <string>
#include <chrono>
#include <thread>

struct Person
{
    std::string name_;
    int age_;
    Person() = default;
    Person(const std::string& name, int age) : name_(name), age_(age) {}
};

std::ostream& operator<<(std::ostream& os, Person person)
{
    return os << "(" << person.name_ << ", " << person.age_ << ")";
}

constexpr size_t QUEUE_SIZE = 10;
Common::LFQueue<Person> queue(QUEUE_SIZE);

void producerThread()
{
    int pushCount = 0;
    while (1) {
        bool result = queue.push(Person("Tom", pushCount));
        if (result) pushCount++;
        std::cout << "Pushing: " << pushCount << "\n";

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(0.1s);
    }
}

void consumerThread()
{
    int getCount = 0;
    bool success = false;
    while (1) {
        auto ptr = queue.get();
        if (ptr != nullptr) {
            getCount++;
            success = true;
        } else success = false;
        std::cout << "Getting: " << getCount << "; success? " << (success ? "yes" : "no") << "\n";

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(0.05s);
    }
}

void printingThread()
{
    while (1) {
        queue.print();

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(0.05s);
    }
}

int main()
{
    std::thread producer(producerThread);
    std::thread consumer(consumerThread);
    std::thread printer(printingThread);

    producer.join();
    consumer.join();
    printer.join();

    std::cout << "program exitting..." << std::endl;
    return 0;
}