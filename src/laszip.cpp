#include "lasunzipper.h"
#include "laszip_error.h"
#include "laszipper.h"
#include "python_istreambuf.h"
#include "python_ostreambuf.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <laszip_api.h>

#include <utility>

#include <iostream>

namespace py = pybind11;
using namespace pybind11::literals;

void no_op_delete(void *) {}

template <typename T> py::array_t<T> wrap_as_py_array(T *array, size_t len)
{
    auto capsule = py::capsule(array, no_op_delete);
    return py::array(len, array, capsule);
}

class LasZipDll
{
  public:
    LasZipDll()
    {
        if (laszip_create(&m_laszipPtr))
        {
            throw laszip_error("Failed to create laszip pointer");
        }
    }

    bool open_reader(const char *path)
    {
        laszip_BOOL is_compressed;
        if (laszip_open_reader(m_laszipPtr, path, &is_compressed))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        m_readerIsOpen = true;
        return is_compressed == 1;
    }

    bool open_reader(py::object &file_object)
    {
        m_inputStreamBuf = std::make_unique<python_istreambuf>(file_object);
        m_inputStream = std::make_unique<std::istream>(m_inputStreamBuf.get());
        laszip_BOOL is_compressed;
        if (laszip_open_reader_stream(m_laszipPtr, *m_inputStream, &is_compressed))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        m_readerIsOpen = true;
        return is_compressed;
    }

    void open_writer(const char *path, bool compress)
    {
        auto do_compress = static_cast<laszip_BOOL>(compress);
        if (laszip_open_writer(m_laszipPtr, path, do_compress))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        m_writerIsOpen = true;
    }

    void open_writer(py::object &file_object, bool compress, bool do_not_write_header = false)
    {
        auto do_compress = static_cast<laszip_BOOL>(compress);
        m_outputStreamBuf = std::make_unique<python_ostreambuf>(file_object);
        m_outputStream = std::make_unique<std::ostream>(m_outputStreamBuf.get());
        if (laszip_open_writer_stream(
                m_laszipPtr, *m_outputStream, do_compress, static_cast<laszip_BOOL>(do_not_write_header)))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        m_writerIsOpen = true;
    }

    laszip_header *header()
    {
        laszip_header *header{nullptr};
        if (laszip_get_header_pointer(m_laszipPtr, &header))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        return header;
    }

    laszip_point *point()
    {
        laszip_point *point{nullptr};
        if (laszip_get_point_pointer(m_laszipPtr, &point))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        return point;
    }

    void read_point()
    {
        if (laszip_read_point(m_laszipPtr))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
    }

    void set_header(const laszip_header &header)
    {
        if (laszip_set_header(m_laszipPtr, &header))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
    }

    void set_point_type_and_size(laszip_U8 point_type, laszip_U16 point_size)
    {
        if (laszip_set_point_type_and_size(m_laszipPtr, point_size, point_size))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
    }

    void set_point(const laszip_point &point)
    {
        if (laszip_set_point(m_laszipPtr, &point))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
    }

    void write_point()
    {
        if (laszip_write_point(m_laszipPtr))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
    }

    void update_inventory()
    {
        if (laszip_update_inventory(m_laszipPtr))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
    }

    const char *get_warning()
    {
        char *warning;
        if (laszip_get_warning(m_laszipPtr, &warning))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        return warning;
    }

    void close_reader()
    {
        if (!m_readerIsOpen)
        {
            return;
        }
        if (laszip_close_reader(m_laszipPtr))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        m_readerIsOpen = false;
    }

    void close_writer()
    {
        if (!m_writerIsOpen)
        {
            return;
        }

        if (laszip_close_writer(m_laszipPtr))
        {
            throw laszip_error::last_error(m_laszipPtr);
        }
        m_writerIsOpen = false;
    }

    ~LasZipDll()
    {
        if (m_readerIsOpen)
        {
            laszip_close_reader(m_laszipPtr);
        }

        if (m_writerIsOpen)
        {
            laszip_close_writer(m_laszipPtr);
        }

        if (m_outputStreamBuf)
        {
            m_outputStreamBuf->pubsync();
        }

        laszip_destroy(m_laszipPtr);
    }

  private:
    laszip_POINTER m_laszipPtr{nullptr};

    bool m_readerIsOpen{false};
    std::unique_ptr<python_istreambuf> m_inputStreamBuf{nullptr};
    std::unique_ptr<std::istream> m_inputStream{nullptr};

