#include <iostream>
#include <cstdint>
#include <cstdlib>

enum class Side : int16_t 
{
    BUY = 1, SELL = -1
};

int main()
{
    // effectivelt int16_t is just short
    const Side fillSide = rand() % 2 ? Side::BUY : Side::SELL;
    const int fillQuantity = 10;
    std::cout << "fillSide is: " << (fillSide == Side::BUY ? "BUY" : (fillSide == Side::SELL ? "SELL" : "INVALID")) << std::endl;

    { // branching
        int lastBuyQty = 0, lastSellQty = 0, position = 0;
        if (fillSide == Side::BUY) {
            position += fillQuantity;
            lastBuyQty = fillQuantity;
        } else if (fillSide == Side::SELL) {
            position -= fillQuantity;
            lastSellQty = fillQuantity;
        } else {
            printf("Bruh your program sucks");
        }
    }

    { // non-branching
        int lastQty[3] = {0, 0, 0}, position = 0;

        auto sideToInt = [](Side side) noexcept { return static_cast<int16_t>(side); };
        const auto intFillSide = sideToInt(fillSide);
        position += intFillSide * fillQuantity;
        lastQty[intFillSide + 1] = fillQuantity;
    }

    return 0;
}
