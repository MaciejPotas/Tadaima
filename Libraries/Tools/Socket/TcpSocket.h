// -----------------------------------------------------------------------------
// File:        TcpSocket.h
// Description: Header file with the tcp socket implementation.
// -----------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "Socket.h"
#include <string>
#include <memory>

//------------------------------------------------------------------------------
// Typedefs, Classes and Structures
//------------------------------------------------------------------------------

namespace tools
{
    class Logger;
    class TcpSocketImpl;

    class TcpSocket : public Socket
    {
    public:
        struct ErrorCode
        {
            static constexpr int NoError = 0;
            static constexpr int FailedToCreate = -1;
            static constexpr int NotCreated = -2;
            static constexpr int FailedToConnect = -3;
            static constexpr int NotConnected = -4;
            static constexpr int FailedToSetOpt = -5;
            static constexpr int FailedToSetCtrl = -6;
            static constexpr int InvalidIp = -7;
            static constexpr int InvalidPort = -8;
            static constexpr int FailedToRead = -9;
            static constexpr int FailedToWrite = -10;
        };

        TcpSocket(const std::string& ip, uint16_t port, Logger* p_logger = nullptr);
        virtual ~TcpSocket() override;

        virtual int Open(unsigned ms_timeout) override;
        virtual int Close(unsigned ms_timeout) override;
        virtual bool IsReady() const override;

        virtual int Recv(Buffer& buffer, unsigned ms_timeout) override;
        virtual int Send(const Buffer& buffer, unsigned ms_timeout) override;

    private:
        std::shared_ptr<TcpSocketImpl> m_impl{};
    };

}
