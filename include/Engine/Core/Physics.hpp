#pragma once
#include <glm/glm.hpp>
#include <vector>
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

    struct Ray {
        bool hit;
        float distance;
        glm::vec2 start;
        glm::vec2 end;
    };

    std::vector<Ray> RayCast(AABB box, std::span<const AABB> walls, float maxDist);

    bool Overlaps(AABB box, std::span<const AABB> walls);
    bool Overlaps(AABB box, AABB wall);
}