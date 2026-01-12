/*
** R-Type ECS - Serialization Implementation
** JSON serialization for components and entities
*/

#include "Serialization.hpp"
#include "Registry.hpp"

namespace rtype::ecs {

    SerializationRegistry& SerializationRegistry::instance() {
        static SerializationRegistry instance;
        return instance;
    }

    nlohmann::json SerializationRegistry::serializeEntity(
        const Registry& registry, EntityId entity) const 
    {
        nlohmann::json result;
        result["id"] = entity;
        result["components"] = nlohmann::json::object();

        // Get entity signature to know what components it has
        Signature signature = registry.getSignature(entity);

        // For each registered serializer, check if entity has that component
        for (const auto& [typeIdx, serializer] : m_serializers) {
            // Note: This is a simplified implementation. 
            // In a full implementation, you'd need a mapping from type_index 
            // to ComponentTypeId to check the signature.
            // For now, we serialize based on what the serializer knows about.
            
            // The serializer's serialize function will be called with the component
            // This requires the caller to iterate components differently
        }

        return result;
    }

    EntityId SerializationRegistry::deserializeEntity(
        Registry& registry, const nlohmann::json& json) const 
    {
        Entity entity = registry.createEntity();
        EntityId entityId = entity.id;

        if (!json.contains("components") || !json["components"].is_object()) {
            return entityId;
        }

        const auto& components = json["components"];
        
        for (const auto& [typeName, componentData] : components.items()) {
            auto typeIt = m_nameToType.find(typeName);
            if (typeIt == m_nameToType.end()) {
                continue; // Unknown component type, skip
            }

            auto serializerIt = m_serializers.find(typeIt->second);
            if (serializerIt == m_serializers.end()) {
                continue;
            }

            // Create component and deserialize
            auto component = serializerIt->second.create();
            if (component) {
                serializerIt->second.deserialize(component.get(), componentData);
                // Note: Adding the component to the entity would require 
                // a type-erased addComponent method in Registry
            }
        }

        return entityId;
    }

    nlohmann::json SerializationRegistry::serializeEntities(
        const Registry& registry, 
        const std::vector<EntityId>& entities) const 
    {
        nlohmann::json result = nlohmann::json::array();
        
        for (EntityId entity : entities) {
            if (registry.entityExists(entity)) {
                result.push_back(serializeEntity(registry, entity));
            }
        }

        return result;
    }

    std::vector<EntityId> SerializationRegistry::deserializeEntities(
        Registry& registry, 
        const nlohmann::json& json) const 
    {
        std::vector<EntityId> entities;

        if (!json.is_array()) {
            return entities;
        }

        entities.reserve(json.size());

        for (const auto& entityJson : json) {
            EntityId id = deserializeEntity(registry, entityJson);
            entities.push_back(id);
        }

        return entities;
    }

} // namespace rtype::ecs
