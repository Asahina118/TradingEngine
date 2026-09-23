#include "chapter 4/mem_pool.h"
#include <iostream>

static int dtorCallCount = 0;

struct Data
{
    int x_;
    int y_;
    Data() = default;
    Data(int x, int y) : x_(x), y_(y) {}
};

std::ostream& operator<<(std::ostream& os, const Data& data) {
    return os << "(" << data.x_ << ", " << data.y_ << ")";
}

template <typename L, typename R>
struct Pair
{
    L left_;
    R right_;

public:
    Pair() {std::cout << "pair ctor called" << std::endl;}
    Pair(L left, R right) : left_(left), right_(right) {}
    L getLeft() {return left_;}
    R getRight() {return right_;}
    ~Pair() 
    {
        std::cout << "pair destructor called" << std::endl;
        ++dtorCallCount;
    }

    void print()
    {
        std::cout << "(" << left_ << ", " << right_ << ")" << std::endl;
    }
};

int main()
{
    constexpr size_t POOL_SIZE = 3;
    Common::MemPool<Pair<double, int>> pool(sizeof(Pair<double, int>));
    auto pairPtr = pool.allocate(12.99, 8);
    pairPtr->print();

    std::cout << "number of times dtor called is: " << dtorCallCount << std::endl;

    std::cout << "program exitting..." << std::endl;

    Common::MemPool<Pair<std::string, Data>> pool2(sizeof(Pair<std::string, Data>));
    std::string keyString = "key";
    auto pairPtr2 = pool2.allocate(keyString, Data(11, -911));
    pairPtr2->print();
    pool2.deallocate(pairPtr2);
    pairPtr2->print();
    return 0;
}