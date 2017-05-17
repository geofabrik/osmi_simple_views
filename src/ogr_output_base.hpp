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

    /// maximum length of a string field
    static constexpr size_t MAX_FIELD_LENGTH = 254;

    void set_important_ogr_options(std::string& output_format, std::vector<std::string>& gdal_options);

public:
    OGROutputBase() = delete;

    OGROutputBase(osmium::util::VerboseOutput& verbose_output, int epsg = 3857);
};

#endif /* SRC_OGR_OUTPUT_BASE_HPP_ */
