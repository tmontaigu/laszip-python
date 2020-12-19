#ifndef LASZIP_BIND_PYTHON_OSTREAMBUF_H
#define LASZIP_BIND_PYTHON_OSTREAMBUF_H

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <iostream>

class python_ostreambuf : public std::basic_streambuf<char>
{
  private:
    typedef std::basic_streambuf<char> base_t;

  public:
    typedef base_t::char_type char_type;
    typedef base_t::int_type int_type;
    typedef base_t::pos_type pos_type;
    typedef base_t::off_type off_type;
    typedef base_t::traits_type traits_type;

    explicit python_ostreambuf(const py::object &file_obj)
        : m_write_fn(file_obj.attr("write").cast<pybind11::function>()),
          m_tell_fn(file_obj.attr("tell").cast<pybind11::function>()),
          m_seek_fn(file_obj.attr("seek").cast<pybind11::function>()), m_write_buf(),
          m_pos_in_file(m_tell_fn().cast<pybind11::int_>())
    {
        setp(&m_write_buf[0], &m_write_buf[0] + m_write_buf.size());
    }

  protected:
    int_type overflow(int_type ch) override
    {
        py::bytes bytes_to_write(pbase(), pptr() - pbase());

        std::size_t num_bytes_written = m_write_fn(bytes_to_write).cast<py::int_>();
        assert(num_bytes_written == pptr() - pbase());

        m_pos_in_file += num_bytes_written;
        if (!traits_type::eq_int_type(ch, traits_type::eof()))
        {
            char c = traits_type::to_char_type(ch);
            py::bytes b(&c, 1);
            m_write_fn(b);
            num_bytes_written++;
        }

        if (num_bytes_written > 0)
        {
            m_pos_in_file += num_bytes_written;
            setp(pbase(), epptr());
        }
        return traits_type::eq_int_type(ch, traits_type::eof()) ? traits_type::not_eof(ch) : ch;
    }

    int sync() override
    {
        overflow(traits_type::eof());
        return 0;
    }

    pos_type seekpos(pos_type off, std::ios_base::openmode which) override
    {
        return seekoff(off, std::ios_base::beg, which);
    }

    std::streamsize xsputn(const char *ptr, std::streamsize count) override
    {
        auto res = base_t::xsputn(ptr, count);
        return res;
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which) override
    {
        assert(which == std::ios_base::out);
        int whence;
        switch (way)
        {
        case std::ios_base::beg:
            whence = 0;
            break;
        case std::ios_base::cur:
            whence = 1;
            break;
        case std::ios_base::end:
            whence = 2;
            break;
        default:
            return traits_type::eof();
        }

        overflow(traits_type::eof());
        assert(pptr() - pbase() == 0);
        m_seek_fn(off, whence);
        off_type new_pos = m_tell_fn().cast<py::int_>();
        return new_pos;
    }

  private:
    py::function m_write_fn;
    py::function m_tell_fn;
    py::function m_seek_fn;

    std::array<char, 1024> m_write_buf;

    off_type m_pos_in_file;
};

#endif // LASZIP_BIND_PYTHON_OSTREAMBUF_H
