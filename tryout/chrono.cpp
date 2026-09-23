#include <chrono>
#include <iostream>

// using namespace std::chrono;

// same as std::chrono::duration::seconds
using MyOwnMSType = std::chrono::duration<int, std::ratio<1, 1000>>;
using MyOwnMSTypeF = std::chrono::duration<float, std::ratio<1, 1000>>;
using MyOwnHundredthS = std::chrono::duration<int, std::ratio<1, 100>>;



void func(MyOwnMSType) {}

int main()
{
    int duration = 3;
    MyOwnMSType durationMs(2);
    MyOwnMSTypeF durationMsF(6.4f);

    std::chrono::seconds myNewDuration(15);
    std::cout << "time passed is " << myNewDuration.count() << std::endl;

    return 0;
}