#ifndef LASZIP_BIND_PYTHON_ISTREAMBUF_H
#define LASZIP_BIND_PYTHON_ISTREAMBUF_H

#include <iosfwd>
#include <pybind11/pybind11.h>
#include <streambuf>

namespace py = pybind11;

#define DEFAULT_BUFFER_SIZE 8192

class python_istreambuf : public std::basic_streambuf<char>
{
  private:
    typedef std::basic_streambuf<char> base_t;

  public:
    typedef base_t::char_type char_type;
    typedef base_t::int_type int_type;
    typedef base_t::pos_type pos_type;
    typedef base_t::off_type off_type;
    typedef base_t::traits_type traits_type;

    explicit python_istreambuf(const py::object &file_obj)
        : m_read_fn(file_obj.attr("read").cast<pybind11::function>()),
          m_tell_fn(file_obj.attr("tell").cast<pybind11::function>()),
          m_seek_fn(file_obj.attr("seek").cast<pybind11::function>()), m_read_buf(),
          m_pos_in_file(m_tell_fn().cast<pybind11::int_>())
    {
    }

  protected:
    int_type underflow() override
    {
        py::bytes data_read = m_read_fn(DEFAULT_BUFFER_SIZE).cast<py::bytes>();

        // TODO this copies data (the conversiion to std::string)
        //      we can probably avoid this by using the buffer directly
        m_read_buf = std::move(std::string(data_read));

        m_pos_in_file += m_read_buf.size();
        setg(&m_read_buf[0], &m_read_buf[0], &m_read_buf[0] + m_read_buf.size());

        if (m_read_buf.empty())
        {
            return traits_type::eof();
        }
        return traits_type::to_int_type(m_read_buf[0]);
    }

    std::streamsize showmanyc() override
    {
        int_type status = underflow();
        if (status == traits_type::eof())
        {
            return -1;
        }
        return egptr() - gptr();
    }

    int sync() override
    {
        int result = 0;
        if (gptr() && gptr() < egptr())
        {
            m_seek_fn(gptr() - egptr(), 1);
        }
        return result;
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which) override
    {
        assert(which == std::ios_base::in);

        if (!gptr())
        {
            if (traits_type::eq_int_type(underflow(), traits_type::eof()))
            {
                return traits_type::eof();
            }
        }

        int whence;
        switch (way)
        {
        case std::ios_base::beg:
            whence = 0;
            break;
        case std::ios_base::cur:
            // Because the true pos in the python file obj
            // is: the pos you think it is + buffer_size
            // So we need to move the off to reflect that
            off -= egptr() - gptr();
            whence = 1;
            break;
        case std::ios_base::end:
            whence = 2;
            break;
        default:
            return traits_type::eof();
        }

        m_seek_fn(off, whence);
        off_type new_pos = m_tell_fn().cast<py::int_>();
        underflow();
        return new_pos;
    }

    pos_type seekpos(pos_type sp, std::ios_base::openmode which) override
    {
        return seekoff(sp, std::ios_base::beg, which);
    }

  private:
    py::function m_read_fn;
    py::function m_tell_fn;
    py::function m_seek_fn;

    std::string m_read_buf;

    off_type m_pos_in_file;
};

#endif // LASZIP_BIND_PYTHON_ISTREAMBUF_H
