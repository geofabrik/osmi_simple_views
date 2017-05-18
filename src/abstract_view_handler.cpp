/*
 * abstract_view_handler.cpp
 *
 *  Created on:  2017-04-27
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "abstract_view_handler.hpp"

AbstractViewHandler::AbstractViewHandler(std::string& output_filename, std::string& output_format,
        osmium::util::VerboseOutput& verbose_output, int epsg) :
        OGROutputBase(verbose_output, output_format, epsg),
        m_dataset(output_format, output_filename, gdalcpp::SRS(epsg), get_gdal_default_options(output_format)) {
    m_dataset.enable_auto_transactions(10000);
}

AbstractViewHandler::~AbstractViewHandler() {}

bool AbstractViewHandler::all_nodes_valid(const osmium::WayNodeList& wnl) {
    for (const osmium::NodeRef& nd_ref : wnl) {
        if (!nd_ref.location().valid()) {
            m_verbose_output << "Invalid location for node " << nd_ref.ref() << "\n";
            return false;
        }
    }
    return true;
}

gdalcpp::Dataset* AbstractViewHandler::get_dataset_pointer() {
    return &m_dataset;
}
