#include "lasunzipper.h"
#include "laszip_error.h"
#include <laszip_api.h>

#include <algorithm>
#include <limits>

LasUnZipper::LasUnZipper(py::object &file_obj) : LasUnZipper(file_obj, laszip_DECOMPRESS_SELECTIVE_ALL) {}

LasUnZipper::LasUnZipper(py::object &file_obj, laszip_U32 decompression_selection)
    : m_ibuf(file_obj), m_input_stream(&m_ibuf)
{
    laszip_BOOL is_compressed;
    if (laszip_create(&m_reader))
    {
        throw laszip_error("Failed to create the reader");
    }

    if (laszip_decompress_selective(m_reader, decompression_selection))
    {
        throw laszip_error::last_error(m_reader);
    };

    if (laszip_open_reader_stream(m_reader, m_input_stream, &is_compressed))
    {
        throw laszip_error::last_error(m_reader);
    }

    if (laszip_get_header_pointer(m_reader, &m_header))
    {
        throw laszip_error::last_error(m_reader);
    }

    if (laszip_get_point_pointer(m_reader, &m_point))
    {
        throw laszip_error::last_error(m_reader);
    }

    laszip_BOOL dont_compress = 0;
    laszip_BOOL dont_write_header = 1;
    if (laszip_create(&m_writer))
    {
        throw laszip_error("Failed to create the reader");
    }

    if (laszip_set_header(m_writer, m_header))
    {
        throw laszip_error::last_error(m_writer);
    }

    if (laszip_open_writer_stream(m_writer, m_output_stream, dont_compress, dont_write_header))
    {
        throw laszip_error::last_error(m_writer);
    }
}

void LasUnZipper::decompress_into(py::buffer &buffer)
{

    if (m_header->point_data_record_length == 0)
    {
        return;
    }

    py::buffer_info buf_info = buffer.request();

    if (buf_info.itemsize != sizeof(char))
    {
        throw std::invalid_argument("Buffer must be byte buffer");
    }

    if (buf_info.ndim != 1)
    {
        throw std::invalid_argument("Buffer must be one dimensional");
    }

    size_t num_points_to_read = buf_info.size / static_cast<py::ssize_t>(m_header->point_data_record_length);

    // How many bytes the `m_output_stream` can store
    size_t max_bytes_in_output_stream = std::numeric_limits<std::stringstream::int_type>::max();
    size_t max_points_before_filling_stream =
        max_bytes_in_output_stream / static_cast<size_t>(m_header->point_data_record_length);

    auto *out_ptr = static_cast<char *>(buf_info.ptr);
    while (num_points_to_read != 0)
    {
        py::ssize_t num_points_for_this_iter = std::min(num_points_to_read, max_points_before_filling_stream);
        for (py::ssize_t i = 0; i < num_points_for_this_iter; ++i)
        {
            if (laszip_read_point(m_reader))
            {
                throw laszip_error::last_error(m_reader);
            }

            if (laszip_set_point(m_writer, m_point))
            {
                throw laszip_error::last_error(m_writer);
            }

            if (laszip_write_point(m_writer))
            {
                throw laszip_error::last_error(m_writer);
            }
        }
        num_points_to_read -= num_points_for_this_iter;
        size_t bytes_read =
            num_points_for_this_iter * static_cast<size_t>(m_header->point_data_record_length);

        m_output_stream.read(out_ptr, bytes_read);
        m_output_stream.seekg(0, std::ios_base::beg);
        m_output_stream.seekp(0, std::ios_base::beg);
        out_ptr += bytes_read;
    }
}

void LasUnZipper::close()
{
    if (!m_closed)
    {
        m_closed = true;
        if (laszip_close_reader(m_reader))
        {
            throw laszip_error::last_error(m_reader);
        }

        if (laszip_close_writer(m_writer))
        {
            throw laszip_error::last_error(m_writer);
        }
    }
}

LasUnZipper::~LasUnZipper()
{
    if (!m_closed)
    {
        // Destructors should not throw
        laszip_close_reader(m_reader);
        laszip_close_reader(m_writer);
    }
    laszip_destroy(m_reader);
    laszip_destroy(m_writer);
}

const laszip_header &LasUnZipper::header() const
{
    assert(m_header);
    return *m_header;
}

void LasUnZipper::seek(laszip_I64 index)
{
    if (laszip_seek_point(m_reader, index))
    {
        throw laszip_error::last_error(m_reader);
    }
}
