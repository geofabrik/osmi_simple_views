/*
 *  Created on:  2026-06-11
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_TURN_RESTRICTIONS_MANAGER_HPP_
#define SRC_TURN_RESTRICTIONS_MANAGER_HPP_

#include <gdalcpp.hpp>
#include <osmium/relations/relations_manager.hpp>
#include "ogr_output_base.hpp"
#include "turn_restriction.hpp"

class TurnRestrictionsManager : public osmium::relations::RelationsManager<TurnRestrictionManager,
false, true, false>, public OGROutputBase {

    std::unique_ptr<gdalcpp::Layer> m_restrictions_n;
    std::unique_ptr<gdalcpp::Layer> m_restrictions_w;
    std::unique_ptr<gdalcpp::Layer> m_invalid_restrictions_n;
    std::unique_ptr<gdalcpp::Layer> m_invalid_restrictions_w;

    bool enabled;

    void write_invalid_point(const osmium::Relation& relation,
            const ValidationResult& result, std::unique_ptr<OGRGeometry>&& geometry);
    void write_invalid_line(const osmium::Relation& relation,
            const ValidationResult& result, std::unique_ptr<OGRGeometry>&& geometry);

public:
    enum Restriction : char {
        undefined, invalid,
        no_u_turn, no_left_turn, no_right_turn, no_straight_on,
        only_u_turn, only_left_turn, only_right_turn, only_straight_on,
        no_entry, no_exit
    };

    static Restriction parse_restriction(const char* value);

    TurnRestrictionsManager() = delete;

    TurnRestrictionsManager(Options& options);

    static constexpr const char* node_layer_name = "restriction_n";
    static constexpr const char* way_layer_name = "restriction_w";
    static constexpr const char* invalid_node_layer_name = "invalid_restrictions_n";
    static constexpr const char* invalid_way_layer_name = "invalid_restrictions_w";

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

    /**
     * Assign the pointer pointing to a dataset.
     */
    void create_layer(gdalcpp::Dataset* dataset);

private:
    ValidationResult check_tagging(const osmium::Relation& rel, TurnRestrictionsManager::Restriction& restriction_type);
};


#endif /* SRC_TURN_RESTRICTIONS_MANAGER_HPP */
