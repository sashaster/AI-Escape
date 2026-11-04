#include <Engine/Core/Physics.hpp>
#include <Engine/Core/Core.hpp>


namespace Engine::Physics {

    constexpr std::array<glm::vec2, 4> kDirs = {{
        {0, 1}, {1, 0}, {0, -1}, {-1, 0}
    }};

    static AABB Offset(const AABB box, const glm::vec2 delta) {
        return {.min = box.min + delta, .max = box.max + delta};
    }

    static Sweep SweepCast(const AABB box, const glm::vec2 dir, const std::span<const AABB> walls, const float maxDist) {
        constexpr auto step = 0.1f;
        const auto stepCount = static_cast<int>(std::round(maxDist/step));
        for (auto i = 1; i <= stepCount; ++i) {
            if (Overlaps(Offset(box, dir * (static_cast<float>(i) * step)), walls)) {
                const auto distance = static_cast<float>(i - 1) * step;
                return {.hit = true, .distance = distance, .box = Offset(box, dir * distance)};
            }
        }
        return {.hit = false, .distance = maxDist, .box = Offset(box, dir * maxDist)};
    }

    std::array<Sweep, 4> SweepCast(const AABB box, const std::span<const AABB> walls, const float maxDist) {
        std::array<Sweep, 4> sweeps{};
        for (std::size_t d = 0; d < kDirs.size(); ++d) {
            sweeps[d] = SweepCast(box, kDirs[d], walls, maxDist);
        }
        return sweeps;
    }

    bool Overlaps(const AABB box, const std::span<const AABB> walls) {
        for (const auto wall: walls) {
            if (Overlaps(box, wall)) {
                return true;
            }
        }
        return false;
    }

    bool Overlaps(const AABB box, const AABB wall) {
        return box.min.x < wall.max.x &&
               box.max.x > wall.min.x &&
               box.min.y < wall.max.y &&
               box.max.y > wall.min.y;
    }

}
