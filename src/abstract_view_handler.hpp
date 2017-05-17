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
#include <osmium/osm/way.hpp>
#include "ogr_output_base.hpp"

class AbstractViewHandler : public osmium::handler::Handler, public OGROutputBase {
protected:
    /// ORG dataset
    gdalcpp::Dataset m_dataset;

    /**
     * Check if all nodes of the way are valid.
     */
    bool all_nodes_valid(const osmium::WayNodeList& wnl);

public:
    AbstractViewHandler() = delete;

    AbstractViewHandler(std::string& output_filename, std::string& output_format, std::vector<std::string>& gdal_options,
            osmium::util::VerboseOutput& verbose_output, int epsg = 3857);

    virtual ~AbstractViewHandler();

    void node(const osmium::Node&) {}

    void way(const osmium::Way&) {}

    /**
     * Get a pointer to the dataset being used. The ownership will stay at AbstractViewHandler.
     */
    gdalcpp::Dataset* get_dataset_pointer();
};



#endif /* SRC_ABSTRACT_VIEW_HANDLER_HPP_ */
