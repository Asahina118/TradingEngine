#include <csignal>

#include "matcher/matching_engine.h"

Common::Logger* logger = nullptr;
Exchange::MatchingEngine* matchingEngine = nullptr;

void signalHandler(int)
{
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(10s);

    delete logger; logger = nullptr;
    delete matchingEngine; matchingEngine = nullptr;
    std::cout << "Shutting down gracefully..." << std::endl;

    std::this_thread::sleep_for(10s);

    std::cout << "Shut down completed. Now exiting..." << std::endl;
    exit(EXIT_SUCCESS);
}

int main(int, char**)
{
    std::cout << "running the main in " << __FILE__ << std::endl;
    logger = new Common::Logger("exchange_main.log");
    std::signal(SIGINT, signalHandler);

    const int sleepTime = 100 * 1000;

    Exchange::ClientRequestLFQueue clientRequests(ME_MAX_CLIENT_UPDATES);
    Exchange::ClientResponseLFQueue clientResponses(ME_MAX_CLIENT_UPDATES);
    Exchange::MEMarketUpdateLFQueue marketUpdates(ME_MAX_MARKET_UPDATES);

    std::string timeStr;

    std::cout << "calling matchingEngine: " << __FILE__ << std::endl;
    matchingEngine = new Exchange::MatchingEngine(&clientRequests, &clientResponses, &marketUpdates);
    
    while (1)
    {
        // indefinitely waiting for interrupt signals from the OS (users)
        std::cout << "sleeping in main()" << std::endl;
        usleep(sleepTime * 1000);
    }

    std::cout << "Shutting down gracefully..." << std::endl;
}