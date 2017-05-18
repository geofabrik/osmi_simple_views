/*
 * ogr_output_base.hpp
 *
 *  Created on:  2017-05-17
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_OGR_OUTPUT_BASE_HPP_
#define SRC_OGR_OUTPUT_BASE_HPP_

#include <gdalcpp.hpp>

#include <osmium/geom/ogr.hpp>

#ifdef ONLYMERCATOROUTPUT
    #include <osmium/geom/mercator_projection.hpp>
#else
    #include <osmium/geom/projection.hpp>
#endif

#include <osmium/util/verbose_output.hpp>

/**
 * Provide commont things for working with GDAL. This class does not care for the dataset
 * because the dataset is shared.
 */
class OGROutputBase {
protected:
    /**
     * If ONLYMERCATOROUTPUT is defined, output coordinates are always Web
     * Mercator coordinates. If it is not defined, we will transform them if
     * the output SRS is different from the input SRS (4326).
     */
#ifdef ONLYMERCATOROUTPUT
    /// factory to build OGR geometries in Web Mercator projection
    osmium::geom::OGRFactory<osmium::geom::MercatorProjection> m_factory;
#else
    /// factory to build OGR geometries with a coordinate transformation if necessary
    osmium::geom::OGRFactory<osmium::geom::Projection> m_factory;
#endif

    /// reference to output manager for STDERR
    osmium::util::VerboseOutput& m_verbose_output;

    const std::vector<std::string> GDAL_DEFAULT_OPTIONS;

    /// maximum length of a string field
    static constexpr size_t MAX_FIELD_LENGTH = 254;

    /**
     * \brief Add default options for the to the back of a vector of options.
     *
     * Default options are added at the *back* of the vector. If you read the vector in a later step
     * to set them via the functions provided by the GDAL library, do it in reverse order. Otherwise
     * the defaults will overwrite your explicitly set options.
     *
     * \param output_format output format
     * \param gdal_options vector where to add the default options.
     */
    static std::vector<std::string> get_gdal_default_options(std::string& output_format);

public:
    OGROutputBase() = delete;

    OGROutputBase(osmium::util::VerboseOutput& verbose_output, std::string& output_format, int epsg);
};

#endif /* SRC_OGR_OUTPUT_BASE_HPP_ */
