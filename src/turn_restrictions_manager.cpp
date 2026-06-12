/*
 *  Created on:  2026-06-11
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "turn_restrictions_manager.hpp"
#include "tagging_view_handler.hpp"

TurnRestrictionsManager::TurnRestrictionsManager(Options& options) :
        OGROutputBase(options) {
    init_vehicle_classes_lengths();
}

void TurnRestrictionsManager::init_vehicle_classes_lengths() {
    for (size_t i = 0; i < vehicle_classes_count; ++i) {
        vehicle_classes_lengths[i] = strlen(vehicle_classes.at(i));
    }
}

bool TurnRestrictionsManager::new_relation(const osmium::Relation& relation) const noexcept {
    const char* type = relation.get_value_by_key("type");
    if (type && !strcmp(type, "restriction")) {
        return true;
    }
    if (relation.tags().has_key("restriction")) {
        return true;
    }
    std::array<const char*, 6> restriction_keys = {"restriction:bicycle", "restriction:bus", "restriction:conditional",
            "restriction:hgv", "restriction:hgv:conditional", "restriction:motorcar"};
    for (const char* k : restriction_keys) {
        if (relation.tags().has_key(k)) {
            return true;
        }
    }
    return false;
}

bool TurnRestrictionsManager::new_member(const osmium::Relation&,
        const osmium::RelationMember&, std::size_t) const noexcept {
    return true;
}

void TurnRestrictionsManager::write_invalid_point(const osmium::Relation& relation,
        const ValidationResult& result, std::unique_ptr<OGRGeometry>&& geometry) {
    gdalcpp::Feature feature(*(m_invalid_restrictions_n.get()), std::move(geometry));
    TaggingViewHandler::set_basic_fields(feature, relation, "message", result.message.value().c_str());
    feature.add_to_layer();
}

void TurnRestrictionsManager::write_invalid_line(const osmium::Relation& relation,
        const ValidationResult& result, std::unique_ptr<OGRGeometry>&& geometry) {
    gdalcpp::Feature feature(*(m_invalid_restrictions_w.get()), std::move(geometry));
    TaggingViewHandler::set_basic_fields(feature, relation, "message", result.message.value().c_str());
    feature.add_to_layer();
}

void TurnRestrictionsManager::write_valid(const osmium::Relation& relation,
        std::unique_ptr<OGRGeometry>&& point, std::unique_ptr<OGRGeometry>&& multilinestring) {
    const char* r_value = relation.get_value_by_key("restriction");
    {
        gdalcpp::Feature feature(*(m_restrictions_w.get()), std::move(multilinestring));
        TaggingViewHandler::set_basic_fields(feature, relation, "restriction", r_value);
        feature.add_to_layer();
    }
    if (!point->IsEmpty()) {
        gdalcpp::Feature feature(*(m_restrictions_n.get()), std::move(point));
        TaggingViewHandler::set_basic_fields(feature, relation, "restriction", r_value);
        feature.add_to_layer();
    }
}

void TurnRestrictionsManager::write(const osmium::Relation& relation,
        const ValidationResult& validation, std::unique_ptr<OGRPoint>&& point,
        std::unique_ptr<OGRMultiLineString>&& ml) {
    if (!ml->IsEmpty()) {
        std::unique_ptr<OGRGeometry> geom {static_cast<OGRGeometry*>(ml.release())};
        if (validation.failed()) {
            write_invalid_line(relation, validation, std::move(geom));
        } else {
            write_valid(relation, std::move(point), std::move(ml));
        }
    } else {
        std::unique_ptr<OGRGeometry> geom {static_cast<OGRGeometry*>(point.release())};
        write_invalid_point(relation, validation, std::move(geom));
    }
}

TurnRestrictionsManager::Restriction TurnRestrictionsManager::parse_restriction(const char* value,
        const bool bicycle) {
    if (!value) {
        return Restriction::undefined;
    }
    if (!strcmp(value, "no_u_turn")) {
        return Restriction::no_u_turn;
    }
    if (!strcmp(value, "no_left_turn")) {
        return Restriction::no_left_turn;
    }
    if (!strcmp(value, "no_right_turn")) {
        return Restriction::no_left_turn;
    }
    if (!strcmp(value, "no_straight_on")) {
        return Restriction::no_straight_on;
    }
    if (!strcmp(value, "only_u_turn")) {
        return Restriction::only_u_turn;
    }
    if (!strcmp(value, "only_left_turn")) {
        return Restriction::only_left_turn;
    }
    if (!strcmp(value, "only_right_turn")) {
        return Restriction::only_left_turn;
    }
    if (!strcmp(value, "only_straight_on")) {
        return Restriction::only_straight_on;
    }
    if (!strcmp(value, "no_entry")) {
        return Restriction::no_entry;
    }
    if (!strcmp(value, "no_exit")) {
        return Restriction::no_exit;
    }
    if (bicycle && !strcmp(value, "stop")) {
        return Restriction::stop;
    }
    if (bicycle && !strcmp(value, "give_way")) {
        return Restriction::give_way;
    }
    return Restriction::invalid;
}

TurnRestrictionsManager::VehicleSuperclass TurnRestrictionsManager::is_valid_restriction_key(const char* key) const {
    if (strncmp(key, "restriction:", 12)) {
        return VehicleSuperclass::invalid;
    }
    const char* key_wo_prefix = key + 12;
    const size_t keylen = strlen(key);
    for (size_t i = 0; i < vehicle_classes_count; ++i) {
        const size_t l = vehicle_classes_lengths.at(i);
        if (keylen - 12 < l) {
            continue;
        }
        const char* v = vehicle_classes.at(i);
        if (strncmp(key_wo_prefix, v, l)) {
            continue;
        }
        const char* after_vehicle_class = key_wo_prefix + l;
        if (*after_vehicle_class == '\0') {
            return (i == 0) ? VehicleSuperclass::bicycle : VehicleSuperclass::other;
        } else if (*after_vehicle_class != ':') {
            return VehicleSuperclass::invalid;
        }
        const char* cond_suffix = key_wo_prefix + l + 1;
        if (!strcmp(cond_suffix, "conditional")) {
            return VehicleSuperclass::conditional;
        }
    }
    return VehicleSuperclass::invalid;
}

ValidationResult TurnRestrictionsManager::check_tagging(const osmium::Relation& rel,
        TurnRestrictionsManager::Restriction& restriction_type) {
    const char* type = rel.get_value_by_key("type");
    if (!type || strcmp(type, "restriction")) {
        return ValidationResult{rel.type(), rel.id(), "Invalid value of type=* key"};
    }
    restriction_type = parse_restriction(rel.get_value_by_key("restriction"), false);
    if (restriction_type == Restriction::invalid) {
        return ValidationResult{rel.type(), rel.id(), "Invalid value of restriction=* key"};
    }
    if (restriction_type == Restriction::undefined) {
        bool found = false;
        // If restriction=* is unset, look for restriction:*=* instead.
        for (const osmium::Tag& t : rel.tags()) {
            VehicleSuperclass vsc = is_valid_restriction_key(t.key());
            if (vsc != VehicleSuperclass::invalid && vsc != VehicleSuperclass::conditional) {
                restriction_type = parse_restriction(t.value(), (vsc == VehicleSuperclass::bicycle));
                if (restriction_type == Restriction::invalid) {
                    return ValidationResult{rel.type(), rel.id(), std::string{"Invalid value of key "} + t.key()};
                }
                found = (restriction_type != Restriction::undefined);
            }
        }
        if (!found) {
            return ValidationResult{rel.type(), rel.id(), "No restriction=* or restriction:*=*"};
        }
    }
    return ValidationResult{};
}

void TurnRestrictionsManager::complete_relation(const osmium::Relation& relation) {
    Restriction restriction;
    ValidationResult validation = check_tagging(relation, restriction);
    std::unique_ptr<OGRPoint> point;
    std::unique_ptr<OGRMultiLineString> ml {new OGRMultiLineString()};
    osmium::object_id_type member_node_id = 0;
    const osmium::Way* from_way = nullptr;
    const osmium::Way* to_way = nullptr;
    std::vector<RestrictionMemberWay> via_ways;
    for (const osmium::RelationMember& member : relation.members()) {
        if (member.ref() <= 0) {
            continue;
        }
        if (member.type() == osmium::item_type::relation) {
            validation.reset(osmium::item_type::relation, member.ref(), "relation as member");
        } else if (member.type() == osmium::item_type::way) {
            const osmium::Way* way = this->get_member_way(member.ref());
            try {
                std::unique_ptr<OGRLineString> linestring = m_factory.create_linestring(*way);
                ml->addGeometryDirectly(linestring.release());
            }
            catch (osmium::geometry_error& e) {
            }
            if (!member.role()) {
                validation.reset(osmium::item_type::way, member.ref(), "way with empty role");
                member_node_id = member.ref();
            } else if (!strcmp(member.role(), "from")) {
                if (from_way && restriction != Restriction::no_entry) {
                    validation.reset(osmium::item_type::way, member.ref(), "multiple from members");
                } else {
                    from_way = way;
                }
            } else if (!strcmp(member.role(), "to")) {
                if (to_way && restriction != Restriction::no_exit) {
                    validation.reset(osmium::item_type::way, member.ref(), "multiple to members");
                } else {
                    to_way = way;
                }
            } else if (strcmp(member.role(), "via")) {
                if (member_node_id) {
                    validation.reset(osmium::item_type::way, member.ref(), "via way and node in one relation");
                } else {
                    via_ways.emplace_back(*way);
                }
            } else {
                validation.reset(osmium::item_type::way, member.ref(), "invalid role for way member");
            }
        } else if (member.type() == osmium::item_type::node) {
            try {
                const osmium::Node* node = this->get_member_node(member.ref());
                point = m_factory.create_point(*node);
            }
            catch (osmium::geometry_error& e) {
            }
            if (!member.role()) {
                validation.reset(osmium::item_type::node, member.ref(), "empty role for node member");
            } else if (!strcmp(member.role(), "via")) {
                if (member_node_id) {
                    validation.reset(osmium::item_type::node, member.ref(), "multiple via nodes");
                } else if (!via_ways.empty()) {
                    validation.reset(osmium::item_type::node, member.ref(), "via way and node in one relation");
                } else {
                    member_node_id = member.ref();
                }
            } else {
                validation.reset(osmium::item_type::node, member.ref(), "invalid role for node member");
            }
        }
    }
    if (!from_way) {
        validation.message = "From way missing";
    } else if (!to_way) {
        validation.message = "To way missing";
    }
    if (validation.failed()) {
        write(relation, validation, std::move(point), std::move(ml));
        return;
    }
    if (restriction != Restriction::no_exit && restriction != Restriction::no_entry) {
        // Connectivity checks do not support no_exit and no_entry restrictions.
        if (via_ways.empty()) {
            TurnRestriction tr {*from_way, member_node_id, *to_way};
            validation = tr.validate_members();
        } else {
            ViaWayTurnRestriction tr {*from_way, *to_way, std::move(via_ways)};
            validation = tr.validate_members();
        }
    }
    if (validation.message.has_value()) {
        if (!ml->IsEmpty()) {
            std::unique_ptr<OGRGeometry> geom {static_cast<OGRGeometry*>(ml.release())};
            write_invalid_line(relation, validation, std::move(geom));
        } else {
            std::unique_ptr<OGRGeometry> geom {static_cast<OGRGeometry*>(point.release())};
            write_invalid_point(relation, validation, std::move(geom));
        }
    }
}

void TurnRestrictionsManager::create_layer(gdalcpp::Dataset* dataset) {
    m_restrictions_n =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, node_layer_name,
                    wkbPoint, get_gdal_default_layer_options()));
    m_restrictions_n->add_field("rel_id", OFTString, 10);
    m_restrictions_n->add_field("lastchange", OFTString, 21);
    m_restrictions_n->add_field("restriction", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_n->add_field("other_tags", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_w =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, way_layer_name,
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_restrictions_w->add_field("rel_id", OFTString, 10);
    m_restrictions_w->add_field("lastchange", OFTString, 21);
    m_restrictions_w->add_field("restriction", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_w->add_field("other_tags", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_invalid_restrictions_n =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, invalid_node_layer_name,
            wkbPoint, get_gdal_default_layer_options()));
    m_invalid_restrictions_n->add_field("rel_id", OFTString, 10);
    m_invalid_restrictions_n->add_field("lastchange", OFTString, 21);
    m_invalid_restrictions_n->add_field("message", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_invalid_restrictions_n->add_field("error_type", OFTString, 1);
    m_invalid_restrictions_n->add_field("error_id", OFTInteger64, TaggingViewHandler::MAX_STRING_LENGTH);
    m_invalid_restrictions_w =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, invalid_way_layer_name,
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_invalid_restrictions_w->add_field("rel_id", OFTString, 10);
    m_invalid_restrictions_w->add_field("lastchange", OFTString, 21);
    m_invalid_restrictions_w->add_field("message", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_invalid_restrictions_w->add_field("error_type", OFTString, 1);
    m_invalid_restrictions_w->add_field("error_id", OFTInteger64, TaggingViewHandler::MAX_STRING_LENGTH);
}
