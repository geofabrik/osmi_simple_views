/*
 * highway_view_handler.cpp
 *
 *  Created on:  2017-04-27
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "highway_view_handler.hpp"


HighwayViewHandler::HighwayViewHandler(std::string& output_filename, std::string& output_format,
        std::vector<std::string>& gdal_options, osmium::util::VerboseOutput& verbose_output,
        int epsg /*= 3857*/) :
        AbstractViewHandler(output_filename, output_format, gdal_options, verbose_output, epsg),
        m_highway_lanes(m_dataset, "highway_lanes", wkbLineString),
        m_highway_maxheight(m_dataset, "highway_maxheight", wkbLineString),
        m_highway_maxspeed(m_dataset, "highway_maxspeed", wkbLineString),
        m_highway_name_fixme(m_dataset, "highway_name_fixme", wkbLineString),
        m_highway_name_missing_major(m_dataset, "highway_name_missing_major", wkbLineString),
        m_highway_name_missing_minor(m_dataset, "highway_name_missing_minor", wkbLineString),
        m_highway_oneway(m_dataset, "highway_oneway", wkbLineString),
        m_highway_type_deprecated(m_dataset, "highway_type_deprecated", wkbLineString),
        m_highway_road(m_dataset, "highway_road", wkbLineString),
        m_highway_type_unknown(m_dataset, "highway_type_unknown", wkbLineString)
        {
    // add fields to layers
    m_highway_lanes.add_field("way_id", OFTString, 10);
    m_highway_lanes.add_field("lanes", OFTString, 40);
    m_highway_lanes.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_maxheight.add_field("way_id", OFTString, 10);
    m_highway_maxheight.add_field("maxheight", OFTString, 40);
    m_highway_maxheight.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_maxspeed.add_field("way_id", OFTString, 10);
    m_highway_maxspeed.add_field("maxspeed", OFTString, 40);
    m_highway_maxspeed.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_name_fixme.add_field("way_id", OFTString, 10);
    m_highway_name_fixme.add_field("name", OFTString, 20);
    m_highway_name_fixme.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_name_missing_major.add_field("way_id", OFTString, 10);
    m_highway_name_missing_major.add_field("highway", OFTString, 20);
    m_highway_name_missing_major.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_name_missing_minor.add_field("way_id", OFTString, 10);
    m_highway_name_missing_minor.add_field("highway", OFTString, 20);
    m_highway_name_missing_minor.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_oneway.add_field("way_id", OFTString, 10);
    m_highway_oneway.add_field("oneway", OFTString, 40);
    m_highway_oneway.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_type_deprecated.add_field("way_id", OFTString, 10);
    m_highway_type_deprecated.add_field("highway", OFTString, 40);
    m_highway_type_deprecated.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_road.add_field("way_id", OFTString, 10);
    m_highway_road.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_type_unknown.add_field("way_id", OFTString, 10);
    m_highway_type_unknown.add_field("highway", OFTString, 40);
    m_highway_type_unknown.add_field("tags", OFTString, MAX_FIELD_LENGTH);

    // register checks
    register_check(lanes_ok, "lanes", &m_highway_lanes);
    register_check(name_not_fixme, "name", &m_highway_name_fixme);
}

void HighwayViewHandler::register_check(std::function<bool (const char*)> function, std::string key, gdalcpp::Layer* layer) {
    m_checks.push_back(function);
    m_keys.push_back(key);
    m_layers.push_back(layer);
}

std::string HighwayViewHandler::tags_string(const osmium::TagList& tags, const char* not_include) {
    std::string tag_str;
    // only add tags to the tags string if their key and value are shorter than 50 characters
    for (const osmium::Tag& t : tags) {
        if (!strcmp(t.key(), not_include)) {
            continue;
        }
        size_t add_length = strlen(t.key()) + strlen(t.value()) + 2;
        if (add_length < 50 && tag_str.length() + add_length < MAX_FIELD_LENGTH) {
            tag_str += t.key();
            tag_str += '=';
            tag_str += t.value();
            tag_str += '|';
        }
    }
    // remove last | from tag_str
    tag_str.pop_back();
    return tag_str;
}

void HighwayViewHandler::set_fields(gdalcpp::Layer* layer, const osmium::Way& way, const char* third_field_name,
        const char* third_field_value, std::string& other_tags) {
    gdalcpp::Feature feature(*layer, m_factory.create_linestring(way));
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", way.id());
    feature.set_field("way_id", idbuffer);
    feature.set_field("tags", other_tags.c_str());
    if (third_field_name && third_field_value) {
        feature.set_field(third_field_name, third_field_value);
    }
    feature.add_to_layer();
}

bool HighwayViewHandler::lanes_ok(const char* lanes_value) {
    char* rest;
    long int lanes_read = std::strtol(lanes_value, &rest, 10);
    if (*rest || lanes_read <= 0 || lanes_read > 16) {
        return false;
    }
    return true;
}

bool HighwayViewHandler::name_not_fixme(const char* name_value) {
    if (!strcmp(name_value, "fixme") || !strcmp(name_value, "unknown")) {
        return false;
    }
    return true;
}

void HighwayViewHandler::check_them_all(const osmium::Way& way) {
    for (size_t i = 0; i < m_layers.size(); ++i) {
        const char* value = way.get_value_by_key(m_keys.at(i).c_str());
        if (value && !m_checks.at(i)(value)) {
            std::string tags_str = tags_string(way.tags(), m_keys.at(i).c_str());
            set_fields(m_layers.at(i), way, m_keys.at(i).c_str(), value, tags_str);
        }
    }
}

void HighwayViewHandler::way(const osmium::Way& way) {
    if (way.get_value_by_key("highway")) {
        check_them_all(way);
    }
}
