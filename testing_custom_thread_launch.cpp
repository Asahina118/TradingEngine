#include "Common/thread_utils.h"
#include <vector>
#include <thread>
int main()
{
    auto myFunction = [](int integer) {std::cout << "captured the integer: " << integer << std::endl;;};
    std::thread* t = Common::createAndStartThread(
        -1,
        "my thread",
        myFunction,
        622
    );
    std::vector<std::thread*> threads;

    for (int i = 77 ; i < 1000 ; i += 51)
    {
        threads.push_back(Common::createAndStartThread(
            -1, "thread: " + std::to_string(i),
            myFunction,
            i
        ));
    }

    t->join();
    for (std::thread*& t : threads) t->join();
    std::cout << "exiting.." << std::endl;
    return 0;
}