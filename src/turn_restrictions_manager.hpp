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

class TurnRestrictionsManager : public osmium::relations::RelationsManager<TurnRestrictionsManager,
false, true, false>, public OGROutputBase {

    std::unique_ptr<gdalcpp::Layer> m_restrictions_n;
    std::unique_ptr<gdalcpp::Layer> m_restrictions_w;
    std::unique_ptr<gdalcpp::Layer> m_invalid_restrictions_n;
    std::unique_ptr<gdalcpp::Layer> m_invalid_restrictions_w;

    static constexpr size_t vehicle_classes_count = 41;

    void write_invalid_point(const osmium::Relation& relation,
            const ValidationResult& result, std::unique_ptr<OGRGeometry>&& geometry);
    void write_invalid_line(const osmium::Relation& relation,
            const ValidationResult& result, std::unique_ptr<OGRGeometry>&& geometry);
    void write_valid(const osmium::Relation& relation,
        std::unique_ptr<OGRGeometry>&& point, std::unique_ptr<OGRGeometry>&& multilinestring);
    void write(const osmium::Relation& relation, const ValidationResult& validation,
            std::unique_ptr<OGRPoint>&& point, std::unique_ptr<OGRMultiLineString>&& ml);

    void init_vehicle_classes_lengths();

public:
    enum class Restriction : char {
        undefined, invalid,
        no_u_turn, no_left_turn, no_right_turn, no_straight_on,
        only_u_turn, only_left_turn, only_right_turn, only_straight_on,
        no_entry, no_exit,
        stop, give_way
    };

    enum class VehicleSuperclass : char {
        bicycle, invalid, other, conditional
    };

    static constexpr std::array<const char*, vehicle_classes_count> vehicle_classes = {
        "bicycle", "vehicle", "kick_scooter", "carriage", "cycle_rickshaw", "hand_cart",
        "trailer", "caravan", "motor_vehicle", "motorcycle", "moped", "speed_pedelec",
        "small_electric_vehicle", "motorcar", "motorhome", "tourist_bus", "coach", "goods", "hgv",
        "hgv_articulated", "bdouble", "agricultural", "auto_rickshaw", "nev", "golf_cart",
        "microcar", "atv", "psv", "bus", "taxi", "minibus", "share_taxi", "rideshare",
        "hov", "carpool", "car_sharing", "emergency", "hazmat", "hazmat:water", "school_bus",
        "disabled"
    };
    std::array<size_t, vehicle_classes_count> vehicle_classes_lengths;

    static Restriction parse_restriction(const char* value, const bool bicycle);

    /**
     * Check if the provided key is a valid restriction key.
     *
     * Returns VehicleSuperclass::other or VehicleSuperclass::bicycle if the argument key starts
     * with "restriction:" followed by a vehicle class. After the vehicle class, the string ma
     *  continue with ":conditional".
     *
     * VehicleSuperclass::bicycle is returned if the vehicle class matches "bicycle".
     */
    VehicleSuperclass is_valid_restriction_key(const char* key) const;

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
    void create_layer(CreateLayerFunc create_layer);

    void close();

private:
    ValidationResult check_tagging(const osmium::Relation& rel, TurnRestrictionsManager::Restriction& restriction_type);
};


#endif /* SRC_TURN_RESTRICTIONS_MANAGER_HPP */
