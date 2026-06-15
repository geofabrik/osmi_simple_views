/*
 * handler_collection.hpp
 *
 *  Created on:  2017-07-20
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_HANDLER_COLLECTION_HPP_
#define SRC_HANDLER_COLLECTION_HPP_

#include <gdalcpp.hpp>
#include <osmium/area/multipolygon_collector.hpp>
#include <osmium/area/assembler.hpp>

#include "highway_view_handler.hpp"
#include "geometry_view_handler.hpp"
#include "options.hpp"
#include "places_handler.hpp"
#include "tagging_view_handler.hpp"
#include "any_relation_collector.hpp"
#include "highway_relation_manager.hpp"
#include "turn_restrictions_manager.hpp"
#include "sac_scale_view_handler.hpp"

/**
 * The handler collection manages all handlers and calls their node, way, relation and area callbacks one
 * after another. This allows us to only instanciate those handlers which are necessary.
 *
 * The HandlerCollection class must include the header file of the handler class and the handler class must
 * be derived from AbstractViewHandler.
 */
class HandlerCollection : public osmium::handler::Handler {
    Options& m_options;

public:
    struct DatasetWithView {
        ViewType view;
        std::unique_ptr<gdalcpp::Dataset> dataset;
        explicit DatasetWithView(ViewType v, std::unique_ptr<gdalcpp::Dataset>&& d);
    };

private:
    /// ORG dataset
    // This has to be a vector of unique_ptr because a vector of objects themselves fails to
    // compile due to "copy constructor of 'Dataset' is implicitly deleted because field
    // 'm_options' has a deleted copy constructor".
    std::vector<DatasetWithView> m_datasets;

    std::vector<std::unique_ptr<AbstractViewHandler>> m_handlers;
    PlacesHandler* m_places_handler = nullptr;
    osmium::area::MultipolygonCollector<osmium::area::Assembler>::HandlerPass2* m_mp_collector_handler2 = nullptr;
    // Storing relation collectors holding GDAL layers as pointers
    // because they go out of scope before we rename the datasets they share with their node/way handlers.
    HighwayRelationManager* highway_relation_collector = nullptr;
    AnyRelationCollector* any_relation_collector = nullptr;
    TurnRestrictionsManager* turn_restrictions_manager = nullptr;

    /**
     * Add a new dataset to the vector if the last one cannot be use for multiple layers
     */
    DatasetWithView* ensure_writeable_dataset(const ViewType view, const char* layer_name);

    std::unique_ptr<gdalcpp::Layer> create_layer(const ViewType view, const char* layer_name, OGRwkbGeometryType type);

    /**
     * Get filename suffix by output format with a leading dot.
     *
     * If the format is not found, an empty string is returned.
     *
     * This method is required because GDAL does not provide such a method.
     */
    std::string filename_suffix();

    /**
     * Close all datasets of a view and return a vector of the names of those datasets.
     */
    std::vector<std::string> close_datasets(const ViewType view_type);

    void rename_output_files(const std::string& view_name, const std::vector<std::string>&& dataset_names);

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
    HandlerCollection(Options& options);

    /**
     * Add proper file name suffix to the output files. If there is one output dataset only,
     * give it the name of the view.
     *
     * @arg view_name name of the view.
     */
    void give_correct_name();

    /**
     * \brief Create and register a new handler.
     *
     * \arg view handler to be added
     */
    void add_handler(const ViewType view);

    void set_any_relation_collector(AnyRelationCollector& collector);

    void set_highway_relation_manager(HighwayRelationManager& manager);

    void set_turn_restrictions_manager(TurnRestrictionsManager& manager);

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

    void flush();
};



#endif /* SRC_HANDLER_COLLECTION_HPP_ */
