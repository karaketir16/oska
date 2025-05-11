#ifndef OSKA_EVENTS_HPP
#define OSKA_EVENTS_HPP

#include <tuple>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <mutex>

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

class EventQueueInterface {
public:
    virtual void push(const EventWrapper& ev) = 0;
    virtual bool pop(EventWrapper& out) = 0;
};

class EventLoopInterface {
public:
    virtual void post(size_t tag, void* data) = 0;
    virtual void connect(size_t tag, Callback cb) = 0;
    virtual void run() = 0;
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
    void connect(EventLoopInterface* loop, Func handler) {
        using ExpectedArgs = typename EventTraits<EventTag>::Args;

        static_assert(is_invocable_from_tuple<ExpectedArgs, Func>::value,
                    "Handler is not callable with arguments from EventTraits");

        Callback cb = [handler](void* data) {
            auto tuple = static_cast<ExpectedArgs*>(data);
            std::apply(handler, *tuple);
            delete tuple;
        };

        auto tag = oska::TypeId<EventTag>::value();

        std::unique_lock<std::mutex> lock(mtx);
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

        std::unique_lock<std::mutex> lock(mtx);
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
        EventLoopInterface* target;
        Callback callback;
    };

    std::unordered_map<size_t, Binding> bindings;
    std::mutex mtx;
};

// ---- Global Manager Instance ---- //
inline CormanManager Corman;

} // namespace oska

#endif // OSKA_EVENTS_HPP
