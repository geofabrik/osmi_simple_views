/*
 *  Created on: 2026-06-11
 *      Author: Michael Reichert
 */

#include <algorithm>
#include "turn_restriction.hpp"

RestrictionMemberWay::RestrictionMemberWay(const osmium::Way& way) :
    start_node(way.nodes().front().ref()),
    end_node(way.nodes().back().ref()),
    way_id(way.id()) {}

void RestrictionMemberWay::reverse() {
    std::swap(start_node, end_node);
}

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

void ValidationResult::reset(osmium::item_type type, osmium::object_id_type id, std::string&& msg) {
    object_type = type;
    object_id = id;
    message = std::move(msg);
}

bool ValidationResult::ValidationResult::failed() const {
    return message.has_value();
}

TurnRestriction::TurnRestriction(const osmium::Way& from, const osmium::object_id_type via_node_id, const osmium::Way& to) :
        m_from_way(from),
        m_to_way(to),
        m_via_node(via_node_id) {}

ValidationResult TurnRestriction::check_circles(ValidationResult&& result) const {
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

ValidationResult TurnRestriction::validate_members() {
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
    return check_circles(std::move(result));
}

ViaWayTurnRestriction::ViaWayTurnRestriction(const osmium::Way& from, const osmium::Way& to,
        std::vector<RestrictionMemberWay>&& vias) :
        TurnRestriction::TurnRestriction(from, 0, to),
        m_via_ways(std::move(vias)) {}

ValidationResult ViaWayTurnRestriction::validate_members() {
    ValidationResult result;
    // Deal with simple case (one via way only) first.
    if (m_via_ways.size() == 1) {
        const auto& via_way = m_via_ways.front();
        if (via_way.start_node == m_from_way.start_node || via_way.start_node == m_from_way.end_node) {
            if (via_way.end_node != m_to_way.start_node && via_way.end_node != m_to_way.end_node) {
                result.reset(osmium::item_type::way, via_way.way_id, "via way not connected with to way");
            }
        } else if (via_way.end_node == m_from_way.start_node || via_way.end_node == m_from_way.end_node) {
            if (via_way.start_node != m_to_way.start_node && via_way.start_node != m_to_way.end_node) {
                result.reset(osmium::item_type::way, via_way.way_id, "via way not connected with from way");
            }
        } else {
            result.reset(osmium::item_type::way, via_way.way_id, "via way is not connected to any other member");
        }
        return check_circles(std::move(result));
    }
    // more complex via chains
    const size_t via_count = m_via_ways.size();
    std::vector<bool> used;
    used.reserve(via_count);
    for (size_t v = 0; v < via_count; ++v) {
        used.push_back(false);
    }
    RestrictionMemberWay* last = &m_from_way;
    for (size_t processed = 0; processed < via_count; ++processed) {
        // Find next way by looking at their start nodes and end nodes
        bool found = false;
        for (size_t i = 0; i < via_count; ++i) {
            if (used.at(i)) {
                continue;
            }
            auto& via_way = m_via_ways.at(i);
            if (last->end_node == via_way.start_node) {
                used.at(i) = true;
                last = &via_way;
                ++processed;
                found = true;
                break;
            } else if (last->end_node == via_way.end_node) {
                used.at(i) = true;
                last = &via_way;
                last->reverse();
                ++processed;
                found = true;
                break;
            // We reverse ways whose start node is connected to the next way.
            // However, the from way is never reversed.
            } else if (processed == 0 && last->start_node == via_way.start_node) {
                used.at(i) = true;
                last = &via_way;
                ++processed;
                found = true;
                break;
            } else if (processed == 0 && last->start_node == via_way.end_node) {
                used.at(i) = true;
                last = &via_way;
                last->reverse();
                ++processed;
                found = true;
                break;
            }
        }
        if (!found) {
            if (processed == 0) {
                result.reset(osmium::item_type::way, last->way_id, "from way is not connected to any via way");
            } else if (processed < via_count) {
                result.reset(osmium::item_type::way, last->way_id, "via way is not connected to the next via way");
            }
            return result;
        }
    }
    // Check if last via way is connected to the end way
    if (last->end_node != m_to_way.start_node && last->end_node != m_to_way.end_node) {
        result.reset(osmium::item_type::way, last->way_id, "last via way is not connected to the to way");
    }
    return check_circles(std::move(result));
}

ValidationResult ViaWayTurnRestriction::check_circles(ValidationResult&& result) const {
    for (const auto& v : m_via_ways) {
        if (v.start_node == v.end_node) {
            result.reset(osmium::item_type::way, v.way_id, "via way is a loop");
        }
    }
    return TurnRestriction::check_circles(std::move(result));
}
