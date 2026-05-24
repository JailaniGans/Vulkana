// ===================================================================
// Vulkana Network System - Implementation
// ===================================================================
// Implements the NetworkSystem subsystem for managing networked gameplay.
// Handles connections, replication, and RPC routing.
// ===================================================================

#include "NetworkSystem.hpp"
#include "core/Log.hpp"
#include <algorithm>
#include <cstring>

namespace Vulkana
{
    // ================================================================
    // ReplicationChannel Implementation
    // ================================================================

    bool ReplicationChannel::FlushUpdates(float deltaTime)
    {
        m_lastReplicationTime += deltaTime;

        if (m_lastReplicationTime < m_replicationInterval)
        {
            return true;
        }

        m_lastReplicationTime = 0.0f;

        // Serialize and send all pending updates
        if (!m_pendingUpdates.empty())
        {
            std::vector<uint8_t> serialized = SerializeUpdates();
            FNetPacket packet(serialized.size());
            packet.Data = std::move(serialized);

            if (!m_connectionSocket || m_connectionSocket < 0)
            {
                LOG_WARN("Channel " + std::to_string(m_channelId) + 
                        " has invalid socket");
                return false;
            }
        }

        // Send all pending RPCs
        while (!m_pendingRPCs.empty())
        {
            // RPC serialization would go here
            m_pendingRPCs.pop();
        }

        return true;
    }

    bool ReplicationChannel::ReceiveUpdates(FNetPacket& packet)
    {
        if (packet.Data.empty())
        {
            return false;
        }

        return DeserializeUpdates(packet.Data);
    }

    std::vector<uint8_t> ReplicationChannel::SerializeUpdates()
    {
        std::vector<uint8_t> buffer;

        while (!m_pendingUpdates.empty())
        {
            const FReplicationUpdate& update = m_pendingUpdates.front();

            // Serialize ActorId (4 bytes)
            buffer.push_back((update.ActorId >> 0) & 0xFF);
            buffer.push_back((update.ActorId >> 8) & 0xFF);
            buffer.push_back((update.ActorId >> 16) & 0xFF);
            buffer.push_back((update.ActorId >> 24) & 0xFF);

            // Serialize ComponentId (4 bytes)
            buffer.push_back((update.ComponentId >> 0) & 0xFF);
            buffer.push_back((update.ComponentId >> 8) & 0xFF);
            buffer.push_back((update.ComponentId >> 16) & 0xFF);
            buffer.push_back((update.ComponentId >> 24) & 0xFF);

            // Serialize PropertyName (length-prefixed string)
            uint16_t nameLen = (uint16_t)update.PropertyName.size();
            buffer.push_back((nameLen >> 0) & 0xFF);
            buffer.push_back((nameLen >> 8) & 0xFF);
            buffer.insert(buffer.end(), update.PropertyName.begin(), 
                         update.PropertyName.end());

            // Serialize PropertyData (length-prefixed)
            uint32_t dataLen = (uint32_t)update.PropertyData.size();
            buffer.push_back((dataLen >> 0) & 0xFF);
            buffer.push_back((dataLen >> 8) & 0xFF);
            buffer.push_back((dataLen >> 16) & 0xFF);
            buffer.push_back((dataLen >> 24) & 0xFF);
            buffer.insert(buffer.end(), update.PropertyData.begin(), 
                         update.PropertyData.end());

            // Serialize Timestamp (4 bytes)
            buffer.push_back((update.Timestamp >> 0) & 0xFF);
            buffer.push_back((update.Timestamp >> 8) & 0xFF);
            buffer.push_back((update.Timestamp >> 16) & 0xFF);
            buffer.push_back((update.Timestamp >> 24) & 0xFF);

            m_pendingUpdates.pop();
        }

        return buffer;
    }

    bool ReplicationChannel::DeserializeUpdates(const std::vector<uint8_t>& data)
    {
        // Placeholder for deserialization logic
        // In a full implementation, would parse the serialized format
        return !data.empty();
    }

    // ================================================================
    // NetworkSystem Implementation
    // ================================================================

