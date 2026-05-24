// ===================================================================
// Vulkana Network Driver - Platform-Specific Implementation
// ===================================================================
// Windows-based implementation using WinSock2.
// For POSIX systems, adapt to use BSD socket APIs.
// ===================================================================

#include "NetworkDriver.hpp"
#include "core/Log.hpp"

#ifdef _WIN32
    #pragma warning(disable: 4244)  // Suppress SOCKET->int conversion warnings
    #pragma comment(lib, "ws2_32.lib")
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define SHUT_RDWR SD_BOTH
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define closesocket close
#endif

namespace Vulkana
{
    // ================================================================
    // Constructor & Destructor
    // ================================================================

    NetworkDriver::NetworkDriver()
        : m_serverSocket(-1), m_clientSocket(-1), m_isInitialized(false)
    {
    }

    NetworkDriver::~NetworkDriver()
    {
        if (m_isInitialized)
        {
            Shutdown();
        }
    }

    // ================================================================
    // Initialization & Cleanup
    // ================================================================

    bool NetworkDriver::Initialize()
    {
        if (m_isInitialized)
        {
            LOG_WARN("NetworkDriver already initialized");
            return true;
        }

        if (!InitializePlatform())
        {
            LOG_ERROR("Failed to initialize platform network subsystem");
            return false;
        }

        m_serverSocket = -1;
        m_clientSocket = -1;
        m_isInitialized = true;

        LOG_INFO("NetworkDriver initialized successfully");
        return true;
    }

    void NetworkDriver::Shutdown()
    {
        if (!m_isInitialized)
        {
            return;
        }

        if (m_serverSocket != -1)
        {
            closesocket(m_serverSocket);
            m_serverSocket = -1;
        }

        if (m_clientSocket != -1)
        {
            closesocket(m_clientSocket);
            m_clientSocket = -1;
        }

        ShutdownPlatform();
        m_isInitialized = false;

        LOG_INFO("NetworkDriver shutdown complete");
    }

    // ================================================================
    // Platform Initialization (Windows)
    // ================================================================

