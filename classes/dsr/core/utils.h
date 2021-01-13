//
// Created by juancarlos on 21/8/20.
//

#ifndef UTILS_H
#define UTILS_H

#define PRINT_TIME(type, s)    {                 \
                        auto now = get_unix_timestamp(); \
                        std::chrono::nanoseconds now_ms(now);\
                        auto dt = now_ms - std::chrono::nanoseconds(s.dk().ds().begin()->second.timestamp());\
                        auto x = dt;\
                        fprintf(stdout, "%s att: %s", type  , std::string(std::to_string(x.count()/1000000.0) + "ms\n").data() );\
                        }

typedef unsigned __int128 uint128_t;


class hash_tuple {

    template<class T>
    struct component {
        const T &value;

        component(const T &value) : value(value) {}

        uintmax_t operator,(uintmax_t n) const {
            n ^= std::hash<T>()(value);
            n ^= n << (sizeof(uintmax_t) * 4 - 1);
            return n ^ std::hash<uintmax_t>()(n);
        }
    };

public:
    template<class Tuple>
    size_t operator()(const Tuple &tuple) const {
        return std::hash<uintmax_t>()(
                std::apply([](const auto &... xs) { return (component(xs), ..., 0); }, tuple));
    }
};


struct hash_pair
{
    template<class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

static uint64_t get_unix_timestamp() { //Move to utilities?
    auto now = std::chrono::system_clock::now();
    std::chrono::time_point pt_ms = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    uint64_t millis = pt_ms.time_since_epoch().count();
    return millis;
}

#endif //UTILS_H
