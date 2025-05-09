#ifndef OSKA_EVENTS_HPP
#define OSKA_EVENTS_HPP

#include <cstdint>
#include <unordered_map>
#include <queue>
#include <functional>
#include <tuple>
#include <memory>

namespace oska {

class EventID {
public:
    using IdType = uint32_t;

    static EventID make() {
        return EventID{nextID++};
    }

    IdType value() const { return id; }
    operator IdType() const { return id; }
    bool operator==(const EventID& other) const { return id == other.id; }

    
private:
    explicit EventID(IdType i) : id(i) {}
    IdType id;
    static inline IdType nextID = 0;
};

using Callback = std::function<void(void*)>;

struct EventWrapper {
    EventID::IdType id;
    void* data;
};

class EventQueue {
public:
    void push(const EventWrapper& ev) {
        queue.push(ev);
    }

    bool pop(EventWrapper& out) {
        if (queue.empty()) return false;
        out = queue.front();
        queue.pop();
        return true;
    }

    bool empty() const {
        return queue.empty();
    }

private:
    std::queue<EventWrapper> queue;
};

class EventLoop {
public:
    void post(EventID::IdType id, void* data) {
        queue.push({id, data});
    }

    void connect(EventID::IdType id, Callback cb) {
        callbacks[id] = cb;
    }

    void run() {
        while (true) {
            EventWrapper ev;
            if (queue.pop(ev)) {
                auto it = callbacks.find(ev.id);
                if (it != callbacks.end()) {
                    it->second(ev.data);
                }
            }
        }
    }

private:
    EventQueue queue;
    std::unordered_map<EventID::IdType, Callback> callbacks;
};

class CormanManager {
public:
    template<typename... Args>
    void connect(EventID::IdType id, EventLoop* loop, std::function<void(Args...)> handler) {
        Callback cb = [handler](void* data) {
            auto tuple = static_cast<std::tuple<Args...>*>(data);
            std::apply(handler, *tuple);
            delete tuple;
        };
        bindings[id] = {loop, cb};
        if (loop) loop->connect(id, cb);
    }

    template<typename... Args>
    void connect(EventID::IdType id, EventLoop* loop, void (*handler)(Args...)) {
        connect(id, loop, std::function<void(Args...)>(handler));
    }

    template<typename... Args>
    void gen(EventID::IdType id, Args... args) {
        auto tuple = new std::tuple<Args...>(args...);
        dispatch(id, static_cast<void*>(tuple));
    }

private:
    void dispatch(EventID::IdType id, void* data) {
        auto it = bindings.find(id);
        if (it != bindings.end()) {
            if (it->second.target) {
                it->second.target->post(id, data);
            }
        }
    }

    struct Binding {
        EventLoop* target;
        Callback callback;
    };

    std::unordered_map<EventID::IdType, Binding> bindings;
};

inline CormanManager Corman;

#define OSKA_EVENT(name) inline const oska::EventID name = oska::EventID::make();

} // namespace oska

#endif // OSKA_EVENTS_HPP
