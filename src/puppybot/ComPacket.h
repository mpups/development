/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __COM_PACKET_H__
#define __COM_PACKET_H__

#include <cstdint>
#include <vector>
#include <type_traits>
#include <memory>
#include <queue>

class ComPacket
{
public:
     typedef std::shared_ptr<ComPacket> SharedPacket;
     typedef std::shared_ptr<const ComPacket> ConstSharedPacket;
     typedef std::queue< SharedPacket > PacketContainer;

     enum class Type : uint32_t {
        Invalid = 0,
        AvInfo,
        AvData,
        Odometry,
        Last,
     };

    ComPacket( const ComPacket& ) = delete;

    /// Default constructed invalid packet:
    ComPacket() : m_type( ComPacket::Type::Invalid ) {};

    /// Construct a com packet from raw buffer of unsigned 8-bit data:
    ComPacket( ComPacket::Type type, uint8_t* buffer, int size ) : m_type(type), m_data( buffer, buffer+size ) {};

    /// Construct ComPacket with preallocated data size (but no valid data).
    ComPacket( ComPacket::Type type, int size ) : m_type(type), m_data( size ) {};

    virtual ~ComPacket() {};

    /// @param p The ComPacket to be moved - it will become of invalid type, with an empty data vector.
    ComPacket( ComPacket&& p ) : m_type( ComPacket::Type::Invalid ) {
        std::swap( p.m_type, m_type );
        std::swap( p.m_data, m_data );
    }; /// @todo use delgating constructor to create  invalid packet when upgraded to gcc-4.7+

    ComPacket::Type GetType()   const { return m_type; };
    const uint8_t* GetDataPtr() const { return &(m_data[0]); };
    uint8_t* GetDataPtr() { return &(m_data[0]); };
    std::vector<uint8_t>::size_type GetDataSize() const noexcept { return m_data.size(); };
    const std::vector<uint8_t>& GetData() const { return m_data; };
    std::vector<uint8_t>& GetData() { return m_data; };
protected:

private:
    ComPacket::Type m_type;
    std::vector<uint8_t> m_data;
};

// Need to define a hash function to use strongly typed enum as a map key:
namespace std
{
    template <>
    struct hash<ComPacket::Type>
    {
        size_t operator()(const ComPacket::Type& type) const
        {
            return hash<int>()( static_cast<int>(type) );
        }
    };
}

#endif /* __COM_PACKET_H__ */

