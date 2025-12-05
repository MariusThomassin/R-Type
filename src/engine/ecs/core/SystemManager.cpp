/*
** R-Type ECS - SystemManager Implementation
** Manages and updates all ECS systems
*/

#include "SystemManager.hpp"
#include "ISystem.hpp"
#include "Types.hpp"
#include <algorithm>

namespace rtype::ecs {

    void SystemManager::updateAll(float dt) {
        for (ISystem* system : m_systemOrder) {
            if (system->isEnabled()) {
                system->update(dt);
            }
        }
    }

    void SystemManager::updatePhase(SystemPhase phase, float dt) {
        for (ISystem* system : m_systemOrder) {
            if (system->isEnabled() && system->getPhase() == phase) {
                system->update(dt);
            }
        }
    }

    std::size_t SystemManager::getSystemCount() const {
        return m_systems.size();
    }

    void SystemManager::sortSystems() {
        std::sort(m_systemOrder.begin(), m_systemOrder.end(),
            [](const ISystem* a, const ISystem* b) {
                return static_cast<int>(a->getPhase()) <
                       static_cast<int>(b->getPhase());
            }
        );
    }

} // namespace rtype::ecs