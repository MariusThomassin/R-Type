/*
** R-Type ECS - InputSystem Implementation
** Handles player input via EventBus
*/

#include "InputSystem.hpp"
#include "client/NetworkClient.hpp"
#include "shared/network/Protocol.hpp"
#include <iostream>

namespace rtype::ecs {

    void InputSystem::sendInputToServer(const ClientInputMessage& msg) {
        if (!m_networkClient) {
            return;
        }

        // Debug log for SHOOT input
        if (msg.inputFlags & 0x10) {  // INPUT_SHOOT
            std::cout << "[InputSystem] Sending INPUT_SHOOT to server (flags=0x"
                      << std::hex << (int)msg.inputFlags << std::dec << ")" << std::endl;
        }

        // Convert local struct to network::ClientInputMessage
        network::ClientInputMessage networkMsg;
        networkMsg.sequenceNumber = msg.sequenceNumber;
        networkMsg.inputFlags = msg.inputFlags;
        networkMsg.deltaTime = msg.deltaTime;

        m_networkClient->sendInput(networkMsg);
    }

} // namespace rtype::ecs
