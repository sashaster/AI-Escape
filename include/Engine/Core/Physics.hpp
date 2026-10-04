#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace Engine::Physics{

    struct AABB {
        glm::vec2 min;
        glm::vec2 max;
    };

    bool Overlaps(const AABB& box, const std::vector<AABB>& walls);
    bool Overlaps(const AABB& box, const AABB& wall);


}