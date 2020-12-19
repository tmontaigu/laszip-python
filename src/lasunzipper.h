#ifndef LASZIP_BIND_LASUNZIPPER_H
#define LASZIP_BIND_LASUNZIPPER_H

#include "python_istreambuf.h"

#include <pybind11/pybind11.h>

#include <laszip/laszip_api.h>

#include <sstream>

namespace py = pybind11;

class LasUnZipper
{
  public:
    explicit LasUnZipper(py::object &file_obj);

    void decompress_into(py::buffer &buffer);

    const laszip_header& header() const;

    void close();

    ~LasUnZipper();

  private:
    python_istreambuf m_ibuf;
    std::istream m_input_stream;
    std::stringstream m_output_stream;

    bool m_closed{false};
    laszip_POINTER m_reader{nullptr};
    laszip_POINTER m_writer{nullptr};
    laszip_header *m_header{nullptr};
    laszip_point *m_point{nullptr};
};
#endif // LASZIP_BIND_LASUNZIPPER_H
