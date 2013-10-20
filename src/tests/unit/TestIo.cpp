#include <gtest/gtest.h>
#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>

#include "../../io/VectorStream.h"
#include "../../packetcomms/ComPacket.h"

struct Type1
{
    int32_t axis1;
    int32_t axis2;
    int32_t max;
};

struct Type2
{
    bool b;
    float f;
    Type1 t;
};

template<class Archive>
void serialize(Archive& archive,
               Type1& t1)
{
    archive( t1.axis1, t1.axis2, t1.max );
}

template<class Archive>
void serialize(Archive& archive,
               Type2& t2)
{
    archive( t2.b, t2.f, t2.t );
}

TEST( IO, VectorStream )
{
    Type1 in1{1,2,3};
    constexpr int intIn = 35;
    Type1 in2{4000,5000,6000};
    Type2 in3{true,0.1f,{10,11,12}};

    const ComPacket pkt;
    {
        VectorOutputStream vs;
        std::ostream archiveStream(&vs);
        cereal::PortableBinaryOutputArchive archive(archiveStream);
        archive(in1);
        archive(intIn);
        archive(in2,in3);

        // If we wanted to emplace the serialised data into the
        // comms-system then we would need to move the storage out
        // of VectorOutputStream into a ComPacket so test that here:
        const_cast<ComPacket&>(pkt) = ComPacket(IdManager::InvalidPacket,std::move(vs.Get()));
    }

    Type2 out3;
    Type1 out1;
    Type1 out2;
    {
        const VectorStream::Buffer& buffer = pkt.GetData();
        const size_t sizeBefore = buffer.size();
        VectorInputStream vsIn(buffer);
        std::istream achiveInputStream(&vsIn);
        cereal::PortableBinaryInputArchive archive(achiveInputStream);
        int intOut = 0;
        archive( out1, intOut, out2, out3 );
        const size_t sizeAfter = buffer.size();
        EXPECT_EQ( intOut, intIn );
        // Need to check because VectorInputStream uses const cast inside
        EXPECT_EQ( sizeBefore, sizeAfter );
    }

    EXPECT_EQ( in1.axis1, out1.axis1);
    EXPECT_EQ( in1.axis2, out1.axis2);
    EXPECT_EQ( in1.max,   out1.max);
    EXPECT_EQ( in2.axis1, out2.axis1);
    EXPECT_EQ( in2.axis2, out2.axis2);
    EXPECT_EQ( in2.max,   out2.max);

    EXPECT_EQ( out3.b,       in3.b);
    EXPECT_EQ( out3.f,       in3.f);
    EXPECT_EQ( out3.t.axis1, in3.t.axis1);
    EXPECT_EQ( out3.t.axis2, in3.t.axis2);
    EXPECT_EQ( out3.t.max,   out3.t.max);
}
