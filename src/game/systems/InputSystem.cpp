/*
** R-Type ECS - InputSystem Implementation
** Handles player input via EventBus
*/

#include "InputSystem.hpp"
#include "client/NetworkClient.hpp"
#include "shared/network/Protocol.hpp"

namespace rtype::ecs {

    void InputSystem::sendInputToServer(const ClientInputMessage& msg) {
        if (!m_networkClient) {
            return;
        }

        // Convert local struct to network::ClientInputMessage
        network::ClientInputMessage networkMsg;
        networkMsg.sequenceNumber = msg.sequenceNumber;
        networkMsg.inputFlags = msg.inputFlags;
        networkMsg.deltaTime = msg.deltaTime;

        m_networkClient->sendInput(networkMsg);
    }

} // namespace rtype::ecs
