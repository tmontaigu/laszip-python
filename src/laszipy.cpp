#include "lasunzipper.h"
#include "laszip_error.h"
#include "laszipper.h"
#include "python_istreambuf.h"
#include "python_ostreambuf.h"

#include <pybind11/pybind11.h>

#include <laszip/laszip_api.h>

#include <utility>

#include <iostream>

namespace py = pybind11;


PYBIND11_MODULE(laszipy, m)
{

    py::register_exception<laszip_error>(m, "LaszipError");

    py::class_<LasUnZipper>(m, "LasUnZipper")
        .def(py::init<py::object &>())
        .def_property_readonly("header", &LasUnZipper::header)
        .def("decompress_into", &LasUnZipper::decompress_into)
        .def("close", &LasUnZipper::close);

    py::class_<LasZipper>(m, "LasZipper")
        .def(py::init<py::object &, py::bytes &>())
        .def_property_readonly("header", &LasZipper::header)
        .def("compress", &LasZipper::compress)
        .def("done", &LasZipper::done);

    py::class_<laszip_header>(m, "LasZipHeader")
        .def_readwrite("file_source_ID", &laszip_header::file_source_ID)
        .def_readwrite("global_encoding", &laszip_header::global_encoding)
        .def_readwrite("project_ID_GUID_data_1", &laszip_header::project_ID_GUID_data_1)
        .def_readwrite("project_ID_GUID_data_2", &laszip_header::project_ID_GUID_data_2)
        .def_readwrite("project_ID_GUID_data_3", &laszip_header::project_ID_GUID_data_3)
        //.def_readwrite("project_ID_GUID_data_4", &laszip_header::project_ID_GUID_data_4)
        .def_readwrite("version_major", &laszip_header::version_major)
        .def_readwrite("version_minor", &laszip_header::version_minor)
        // systemident, gensoft
        .def_readwrite("file_creation_day", &laszip_header::file_creation_day)
        .def_readwrite("file_creation_year", &laszip_header::file_creation_year)
        .def_readwrite("header_size", &laszip_header::header_size)
        .def_readwrite("offset_to_point_data", &laszip_header::offset_to_point_data)
        .def_readwrite("number_of_variable_length_records", &laszip_header::number_of_variable_length_records)
        .def_readwrite("point_data_format", &laszip_header::point_data_format)
        .def_readwrite("point_data_record_length", &laszip_header::point_data_record_length)
        .def_readwrite("number_of_point_records", &laszip_header::number_of_point_records)
        // pts by return
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
        .def_readwrite("extended_number_of_point_records", &laszip_header::extended_number_of_point_records);
    // extended points by ret

    // vlrs
    // userdata in & afterheader
}
