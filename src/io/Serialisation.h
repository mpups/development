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

template<typename ...Args>
void Serialise(VectorOutputStream& stream, const Args&... types)
{
    std::ostream archiveStream(&stream);
    cereal::PortableBinaryOutputArchive archive(archiveStream);
    archive(std::forward<const Args&>(types)...);
}

template<typename ...Args>
void Deserialise(VectorInputStream& vis, Args&... types)
{
    std::istream stream(&vis);
    cereal::PortableBinaryInputArchive archive(stream);
    archive(std::forward<Args&>(types)...);
}

#endif // SERIALISATION_H
