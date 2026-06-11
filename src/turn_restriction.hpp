/*
 *  Created on: 2026-06-11
 *      Author: michael
 */

#ifndef SRC_TURN_RESTRICTION_HPP_
#define SRC_TURN_RESTRICTION_HPP_

#include <optional>
#include <osmium/osm/way.hpp>

struct RestrictionMemberWay {
    osmium::object_id_type start_node;
    osmium::object_id_type end_node;
    osmium::object_id_type way_id;
    RestrictionMemberWay() = delete;
    RestrictionMemberWay(const osmium::Way& way);
};

struct ValidationResult {
    osmium::item_type object_type;
    osmium::object_id_type object_id;
    std::optional<std::string> message;
    ValidationResult();
    ValidationResult(osmium::item_type type, osmium::object_id_type id, std::string&& msg);
    bool failed() const;
};

class TurnRestriction {
    RestrictionMemberWay m_from_way;
    RestrictionMemberWay m_to_way;
    osmium::object_id_type m_via_node;

public:
    TurnRestriction(const osmium::Way& from, const osmium::object_id_type via_node_id, const osmium::Way& to);
    ValidationResult valid_members() const;
};



#endif /* SRC_TURN_RESTRICTION_HPP_ */
