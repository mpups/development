#ifndef FreeType_LIBRARY_H
#define FreeType_LIBRARY_H

#include "ft_exports.h"

extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace Ft
{

/**
    Wrapper for a handle to FreeType2 FT_Library handle.
**/
class FTCPP_API Library
{
friend class Face;

public:
    Library();
    ~Library();

    Library(const Library&) = delete;

    FT_Error GetError() const { return m_err; };

protected:

private:
    FT_Library m_lib;
    FT_Error m_err;
};

} // end namespace Ft

#endif // FreeType_LIBRARY_H
