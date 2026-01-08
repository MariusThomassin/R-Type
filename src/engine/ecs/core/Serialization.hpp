/*
** R-Type ECS - Serialization Framework
** JSON serialization for components and entities
*/

#pragma once

#include "engine/ecs/core/Types.hpp"
#include "engine/ecs/core/IComponent.hpp"

#include <nlohmann/json.hpp>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace rtype::ecs {

    class Registry;  // Forward declaration

    /**
     * @brief Interface for serializable components
     * 
     * Components that implement this interface can be serialized
     * to/from JSON for saving, loading, and networking.
     */
    class ISerializable {
    public:
        virtual ~ISerializable() = default;

        /**
         * @brief Serialize the component to JSON
         * @param out JSON object to write to
         */
        virtual void serialize(nlohmann::json& out) const = 0;

        /**
         * @brief Deserialize the component from JSON
         * @param in JSON object to read from
         */
        virtual void deserialize(const nlohmann::json& in) = 0;

        /**
         * @brief Get component version for migration
         */
        virtual uint32_t getVersion() const { return 1; }
    };

    /**
     * @brief Type-erased component serializer
     */
    class ComponentSerializer {
    public:
        using SerializeFunc = std::function<nlohmann::json(const void*)>;
        using DeserializeFunc = std::function<void(void*, const nlohmann::json&)>;
        using CreateFunc = std::function<std::unique_ptr<IComponent>()>;

        ComponentSerializer() = default;

        ComponentSerializer(
            std::string typeName,
            SerializeFunc serializeFunc,
            DeserializeFunc deserializeFunc,
            CreateFunc createFunc)
            : m_typeName(std::move(typeName))
            , m_serialize(std::move(serializeFunc))
            , m_deserialize(std::move(deserializeFunc))
            , m_create(std::move(createFunc)) {}

        nlohmann::json serialize(const void* component) const {
            return m_serialize ? m_serialize(component) : nlohmann::json{};
        }

        void deserialize(void* component, const nlohmann::json& json) const {
            if (m_deserialize) {
                m_deserialize(component, json);
            }
        }

        std::unique_ptr<IComponent> create() const {
            return m_create ? m_create() : nullptr;
        }

        const std::string& getTypeName() const { return m_typeName; }

    private:
        std::string m_typeName;
        SerializeFunc m_serialize;
        DeserializeFunc m_deserialize;
        CreateFunc m_create;
    };

    /**
     * @brief Central serialization registry
     * 
     * Manages component serializers and provides entity serialization.
     * 
     * Usage:
     * ```cpp
     * // Register a serializer
     * SerializationRegistry::instance().registerComponent<TransformComponent>(
     *     "Transform",
     *     [](const TransformComponent& c) -> nlohmann::json {
     *         return {{"x", c.x}, {"y", c.y}, {"rotation", c.rotation}};
     *     },
     *     [](TransformComponent& c, const nlohmann::json& j) {
     *         c.x = j.value("x", 0.0f);
     *         c.y = j.value("y", 0.0f);
     *         c.rotation = j.value("rotation", 0.0f);
     *     }
     * );
     * 
     * // Serialize an entity
     * auto json = SerializationRegistry::instance().serializeEntity(registry, entityId);
     * ```
     */
    class SerializationRegistry {
    public:
        static SerializationRegistry& instance();

        /**
         * @brief Register a component serializer
         * @tparam T Component type
         * @param typeName String identifier for the type
         * @param serialize Serialization function
         * @param deserialize Deserialization function
         */
        template <typename T>
        void registerComponent(
            const std::string& typeName,
            std::function<nlohmann::json(const T&)> serialize,
            std::function<void(T&, const nlohmann::json&)> deserialize)
        {
            static_assert(std::is_base_of<IComponent, T>::value,
                "T must derive from IComponent");

            std::type_index typeIdx(typeid(T));

            auto serializeWrapper = [serialize](const void* ptr) -> nlohmann::json {
                return serialize(*static_cast<const T*>(ptr));
            };

            auto deserializeWrapper = [deserialize](void* ptr, const nlohmann::json& json) {
                deserialize(*static_cast<T*>(ptr), json);
            };

            auto createWrapper = []() -> std::unique_ptr<IComponent> {
                return std::make_unique<T>();
            };

            m_serializers[typeIdx] = ComponentSerializer(
                typeName, serializeWrapper, deserializeWrapper, createWrapper);
            m_nameToType[typeName] = typeIdx;
        }

        /**
         * @brief Register using ISerializable interface
         */
        template <typename T>
        void registerSerializable(const std::string& typeName) {
            static_assert(std::is_base_of<ISerializable, T>::value,
                "T must derive from ISerializable");

            registerComponent<T>(
                typeName,
                [](const T& c) -> nlohmann::json {
                    nlohmann::json j;
                    c.serialize(j);
                    return j;
                },
                [](T& c, const nlohmann::json& j) {
                    c.deserialize(j);
                }
            );
        }

        /**
         * @brief Serialize a single component
         * @tparam T Component type
         * @param component The component to serialize
         * @return JSON representation
         */
        template <typename T>
        nlohmann::json serializeComponent(const T& component) const {
            std::type_index typeIdx(typeid(T));
            auto it = m_serializers.find(typeIdx);
            if (it == m_serializers.end()) {
                throw std::runtime_error("No serializer registered for component type");
            }
            return it->second.serialize(&component);
        }

        /**
         * @brief Deserialize a single component
         * @tparam T Component type
         * @param component The component to deserialize into
         * @param json JSON data
         */
        template <typename T>
        void deserializeComponent(T& component, const nlohmann::json& json) const {
            std::type_index typeIdx(typeid(T));
            auto it = m_serializers.find(typeIdx);
            if (it == m_serializers.end()) {
                throw std::runtime_error("No serializer registered for component type");
            }
            it->second.deserialize(&component, json);
        }

        /**
         * @brief Serialize an entity with all its registered components
         * @param registry The registry containing the entity
         * @param entity The entity to serialize
         * @return JSON object with all serializable components
         */
        nlohmann::json serializeEntity(const Registry& registry, EntityId entity) const;

        /**
         * @brief Deserialize an entity from JSON
         * @param registry The registry to create entity in
         * @param json JSON data
         * @return The created entity ID
         */
        EntityId deserializeEntity(Registry& registry, const nlohmann::json& json) const;

        /**
         * @brief Serialize multiple entities (for level saving)
         * @param registry The registry
         * @param entities Vector of entities to serialize
         * @return JSON array of entity data
         */
        nlohmann::json serializeEntities(const Registry& registry, 
                                          const std::vector<EntityId>& entities) const;

        /**
         * @brief Deserialize multiple entities
         * @param registry The registry
         * @param json JSON array of entity data
         * @return Vector of created entity IDs
         */
        std::vector<EntityId> deserializeEntities(Registry& registry, 
                                                   const nlohmann::json& json) const;

        /**
         * @brief Check if a component type is registered
         */
        template <typename T>
        bool isRegistered() const {
            return m_serializers.find(std::type_index(typeid(T))) != m_serializers.end();
        }

        /**
         * @brief Get type name for a component type
         */
        template <typename T>
        std::string getTypeName() const {
            auto it = m_serializers.find(std::type_index(typeid(T)));
            if (it != m_serializers.end()) {
                return it->second.getTypeName();
            }
            return "";
        }

        // Singleton
        SerializationRegistry(const SerializationRegistry&) = delete;
        SerializationRegistry& operator=(const SerializationRegistry&) = delete;

    private:
        SerializationRegistry() = default;
        ~SerializationRegistry() = default;

        std::unordered_map<std::type_index, ComponentSerializer> m_serializers;
        std::unordered_map<std::string, std::type_index> m_nameToType;
    };

    // Convenience macro
    #define SERIALIZATION rtype::ecs::SerializationRegistry::instance()

} // namespace rtype::ecs
