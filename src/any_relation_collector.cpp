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

AnyRelationCollector::AnyRelationCollector(osmium::util::VerboseOutput& verbose_output, std::string& output_format,
        int epsg  /*= 3857*/) :
        OGROutputBase(verbose_output, output_format, epsg) { }

bool AnyRelationCollector::keep_relation(const osmium::Relation& relation) const {
    const char* type = relation.get_value_by_key("type");
    return (type && (!strcmp(type, "multipolygon") || !strcmp(type, "boundary")));
}

bool AnyRelationCollector::keep_member(const osmium::relations::RelationMeta&,
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

void AnyRelationCollector::complete_relation(osmium::relations::RelationMeta&) {}

void AnyRelationCollector::set_dataset_ptr(gdalcpp::Dataset* dataset_ptr) {
    m_dataset_ptr = dataset_ptr;
    m_tagging_ways_without_tags =
            std::unique_ptr<gdalcpp::Layer>(new gdalcpp::Layer(*m_dataset_ptr, "tagging_ways_without_tags",
            wkbLineString, GDAL_DEFAULT_OPTIONS));

    m_tagging_ways_without_tags->add_field("way_id", OFTString, 10);
    m_tagging_ways_without_tags->add_field("lastchange", OFTString, 21);
}

void AnyRelationCollector::release_dataset() {
    m_tagging_ways_without_tags.reset();
    m_dataset_ptr = nullptr;
}
