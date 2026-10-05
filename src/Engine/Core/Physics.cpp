#include <Engine/Core/Physics.hpp>


namespace Engine::Physics {

    static constexpr float kMaxDistance = 10.0f;

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

    static Ray RayCast(const glm::vec2 origin, glm::vec2 dir, const std::span<const AABB> walls, const float maxDist) {
        dir = glm::normalize(dir);
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

    std::tuple<Ray, Ray, Ray, Ray> RayCast(const AABB box, const std::span<const AABB> walls) {
        const auto [min, max] = box;
        const auto center = box.Center();
        return {
            RayCast({center.x, max.y}, {0.0f, 1.0f}, walls, kMaxDistance),
            RayCast({max.x, center.y}, {1.0f, 0.0f}, walls, kMaxDistance),
            RayCast({center.x, min.y}, {0.0f, -1.0f}, walls, kMaxDistance),
            RayCast({min.x, center.y}, {-1.0f, 0.0f}, walls, kMaxDistance)
        };
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