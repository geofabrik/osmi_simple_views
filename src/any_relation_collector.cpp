/*
 *  © 2017 Geofabrik GmbH
 *
 *  This file is part of osmi_simple_views.
 *
 *  osmi_simple_views is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  osmi_simple_views is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with osmi_simple_views. If not, see <http://www.gnu.org/licenses/>.
 */


#include "any_relation_collector.hpp"
#include "tagging_view_handler.hpp"

AnyRelationCollector::AnyRelationCollector(Options& options) :
        OGROutputBase(options) { }

bool AnyRelationCollector::keep_relation(const osmium::Relation& relation) const {
    // whitelisted route=piste/ski/ferry because both can contain member ways without tags.
    return relation.tags().has_tag("type", "multipolygon")
            || relation.tags().has_tag("type", "boundary")
            || (relation.tags().has_tag("type", "route") && relation.tags().has_tag("route", "piste"))
            || (relation.tags().has_tag("type", "route") && relation.tags().has_tag("route", "ski"))
            || (relation.tags().has_tag("type", "route") && relation.tags().has_tag("route", "ferry"));
}

bool AnyRelationCollector::keep_member(const osmium::relations::RelationMeta&,
        const osmium::RelationMember& member) const {
    return (member.type() == osmium::item_type::way);
}

void AnyRelationCollector::way_not_in_any_relation(const osmium::Way& way) {
    if (way.tags().size() > 0 || !m_tagging_ways_without_tags || !coordinates_valid(way.nodes())) {
        // m_tagging_ways_without_tags is empty if AnyRelationCollector was initialized but no tagging view should be produced
        return;
    }
    try {
        std::unique_ptr<OGRGeometry> geometry;
        geometry =m_factory.create_linestring(way);
        gdalcpp::Feature feature(*(m_tagging_ways_without_tags.get()), std::move(geometry));
        TaggingViewHandler::set_basic_fields(feature, way, nullptr, nullptr);
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_options.verbose_output << err.what() << "\n";
    }
}

void AnyRelationCollector::complete_relation(osmium::relations::RelationMeta&) {}

void AnyRelationCollector::create_layer(gdalcpp::Dataset* dataset) {
    m_tagging_ways_without_tags =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*dataset, "tagging_ways_without_tags",
            wkbLineString, get_gdal_default_layer_options()));

    m_tagging_ways_without_tags->add_field("way_id", OFTString, 10);
    m_tagging_ways_without_tags->add_field("lastchange", OFTString, 21);
}
