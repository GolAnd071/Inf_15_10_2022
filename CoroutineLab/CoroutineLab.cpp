#include <coroutine>
#include <vector>
#include <algorithm>
#include <iostream>

// The caller-level type
struct Generator {
    // The coroutine level type
    struct promise_type {
        using Handle = std::coroutine_handle<promise_type>;

        Generator get_return_object() {
            return Generator{ Handle::from_promise(*this) };
        }

        std::suspend_always initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }

        void unhandled_exception() { }

        void return_void() noexcept {}

        int current_value;
    };

    explicit Generator(promise_type::Handle coro) : coro_(coro) {}

    ~Generator() {
        if (coro_) coro_.destroy();
    }

    // Make move-only
    Generator(const Generator&) = delete;

    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& t) noexcept : coro_(t.coro_) {
        t.coro_ = {};
    }

    Generator& operator=(Generator&& t) noexcept {
        if (this == &t) return *this;
        if (coro_) coro_.destroy();
        coro_ = t.coro_;
        t.coro_ = {};
        return *this;
    }

    int get_next() {
        coro_.resume();
        return coro_.promise().current_value;
    }

private:
    promise_type::Handle coro_;
};

Generator myCoroutine() {
    int x = 0;
    while (true) {
        co_yield x++;
    }
}

Generator evenCoroutine() {
    int x = 0;
    while (true) {
        co_yield x += 2;
    }
}

Generator factCoroutine() {
    int x = 1, i = 1;
    while (true) {
        co_yield x *= i++;
    }
}

Generator primeCoroutine() {
    int x = 0, a = 1;
    std::vector<int> pr;
    std::generate_n(std::back_inserter(pr), 100, [&a]() { return ++a; });

    while (true) {
        co_yield x = pr[0];
        a = pr[0];
        for (int i = a; i < pr.back(); i += a)
            if (std::find(pr.begin(), pr.end(), i) != pr.end())
                pr.erase(std::find(pr.begin(), pr.end(), i));
    }
}

void func(Generator (*gen)())
{
    auto c = gen();
    for (int i = 0; i < 10; ++i) {
        std::cout << c.get_next() << '\n';
    }
}

int main() {
    func(evenCoroutine);
    func(factCoroutine);
    func(primeCoroutine);
}