    bool m_writerIsOpen{false};
    std::unique_ptr<python_ostreambuf> m_outputStreamBuf{nullptr};
    std::unique_ptr<std::ostream> m_outputStream{nullptr};
};

PYBIND11_MODULE(laszip, m)
{
    py::register_exception<laszip_error>(m, "LaszipError");

    m.attr("DECOMPRESS_SELECTIVE_ALL") = laszip_DECOMPRESS_SELECTIVE_ALL;

    m.attr("DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY") = laszip_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY;
    m.attr("DECOMPRESS_SELECTIVE_Z") = laszip_DECOMPRESS_SELECTIVE_Z;
    m.attr("DECOMPRESS_SELECTIVE_CLASSIFICATION") = laszip_DECOMPRESS_SELECTIVE_CLASSIFICATION;
    m.attr("DECOMPRESS_SELECTIVE_FLAGS") = laszip_DECOMPRESS_SELECTIVE_FLAGS;
    m.attr("DECOMPRESS_SELECTIVE_INTENSITY") = laszip_DECOMPRESS_SELECTIVE_INTENSITY;
    m.attr("DECOMPRESS_SELECTIVE_SCAN_ANGLE") = laszip_DECOMPRESS_SELECTIVE_SCAN_ANGLE;
    m.attr("DECOMPRESS_SELECTIVE_USER_DATA") = laszip_DECOMPRESS_SELECTIVE_USER_DATA;
    m.attr("DECOMPRESS_SELECTIVE_POINT_SOURCE") = laszip_DECOMPRESS_SELECTIVE_POINT_SOURCE;
    m.attr("DECOMPRESS_SELECTIVE_GPS_TIME") = laszip_DECOMPRESS_SELECTIVE_GPS_TIME;
    m.attr("DECOMPRESS_SELECTIVE_RGB") = laszip_DECOMPRESS_SELECTIVE_RGB;
    m.attr("DECOMPRESS_SELECTIVE_NIR") = laszip_DECOMPRESS_SELECTIVE_NIR;
    m.attr("DECOMPRESS_SELECTIVE_WAVEPACKET") = laszip_DECOMPRESS_SELECTIVE_WAVEPACKET;
    m.attr("DECOMPRESS_SELECTIVE_BYTE0") = laszip_DECOMPRESS_SELECTIVE_BYTE0;
    m.attr("DECOMPRESS_SELECTIVE_BYTE1") = laszip_DECOMPRESS_SELECTIVE_BYTE1;
    m.attr("DECOMPRESS_SELECTIVE_BYTE2") = laszip_DECOMPRESS_SELECTIVE_BYTE2;
    m.attr("DECOMPRESS_SELECTIVE_BYTE3") = laszip_DECOMPRESS_SELECTIVE_BYTE3;
    m.attr("DECOMPRESS_SELECTIVE_BYTE4") = laszip_DECOMPRESS_SELECTIVE_BYTE4;
    m.attr("DECOMPRESS_SELECTIVE_BYTE5") = laszip_DECOMPRESS_SELECTIVE_BYTE5;
    m.attr("DECOMPRESS_SELECTIVE_BYTE6") = laszip_DECOMPRESS_SELECTIVE_BYTE6;
    m.attr("DECOMPRESS_SELECTIVE_BYTE7") = laszip_DECOMPRESS_SELECTIVE_BYTE7;
    m.attr("DECOMPRESS_SELECTIVE_EXTRA_BYTES") = laszip_DECOMPRESS_SELECTIVE_EXTRA_BYTES;
    m.attr("DECOMPRESS_SELECTIVE_ALL") = laszip_DECOMPRESS_SELECTIVE_ALL;

    py::class_<LasUnZipper>(m, "LasUnZipper")
        .def(py::init<py::object &>(), "file_object"_a)
        .def(py::init<py::object &, laszip_U32>(), "file_object"_a, "decompression_selection"_a)
        .def_property_readonly("header", &LasUnZipper::header)
        .def("decompress_into", &LasUnZipper::decompress_into, "buffer"_a)
        .def("seek", &LasUnZipper::seek, "index"_a)
        .def("close", &LasUnZipper::close);

    py::class_<LasZipper>(m, "LasZipper")
        .def(py::init<py::object &, py::bytes &>())
        .def_property_readonly("header", &LasZipper::header)
        .def("compress", &LasZipper::compress)
        .def("done", &LasZipper::done);

    py::class_<LasZipDll>(m, "LasZipDll")
        .def(py::init<>())
        .def("open_reader", (bool (LasZipDll::*)(const char *))(&LasZipDll::open_reader))
        .def("open_reader", (bool (LasZipDll::*)(py::object &))(&LasZipDll::open_reader))
        .def("header", &LasZipDll::header, py::return_value_policy::reference)
        .def("point", &LasZipDll::point, py::return_value_policy::reference)
        .def("read_point", &LasZipDll::read_point)
        .def("close_reader", &LasZipDll::close_reader)
        .def("close_writer", &LasZipDll::close_writer)
        .def("set_header", &LasZipDll::set_header)
        .def("get_warning", &LasZipDll::get_warning)
        .def("set_point_type_and_size", &LasZipDll::set_point_type_and_size)
        .def("set_point", &LasZipDll::set_point)
        .def("write_point", &LasZipDll::write_point)
        .def("update_inventory", &LasZipDll::update_inventory);

    py::class_<laszip_header>(m, "LasZipHeader")
        .def(py::init<>())
        .def_readwrite("file_source_ID", &laszip_header::file_source_ID)
        .def_readwrite("global_encoding", &laszip_header::global_encoding)
        .def_readwrite("project_ID_GUID_data_1", &laszip_header::project_ID_GUID_data_1)
        .def_readwrite("project_ID_GUID_data_2", &laszip_header::project_ID_GUID_data_2)
        .def_readwrite("project_ID_GUID_data_3", &laszip_header::project_ID_GUID_data_3)
        .def_property(
            "project_ID_GUID_data_4",
            [](const laszip_header &header) { return py::str(header.project_ID_GUID_data_4, 8); },
            [](laszip_header &header, const std::string &new_value) {
                if (new_value.size() != 8)
                {
                    throw std::invalid_argument("project_ID_GUID_data_4 must be 8 bytes long");
                }
                std::copy(new_value.begin(), new_value.end(), header.project_ID_GUID_data_4);
            })
        .def_readwrite("version_major", &laszip_header::version_major)
        .def_readwrite("version_minor", &laszip_header::version_minor)
        .def_property(
            "system_identifier",
            [](const laszip_header &header) { return py::str(header.system_identifier, 32); },
            [](laszip_header &header, const std::string &new_value) {
                if (new_value.size() > 32)
                {
                    throw std::invalid_argument("system_identifier cannot exceed 32 bytes");
                }
                std::fill(header.system_identifier, header.system_identifier + 32, '\0');
                std::copy(new_value.begin(), new_value.end(), header.system_identifier);
            })
        .def_property(
            "generating_software",
            [](const laszip_header &header) { return py::str(header.generating_software, 32); },
            [](laszip_header &header, const std::string &new_value) {
                if (new_value.size() > 32)
                {
                    throw std::invalid_argument("generating_software cannot exceed 32 bytes");
                }
                std::fill(header.generating_software, header.generating_software + 32, '\0');
                std::copy(new_value.begin(), new_value.end(), header.generating_software);
            })
        .def_readwrite("file_creation_day", &laszip_header::file_creation_day)
        .def_readwrite("file_creation_year", &laszip_header::file_creation_year)
        .def_readwrite("header_size", &laszip_header::header_size)
        .def_readwrite("offset_to_point_data", &laszip_header::offset_to_point_data)
        .def_readwrite("number_of_variable_length_records", &laszip_header::number_of_variable_length_records)
        .def_readwrite("point_data_format", &laszip_header::point_data_format)
        .def_readwrite("point_data_record_length", &laszip_header::point_data_record_length)
        .def_readwrite("number_of_point_records", &laszip_header::number_of_point_records)
        .def_property_readonly("number_of_points_by_return",
                               [](const laszip_header &header) {
                                   return wrap_as_py_array(header.number_of_points_by_return, 5);
                               })
        .def_readwrite("x_scale_factor", &laszip_header::x_scale_factor)
        .def_readwrite("y_scale_factor", &laszip_header::y_scale_factor)
        .def_readwrite("z_scale_factor", &laszip_header::z_scale_factor)
        .def_readwrite("x_offset", &laszip_header::x_offset)
        .def_readwrite("y_offset", &laszip_header::y_offset)
        .def_readwrite("z_offset", &laszip_header::z_offset)
        .def_readwrite("max_x", &laszip_header::max_x)
        .def_readwrite("min_x", &laszip_header::min_x)
        .def_readwrite("max_y", &laszip_header::max_y)
        .def_readwrite("min_y", &laszip_header::min_y)
        .def_readwrite("max_z", &laszip_header::max_z)
        .def_readwrite("min_z", &laszip_header::min_z)
        // 1.3 things
        .def_readwrite("start_of_waveform_data_packet_record",
                       &laszip_header::start_of_waveform_data_packet_record)
        // 1.4
        .def_readwrite("start_of_first_extended_variable_length_record",
                       &laszip_header::start_of_first_extended_variable_length_record)
        .def_readwrite("number_of_extended_variable_length_records",
                       &laszip_header::number_of_extended_variable_length_records)
        .def_readwrite("extended_number_of_point_records", &laszip_header::extended_number_of_point_records)
        .def_property_readonly("extended_number_of_points_by_return", [](const laszip_header &header) {
            return wrap_as_py_array(header.extended_number_of_points_by_return, 15);
        });

    py::class_<laszip_point>(m, "LasZipPoint")
        .def(py::init<>())
        .def_readwrite("X", &laszip_point::X)
        .def_readwrite("Y", &laszip_point::Y)
        .def_readwrite("Z", &laszip_point::Z)
        .def_readwrite("intensity", &laszip_point::intensity)
        .def_property(
            "return_number",
            [](const laszip_point &point) { return point.return_number; },
            [](laszip_point &point, int value) { point.return_number = value; })
        .def_property(
            "number_of_returns",
            [](const laszip_point &point) { return point.number_of_returns; },
            [](laszip_point &point, int value) { point.number_of_returns = value; })
        .def_property(
            "scan_direction_flag",
            [](const laszip_point &point) { return point.scan_direction_flag; },
            [](laszip_point &point, int value) { point.scan_direction_flag = value; })
        .def_property(
            "edge_of_flight_line",
            [](const laszip_point &point) { return point.edge_of_flight_line; },
            [](laszip_point &point, int value) { point.edge_of_flight_line = value; })
        .def_property(
            "classification",
            [](const laszip_point &point) { return point.classification; },
            [](laszip_point &point, int value) { point.classification = value; })
        .def_property(
            "synthetic_flag",
            [](const laszip_point &point) { return point.synthetic_flag; },
            [](laszip_point &point, int value) { point.synthetic_flag = value; })
        .def_property(
            "keypoint_flag",
            [](const laszip_point &point) { return point.keypoint_flag; },
            [](laszip_point &point, int value) { point.keypoint_flag = value; })
        .def_property(
            "withheld_flag",
            [](const laszip_point &point) { return point.withheld_flag; },
            [](laszip_point &point, int value) { point.withheld_flag = value; })
        .def_readwrite("scan_angle_rank", &laszip_point::scan_angle_rank)
        .def_readwrite("user_data", &laszip_point::user_data)
        .def_readwrite("point_source_ID", &laszip_point::point_source_ID)
        .def_readwrite("extended_scan_angle", &laszip_point::extended_scan_angle)
        .def_property(
            "extended_point_type",
            [](const laszip_point &point) { return point.extended_point_type; },
            [](laszip_point &point, int value) { point.extended_point_type = value; })
        .def_property(
            "extended_scanner_channel",
            [](const laszip_point &point) { return point.extended_scanner_channel; },
            [](laszip_point &point, int value) { point.extended_scanner_channel = value; })
        .def_property(
            "extended_classification_flags",
            [](const laszip_point &point) { return point.extended_classification_flags; },
            [](laszip_point &point, int value) { point.extended_classification_flags = value; })
        .def_property(
            "extended_classification",
            [](const laszip_point &point) { return point.extended_classification; },
            [](laszip_point &point, int value) { point.extended_classification = value; })
        .def_property(
            "extended_return_number",
            [](const laszip_point &point) { return point.extended_return_number; },
            [](laszip_point &point, int value) { point.extended_return_number = value; })
        .def_property(
            "extended_number_of_returns",
            [](const laszip_point &point) { return point.extended_number_of_returns; },
            [](laszip_point &point, int value) { point.extended_number_of_returns = value; })
        .def_readwrite("gps_time", &laszip_point::gps_time)
        .def_property_readonly("rgb",
                               [](const laszip_point &point) { return wrap_as_py_array(point.rgb, 4); })
        .def_property_readonly(
            "wave_packet", [](const laszip_point &point) { return wrap_as_py_array(point.wave_packet, 4); })
        .def_property_readonly("extra_bytes", [](const laszip_point &point) {
            return wrap_as_py_array(point.extra_bytes, point.num_extra_bytes);
        });

    m.def("get_version", []() {
        laszip_U8 major{0}, minor{0};
        laszip_U16 revision{0};
        laszip_U32 build{0};

        laszip_get_version(&major, &minor, &revision, &build);

        return py::make_tuple(major, minor, revision, build);
    });
}
