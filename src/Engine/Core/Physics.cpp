#include <Engine/Core/Physics.hpp>
#include <Engine/Core/Core.hpp>


namespace Engine::Physics {

    static bool Touches(const AABB box, const AABB wall) {
        return box.min.x <= wall.max.x &&
               box.max.x >= wall.min.x &&
               box.min.y <= wall.max.y &&
               box.max.y >= wall.min.y;
    }

    static bool Touches(const AABB box, const std::span<const AABB> walls) {
        for (const auto wall: walls) {
            if (Touches(box, wall)) {
                return true;
            }
        }
        return false;
    }

    // TODO: replace with slab method
    static Ray RayCast(const glm::vec2 origin, const glm::vec2 dir, const std::span<const AABB> walls, const float maxDist) {
        constexpr auto step = 0.1f;
        const auto stepCount = static_cast<int>(std::round(maxDist/step));
        for (auto i = 0; i <= stepCount; ++i) {
            const auto distance = static_cast<float>(i) * step;
            if (const glm::vec2 pos = origin + dir * distance; Touches(AABB{.min = pos, .max = pos}, walls)) {
                return {.hit = true, .distance = distance, .start = origin, .end = pos};
            }
        }
        return {.hit = false, .distance = maxDist, .start = origin, .end = origin + dir * maxDist};
    }

    std::array<Ray, 8> RayCast(const AABB box, const std::span<const AABB> walls, const float maxDist) {
        const auto center = box.Center();
        const auto half = box.HalfSize();
        std::array<Ray, 8> rays{};

        auto i = 0;
        for (auto y = 1; y >= -1; --y) {
            for (auto x = -1; x <= 1; ++x) {
                if (x == 0 && y == 0) {
                    continue;
                }
                const auto dir = glm::vec2(static_cast<float>(x), static_cast<float>(y));
                const auto origin = center + dir * half;
                rays[i++] = RayCast(origin, glm::normalize(dir), walls, maxDist);
            }
        }
        return rays;
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