    NetworkSystem::NetworkSystem()
        : TSubsystem("NetworkSystem"),
          m_isServer(false),
          m_isConnected(false),
          m_nextConnectionId(1),
          m_nextChannelId(1),
          m_totalBytesReceived(0),
          m_totalBytesSent(0),
          m_startTime(std::chrono::high_resolution_clock::now())
    {
    }

    NetworkSystem::~NetworkSystem()
    {
    }

    // ================================================================
    // Subsystem Lifecycle
    // ================================================================

    void NetworkSystem::Initialize()
    {
        if (!m_driver.Initialize())
        {
            LOG_ERROR("Failed to initialize NetworkDriver");
            return;
        }

        TSubsystem::Initialize();
        LOG_INFO("NetworkSystem initialized");
    }

    void NetworkSystem::Tick(float dt)
    {
        TSubsystem::Tick(dt);

        if (m_isServer)
        {
            ProcessIncomingConnections();
        }

        ProcessIncomingData();
        ProcessReplication(dt);
    }

    void NetworkSystem::Shutdown()
    {
        // Close all connections
        for (auto& conn : m_connections)
        {
            if (conn.second && conn.second->SocketHandle >= 0)
            {
                m_driver.CloseSocket(conn.second->SocketHandle);
            }
        }
        m_connections.clear();

        if (m_isServer)
        {
            StopServer();
        }

        if (m_isConnected)
        {
            DisconnectFromServer();
        }

        m_driver.Shutdown();
        TSubsystem::Shutdown();

        LOG_INFO("NetworkSystem shutdown complete");
    }

    // ================================================================
    // Server Operations
    // ================================================================

    bool NetworkSystem::StartServer(const FNetAddress& bindAddress, int maxClients)
    {
        if (!m_driver.Initialize())
        {
            LOG_ERROR("Failed to initialize driver for server");
            return false;
        }

        if (!m_driver.CreateServerSocket(bindAddress, maxClients))
        {
            LOG_ERROR("Failed to create server socket");
            return false;
        }

        m_isServer = true;
        LOG_INFO("Server started on " + bindAddress.Address + ":" + 
                std::to_string(bindAddress.Port));
        return true;
    }

    void NetworkSystem::StopServer()
    {
        m_isServer = false;
        LOG_INFO("Server stopped");
    }

    // ================================================================
    // Client Operations
    // ================================================================

    bool NetworkSystem::ConnectToServer(const FNetAddress& serverAddress)
    {
        if (!m_driver.Initialize())
        {
            LOG_ERROR("Failed to initialize driver for client");
            return false;
        }

        if (!m_driver.CreateClientSocket(serverAddress))
        {
            LOG_ERROR("Failed to connect to server");
            return false;
        }

        m_isConnected = true;

        // Create a client connection entry
        auto clientConn = std::make_unique<FNetConnection>(
            m_nextConnectionId++, m_driver.GetSocketState(0) == ESocketState::Connected ? 0 : -1,
            serverAddress, false
        );
        clientConn->ReplicationChannel = std::make_unique<ReplicationChannel>(
            m_nextChannelId++, clientConn->SocketHandle
        );

        m_connections[clientConn->ConnectionId] = std::move(clientConn);
        LOG_INFO("Connected to server");
        return true;
    }

    void NetworkSystem::DisconnectFromServer()
    {
        m_driver.DisconnectClient();
        m_isConnected = false;
        m_connections.clear();
        LOG_INFO("Disconnected from server");
    }

    // ================================================================
    // Replication Interface
    // ================================================================

    void NetworkSystem::ReplicateActor(uint32_t actorId, 
                                      EReplicationType replicationType)
    {
        m_replicatedActors[actorId] = replicationType;
        LOG_INFO("Actor " + std::to_string(actorId) + 
                " registered for replication");
    }

    void NetworkSystem::StopReplicatingActor(uint32_t actorId)
    {
        m_replicatedActors.erase(actorId);
        LOG_INFO("Actor " + std::to_string(actorId) + 
                " unregistered from replication");
    }

    void NetworkSystem::QueuePropertyUpdate(uint32_t actorId, uint32_t componentId,
                                           const std::string& propertyName,
                                           const std::vector<uint8_t>& propertyData)
    {
        FReplicationUpdate update;
        update.ActorId = actorId;
        update.ComponentId = componentId;
        update.PropertyName = propertyName;
        update.PropertyData = propertyData;
        update.Timestamp = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - m_startTime
        ).count();

