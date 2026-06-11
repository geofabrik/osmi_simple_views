/*
 *  Created on: 2026-06-11
 *      Author: michael
 */

#include "turn_restriction.hpp"

RestrictionMemberWay::RestrictionMemberWay(const osmium::Way& way) :
    start_node(way.nodes().front().ref()),
    end_node(way.nodes().back().ref()),
    way_id(way.id()) {}

ValidationResult::ValidationResult() :
        object_type(),
        object_id(),
        message() {}

ValidationResult::ValidationResult(osmium::item_type type, osmium::object_id_type id, std::string&& msg) :
        object_type(type),
        object_id(id),
        message() {
        message = msg;
}

bool ValidationResult::ValidationResult::failed() const {
    return message.has_value();
}

TurnRestriction::TurnRestriction(const osmium::Way& from, const osmium::object_id_type via_node_id, const osmium::Way& to) :
        m_from_way(from),
        m_to_way(to),
        m_via_node(via_node_id) {}

ValidationResult TurnRestriction::valid_members() const {
    ValidationResult result;
    // We don't have to check here if all members are set because that is handled by the TurnRestrictionManager.
    // Check if from way is connected to via node
    if (m_from_way.start_node != m_via_node && m_from_way.end_node != m_via_node) {
        result.message = "Start way not connected with via node";
        result.object_type = osmium::item_type::way;
        result.object_id = m_from_way.way_id;
        return result;
    }
    if (m_to_way.start_node != m_via_node && m_to_way.end_node != m_via_node) {
        result.message = "End way not connected with via node";
        result.object_type = osmium::item_type::way;
        result.object_id = m_to_way.way_id;
        return result;
    }
    if (m_from_way.start_node == m_from_way.end_node) {
        result.message = "Start way is a loop.";
        result.object_type = osmium::item_type::way;
        result.object_id = m_from_way.way_id;
        return result;
    }
    if (m_to_way.start_node == m_to_way.end_node) {
        result.message = "End way is a loop.";
        result.object_type = osmium::item_type::way;
        result.object_id = m_to_way.way_id;
        return result;
    }
    return result;
}
