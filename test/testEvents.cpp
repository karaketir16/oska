#include "oska_events.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace oska;

// Event IDs
OSKA_EVENT(evNoArgs)
OSKA_EVENT(evOneArg)
OSKA_EVENT(evTwoArgs)


// Event loops (simulating cores)
EventLoop coreA;
EventLoop coreB;

// Handlers
void handleNoArgs() {
    std::cout << "[coreA] No-arg handler executed.\n";
}

void handleOneArg(int x) {
    std::cout << "[coreB] One-arg handler: x = " << x << "\n";
}

void handleTwoArgs(int a, const char* b) {
    std::cout << "[coreB] Two-arg handler: a = " << a << ", b = " << b << "\n";
}

int main() {
    // Register handlers
    Corman.connect(evNoArgs, &coreA, handleNoArgs);
    Corman.connect(evOneArg, &coreB, handleOneArg);
    Corman.connect(evTwoArgs, &coreB, handleTwoArgs);

    // Start loops on threads
    std::thread threadA([] { coreA.run(); });
    std::thread threadB([] { coreB.run(); });

    // Generate events
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Corman.gen(evNoArgs);
    Corman.gen(evOneArg, 42);
    Corman.gen(evTwoArgs, 7, "oska", 6);

    // Let events process briefly, then exit
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Exiting test...\n";
    std::exit(0);
}
