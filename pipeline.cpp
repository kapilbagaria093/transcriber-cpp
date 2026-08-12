#include <iostream>
#include <string>
#include <utility>

// implementation of functional chaining, NOT PIPELINING

// resource to learn and implement actual pipelining using thread-safe queues (actual pipelining)
// https://www.youtube.com/watch?v=6UYXSu_9dXI
// https://www.youtube.com/watch?v=BedUiLRDOKo

// Pipeline operator
template <typename T, typename F>
auto operator|(T&& value, F&& func) {
    return std::forward<F>(func)(std::forward<T>(value));
}

// Convert any function into a pipeline-compatible callable
template <typename F>
auto pipe(F&& func) {
    return [func = std::forward<F>(func)](auto&& value) {
        return func(std::forward<decltype(value)>(value));
    };
}


// Normal functions
int doubleI(int x) {
    std::cout << x*2 << "\n";
    return x * 2;
}

int add3(int x) {
    std::cout << x+2 << "\n";
    return x + 3;
}

std::string to_string(int x) {
    std::cout << std::to_string(x) << "\n";
    return std::to_string(x);
}


int main() {

    std::string result = 5
                       | pipe(doubleI)
                       | pipe(add3)
                       | pipe(to_string);

    std::cout << result << '\n';

    return 0;
}