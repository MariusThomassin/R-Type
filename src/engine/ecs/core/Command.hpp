/*
** R-Type ECS - Command Pattern for Registry
** Deferred entity/component operations for thread safety
*/

#pragma once

#include "Types.hpp"
#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace rtype::ecs {

    class Registry;  // Forward declaration

    /**
     * @brief Base class for registry commands
     * 
     * Commands encapsulate operations to be executed later,
     * enabling thread-safe deferred modifications.
     */
    class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual void execute(Registry& registry) = 0;
        virtual std::string getDescription() const = 0;
    };

    /**
     * @brief Command to create an entity
     */
    class CreateEntityCommand : public ICommand {
    public:
        using Callback = std::function<void(EntityId)>;

        explicit CreateEntityCommand(Callback callback = nullptr)
            : m_callback(std::move(callback)) {}

        void execute(Registry& registry) override;
        std::string getDescription() const override { return "CreateEntity"; }

    private:
        Callback m_callback;
    };

    /**
     * @brief Command to destroy an entity
     */
    class DestroyEntityCommand : public ICommand {
    public:
        explicit DestroyEntityCommand(EntityId entity)
            : m_entity(entity) {}

        void execute(Registry& registry) override;
        std::string getDescription() const override { return "DestroyEntity"; }

    private:
        EntityId m_entity;
    };

    /**
     * @brief Command to add a component to an entity
     */
    template <typename T>
    class AddComponentCommand : public ICommand {
    public:
        AddComponentCommand(EntityId entity, T component)
            : m_entity(entity)
            , m_component(std::move(component)) {}

        void execute(Registry& registry) override;
        std::string getDescription() const override { return "AddComponent<" + std::string(typeid(T).name()) + ">"; }

    private:
        EntityId m_entity;
        T m_component;
    };

    /**
     * @brief Command to remove a component from an entity
     */
    template <typename T>
    class RemoveComponentCommand : public ICommand {
    public:
        explicit RemoveComponentCommand(EntityId entity)
            : m_entity(entity) {}

        void execute(Registry& registry) override;
        std::string getDescription() const override { return "RemoveComponent<" + std::string(typeid(T).name()) + ">"; }

    private:
        EntityId m_entity;
    };

    /**
     * @brief Generic command wrapping a lambda
     */
    class LambdaCommand : public ICommand {
    public:
        using CommandFunc = std::function<void(Registry&)>;

        explicit LambdaCommand(CommandFunc func, std::string description = "LambdaCommand")
            : m_func(std::move(func))
            , m_description(std::move(description)) {}

        void execute(Registry& registry) override {
            if (m_func) m_func(registry);
        }

        std::string getDescription() const override { return m_description; }

    private:
        CommandFunc m_func;
        std::string m_description;
    };

    /**
     * @brief Command buffer for deferred registry operations
     * 
     * Thread-safe queue for commands that will be executed
     * on the main thread during the next flush.
     * 
     * Usage:
     * ```cpp
     * // From worker thread
     * commandBuffer.enqueue<DestroyEntityCommand>(entityId);
     * commandBuffer.enqueue([](Registry& reg) {
     *     reg.createEntity();
     * });
     * 
     * // From main thread (once per frame)
     * commandBuffer.flush(registry);
     * ```
     */
    class CommandBuffer {
    public:
        CommandBuffer() = default;
        ~CommandBuffer() = default;

        // Non-copyable, but movable
        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&& other) noexcept {
            std::lock_guard<std::mutex> lock(other.m_mutex);
            m_commands = std::move(other.m_commands);
        }
        CommandBuffer& operator=(CommandBuffer&& other) noexcept {
            if (this != &other) {
                std::lock_guard<std::mutex> lock1(m_mutex);
                std::lock_guard<std::mutex> lock2(other.m_mutex);
                m_commands = std::move(other.m_commands);
            }
            return *this;
        }

        /**
         * @brief Enqueue a command for deferred execution
         * @tparam CommandType Type of command (must derive from ICommand)
         * @tparam Args Constructor argument types
         * @param args Arguments forwarded to command constructor
         */
        template <typename CommandType, typename... Args>
        void enqueue(Args&&... args) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_commands.push(std::make_unique<CommandType>(std::forward<Args>(args)...));
        }

        /**
         * @brief Enqueue a lambda as a command
         * @param func Function to execute
         * @param description Optional description for debugging
         */
        void enqueue(std::function<void(Registry&)> func, std::string description = "Lambda") {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_commands.push(std::make_unique<LambdaCommand>(std::move(func), std::move(description)));
        }

        /**
         * @brief Execute all queued commands on the registry
         * @param registry The registry to execute commands on
         * @return Number of commands executed
         */
        std::size_t flush(Registry& registry);

        /**
         * @brief Get the number of pending commands
         */
        std::size_t size() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_commands.size();
        }

        /**
         * @brief Check if there are pending commands
         */
        bool empty() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_commands.empty();
        }

        /**
         * @brief Clear all pending commands without executing
         */
        void clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::queue<std::unique_ptr<ICommand>> empty;
            std::swap(m_commands, empty);
        }

    private:
        mutable std::mutex m_mutex;
        std::queue<std::unique_ptr<ICommand>> m_commands;
    };

    /**
     * @brief Transaction for grouping multiple operations
     * 
     * Provides a RAII-style interface for grouping operations.
     * Commands are queued and only executed when commit() is called.
     * If the transaction is destroyed without commit(), changes are discarded.
     */
    class Transaction {
    public:
        explicit Transaction(Registry& registry)
            : m_registry(registry)
            , m_committed(false) {}

        ~Transaction() {
            if (!m_committed) {
                rollback();
            }
        }

        // Non-copyable, movable
        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;
        Transaction(Transaction&& other) noexcept
            : m_registry(other.m_registry)
            , m_buffer(std::move(other.m_buffer))
            , m_committed(other.m_committed) {
            other.m_committed = true;  // Prevent other from rolling back
        }

        /**
         * @brief Queue an entity creation
         * @param callback Function to call with the new entity ID
         */
        void createEntity(std::function<void(EntityId)> callback = nullptr) {
            m_buffer.enqueue<CreateEntityCommand>(std::move(callback));
        }

        /**
         * @brief Queue an entity destruction
         * @param entity Entity to destroy
         */
        void destroyEntity(EntityId entity) {
            m_buffer.enqueue<DestroyEntityCommand>(entity);
        }

        /**
         * @brief Queue adding a component
         * @tparam T Component type
         * @param entity Target entity
         * @param component Component data
         */
        template <typename T>
        void addComponent(EntityId entity, T component) {
            m_buffer.enqueue<AddComponentCommand<T>>(entity, std::move(component));
        }

        /**
         * @brief Queue removing a component
         * @tparam T Component type
         * @param entity Target entity
         */
        template <typename T>
        void removeComponent(EntityId entity) {
            m_buffer.enqueue<RemoveComponentCommand<T>>(entity);
        }

        /**
         * @brief Queue a custom operation
         * @param func Function to execute
         */
        void execute(std::function<void(Registry&)> func) {
            m_buffer.enqueue(std::move(func), "CustomOperation");
        }

        /**
         * @brief Commit all queued operations
         * @return Number of operations executed
         */
        std::size_t commit();

        /**
         * @brief Discard all queued operations
         */
        void rollback() {
            m_buffer.clear();
            m_committed = true;  // Prevent double-rollback in destructor
        }

        /**
         * @brief Check if this transaction has been committed
         */
        bool isCommitted() const { return m_committed; }

        /**
         * @brief Get number of pending operations
         */
        std::size_t pendingCount() const { return m_buffer.size(); }

    private:
        Registry& m_registry;
        CommandBuffer m_buffer;
        bool m_committed;
    };

} // namespace rtype::ecs
