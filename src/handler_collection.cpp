/*
 * handler_collection.cpp
 *
 *  Created on:  2017-07-21
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "handler_collection.hpp"

HandlerCollection::HandlerCollection(Options& options) :
    m_options(options),
    m_datasets()/*,
    m_dataset_names()*/ {}

HandlerCollection::DatasetWithView::DatasetWithView(ViewType v, std::unique_ptr<gdalcpp::Dataset>&& d) :
        view(v),
        dataset(std::move(d)) {}

void HandlerCollection::give_correct_name() {
    for (auto& h : m_handlers) {
        h->close();
        switch (h->view_type()) {
        case ViewType::highways :
            if (highway_relation_collector) {
                highway_relation_collector->close();
            }
            break;
        case ViewType::tagging :
            if (any_relation_collector) {
                any_relation_collector->close();
            }
            break;
        case ViewType::turn_restrictions :
            if (turn_restrictions_manager) {
                turn_restrictions_manager->close();
            }
            break;
        default:
            break;
        }
        rename_output_files(h->view_name(), close_datasets(h->view_type()));
    }
}

std::vector<std::string> HandlerCollection::close_datasets(const ViewType view_type) {
    // Paths to datasets. This vector is populated before closing a dataset.
    std::vector<std::string> dataset_names;
    for (auto& d : m_datasets) {
        if (d.view != view_type) {
            continue;
        }
        dataset_names.push_back(d.dataset->dataset_name());
        d.dataset.reset();
    }
    return dataset_names;
}

std::string HandlerCollection::filename_suffix() {
    if (Options::case_insensitive_comp_left(m_options.output_format, "geojson")) {
        return ".json";
    } else if (Options::case_insensitive_comp_left(m_options.output_format, "sqlite")) {
        return ".db";
    }
    return "";
}

void HandlerCollection::rename_output_files(const std::string& view_name,
        const std::vector<std::string>&& dataset_names) {
    if (dataset_names.size() == 1 && filename_suffix().length()) {
        // rename output file if there is one output dataset only
        std::string destination_name {m_options.output_directory};
        destination_name += '/';
        destination_name += view_name;
        destination_name += filename_suffix();
        if (access(destination_name.c_str(), F_OK) == 0) {
            std::cerr << "ERROR: Cannot rename output file from to " << destination_name << " because file exists already.\n";
        } else {
            if (rename(dataset_names.front().c_str(), destination_name.c_str())) {
                std::cerr << "ERROR: Rename from " << dataset_names.front() << " to " << destination_name << "failed.\n";
            }
        }
    } else if (dataset_names.size() > 1 && filename_suffix().length()) {
        for (auto& d: dataset_names) {
            std::string destination_name = d;
            destination_name += filename_suffix();
            if (access(destination_name.c_str(), F_OK) == 0) {
                std::cerr << "ERROR: Cannot rename output file from to " << destination_name << " because file exists already.\n";
            } else {
                if (rename(d.c_str(), destination_name.c_str())) {
                    std::cerr << "ERROR: Rename from " << dataset_names.front() << " to " << destination_name << "failed.\n";
                }
            }
        }
    }
}

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
std::vector<std::string> HandlerCollection::get_gdal_default_dataset_options() {
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

HandlerCollection::DatasetWithView* HandlerCollection::ensure_writeable_dataset(
        const ViewType view, const char* layer_name) {
    // Find matching dataset
    if (!m_options.one_layer_per_datasource_only()) {
        for (auto& vd : m_datasets) {
            if (vd.view == view) {
                return &vd;
            }
        }
    }
    std::string output_filename = m_options.output_directory;
    output_filename += '/';
    output_filename += layer_name;
    std::unique_ptr<gdalcpp::Dataset> ds {new gdalcpp::Dataset(m_options.output_format,
            output_filename, gdalcpp::SRS(m_options.srs), get_gdal_default_dataset_options())};
    ds->enable_auto_transactions(10000);
    m_datasets.emplace_back(view, std::move(ds));
    return &(m_datasets.back());
}

std::vector<std::string> HandlerCollection::get_gdal_default_layer_options() {
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

std::unique_ptr<gdalcpp::Layer> HandlerCollection::create_layer(const ViewType view, const char* layer_name, OGRwkbGeometryType type) {
    DatasetWithView* dv = ensure_writeable_dataset(view, layer_name);
    return std::unique_ptr<gdalcpp::Layer>{new gdalcpp::Layer(*(dv->dataset),
            layer_name, type, get_gdal_default_layer_options())};
}

void HandlerCollection::add_handler(const ViewType view) {
    auto cl = [this, view](const char* layer_name, OGRwkbGeometryType type)
            -> std::unique_ptr<gdalcpp::Layer> {
        return this->create_layer(view, layer_name, type);
    };

    std::unique_ptr<AbstractViewHandler> handler;
    if (view == ViewType::geometry) {
        handler.reset(new GeometryViewHandler(m_options, cl));
    } else if (view == ViewType::highways) {
        handler.reset(new HighwayViewHandler(m_options, cl));
    } else if (view == ViewType::sac_scale) {
        handler.reset(new SacScaleViewHandler(m_options, cl));
    } else if (view == ViewType::tagging) {
        handler.reset(new TaggingViewHandler(m_options, cl));
    } else if (view == ViewType::places) {
        handler.reset(new PlacesHandler(m_options, cl));
        m_places_handler = dynamic_cast<PlacesHandler*>(handler.get());
    } else {
        return;
    }
    m_handlers.push_back(std::move(handler));
}

void HandlerCollection::set_any_relation_collector(AnyRelationCollector& collector) {
    any_relation_collector = &collector;
    auto cl = [this](const char* layer_name, OGRwkbGeometryType type)
            -> std::unique_ptr<gdalcpp::Layer> {
        return this->create_layer(ViewType::tagging, layer_name, type);
    };
    any_relation_collector->create_layer(cl);
}

void HandlerCollection::set_highway_relation_manager(HighwayRelationManager& manager) {
    highway_relation_collector = &manager;
    auto cl = [this](const char* layer_name, OGRwkbGeometryType type)
            -> std::unique_ptr<gdalcpp::Layer> {
        return this->create_layer(ViewType::highways, layer_name, type);
    };
    highway_relation_collector->create_layer(cl);
}

void HandlerCollection::set_turn_restrictions_manager(TurnRestrictionsManager& manager) {
    turn_restrictions_manager = &manager;
    auto cl = [this](const char* layer_name, OGRwkbGeometryType type)
            -> std::unique_ptr<gdalcpp::Layer> {
        return this->create_layer(ViewType::turn_restrictions, layer_name, type);
    };
    turn_restrictions_manager->create_layer(cl);
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
    if (any_relation_collector) {
        any_relation_collector->handler().node(node);
    }
    if (highway_relation_collector) {
        highway_relation_collector->handler().node(node);
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
        if (any_relation_collector) {
            any_relation_collector->handler().way(way);
        }
        if (highway_relation_collector) {
            highway_relation_collector->handler().way(way);
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
        if (any_relation_collector) {
            any_relation_collector->handler().relation(relation);
        }
        if (highway_relation_collector) {
            highway_relation_collector->handler().relation(relation);
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


void HandlerCollection::flush() {
    for (std::unique_ptr<AbstractViewHandler>& handler : m_handlers) {
        handler->flush();
    }
    if (m_mp_collector_handler2) {
        m_mp_collector_handler2->flush();
    }
    if (any_relation_collector) {
        any_relation_collector->handler().flush();
    }
    if (highway_relation_collector) {
        highway_relation_collector->handler().flush();
    }
}
