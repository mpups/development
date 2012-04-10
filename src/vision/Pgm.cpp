#include "Pgm.h"

#include "Image.h"

#include <fstream>
#include <stdint.h>

namespace robo
{

/**
    Write a pgm file from the specified image. This will overwrite
    any existing file with the same file name.

    @param fileName name of resulting file.
    @param image image to write.
*/
bool WritePgm( const char* fileName, const Image<uint8_t>& image )
{
    std::ofstream file( fileName );

    if ( file.is_open() )
    {
        std::string pname;
        file << "P5 " << image.Width() << " " << image.Height() << " 255\n"; // NOT std::endl
        for ( uint32_t i=0; i<image.Height(); ++i )
        {
            file.write( reinterpret_cast<const char*>( image[i] ), image.Width() );
        }

        if ( file.bad() == false )
        {
            return true;
        }
    }

    return false;
}

/**
    Read a pgm file with the specified fileName.

    This only reads pgm images without comments because the standard is WRONG.
    (PGM is supposed to be the simplest image format so why did they make it
    complicated to parse by allowing comments to appear anywhere in the header?)

    The image will be (re-)allocated to the correct size.
*/
bool ReadPgm( const char* fileName, Image<uint8_t>& image )
{
    std::ifstream file( fileName );

    if ( file.is_open() )
    {
        std::string pname;
        file >> pname;
        if ( pname == "P5" )
        {
            uint32_t w, h, s;
            file >> w;
            file >> h;
            file >> s;
            file.get(); // read the mandatory whitespace character
            image.Allocate( w, h );
            for ( uint32_t i=0; i<image.Height(); ++i )
            {
                file.read( reinterpret_cast<char*>( image[i] ), image.Width() );
            }

            if ( file.bad() == false )
            {
                return true;
            }
        }
    }

    return false;
}

} // end of namespace robo

