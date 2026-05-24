// ===================================================================
// Vulkana Network Driver - Low-Level Socket Abstraction
// ===================================================================
// Provides cross-platform TCP socket operations for client/server
// communication. Wraps WinSock2 (Windows) and BSD sockets (POSIX).
// ===================================================================

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <chrono>

namespace Vulkana
{
    // ================================================================
    // Socket Types & Constants
    // ================================================================

    enum class ESocketType : uint8_t
    {
        None = 0,
        Server,
        Client,
        Connection
    };

    enum class ESocketState : uint8_t
    {
        Closed = 0,
        Listening,
        Connecting,
        Connected,
        Error
    };

    // Network address/port pair
    struct FNetAddress
    {
        std::string Address;
        uint16_t Port;

        FNetAddress() : Address("127.0.0.1"), Port(0) {}
        FNetAddress(const std::string& addr, uint16_t port) 
            : Address(addr), Port(port) {}
    };

    // ================================================================
    // Network Packet Structure
    // ================================================================
    struct FNetPacket
    {
        static constexpr size_t MAX_PACKET_SIZE = 65535;

        std::vector<uint8_t> Data;
        FNetAddress RemoteAddress;
        std::chrono::high_resolution_clock::time_point Timestamp;

        FNetPacket() = default;
        explicit FNetPacket(size_t capacity)
        {
            Data.reserve(capacity);
        }

        void Clear()
        {
            Data.clear();
        }

        size_t GetSize() const { return Data.size(); }
    };

    // ================================================================
    // Network Driver
    // ================================================================
    class NetworkDriver
    {
    public:
        NetworkDriver();
        ~NetworkDriver();

        // Deleted copy/move to prevent socket duplication
        NetworkDriver(const NetworkDriver&) = delete;
        NetworkDriver& operator=(const NetworkDriver&) = delete;
        NetworkDriver(NetworkDriver&&) = delete;
        NetworkDriver& operator=(NetworkDriver&&) = delete;

        // ================================================================
        // Initialization & Cleanup
        // ================================================================

        /// Initialize the network subsystem (platform-specific socket setup).
        bool Initialize();

        /// Cleanup and close all sockets.
        void Shutdown();

        // ================================================================
        // Server Operations
        // ================================================================

        /// Create a server socket listening on the given address/port.
        /// Returns true on success, false on failure.
        bool CreateServerSocket(const FNetAddress& bindAddress, 
                               int maxConnections = 32);

        /// Accept an incoming client connection.
        /// Returns a new socket handle if a connection is available, -1 otherwise.
        int AcceptConnection(FNetAddress& outClientAddress);

        // ================================================================
        // Client Operations
        // ================================================================

        /// Create a client socket and connect to the server.
        /// Returns true on success, false on failure.
        bool CreateClientSocket(const FNetAddress& serverAddress);

        /// Disconnect the client socket.
        void DisconnectClient();

        // ================================================================
        // Data Transfer
        // ================================================================

        /// Send data on a socket. Returns bytes sent, -1 on error.
        int SendData(int socketHandle, const void* data, size_t length);

        /// Receive data from a socket. Returns bytes received, 0 if connection closed, -1 on error.
        int ReceiveData(int socketHandle, void* outBuffer, size_t bufferSize);

        /// Send a packet to a specific connection.
        bool SendPacket(int socketHandle, const FNetPacket& packet);

        /// Receive a packet from a connection.
        bool ReceivePacket(int socketHandle, FNetPacket& outPacket);

        // ================================================================
        // Socket State & Query
        // ================================================================

        /// Get the state of a socket.
        ESocketState GetSocketState(int socketHandle) const;

        /// Check if a socket is valid and connected.
        bool IsSocketConnected(int socketHandle) const;

        /// Close a socket handle.
        void CloseSocket(int socketHandle);

        /// Get the local address of a socket.
        FNetAddress GetLocalAddress(int socketHandle) const;

        /// Get the remote address of a socket.
        FNetAddress GetRemoteAddress(int socketHandle) const;

    private:
        // Platform-specific implementation
        int m_serverSocket;
        int m_clientSocket;
        bool m_isInitialized;

        // Platform initialization
        bool InitializePlatform();
        void ShutdownPlatform();
    };

} // namespace Vulkana
