#include "oska_events.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace oska;


// ---- Event Queue ---- //
class EventQueue : public EventQueueInterface {
public:
    void push(const EventWrapper& ev) override {
        queue.push(ev);
    }

    bool pop(EventWrapper& out) override {
        if (queue.empty()) return false;
        out = queue.front();
        queue.pop();
        return true;
    }

private:
    std::queue<EventWrapper> queue;
};

// ---- Event Loop ---- //
class EventLoop : public EventLoopInterface {
public:
    EventLoop() : queue(new EventQueue()) {}

    void post(size_t tag, void* data) override {
        queue->push({tag, data});
    }

    void connect(size_t tag, Callback cb) override {
        callbacks[tag] = cb;
    }

    void run() override {
        while (true) {
            EventWrapper ev;
            if (queue->pop(ev)) {
                auto it = callbacks.find(ev.tag);
                if (it != callbacks.end()) {
                    it->second(ev.data);
                }
            }
        }
    }

private:
    EventQueueInterface* queue;
    std::unordered_map<size_t, Callback> callbacks;
};

OSKA_DEFINE_EVENT(EvPrint, int, std::string)


OSKA_DEFINE_EVENT(evNoArgs)
OSKA_DEFINE_EVENT(evOneArg, int)
OSKA_DEFINE_EVENT(evTwoArgs, int, const char*)

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
    //Corman.connect<evNoArgs>(&coreA, handleOneArg); //should fail in compile time
    Corman.connect<evNoArgs>(&coreA, handleNoArgs);
    Corman.connect<evOneArg>(&coreB, handleOneArg);
    Corman.connect<evTwoArgs>(&coreB, handleTwoArgs);

    oska::Corman.connect<EvPrint>(&coreA, [](int x, std::string str) {
        std::cout << "[coreA] EvPrint handler: x = " << x << ", str = " << str << "\n";
    });
    

    // Start loops on threads
    std::thread threadA([] { coreA.run(); });
    std::thread threadB([] { coreB.run(); });

    // Generate events
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Corman.gen<evNoArgs>();
    Corman.gen<evOneArg>(42);
    Corman.gen<evTwoArgs>(7, "oska");
    oska::Corman.gen<EvPrint>(42, std::string("Hello from Oska"));

    // Let events process briefly, then exit
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Exiting test...\n";
    std::exit(0);
}
