#ifndef VECTORSTREAM_H
#define VECTORSTREAM_H

#include <streambuf>
#include <vector>

/**
    Primarily the vector IO streams are intended for
    use with the 'Cereal' serilisation library to get
    an efficient in memory representation of serialised
    binary streams and to be able to deserialise from
    them. The data can then be transmitted/received
    with no coupling to the serialisation library.
*/
namespace VectorStream
{
typedef std::vector<std::streambuf::char_type> Buffer;
typedef std::streambuf::char_type CharType;
}

/**
    An output stream buffer that writes directly to
    a wrapped ref to a std::vector.
*/
class VectorOutputStream : public std::streambuf
{
public:
    explicit VectorOutputStream( VectorStream::Buffer& v ) : m_v(v) {}

private:
    std::streambuf::int_type overflow( std::streambuf::int_type ch )
    {
        if ( ch == std::streambuf::traits_type::eof() )
        {
            return std::streambuf::traits_type::eof();
        }
        m_v.push_back(ch);
        return ch;
    }

    std::streamsize xsputn( const std::streambuf::char_type* s, std::streamsize n )
    {
        // Make sure vector has enough capacity then push back all the bytes:
        m_v.reserve(m_v.size()+n);
        size_t w = n+1;
        while ( --w != 0 )
        {
            m_v.push_back(*s);
            s += 1;
        }

        return n-w;
    }

    VectorStream::Buffer& m_v;
};

/**
    An input stream buffer that reads directly
    from a wrapped ref to a std::vector.
*/
class VectorInputStream : public std::streambuf
{
public:
    explicit VectorInputStream( VectorStream::Buffer& v )
    {
        std::streambuf::char_type* begin = v.data();
        setg( begin, begin, begin + v.size() );
    }

private:
    std::streambuf::int_type overflow()
    {
        return traits_type::eof();
    }
};

#endif // VECTORSTREAM_H
