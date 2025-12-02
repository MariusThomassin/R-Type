/*
** R-Type ECS - Bullets Module
** Aggregate header for bullet/trajectory components
*/

#pragma once

// Core types and enums
#include "TrajectoryTypes.hpp"
#include "TrajectoryParams.hpp"

// Legacy component (large, monolithic)
#include "TrajectoryComponent.hpp"

// NEW: Refactored trajectory system
// Individual trajectory types in separate files
#include "trajectories/Trajectories.hpp"

// NEW: Optimized component using variants
#include "TrajectoryComponentNew.hpp"

// Factory for legacy component
#include "TrajectoryFactory.hpp"

// Spin component for rotating bullets
#include "SpinComponent.hpp"
