/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __COM_PACKET_H__
#define __COM_PACKET_H__

#include <cstdint>
#include <vector>

class ComPacket
{
public:
     enum class Type : uint32_t {
        Invalid = 0,
        AvInfo,
        AvData,
        Odometry,
        Last,
     };

    /// Default constructed invalid packet:
    ComPacket() : m_type( ComPacket::Type::Invalid ) {};

    /// Construct a com packet from raw buffer of unsigned 8-bit data:
    ComPacket( ComPacket::Type type, uint8_t* buffer, int size ) : m_type(type), m_data( buffer, buffer+size ) {};
    ComPacket( ComPacket::Type type, int size ) : m_type(type), m_data( size ) {};
    virtual ~ComPacket() {};

    ComPacket( ComPacket&& p ) : m_type( ComPacket::Type::Invalid ) { std::swap( p.m_type, m_type ); }; ///@todo use delgating constructor when upgraded to gcc-4.7+

    ComPacket::Type GetType()   const { return m_type; };
    const uint8_t* GetDataPtr() const { return &(m_data[0]); };
    uint8_t* GetDataPtr() { return &(m_data[0]); };
    std::vector<uint8_t>::size_type GetDataSize() const noexcept { return m_data.size(); };
    const std::vector<uint8_t>& GetData() const { return m_data; };
protected:

private:
    ComPacket::Type m_type;
    std::vector<uint8_t> m_data;
};

#endif /* __COM_PACKET_H__ */

