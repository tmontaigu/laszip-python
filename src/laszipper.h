#ifndef LASZIP_BIND_LASZIPPER_H
#define LASZIP_BIND_LASZIPPER_H

#include <sstream>

#include <pybind11/pybind11.h>

#include "python_ostreambuf.h"
#include <laszip_api.h>
namespace py = pybind11;

class LasZipper
{
  public:
    explicit LasZipper(py::object &file_obj, py::bytes &header_bytes);

    size_t compress(py::buffer &buffer);

    void done();

    ~LasZipper();

    const laszip_header &header() const;

  private:
    python_ostreambuf m_b;
    std::ostream m_output_stream;

    std::stringstream m_is;

    bool m_closed{false};
    laszip_POINTER m_reader{nullptr};
    laszip_POINTER m_writer{nullptr};
    laszip_header *m_header{nullptr};
    laszip_point *m_point{nullptr};
};

#endif // LASZIP_BIND_LASZIPPER_H
