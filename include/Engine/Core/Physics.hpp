#pragma once
#include <glm/glm.hpp>
#include <array>
#include <span>


namespace Engine::Physics{

    struct AABB {

        glm::vec2 min;
        glm::vec2 max;

        [[nodiscard]] glm::vec2 Center() const {
            return (min + max) * 0.5f;
        }

        [[nodiscard]] glm::vec2 HalfSize() const {
            return (max - min) * 0.5f;
        }
    };

    struct Sweep {
        bool hit;
        float distance;
        AABB box;
    };

    std::array<Sweep, 4> SweepCast(AABB box, std::span<const AABB> walls, float maxDist);

    bool Overlaps(AABB box, std::span<const AABB> walls);
    bool Overlaps(AABB box, AABB wall);
}