#include <iostream>
#include <vector>

#include "thread_utils.h"

void dummyFunction(int a, int b, bool sleep) 
{
    std::cout << "dummyFunction(" << a + b  << ")" << std::endl;

    if (sleep) {
        std::cout << "dummyFunction sleeping..." << std::endl;

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
    }

    std::cout << "dummyFunction done." << std::endl;
}

// int main()
// {
//     auto funcBody = [](int x, int z)
//     {
//         int y = x + z + 9;

//         std::cout << "thread is running now with y = " << y << std::endl;

//         using namespace std::literals::chrono_literals;
//         std::this_thread::sleep_for(10s);

//         std::cout << "I am out." << std::endl;
//     };

//     using namespace Common;

//     std::thread* bruh1 = createAndStartThread(1, "my name", funcBody, 1, 10);
//     std::thread* bruh2 = createAndStartThread(1, "my name", funcBody, 2, 20);
//     std::thread* bruh3 = createAndStartThread(1, "my third name", funcBody, 2, 20);

//     auto t1 = createAndStartThread(-1, "dummyFunction1", dummyFunction, 12, 21, false);
//     auto t2 = createAndStartThread(1, "dummyFunction2", dummyFunction, 15, 51, true);

//     std::vector<std::thread*> threads{t1, t2, bruh1, bruh2, bruh3};

//     for (std::thread* t : threads) t->join();

//     std::cout << "program exitting" << std::endl;
//     return 0;
// }