        // Queue to all active channels
        for (auto& conn : m_connections)
        {
            if (conn.second && conn.second->ReplicationChannel)
            {
                conn.second->ReplicationChannel->QueueReplicationUpdate(update);
            }
        }
    }

    // ================================================================
    // RPC Interface
    // ================================================================

    void NetworkSystem::QueueRemoteProcedureCall(uint32_t actorId,
                                                const std::string& methodName,
                                                const std::vector<uint8_t>& parameters,
                                                FRemoteProcedureCall::ETarget target)
    {
        FRemoteProcedureCall rpc;
        rpc.ActorId = actorId;
        rpc.MethodName = methodName;
        rpc.Parameters = parameters;
        rpc.Target = target;
        rpc.Timestamp = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - m_startTime
        ).count();

        // Queue to appropriate channels based on target
        for (auto& conn : m_connections)
        {
            if (conn.second && conn.second->ReplicationChannel)
            {
                conn.second->ReplicationChannel->QueueRemoteProcedureCall(rpc);
            }
        }
    }

    void NetworkSystem::RegisterRPCHandler(uint32_t actorId, 
                                          const std::string& methodName,
                                          RPCHandler handler)
    {
        auto key = std::make_pair(actorId, methodName);
        m_rpcHandlers[key] = handler;
    }

    // ================================================================
    // Connection Management
    // ================================================================

    FNetConnection* NetworkSystem::GetConnection(uint32_t connectionId)
    {
        auto it = m_connections.find(connectionId);
        return it != m_connections.end() ? it->second.get() : nullptr;
    }

    // ================================================================
    // State Query
    // ================================================================

    float NetworkSystem::GetServerUptime() const
    {
        auto elapsed = std::chrono::high_resolution_clock::now() - m_startTime;
        return std::chrono::duration<float>(elapsed).count();
    }

    // ================================================================
    // Internal Processing
    // ================================================================

    void NetworkSystem::ProcessIncomingConnections()
    {
        FNetAddress clientAddr;
        int clientSocket = m_driver.AcceptConnection(clientAddr);

        while (clientSocket != -1)
        {
            auto conn = std::make_unique<FNetConnection>(
                m_nextConnectionId++, clientSocket, clientAddr, true
            );
            conn->ReplicationChannel = std::make_unique<ReplicationChannel>(
                m_nextChannelId++, clientSocket
            );
            conn->State = ESocketState::Connected;

            LOG_INFO("New connection accepted: " + std::to_string(conn->ConnectionId));
            m_connections[conn->ConnectionId] = std::move(conn);

            // Try to accept another connection
            clientSocket = m_driver.AcceptConnection(clientAddr);
        }
    }

    void NetworkSystem::ProcessIncomingData()
    {
        for (auto& conn : m_connections)
        {
            if (!conn.second || conn.second->SocketHandle < 0)
            {
                continue;
            }

            FNetPacket packet;
            if (m_driver.ReceivePacket(conn.second->SocketHandle, packet))
            {
                m_totalBytesReceived += static_cast<uint32_t>(packet.GetSize());
                if (conn.second->ReplicationChannel)
                {
                    conn.second->ReplicationChannel->ReceiveUpdates(packet);
                }
            }
        }
    }

    void NetworkSystem::ProcessReplication(float deltaTime)
    {
        for (auto& conn : m_connections)
        {
            if (conn.second && conn.second->ReplicationChannel)
            {
                if (conn.second->ReplicationChannel->FlushUpdates(deltaTime))
                {
                    // Packet would be sent here in full implementation
                }
            }
        }
    }

    void NetworkSystem::DispatchRPCCalls(const FRemoteProcedureCall& rpc)
    {
        auto key = std::make_pair(rpc.ActorId, rpc.MethodName);
        auto it = m_rpcHandlers.find(key);

        if (it != m_rpcHandlers.end())
        {
            try
            {
                it->second(rpc);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("RPC handler error: " + std::string(e.what()));
            }
        }
    }

} // namespace Vulkana
