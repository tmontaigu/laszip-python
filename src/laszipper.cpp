#include "laszipper.h"

#include "laszip_error.h"

#include <limits>

LasZipper::LasZipper(py::object &file_obj, py::bytes &header_bytes)
    : m_is(), m_b(file_obj), m_output_stream(&m_b)
{
    // TODO avoid this copy
    std::string data(header_bytes);
    m_is.write(data.data(), data.size());

    if (laszip_create(&m_reader))
    {
        throw laszip_error("Failed to create the reader");
    }

    laszip_BOOL is_compresesd;
    if (laszip_open_reader_stream(m_reader, m_is, &is_compresesd))
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

    if (laszip_create(&m_writer))
    {
        throw laszip_error("Failed to create the writer");
    }

    if (laszip_set_header(m_writer, m_header))
    {
        throw laszip_error::last_error(m_writer);
    }

    if (laszip_open_writer_stream(m_writer, m_output_stream, 1, 0))
    {
        throw laszip_error::last_error(m_writer);
    }
}

size_t LasZipper::compress(py::buffer &buffer)
{
    if (m_header->point_data_record_length == 0)
    {
        return 0;
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

    size_t num_points_in_buffer = buf_info.size / m_header->point_data_record_length;
    size_t max_bytes_in_stream = std::numeric_limits<std::stringstream::int_type>::max();
    size_t max_points_before_filling_stream =
        max_bytes_in_stream / static_cast<size_t>(m_header->point_data_record_length);

    auto *in_ptr = static_cast<char *>(buf_info.ptr);
    while (num_points_in_buffer != 0)
    {
        m_is.seekp(0);
        m_is.seekg(0);

        py::ssize_t num_points_for_this_iter =
            std::min(num_points_in_buffer, max_points_before_filling_stream);
        size_t num_bytes_for_this_iter =
            num_points_for_this_iter * static_cast<size_t>(m_header->point_data_record_length);
        m_is.write(in_ptr, num_bytes_for_this_iter);

        for (size_t i{0}; i < num_points_for_this_iter; ++i)
        {
            if (laszip_read_point(m_reader))
            {
                throw laszip_error::last_error(m_reader);
            }

            if (laszip_set_point(m_writer, m_point))
            {
                throw laszip_error::last_error(m_writer);
            }

            if (laszip_update_inventory(m_writer))
            {
                throw laszip_error::last_error(m_writer);
            }

            if (laszip_write_point(m_writer))
            {
                throw laszip_error::last_error(m_writer);
            }
        }

        in_ptr += num_points_for_this_iter;
        num_points_in_buffer -= num_points_for_this_iter;
    }

    return num_points_in_buffer;
}

void LasZipper::done()
{
    if (!m_closed)
    {
        if (laszip_close_reader(m_reader))
        {
            throw laszip_error::last_error(m_reader);
        }

        if (laszip_close_writer(m_writer))
        {
            throw laszip_error::last_error(m_writer);
        }

        // FIXME I don't think we should have to call this
        //       maybe something is wrong in python_ostream ?
        m_b.pubsync();
    }
}

LasZipper::~LasZipper()
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
const laszip_header &LasZipper::header() const
{
    assert(m_header);
    return *m_header;
}
