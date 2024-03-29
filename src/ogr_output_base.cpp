/*
 *  © 2017 Geofabrik GmbH
 *
 *  This file is part of osmi_simple_views.
 *
 *  osmi_simple_views is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  osmi_simple_views is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with osmi_simple_views. If not, see <http://www.gnu.org/licenses/>.
 */


#include "ogr_output_base.hpp"

OGROutputBase::OGROutputBase(Options& options) :
        m_options(options) { }

std::vector<std::string> OGROutputBase::get_gdal_default_dataset_options() {
    std::vector<std::string> default_options;
    // default layer creation options
    if (m_options.output_format == "SQlite") {
        CPLSetConfigOption("OGR_SQLITE_PRAGMA", "journal_mode=OFF,TEMP_STORE=MEMORY,temp_store=memory,LOCKING_MODE=EXCLUSIVE");
        CPLSetConfigOption("OGR_SQLITE_CACHE", "600");
        CPLSetConfigOption("OGR_SQLITE_JOURNAL", "OFF");
        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "OFF");
        default_options.emplace_back("SPATIALITE=YES");
    } else if (m_options.output_format == "ESRI Shapefile") {
        default_options.emplace_back("SHAPE_ENCODING=UTF8");
    }

    return default_options;
}

std::vector<std::string> OGROutputBase::get_gdal_default_layer_options() {
    std::vector<std::string> default_options;
    // default layer creation options
    if (m_options.output_format == "SQlite") {
        default_options.emplace_back("SPATIAL_INDEX=NO");
        default_options.emplace_back("COMPRESS_GEOM=NO");
    } else if (m_options.output_format == "ESRI Shapefile") {
        default_options.emplace_back("SHAPE_ENCODING=UTF8");
    }

    return default_options;
}
