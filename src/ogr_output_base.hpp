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


#ifndef SRC_OGR_OUTPUT_BASE_HPP_
#define SRC_OGR_OUTPUT_BASE_HPP_

#include <gdalcpp.hpp>

#include <osmium/geom/ogr.hpp>

#ifdef ONLYMERCATOROUTPUT
    #include <osmium/geom/mercator_projection.hpp>
//#else
//    #include <osmium/geom/projection.hpp>
#endif

#include <osmium/util/verbose_output.hpp>

#include "options.hpp"

/**
 * If ONLYMERCATOROUTPUT is defined, output coordinates are always Web
 * Mercator coordinates. If it is not defined, we will transform them if
 * the output SRS is different from the input SRS (4326).
 */
#ifdef ONLYMERCATOROUTPUT
    using ogr_factory_type = osmium::geom::OGRFactory<osmium::geom::MercatorProjection>;
#else
    using ogr_factory_type = osmium::geom::OGRFactory<>;
#endif

/**
 * Provide commont things for working with GDAL. This class does not care for the dataset
 * because the dataset is shared.
 */
class OGROutputBase {
protected:
    ogr_factory_type m_factory;

    Options& m_options;

    /// maximum length of a string field
    static constexpr size_t MAX_FIELD_LENGTH = 254;

    /**
     * \brief Add default options for the to the back of a vector of options.
     *
     * Default dataset creation options are added at the *back* of the vector. If you read the vector in a later step
     * to set them via the functions provided by the GDAL library, do it in reverse order. Otherwise
     * the defaults will overwrite your explicitly set options.
     *
     * \param output_format output format
     * \param gdal_options vector where to add the default options.
     */
    std::vector<std::string> get_gdal_default_dataset_options();

    /**
     * \brief Add default options for the to the back of a vector of options.
     *
     * Default layer creation options are added at the *back* of the vector. If you read the vector in a later step
     * to set them via the functions provided by the GDAL library, do it in reverse order. Otherwise
     * the defaults will overwrite your explicitly set options.
     *
     * \param output_format output format
     * \param gdal_options vector where to add the default options.
     */
    std::vector<std::string> get_gdal_default_layer_options();

public:
    OGROutputBase() = delete;

    OGROutputBase(Options& options);
};

#endif /* SRC_OGR_OUTPUT_BASE_HPP_ */
