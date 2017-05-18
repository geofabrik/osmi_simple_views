/*
 * ogr_output_base.cpp
 *
 *  Created on:  2017-05-17
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "ogr_output_base.hpp"

OGROutputBase::OGROutputBase(osmium::util::VerboseOutput& verbose_output, std::string& output_format,
        int epsg) :
#ifndef ONLYMERCATOROUTPUT
        m_factory(osmium::geom::Projection(epsg)),
#endif
        m_verbose_output(verbose_output),
        GDAL_DEFAULT_OPTIONS(get_gdal_default_options(output_format)) { }

std::vector<std::string> OGROutputBase::get_gdal_default_options(std::string& output_format) {
    std::vector<std::string> default_options;
    // default layer creation options
    if (output_format == "SQlite") {
        default_options.emplace_back("OGR_SQLITE_SYNCHRONOUS=OFF");
        default_options.emplace_back("SPATIAL_INDEX=NO");
        default_options.emplace_back("COMPRESS_GEOM=NO");
        default_options.emplace_back("SPATIALITE=YES");
////        CPLSetConfigOption("OGR_SQLITE_PRAGMA", "journal_mode=OFF,TEMP_STORE=MEMORY,temp_store=memory,LOCKING_MODE=EXCLUSIVE");
////        CPLSetConfigOption("OGR_SQLITE_CACHE", "600");
////        CPLSetConfigOption("OGR_SQLITE_JOURNAL", "OFF");
    } else if (output_format == "ESRI Shapefile") {
        default_options.emplace_back("SHAPE_ENCODING=UTF8");
    }

    return default_options;
}
