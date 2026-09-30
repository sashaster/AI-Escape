#include <Engine/Physics.hpp>


namespace Engine::Physics {

    bool Overlaps(const AABB& box, const std::vector<AABB>& walls) {
        for (const auto& wall: walls) {
            if (Overlaps(box, wall)) {
                return true;
            }
        }
        return false;
    }

    bool Overlaps(const AABB& box, const AABB& wall) {
        if (box.min.x <= wall.max.x &&
            box.max.x >= wall.min.x &&
            box.min.y <= wall.max.y &&
            box.max.y >= wall.min.y) {
            return true;
        }
        return false;
    }

}