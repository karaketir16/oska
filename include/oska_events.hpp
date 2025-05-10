#ifndef OSKA_EVENTS_HPP
#define OSKA_EVENTS_HPP

#include <tuple>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <type_traits>

namespace oska {

template <typename T>
struct TypeId {
    static size_t value() {
        static const char unique = 0;
        return reinterpret_cast<size_t>(&unique);
    }
};

// ---- EventTraits (Must be specialized for each event) ---- //
template<typename EventTag>
struct EventTraits;

// ---- Macros to declare and define events ---- //
#define OSKA_DECLARE_EVENT(name) struct name;

#define OSKA_DEFINE_EVENT(name, ...)                       \
    struct name {};                                        \
    template<> struct oska::EventTraits<name> {            \
        using Args = std::tuple<__VA_ARGS__>;              \
    };

// ---- Callback and Event Wrapper ---- //
using Callback = std::function<void(void*)>;

struct EventWrapper {
    size_t tag;
    void* data;

    EventWrapper() : tag(oska::TypeId<void>::value()), data(nullptr) {}
    EventWrapper(std::size_t t, void* d) : tag(t), data(d) {}
};

// ---- Event Queue ---- //
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

private:
    std::queue<EventWrapper> queue;
};

// ---- Event Loop ---- //
class EventLoop {
public:
    void post(size_t tag, void* data) {
        queue.push({tag, data});
    }

    void connect(size_t tag, Callback cb) {
        callbacks[tag] = cb;
    }

    void run() {
        while (true) {
            EventWrapper ev;
            if (queue.pop(ev)) {
                auto it = callbacks.find(ev.tag);
                if (it != callbacks.end()) {
                    it->second(ev.data);
                }
            }
        }
    }

private:
    EventQueue queue;
    std::unordered_map<size_t, Callback> callbacks;
};

// ---- Type Traits for Event Arguments ---- //
template<typename Tuple, typename F>
struct is_invocable_from_tuple;

template<typename... Args, typename F>
struct is_invocable_from_tuple<std::tuple<Args...>, F> {
    static constexpr bool value = std::is_invocable_v<F, Args...>;
};


// ---- CormanManager ---- //
class CormanManager {
public:
    template<typename EventTag, typename Func>
    void connect(EventLoop* loop, Func handler) {
        using ExpectedArgs = typename EventTraits<EventTag>::Args;

        static_assert(is_invocable_from_tuple<ExpectedArgs, Func>::value,
                    "Handler is not callable with arguments from EventTraits");

        Callback cb = [handler](void* data) {
            auto tuple = static_cast<ExpectedArgs*>(data);
            std::apply(handler, *tuple);
            delete tuple;
        };

        auto tag = oska::TypeId<EventTag>::value();

        bindings[tag] = {loop, cb};
        if (loop) loop->connect(tag, cb);
    }

    template<typename EventTag, typename... PassedArgs>
    void gen(PassedArgs&&... args) {
        using ExpectedArgs = typename EventTraits<EventTag>::Args;
        using ProvidedArgs = std::tuple<std::decay_t<PassedArgs>...>;

        static_assert(std::is_same<ProvidedArgs, ExpectedArgs>::value,
                      "Argument types do not match EventTraits");

        auto* tuple = new ExpectedArgs{std::forward<PassedArgs>(args)...};
        dispatch(oska::TypeId<EventTag>::value(), static_cast<void*>(tuple));
    }

private:
    void dispatch(size_t tag, void* data) {
        auto it = bindings.find(tag);
        if (it != bindings.end() && it->second.target) {
            it->second.target->post(tag, data);
        }
    }

    struct Binding {
        EventLoop* target;
        Callback callback;
    };

    std::unordered_map<size_t, Binding> bindings;
};

// ---- Global Manager Instance ---- //
inline CormanManager Corman;

} // namespace oska

#endif // OSKA_EVENTS_HPP
