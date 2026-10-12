#pragma once
#include <random>

namespace Engine {

    class Random {
    public:
        static void Seed(const std::uint32_t seed) {
            s_Generator.seed(seed);
        }

        static int Int(const int min, const int max) {
            std::uniform_int_distribution dist(min, max);
            return dist(s_Generator);
        }

        static std::uint32_t Uint(const std::uint32_t min, const std::uint32_t max) {
            std::uniform_int_distribution dist(min, max);
            return dist(s_Generator);
        }

    private:
        inline static std::mt19937 s_Generator{0};
    };

}