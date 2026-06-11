/*
 *  Created on:  2026-06-11
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "turn_restrictions_manager.hpp"
#include "tagging_view_handler.hpp"

TurnRestrictionsManager::TurnRestrictionsManager(Options& options) :
        OGROutputBase(options),
        enabled(false) { }

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

TurnRestrictionsManager::Restriction TurnRestrictionsManager::parse_restriction(const char* value) {
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
    return Restriction::invalid;
}

ValidationResult TurnRestrictionsManager::check_tagging(const osmium::Relation& rel,
        TurnRestrictionsManager::Restriction& restriction_type) {
    const char* type = rel.get_value_by_key("type");
    if (!type || strcmp(type, "restriction")) {
        return ValidationResult{rel.type(), rel.id(), "Invalid value of type=* key"};
    }
    restriction_type = parse_restriction(rel.get_value_by_key("restriction"));
    if (restriction_type == Restriction::invalid) {
        return ValidationResult{rel.type(), rel.id(), "Invalid value of restriction=* key"};
    }
    return ValidationResult{};
}

void TurnRestrictionsManager::complete_relation(const osmium::Relation& relation) {
    Restriction restriction;
    ValidationResult validation = check_tagging(relation, restriction);
    std::unique_ptr<OGRPoint> point;
    std::unique_ptr<OGRMultiLineString> ml {new OGRMultiLineString()};
    osmium::object_id_type member_node_id;
    osmium::Way* from_way;
    osmium::Way* to_way;
    osmium::object_id_type member_relation_id;
    for (const osmium::RelationMember& member : relation.members()) {
        if (member.ref() <= 0) {
            continue;
        }
        if (member.type() == osmium::item_type::relation) {
            member_relation_id = member.ref();
            validation.message = "relation as member";
        } else if (member.type() == osmium::item_type::way) {
            const osmium::Way* way = this->get_member_way(member.ref());
            try {
                std::unique_ptr<OGRLineString> linestring = m_factory.create_linestring(*way);
                ml->addGeometryDirectly(linestring.release());
            }
            catch (osmium::geometry_error& e) {
            }
            if (!member.role()) {
                validation.message = "empty role for node member";
                member_node_id = member.ref();
            } else if (!strcmp(member.role(), "from")) {
                if (from_way && restriction != Restriction::no_entry) {
                    validation.message = "Multiple from members";
                } else {
                    from_way = way;
                }
            } else if (!strcmp(member.role(), "to")) {
                if (to_way && restriction != Restriction::no_exit) {
                    validation.message = "Multiple to members";
                } else {
                    to_way = way;
                }
            } else if (strcmp(member.role(), "via")) {
                validation.message = "Unsupported member role for way member";
            }
        } else if (member.type() == osmium::item_type::node) {
            try {
                const osmium::Node* node = this->get_member_node(member.ref());
                point = m_factory.create_point(*node);
            }
            catch (osmium::geometry_error& e) {
            }
            if (!member.role()) {
                validation.message = "empty role for node member";
            } else if (!strcmp(member.role(), "via")) {
                if (member_node_id) {
                    validation.message = "Multiple via nodes";
                } else {
                    member_node_id = member.ref();
                }
            } else {
                validation.message = "invalid role for node member";
            }
        }
    }
    if (!from_way) {
        validation.message = "From way missing";
    } else if (!to_way) {
        validation.message = "To way missing";
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
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_restrictions_n->add_field("rel_id", OFTString, 10);
    m_restrictions_n->add_field("lastchange", OFTString, 21);
    m_restrictions_n->add_field("key", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_n->add_field("value", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_n->add_field("other_tags", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_w =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, node_layer_name,
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_restrictions_w->add_field("rel_id", OFTString, 10);
    m_restrictions_w->add_field("lastchange", OFTString, 21);
    m_restrictions_w->add_field("key", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_w->add_field("value", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_restrictions_w->add_field("other_tags", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_invalid_restrictions_n =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, invalid_node_layer_name,
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_invalid_restrictions_n->add_field("rel_id", OFTString, 10);
    m_invalid_restrictions_n->add_field("lastchange", OFTString, 21);
    m_invalid_restrictions_n->add_field("message", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
    m_invalid_restrictions_w =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, invalid_way_layer_name,
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_invalid_restrictions_w->add_field("rel_id", OFTString, 10);
    m_invalid_restrictions_w->add_field("lastchange", OFTString, 21);
    m_invalid_restrictions_w->add_field("message", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
}
