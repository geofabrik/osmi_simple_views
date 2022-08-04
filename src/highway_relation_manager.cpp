/*
 * highway_relation_collector.cpp
 *
 *  Created on:  2022-08-02
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "highway_relation_manager.hpp"
#include "tagging_view_handler.hpp"

HighwayRelationManager::HighwayRelationManager(Options& options) :
        OGROutputBase(options),
        enabled(false) { }

void HighwayRelationManager::enable() {
    enabled = true;
}

bool HighwayRelationManager::new_relation(const osmium::Relation& relation) const noexcept {
    if (!enabled) {
        return false;
    }
    const char* highway = relation.tags().get_value_by_key("highway");
    if (!highway) {
        return false;
    }
    const char* type = relation.tags().get_value_by_key("type");
    if (type && !strcmp(type, "multipolygon") &&
        (!strcmp(highway, "footway") || !strcmp(highway, "pedestrian")
            || !strcmp(highway, "service") || !strcmp(highway, "rest_area")
            || !strcmp(highway, "services"))) {
        return false;
    }
    return (highway != nullptr);
}

bool HighwayRelationManager::new_member(const osmium::Relation& /*relation*/,
        const osmium::RelationMember& member, std::size_t /*n*/) const noexcept {
    return (enabled && member.type() == osmium::item_type::way);
}

void HighwayRelationManager::complete_relation(const osmium::Relation& relation) {
    process_relation(relation);
}

void HighwayRelationManager::process_relation(const osmium::Relation& relation) {
    std::unique_ptr<OGRMultiLineString> ml {new OGRMultiLineString()};
    for (const osmium::RelationMember& member : relation.members()) {
        if (member.ref() <= 0) {
            continue;
        }
        const osmium::Way* way = this->get_member_way(member.ref());
        if (!way) {
            continue;
        }
        try {
            std::unique_ptr<OGRLineString> linestring = m_factory.create_linestring(*way);
            ml->addGeometryDirectly(linestring.release());
        }
        catch (osmium::geometry_error& e) {
        }
    }
    std::unique_ptr<OGRGeometry> geom {static_cast<OGRGeometry*>(ml.release())};
    gdalcpp::Feature feature(*(m_relations_with_highway.get()), std::move(geom));
    TaggingViewHandler::set_basic_fields(feature, relation, "highway", relation.tags().get_value_by_key("highway"));
    feature.add_to_layer();
}

void HighwayRelationManager::create_layer(gdalcpp::Dataset* dataset) {
    m_relations_with_highway =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, layer_name,
            wkbMultiLineString, get_gdal_default_layer_options()));
    m_relations_with_highway->add_field("rel_id", OFTString, 10);
    m_relations_with_highway->add_field("lastchange", OFTString, 21);
    m_relations_with_highway->add_field("highway", OFTString, TaggingViewHandler::MAX_STRING_LENGTH);
}
