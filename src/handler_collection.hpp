/*
 * handler_collection.hpp
 *
 *  Created on:  2017-07-20
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_HANDLER_COLLECTION_HPP_
#define SRC_HANDLER_COLLECTION_HPP_

#include <osmium/area/multipolygon_collector.hpp>
#include <osmium/area/assembler.hpp>

#include "highway_view_handler.hpp"
#include "geometry_view_handler.hpp"
#include "places_handler.hpp"
#include "tagging_view_handler.hpp"

/**
 * Available views
 */
enum class ViewType : char {
    none= 0,
    places= 1,
    geometry= 2,
    highways= 3,
    tagging= 4
};

/**
 * The handler collection manages all handlers and calls their node, way, relation and area callbacks one
 * after another. This allows us to only instanciate those handlers which are necessary.
 *
 * The HandlerCollection class must include the header file of the handler class and the handler class must
 * be derived from AbstractViewHandler.
 */
class HandlerCollection : public osmium::handler::Handler {
    std::vector<std::unique_ptr<AbstractViewHandler>> m_handlers;
    PlacesHandler* m_places_handler;
    osmium::area::MultipolygonCollector<osmium::area::Assembler>::HandlerPass2* m_mp_collector_handler2 = nullptr;

public:

    void give_correct_name();

    /**
     * \brief Create and register a new handler.
     *
     * \returns pointer to the handler being added
     */
    gdalcpp::Dataset* add_handler(ViewType view, std::string& output_filename, std::string& output_format,
            osmium::util::VerboseOutput& verbose_output, const char* layer_name, int epsg = 3857);

    /**
     * \brief Add a multipolygon collector.
     *
     * This method has only to be called if the handler to be
     */
    void add_multipolygon_collector(osmium::area::MultipolygonCollector<osmium::area::Assembler>& collector);

    void node(const osmium::Node& node);

    void way(const osmium::Way& way);

    void relation(const osmium::Relation& relation);

    void area(const osmium::Area& area);
};



#endif /* SRC_HANDLER_COLLECTION_HPP_ */
