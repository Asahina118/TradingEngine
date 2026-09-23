#include <iostream>

int main()
{
    const double price = 10.125;
    constexpr double min_price_increment = 0.005;
    int64_t intPrice = 0;

    // no strength reduction
    intPrice = price / min_price_increment;
    std::cout << intPrice << std::endl;
    /*
        strength reduction in which notice that the inverse is calculated in compile time so runtime efficiency is not affected at all!
    */
    constexpr double min_price_increment_inv = 1 / min_price_increment;
    intPrice = price * min_price_increment_inv;
    std::cout << intPrice << std::endl;

    return 0;
}