/*
** R-Type ECS - Command Implementation
** Deferred entity/component operations
*/

#include "Command.hpp"
#include "Registry.hpp"

namespace rtype::ecs {

    void CreateEntityCommand::execute(Registry& registry) {
        Entity entity = registry.createEntity();
        if (m_callback) {
            m_callback(entity.id);
        }
    }

    void DestroyEntityCommand::execute(Registry& registry) {
        if (registry.entityExists(m_entity)) {
            registry.destroyEntity(m_entity);
        }
    }

    std::size_t CommandBuffer::flush(Registry& registry) {
        std::queue<std::unique_ptr<ICommand>> commandsToExecute;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(commandsToExecute, m_commands);
        }

        std::size_t count = 0;
        while (!commandsToExecute.empty()) {
            auto& cmd = commandsToExecute.front();
            cmd->execute(registry);
            commandsToExecute.pop();
            ++count;
        }

        return count;
    }

    std::size_t Transaction::commit() {
        m_committed = true;
        return m_buffer.flush(m_registry);
    }

} // namespace rtype::ecs

// Template implementations need to be visible to callers
// These are defined here after Registry.hpp is included

namespace rtype::ecs {

    template <typename T>
    void AddComponentCommand<T>::execute(Registry& registry) {
        if (registry.entityExists(m_entity)) {
            registry.addComponent<T>(m_entity, std::move(m_component));
        }
    }

    template <typename T>
    void RemoveComponentCommand<T>::execute(Registry& registry) {
        if (registry.entityExists(m_entity) && registry.hasComponent<T>(m_entity)) {
            registry.removeComponent<T>(m_entity);
        }
    }

} // namespace rtype::ecs
