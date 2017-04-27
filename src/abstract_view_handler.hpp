/*
 * view_handler.hpp
 *
 *  Created on:  2017-04-27
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_ABSTRACT_VIEW_HANDLER_HPP_
#define SRC_ABSTRACT_VIEW_HANDLER_HPP_

#include <gdalcpp.hpp>

#include <osmium/handler.hpp>
#include <osmium/geom/ogr.hpp>

#ifdef ONLYMERCATOROUTPUT
    #include <osmium/geom/mercator_projection.hpp>
#else
    #include <osmium/geom/projection.hpp>
#endif

#include <osmium/osm/way.hpp>
#include <osmium/util/verbose_output.hpp>

class AbstractViewHandler : public osmium::handler::Handler {
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
    /// ORG dataset
    gdalcpp::Dataset m_dataset;

    /// maximum length of a string field
    static constexpr size_t MAX_FIELD_LENGTH = 254;

    /**
     * Check if all nodes of the way are valid.
     */
    bool all_nodes_valid(const osmium::WayNodeList& wnl);

public:
    AbstractViewHandler() = delete;

    AbstractViewHandler(std::string& output_filename, std::string& output_format, std::vector<std::string>& gdal_options,
            osmium::util::VerboseOutput& verbose_output, int epsg = 3857);

    virtual ~AbstractViewHandler();

    virtual void way(const osmium::Way& way) = 0;
};



#endif /* SRC_ABSTRACT_VIEW_HANDLER_HPP_ */
