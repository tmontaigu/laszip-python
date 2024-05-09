#ifndef LASZIP_BIND_LASZIP_ERROR_H
#define LASZIP_BIND_LASZIP_ERROR_H

#ifdef USE_VENDORED_LASZIP
#include <laszip_api.h>
#else
#include <laszip/laszip_api.h>
#endif

class laszip_error : public std::runtime_error
{
  public:
    static laszip_error last_error(laszip_POINTER laszip_ptr)
    {
        char *error_msg;
        laszip_get_error(laszip_ptr, &error_msg);
        return laszip_error(error_msg);
    }

    explicit laszip_error(const char *message) : std::runtime_error(message) {}
};

#endif // LASZIP_BIND_LASZIP_ERROR_H
