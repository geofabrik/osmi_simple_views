/*
 * ogr_output_base.cpp
 *
 *  Created on:  2017-05-17
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "ogr_output_base.hpp"

OGROutputBase::OGROutputBase(osmium::util::VerboseOutput& verbose_output, int epsg /*= 3857*/) :
#ifndef ONLYMERCATOROUTPUT
        m_factory(osmium::geom::Projection(epsg)),
#endif
        m_verbose_output(verbose_output) { }

void OGROutputBase::set_important_ogr_options(std::string& output_format, std::vector<std::string>& gdal_options) {
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
