/*
 * handler_collection.cpp
 *
 *  Created on:  2017-07-21
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "handler_collection.hpp"

gdalcpp::Dataset* HandlerCollection::add_handler(ViewType view, std::string output_filename, std::string& output_format,
        osmium::util::VerboseOutput& verbose_output, int epsg /*= 3857*/) {
    std::unique_ptr<AbstractViewHandler> handler;
    gdalcpp::Dataset* dataset_ptr = nullptr;
    if (view == ViewType::geometry) {
        output_filename += "/geometry.db";
        handler.reset(new GeometryViewHandler(output_filename, output_format, verbose_output, epsg));
    } else if (view == ViewType::highways) {
        output_filename += "/highways.db";
        handler.reset(new HighwayViewHandler(output_filename, output_format, verbose_output, epsg));
    } else if (view == ViewType::tagging) {
        output_filename += "/tagging.db";
        handler.reset(new TaggingViewHandler(output_filename, output_format, verbose_output, epsg));
    } else if (view == ViewType::places) {
        output_filename += "/places.db";
        handler.reset(new PlacesHandler(output_filename, output_format, verbose_output, epsg));
        m_places_handler = dynamic_cast<PlacesHandler*>(handler.get());
    } else {
        return nullptr;
    }
    dataset_ptr = handler->get_dataset_pointer();
    m_handlers.push_back(std::move(handler));
    return dataset_ptr;
}

void HandlerCollection::add_multipolygon_collector(osmium::area::MultipolygonCollector<osmium::area::Assembler>& collector) {
    PlacesHandler& pl = *m_places_handler;
    m_mp_collector_handler2 = &(collector.handler([&pl](const osmium::memory::Buffer& area_buffer) {
        osmium::apply(area_buffer, pl);
        }));
}

void HandlerCollection::node(const osmium::Node& node) {
    for (std::unique_ptr<AbstractViewHandler>& handler : m_handlers) {
        handler->node(node);
    }
    if (m_mp_collector_handler2) {
        m_mp_collector_handler2->node(node);
    }
}

void HandlerCollection::way(const osmium::Way& way) {
    for (std::unique_ptr<AbstractViewHandler>& handler  : m_handlers) {
        handler->way(way);
    }
    if (m_mp_collector_handler2) {
        m_mp_collector_handler2->way(way);
    }
}

void HandlerCollection::relation(const osmium::Relation& relation) {
    for (std::unique_ptr<AbstractViewHandler>& handler : m_handlers) {
        handler->relation(relation);
    }
    if (m_mp_collector_handler2) {
        m_mp_collector_handler2->relation(relation);
    }
}

void HandlerCollection::area(const osmium::Area& area) {
    for (std::unique_ptr<AbstractViewHandler>& handler : m_handlers) {
        handler->area(area);
    }
}


