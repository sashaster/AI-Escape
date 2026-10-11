#include <array>

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

    std::vector<Ray> RayCast(const AABB box, const std::span<const AABB> walls, const std::span<const float> angles, const float maxDist) {
        ASSERT(!angles.empty(), "Angles must not be empty")
        const auto center = box.Center();
        const auto half = box.HalfSize();
        std::vector <Ray> rays;
        rays.reserve(angles.size());
        for (const auto angle: angles) {
            const auto rad = glm::radians(angle);
            auto dir = glm::vec2(std::cos(rad), std::sin(rad));
            if (std::abs(dir.x) < 1e-6) {
                dir.x = 0.0f;
            }
            if (std::abs(dir.y) < 1e-6) {
                dir.y = 0.0f;
            }
            const auto origin = center + glm::sign(dir) * half;
            rays.push_back(RayCast(origin, glm::normalize(dir), walls, maxDist));
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