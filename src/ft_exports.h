#ifndef FREETYPECPP_EXPORTS_H
#define FREETYPECPP_EXPORTS_H

// Win-32 specific:
// By default it will be assumed you are building an application which is dynamically linked
// to the library dlls. (The following preprocessor commands drop through in such a way that
// the default export is __declspec(dllimport).

#ifdef WIN32

#ifdef FTCPP_EXPORTS
#define FTCPP_API __declspec(dllexport)
#else
#define FTCPP_API __declspec(dllimport)
#endif

#else
#define FTCPP_API
#endif

#endif  // FREETYPECPP_EXPORTS_H
