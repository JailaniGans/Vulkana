// ===================================================================
// Vulkana Network System - Subsystem for Network Management
// ===================================================================
// Manages networked gameplay through a client-server architecture.
// Handles Actor replication, RPC routing, and connection lifecycle.
// Integrates with the Subsystem framework for engine-level management.
// ===================================================================

#pragma once

#include "core/Subsystem.hpp"
#include "NetworkDriver.hpp"
#include <map>
#include <functional>
#include <queue>
#include <memory>
#include <cstdint>

namespace Vulkana
{
    // Forward declarations
    class Actor;
    class ActorComponent;

    // ================================================================
    // Replication State & Serialization
    // ================================================================

    enum class EReplicationType : uint8_t
    {
        None = 0,
        OwnerOnly,
        SimulatedOnly,
        All
    };

    /// Represents a replicated property change event
    struct FReplicationUpdate
    {
        uint32_t ActorId;
        uint32_t ComponentId;
        std::string PropertyName;
        std::vector<uint8_t> PropertyData;
        uint32_t Timestamp;
    };

    /// Represents an RPC call to be executed
    struct FRemoteProcedureCall
    {
        enum class ETarget : uint8_t
        {
            Server = 0,
            Client,
            All
        };

        uint32_t ActorId;
        std::string MethodName;
        std::vector<uint8_t> Parameters;
        ETarget Target;
        uint32_t Timestamp;
    };

    // ================================================================
    // Replication Channel - Manages Actor/Component Sync
    // ================================================================
    class ReplicationChannel
    {
    public:
        ReplicationChannel(uint32_t channelId, int connectionSocket)
            : m_channelId(channelId), m_connectionSocket(connectionSocket),
              m_lastReplicationTime(0), m_replicationInterval(0.016f)
        {
        }

        /// Queue a property update for replication
        void QueueReplicationUpdate(const FReplicationUpdate& update)
        {
            m_pendingUpdates.push(update);
        }

        /// Queue an RPC call for execution
        void QueueRemoteProcedureCall(const FRemoteProcedureCall& rpc)
        {
            m_pendingRPCs.push(rpc);
        }

        /// Process and send all pending updates/RPCs over the network
        bool FlushUpdates(float deltaTime);

        /// Receive and process incoming updates/RPCs
        bool ReceiveUpdates(FNetPacket& packet);

        uint32_t GetChannelId() const { return m_channelId; }
        int GetConnectionSocket() const { return m_connectionSocket; }
        bool IsActive() const { return m_connectionSocket >= 0; }

    private:
        uint32_t m_channelId;
        int m_connectionSocket;
        float m_lastReplicationTime;
        float m_replicationInterval;

        std::queue<FReplicationUpdate> m_pendingUpdates;
        std::queue<FRemoteProcedureCall> m_pendingRPCs;

        // Serialization helpers
        std::vector<uint8_t> SerializeUpdates();
        bool DeserializeUpdates(const std::vector<uint8_t>& data);
    };

    // ================================================================
    // Connection State - Tracks a Remote Peer
    // ================================================================
    struct FNetConnection
    {
        uint32_t ConnectionId;
        int SocketHandle;
        FNetAddress Address;
        ESocketState State;
        std::unique_ptr<ReplicationChannel> ReplicationChannel;
        uint32_t ConnectedTime;
        bool IsServer;

        FNetConnection(uint32_t id, int socket, const FNetAddress& addr, bool server)
            : ConnectionId(id), SocketHandle(socket), Address(addr),
              State(ESocketState::Closed), ReplicationChannel(nullptr),
              ConnectedTime(0), IsServer(server)
        {
        }
    };

    // ================================================================
    // Network System - Main Subsystem
    // ================================================================
    class NetworkSystem : public TSubsystem<NetworkSystem>
    {
    public:
        NetworkSystem();
        virtual ~NetworkSystem() override;

        // ================================================================
        // Subsystem Lifecycle
        // ================================================================

        virtual void Initialize() override;
        virtual void Tick(float dt) override;
        virtual void Shutdown() override;

        // ================================================================
        // Server Operations
        // ================================================================

        /// Start the server on the given address/port
        bool StartServer(const FNetAddress& bindAddress, int maxClients = 32);

        /// Stop the server and close all connections
        void StopServer();

        /// Check if running as server
        bool IsServer() const { return m_isServer; }

        // ================================================================
        // Client Operations
        // ================================================================

        /// Connect to a remote server
        bool ConnectToServer(const FNetAddress& serverAddress);

        /// Disconnect from the server
        void DisconnectFromServer();

        /// Check if connected to server
        bool IsConnected() const { return m_isConnected; }

        // ================================================================
        // Replication Interface
        // ================================================================

        /// Register an Actor for replication over all connections
        void ReplicateActor(uint32_t actorId, EReplicationType replicationType);

        /// Unregister an Actor from replication
        void StopReplicatingActor(uint32_t actorId);

        /// Queue a property update for replication
        void QueuePropertyUpdate(uint32_t actorId, uint32_t componentId,
                                const std::string& propertyName,
                                const std::vector<uint8_t>& propertyData);

        // ================================================================
        // RPC Interface
        // ================================================================

        /// Queue an RPC call to execute on server/client/all
        void QueueRemoteProcedureCall(uint32_t actorId,
                                     const std::string& methodName,
                                     const std::vector<uint8_t>& parameters,
                                     FRemoteProcedureCall::ETarget target);

        /// Register an RPC handler for a specific method
        using RPCHandler = std::function<void(const FRemoteProcedureCall&)>;
        void RegisterRPCHandler(uint32_t actorId, const std::string& methodName,
                               RPCHandler handler);

        // ================================================================
        // Connection Management
        // ================================================================

        /// Get a connection by ID
        FNetConnection* GetConnection(uint32_t connectionId);

        /// Get all active connections
        const std::map<uint32_t, std::unique_ptr<FNetConnection>>& 
            GetConnections() const { return m_connections; }

        /// Get connection count
        size_t GetConnectionCount() const { return m_connections.size(); }

        // ================================================================
        // State Query
        // ================================================================

        float GetServerUptime() const;
        uint32_t GetTotalBytesReceived() const { return m_totalBytesReceived; }
        uint32_t GetTotalBytesSent() const { return m_totalBytesSent; }

    private:
        NetworkDriver m_driver;
        bool m_isServer;
        bool m_isConnected;
        uint32_t m_nextConnectionId;
        uint32_t m_nextChannelId;

        std::map<uint32_t, std::unique_ptr<FNetConnection>> m_connections;
        std::map<uint32_t, EReplicationType> m_replicatedActors;
        std::map<std::pair<uint32_t, std::string>, RPCHandler> m_rpcHandlers;

        uint32_t m_totalBytesReceived;
        uint32_t m_totalBytesSent;
        std::chrono::high_resolution_clock::time_point m_startTime;

        // Internal processing
        void ProcessIncomingConnections();
        void ProcessIncomingData();
        void ProcessReplication(float deltaTime);
        void DispatchRPCCalls(const FRemoteProcedureCall& rpc);
    };

} // namespace Vulkana
