// -----------------------------------------------------------------------------
// File:        TcpSocket.cpp
// Description: Source file with the tcp socket implementation.
// -----------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "TcpSocket.h"
#include "../Logger/Logger.h"

#include <thread> // sleep_for
#include <span>

//socket communication includes:
#ifdef __linux__
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <cstring>
    #include <fcntl.h>
    #error to be verified if solution is working
#elif _WIN32
    #include <ws2tcpip.h>
    #include <io.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #error other operating system is not suppored
#endif

//------------------------------------------------------------------------------
// Definitions and Macros
//------------------------------------------------------------------------------
using tools::Logger;
using Address = std::pair<std::string, uint16_t>;
using Buffer = tools::Socket::Buffer;
using ErrorCode = tools::TcpSocket::ErrorCode;
using Milliseconds = std::chrono::milliseconds;
using tools::TcpSocket;
using tools::TcpSocketImpl;

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
static constexpr SOCKET SocketError = SOCKET_ERROR;

//------------------------------------------------------------------------------
// Typedefs, Classes and Structures
//------------------------------------------------------------------------------
namespace tools
{
    class TcpSocketImpl
    {
    public:
        TcpSocketImpl(const std::string& ip, uint16_t port, Logger* p_logger);
        ~TcpSocketImpl();

        int Open(Milliseconds timeout);
        int Close();
        inline bool IsReady() const;

        int Recv(Buffer& buffer, Milliseconds timeout);
        int Send(const Buffer& buffer, Milliseconds timeout);

    private:
        Logger* mp_logger = nullptr;
#ifdef _WIN32
        inline static bool m_wsa{ false };
#endif

        Address m_address{};
        SOCKET m_fd{ SocketError };

        int OpenSocket();
        int SetupSocket();
        int CloseSocket();
        inline bool IsSocketOpened() const;
    };
}

//------------------------------------------------------------------------------
// Function Implementation - TcpSocketImpl / construction
//------------------------------------------------------------------------------
TcpSocketImpl::TcpSocketImpl(const std::string& ip, uint16_t port, Logger* p_logger)
    : m_address{ ip, port }
    , mp_logger{ p_logger }
{
    xFunctionInfo(mp_logger);
    xVariable(mp_logger, ip);
    xVariable(mp_logger, port);
}

TcpSocketImpl::~TcpSocketImpl()
{
    xFunctionInfo(mp_logger);

    (void)CloseSocket();
#ifdef _WIN32
    if (m_wsa)
    {
        m_wsa = false;
        WSACleanup();
    }
#endif
}

//------------------------------------------------------------------------------
// Function Implementation - CdiImpl / connection
//------------------------------------------------------------------------------
int TcpSocketImpl::Open(Milliseconds timeout)
{
    xFunctionInfo(mp_logger);
    xVariable(mp_logger, timeout.count());

    int result = ErrorCode::NoError;
    if(!IsReady())
    {
        (void)CloseSocket();

        const auto abort = std::chrono::system_clock::now() + timeout;
        do
        {
            result = OpenSocket();
            if (result != ErrorCode::NoError)
            {
                std::this_thread::sleep_for(Milliseconds(500));
                continue;
            }

            result = SetupSocket();
        } while ((std::chrono::system_clock::now() < abort) && (!IsSocketOpened()));
    }
    xVariable(mp_logger, m_fd);

    xResultInfo(mp_logger, result);
    return result;
}

int TcpSocketImpl::Close()
{
    xFunctionInfo(mp_logger);
    const int result = CloseSocket();
    xResultInfo(mp_logger, result);
    return result;
}

inline bool TcpSocketImpl::IsReady() const
{
    return IsSocketOpened();
}

//------------------------------------------------------------------------------
// Function Implementation - TcpSocketImpl / transfer
//------------------------------------------------------------------------------
int TcpSocketImpl::Recv(Buffer& buffer, Milliseconds timeout)
{
    xFunctionInfo(mp_logger);
    xVariable(mp_logger, timeout.count());
    int result = ErrorCode::NotConnected;

    if (IsSocketOpened())
    {
        constexpr int tmp_size = 1024;
        char tmp_buffer[tmp_size];

        int counter = 0;
        const auto abort = std::chrono::system_clock::now() + timeout;
        do 
        {
            const int read = ::recv(m_fd, tmp_buffer, tmp_size, 0);
            if (read > 0)
            {
                counter += read;
                const std::span wrapper{ tmp_buffer, static_cast<size_t>(read) };
                buffer.insert(buffer.end(), wrapper.begin(), wrapper.end());
            }
            else
            {
                std::this_thread::sleep_for(Milliseconds(10));
            }
        } while ((counter == 0) && (std::chrono::system_clock::now() < abort));

        result = (counter == 0) ? ErrorCode::FailedToRead : counter;
    }

    xResultInfo(mp_logger, result);
    return result;
}

