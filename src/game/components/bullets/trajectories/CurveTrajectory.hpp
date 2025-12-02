/*
** R-Type ECS - CurveTrajectory
** Bezier curves and complex path trajectories
*/

#pragma once

#include "../TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Bezier curve trajectory
     * 
     * Bullet follows a cubic bezier curve defined by 4 control points.
     * Great for sweeping arcs and complex predetermined paths.
     */
    struct BezierTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Bezier;

        // Control points
        float startX = 0.0f, startY = 0.0f;
        float control1X = 0.0f, control1Y = 0.0f;
        float control2X = 0.0f, control2Y = 0.0f;
        float endX = 0.0f, endY = 0.0f;

        float duration = 2.0f;  // Time to traverse the curve

        BezierTrajectory() = default;

        BezierTrajectory(float sx, float sy, float c1x, float c1y,
                        float c2x, float c2y, float ex, float ey,
                        float dur = 2.0f)
            : startX(sx), startY(sy)
            , control1X(c1x), control1Y(c1y)
            , control2X(c2x), control2Y(c2y)
            , endX(ex), endY(ey)
            , duration(dur) {}

        /**
         * @brief Create a simple arc from start to end
         */
        static BezierTrajectory arc(float sx, float sy, float ex, float ey,
                                    float arcHeight, float dur = 1.5f) {
            float midX = (sx + ex) / 2.0f;
            float midY = (sy + ey) / 2.0f - arcHeight;
            return BezierTrajectory(sx, sy, midX, midY, midX, midY, ex, ey, dur);
        }
    };

    /**
     * @brief Figure-8 trajectory
     * 
     * Bullet traces an infinity/figure-8 pattern.
     */
    struct Figure8Trajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Figure8;

        float width = 150.0f;     // Width of the figure 8
        float height = 100.0f;    // Height of the figure 8
        float speed = 2.0f;       // Speed of traversal

        Figure8Trajectory() = default;

        Figure8Trajectory(float w, float h, float spd = 2.0f)
            : width(w), height(h), speed(spd) {}
    };

} // namespace rtype::ecs
