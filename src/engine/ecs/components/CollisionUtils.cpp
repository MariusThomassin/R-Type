/*
** R-Type ECS - CollisionUtils Implementation
** Collision detection utilities and helper functions
*/

#include "CollisionUtils.hpp"

namespace rtype::ecs {

    CollisionLayer CollisionUtils::combineLayer(CollisionLayer a, CollisionLayer b)
    {
        return static_cast<CollisionLayer>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }

    CollisionLayer CollisionUtils::intersectLayer(CollisionLayer a, CollisionLayer b)
    {
        return static_cast<CollisionLayer>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
    }

    bool CollisionUtils::canCollide(CollisionLayer layerA, CollisionLayer maskA, CollisionLayer layerB, CollisionLayer maskB)
    {
        // Check if A's layer is in B's mask AND B's layer is in A's mask
        return isLayerInMask(layerA, maskB) && isLayerInMask(layerB, maskA);
    }

    bool CollisionUtils::isLayerInMask(CollisionLayer layer, CollisionLayer mask)
    {
        return intersectLayer(layer, mask) != CollisionLayer::None;
    }
} // namespace rtype::ecs
