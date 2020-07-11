#include "Library.h"

#include <assert.h>

namespace Ft
{

/**
    Create and initialise a FreeType2 library resource.

    Default constructor adds all default modules.
**/
Library::Library()
{
    m_err = FT_Init_FreeType( &m_lib );

    assert( m_err == 0 );
}

/**
    Release the FreeType2 library resource.
**/
Library::~Library()
{
    m_err = FT_Done_FreeType( m_lib );
    assert( m_err == 0 );
}

} // end namespace Ft
