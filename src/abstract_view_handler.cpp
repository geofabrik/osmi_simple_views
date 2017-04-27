/*
 * abstract_view_handler.cpp
 *
 *  Created on:  2017-04-27
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "abstract_view_handler.hpp"

AbstractViewHandler::AbstractViewHandler(std::string& output_filename, std::string& output_format,
        std::vector<std::string>& gdal_options, osmium::util::VerboseOutput& verbose_output,
        int epsg /*= 3857*/) :
#ifndef ONLYMERCATOROUTPUT
        m_factory(osmium::geom::Projection(epsg)),
#endif
        m_verbose_output(verbose_output),
        m_dataset(output_format, output_filename, gdalcpp::SRS(epsg))
        {
    m_dataset.enable_auto_transactions(10000);
    // default layer creation options
    if (output_format == "SQlite") {
        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "OFF");
        CPLSetConfigOption("OGR_SQLITE_PRAGMA", "journal_mode=OFF,TEMP_STORE=MEMORY,temp_store=memory,LOCKING_MODE=EXCLUSIVE");
        CPLSetConfigOption("OGR_SQLITE_CACHE", "600");
        CPLSetConfigOption("OGR_SQLITE_JOURNAL", "OFF");
        CPLSetConfigOption("SPATIAL_INDEX", "NO");
        CPLSetConfigOption("COMPRESS_GEOM", "NO");
        CPLSetConfigOption("SPATIALITE", "YES");
    } else if (output_format == "ESRI Shapefile") {
        CPLSetConfigOption("SHAPE_ENCODING", "UTF8");
    }
    // apply layer creation options
    for (std::string& option : gdal_options) {
        size_t equal_sign_pos = option.find("=");
        if (equal_sign_pos != std::string::npos) {
            std::string key = option.substr(0, equal_sign_pos);
            std::string value = option.substr(equal_sign_pos+1, option.size() - 1 - equal_sign_pos);
            CPLSetConfigOption(key.c_str(), value.c_str());
        }
    }
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
