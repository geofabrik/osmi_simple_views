/*
 * any_relation_collector.cpp
 *
 *  Created on:  2017-05-17
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "any_relation_collector.hpp"
#include "tagging_view_handler.hpp"

AnyRelationCollector::AnyRelationCollector(osmium::util::VerboseOutput& verbose_output,
        int epsg /*= 3857*/) :
        OGROutputBase(verbose_output, epsg) { }

bool AnyRelationCollector::keep_relation(const osmium::Relation& relation) const {
    const char* type = relation.get_value_by_key("type");
    return (type && (!strcmp(type, "multipolygon") || !strcmp(type, "boundary")));
}

bool AnyRelationCollector::keep_member(const osmium::relations::RelationMeta& relation_meta,
        const osmium::RelationMember& member) const {
    return (member.type() == osmium::item_type::way);
}

void AnyRelationCollector::way_not_in_any_relation(const osmium::Way& way) {
    if (way.tags().size() > 0) {
        return;
    }
    try {
        std::unique_ptr<OGRGeometry> geometry;
        geometry =m_factory.create_linestring(way);
        gdalcpp::Feature feature(*(m_tagging_ways_without_tags.get()), std::move(geometry));
        TaggingViewHandler::set_basic_fields(feature, way, nullptr, nullptr);
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_verbose_output << err.what() << "\n";
    }
}

void AnyRelationCollector::complete_relation(osmium::relations::RelationMeta& relation_meta) {}

void AnyRelationCollector::set_dataset_ptr(gdalcpp::Dataset* dataset_ptr) {
    m_dataset_ptr = dataset_ptr;
    m_tagging_ways_without_tags =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*m_dataset_ptr, "tagging_ways_without_tags", wkbLineString));

    m_tagging_ways_without_tags->add_field("way_id", OFTString, 10);
    m_tagging_ways_without_tags->add_field("lastchange", OFTString, 21);
}

void AnyRelationCollector::release_dataset() {
    m_tagging_ways_without_tags.reset();
    m_dataset_ptr = nullptr;
}
