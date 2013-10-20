/**

    This file contains utilities that wrap up functionality from the Cereal library.

    Also contains cereal compatible serialize functions for some common types.
*/
#ifndef SERIALISATION_H
#define SERIALISATION_H

#include <time.h>

#include "../io/VectorStream.h"
#include <cereal/archives/portable_binary.hpp>

template<typename T>
void serialize(T& archive, timespec& t)
{
    /// @todo - this is probably not portable
    archive(t.tv_sec,t.tv_nsec);
}

template<typename T>
void Serialise(VectorOutputStream& stream, const T& type)
{
    std::ostream archiveStream(&stream);
    cereal::PortableBinaryOutputArchive archive(archiveStream);
    archive(type);
}

template<typename T>
void Deserialise(VectorInputStream& vis, T& type)
{
    std::istream stream(&vis);
    cereal::PortableBinaryInputArchive archive(stream);
    archive(type);
}

#endif // SERIALISATION_H
