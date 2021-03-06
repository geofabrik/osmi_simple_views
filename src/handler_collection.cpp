/*
 * handler_collection.cpp
 *
 *  Created on:  2017-07-21
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "handler_collection.hpp"

HandlerCollection::HandlerCollection(Options& options) :
    m_options(options) {}

void HandlerCollection::give_correct_name() {
    for (auto& h : m_handlers) {
        h->close();
        h->give_correct_name();
    }
}

gdalcpp::Dataset* HandlerCollection::add_handler(ViewType view, const char* layer_name) {
    std::unique_ptr<AbstractViewHandler> handler;
    gdalcpp::Dataset* dataset_ptr = nullptr;
    if (view == ViewType::geometry) {
        handler.reset(new GeometryViewHandler(m_options));
    } else if (view == ViewType::highways) {
        handler.reset(new HighwayViewHandler(m_options));
    } else if (view == ViewType::tagging) {
        handler.reset(new TaggingViewHandler(m_options));
    } else if (view == ViewType::places) {
        handler.reset(new PlacesHandler(m_options));
        m_places_handler = dynamic_cast<PlacesHandler*>(handler.get());
    } else {
        return nullptr;
    }
    if (layer_name) {
        dataset_ptr = handler->get_dataset_pointer(layer_name);
    }
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
    try {
        for (std::unique_ptr<AbstractViewHandler>& handler  : m_handlers) {
            handler->way(way);
        }
        if (m_mp_collector_handler2) {
            m_mp_collector_handler2->way(way);
        }
    } catch (osmium::invalid_location& err) {
        m_options.verbose_output << err.what() << '\n';
    }
}

void HandlerCollection::relation(const osmium::Relation& relation) {
    try {
        for (std::unique_ptr<AbstractViewHandler>& handler : m_handlers) {
            handler->relation(relation);
        }
        if (m_mp_collector_handler2) {
            m_mp_collector_handler2->relation(relation);
        }
    } catch (osmium::invalid_location& err) {
        m_options.verbose_output << err.what() << '\n';
    }
}

void HandlerCollection::area(const osmium::Area& area) {
    try {
        for (std::unique_ptr<AbstractViewHandler>& handler : m_handlers) {
            handler->area(area);
        }
    } catch (osmium::invalid_location& err) {
        m_options.verbose_output << err.what() << '\n';
    }
}
