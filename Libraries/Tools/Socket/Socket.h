// -----------------------------------------------------------------------------
// File:        Socket.h
// Description: Header file with the socket interface.
// -----------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <vector>

//------------------------------------------------------------------------------
// Typedefs, Classes and Structures
//------------------------------------------------------------------------------
namespace tools
{

    class Socket
    {
    public:
        using Buffer = std::vector<uint8_t>;

        virtual ~Socket() = default;

        virtual int Open(unsigned ms_timeout) = 0;
        virtual int Close(unsigned ms_timeout) = 0;
        virtual bool IsReady() const = 0;

        virtual int Recv(Buffer& buffer, unsigned ms_timeout) = 0;
        virtual int Send(const Buffer& buffer, unsigned ms_timeout) = 0;
    };

}