    bool NetworkDriver::InitializePlatform()
    {
#ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            LOG_ERROR("WSAStartup failed");
            return false;
        }
        return true;
#else
        // POSIX systems don't require special initialization
        return true;
#endif
    }

    void NetworkDriver::ShutdownPlatform()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // ================================================================
    // Server Operations
    // ================================================================

    bool NetworkDriver::CreateServerSocket(const FNetAddress& bindAddress,
                                           int maxConnections)
    {
        if (!m_isInitialized)
        {
            LOG_ERROR("NetworkDriver not initialized");
            return false;
        }

        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            LOG_ERROR("Failed to create server socket");
            return false;
        }

        // Allow socket reuse
        int reuseAddr = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
                      (const char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR)
        {
            LOG_WARN("Failed to set SO_REUSEADDR");
            closesocket(sock);
            return false;
        }

        // Bind socket
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(bindAddress.Port);

        if (inet_pton(AF_INET, bindAddress.Address.c_str(), &serverAddr.sin_addr) <= 0)
        {
            LOG_ERROR("Invalid server address");
            closesocket(sock);
            return false;
        }

        if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            LOG_ERROR("Failed to bind server socket");
            closesocket(sock);
            return false;
        }

        // Listen for connections
        if (listen(sock, maxConnections) == SOCKET_ERROR)
        {
            LOG_ERROR("Failed to listen on server socket");
            closesocket(sock);
            return false;
        }

        m_serverSocket = sock;
        LOG_INFO("Server socket created on " + bindAddress.Address + ":" + 
                std::to_string(bindAddress.Port));
        return true;
    }

    int NetworkDriver::AcceptConnection(FNetAddress& outClientAddress)
    {
        if (m_serverSocket == -1)
        {
            return -1;
        }

        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(m_serverSocket, (sockaddr*)&clientAddr, 
                                 &clientAddrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            return -1; // No connection available
        }

        // Extract client address
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
        outClientAddress.Address = std::string(ipStr);
        outClientAddress.Port = ntohs(clientAddr.sin_port);

        LOG_INFO("Accepted connection from " + outClientAddress.Address + ":" +
                std::to_string(outClientAddress.Port));
        return clientSocket;
    }

    // ================================================================
    // Client Operations
    // ================================================================

    bool NetworkDriver::CreateClientSocket(const FNetAddress& serverAddress)
    {
        if (!m_isInitialized)
        {
            LOG_ERROR("NetworkDriver not initialized");
            return false;
        }

        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            LOG_ERROR("Failed to create client socket");
            return false;
        }

        // Connect to server
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverAddress.Port);

        if (inet_pton(AF_INET, serverAddress.Address.c_str(), 
                     &serverAddr.sin_addr) <= 0)
        {
            LOG_ERROR("Invalid server address");
            closesocket(sock);
            return false;
        }

        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            LOG_ERROR("Failed to connect to server");
            closesocket(sock);
            return false;
        }

        m_clientSocket = sock;
        LOG_INFO("Connected to server at " + serverAddress.Address + ":" +
                std::to_string(serverAddress.Port));
        return true;
    }

    void NetworkDriver::DisconnectClient()
    {
        if (m_clientSocket != -1)
        {
            shutdown(m_clientSocket, SHUT_RDWR);
            closesocket(m_clientSocket);
            m_clientSocket = -1;
            LOG_INFO("Disconnected from server");
        }
    }

    // ================================================================
    // Data Transfer
    // ================================================================

    int NetworkDriver::SendData(int socketHandle, const void* data, size_t length)
    {
        if (socketHandle == -1 || !data)
        {
            return -1;
        }

        int bytesSent = send(socketHandle, (const char*)data, (int)length, 0);
        return bytesSent;
    }

    int NetworkDriver::ReceiveData(int socketHandle, void* outBuffer, size_t bufferSize)
    {
        if (socketHandle == -1 || !outBuffer)
        {
            return -1;
        }

        int bytesReceived = recv(socketHandle, (char*)outBuffer, (int)bufferSize, 0);
        return bytesReceived;
    }

    bool NetworkDriver::SendPacket(int socketHandle, const FNetPacket& packet)
    {
        int bytesSent = SendData(socketHandle, packet.Data.data(), packet.Data.size());
        return bytesSent == (int)packet.Data.size();
    }

    bool NetworkDriver::ReceivePacket(int socketHandle, FNetPacket& outPacket)
    {
        outPacket.Data.resize(FNetPacket::MAX_PACKET_SIZE);
        int bytesReceived = ReceiveData(socketHandle, outPacket.Data.data(), 
                                        outPacket.Data.size());

        if (bytesReceived <= 0)
        {
            outPacket.Data.clear();
            return false;
        }

        outPacket.Data.resize(bytesReceived);
        outPacket.Timestamp = std::chrono::high_resolution_clock::now();
        return true;
    }

    // ================================================================
    // Socket State & Query
    // ================================================================

    ESocketState NetworkDriver::GetSocketState(int socketHandle) const
    {
        if (socketHandle == -1)
        {
            return ESocketState::Closed;
        }

        // Simple state check (can be enhanced with socket options)
        return ESocketState::Connected;
    }

    bool NetworkDriver::IsSocketConnected(int socketHandle) const
    {
        return socketHandle != -1 && GetSocketState(socketHandle) == ESocketState::Connected;
    }

    void NetworkDriver::CloseSocket(int socketHandle)
    {
        if (socketHandle != -1)
        {
            shutdown(socketHandle, SHUT_RDWR);
            closesocket(socketHandle);
        }
    }

    FNetAddress NetworkDriver::GetLocalAddress(int socketHandle) const
    {
        FNetAddress localAddr;

        sockaddr_in addr{};
        int addrLen = sizeof(addr);

        if (getsockname(socketHandle, (sockaddr*)&addr, &addrLen) == 0)
        {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN);
            localAddr.Address = std::string(ipStr);
            localAddr.Port = ntohs(addr.sin_port);
        }

        return localAddr;
    }

    FNetAddress NetworkDriver::GetRemoteAddress(int socketHandle) const
    {
        FNetAddress remoteAddr;

        sockaddr_in addr{};
        int addrLen = sizeof(addr);

        if (getpeername(socketHandle, (sockaddr*)&addr, &addrLen) == 0)
        {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN);
            remoteAddr.Address = std::string(ipStr);
            remoteAddr.Port = ntohs(addr.sin_port);
        }

        return remoteAddr;
    }

} // namespace Vulkana
