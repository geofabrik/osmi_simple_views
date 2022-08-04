/*
 * highway_relation_collector.hpp
 *
 *  Created on:  2022-08-02
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_HIGHWAY_RELATION_COLLECTOR_HPP_
#define SRC_HIGHWAY_RELATION_COLLECTOR_HPP_

#include <gdalcpp.hpp>
#include <osmium/relations/relations_manager.hpp>
#include "ogr_output_base.hpp"

class HighwayRelationManager : public osmium::relations::RelationsManager<HighwayRelationManager,
false, true, false>, public OGROutputBase {

    std::unique_ptr<gdalcpp::Layer> m_relations_with_highway;

    bool enabled;

public:
    HighwayRelationManager() = delete;

    HighwayRelationManager(Options& options);

    static constexpr const char* layer_name = "highway_relations_with_highway";

    /**
     * Activate this relation manager.
     *
     * This method has to be called before the manager is used. Otherwise no relations are
     * collected.
     */
    void enable();

    /**
     * This method decides which relations we're interested in, and
     * instructs Osmium to collect their members for us.
     *
     * Only multipolygons and boundary relations may have way members without any tags.
     * All other types of relations must have members with tags.
     */
    bool new_relation(const osmium::Relation& relation) const noexcept;

    /**
     * Tells Osmium which members to keep for a relation of interest.
     */
    bool new_member(const osmium::Relation& /*relation*/, const osmium::RelationMember& member, std::size_t /*n*/) const noexcept;

    void complete_relation(const osmium::Relation& relation);

    void process_relation(const osmium::Relation& relation);

    /**
     * Assign the pointer pointing to a dataset.
     */
    void create_layer(gdalcpp::Dataset* dataset);

};


#endif /* SRC_HIGHWAY_RELATION_COLLECTOR_HPP_ */