int TcpSocketImpl::Send(const Buffer& buffer, Milliseconds timeout)
{
    xFunctionInfo(mp_logger);
    xVariable(mp_logger, buffer.size());
    xVariable(mp_logger, timeout.count());
    int result = ErrorCode::NotConnected;

    if (IsSocketOpened())
    {
        int counter = 0;
        const auto abort = std::chrono::system_clock::now() + timeout;
        do
        {
            const int written = ::send(m_fd, reinterpret_cast<const char*>(buffer.data() + counter), static_cast<int>(buffer.size() - counter), 0);
            if (written > 0)
            {
                counter += written;
            }
            else
            {
                std::this_thread::sleep_for(Milliseconds(10));
            }
        } while ((counter != buffer.size()) && (std::chrono::system_clock::now() < abort));

        result = (counter != buffer.size()) ? ErrorCode::FailedToWrite : counter;
    }

    xResultInfo(mp_logger, result);
    return result;
}

//------------------------------------------------------------------------------
// Function Implementation - TcpSocketImpl / socket
//------------------------------------------------------------------------------
int TcpSocketImpl::OpenSocket()
{
    xFunctionInfo(mp_logger);

    if (!IsSocketOpened())
    {
#ifdef _WIN32
        if (!m_wsa)
        {
            static WSADATA wsadata;
            m_wsa = (WSAStartup(0x0202, &wsadata) > 0);
        }
#endif
        m_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    }

    const int result = IsSocketOpened() ? ErrorCode::NoError : ErrorCode::FailedToCreate;
    xResultInfo(mp_logger, result);
    return result;
}

int TcpSocketImpl::SetupSocket()
{
    xFunctionInfo(mp_logger);
    int result = ErrorCode::NoError;

    try
    {
        if (!IsSocketOpened())
        {
            result = ErrorCode::NotCreated;
            throw std::exception{};
        }

#ifdef __linux__
        using reuse_t = int;
#elif _WIN32
        using reuse_t = char;
#endif
        reuse_t reuse = 1;
        if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == SocketError)
        {
            result = ErrorCode::FailedToSetOpt;
            throw std::exception{};
        }
        
        struct in_addr ip;
        if (inet_pton(AF_INET, m_address.first.c_str(), &ip) == 0)
        {
            result = ErrorCode::InvalidIp;
            throw std::exception{};
        }

        if (m_address.second == 0U)
        {
            result = ErrorCode::InvalidPort;
            throw std::exception{};
        }

        struct sockaddr_in address;
        {
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = ip.S_un.S_addr;
            address.sin_port = htons(m_address.second);
        }

        if (::connect(m_fd, (struct sockaddr*)&address, sizeof(address)) == SocketError)
        {
            result = ErrorCode::FailedToConnect;
            throw std::exception{};
        }

#ifdef __linux__
        if (::fcntl(m_fd, F_SETFL, O_NONBLOCK) == SocketError)
        {
            result = ErrorCode::Socket::SetCtrlFail;
            throw std::exception{};
        }
#elif _WIN32
        static constexpr u_long NonBlocking = 1U;
        u_long flags = NonBlocking;
        if (NO_ERROR != ::ioctlsocket(m_fd, FIONBIO, &flags))
        {
            result = ErrorCode::FailedToSetCtrl;
            throw std::exception{};
        }
#endif
    }
    catch (...)
    {
        (void)CloseSocket();
    }

    xResultInfo(mp_logger, result);
    return result;
}

int TcpSocketImpl::CloseSocket()
{
    xFunctionInfo(mp_logger);

    if (IsSocketOpened())
    {
#ifdef __linux__
        ::close(m_fd.fd);
#elif _WIN32
        ::closesocket(m_fd);
#endif
        m_fd = SocketError;
    }

    constexpr int result = ErrorCode::NoError;
    xResultInfo(mp_logger, result);
    return result;
}

inline bool TcpSocketImpl::IsSocketOpened() const
{
    return m_fd != SocketError;
}

//------------------------------------------------------------------------------
// Function Implementation - Socket
//------------------------------------------------------------------------------
TcpSocket::TcpSocket(const std::string& ip, uint16_t port, Logger* p_logger)
{
    m_impl = std::make_shared<TcpSocketImpl>(ip, port, p_logger);
}

TcpSocket::~TcpSocket()
{
    m_impl.reset();
}

int TcpSocket::Open(unsigned ms_timeout)
{
    return m_impl->Open(Milliseconds(ms_timeout));
}

int TcpSocket::Close(unsigned ms_timeout)
{
    (void)ms_timeout;
    return m_impl->Close();
}

bool TcpSocket::IsReady() const
{
    return m_impl->IsReady();
}

int TcpSocket::Recv(Buffer& buffer, unsigned ms_timeout)
{
    return m_impl->Recv(buffer, Milliseconds(ms_timeout));
}

int TcpSocket::Send(const Buffer& buffer, unsigned ms_timeout)
{
    return m_impl->Send(buffer, Milliseconds(ms_timeout));
}
