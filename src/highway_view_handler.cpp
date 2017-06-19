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


#include "highway_view_handler.hpp"


HighwayViewHandler::HighwayViewHandler(std::string& output_filename, std::string& output_format,
        osmium::util::VerboseOutput& verbose_output, int epsg /*= 3857*/) :
        AbstractViewHandler(output_filename, output_format, verbose_output, epsg),
        m_highway_lanes(m_dataset, "highway_lanes", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_maxheight(m_dataset, "highway_maxheight", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_maxspeed(m_dataset, "highway_maxspeed", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_name_fixme(m_dataset, "highway_name_fixme", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_name_missing_major(m_dataset, "highway_name_missing_major", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_name_missing_minor(m_dataset, "highway_name_missing_minor", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_oneway(m_dataset, "highway_oneway", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_road(m_dataset, "highway_road", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_highway_type_unknown(m_dataset, "highway_type_unknown", wkbLineString, GDAL_DEFAULT_OPTIONS)
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
    m_highway_road.add_field("way_id", OFTString, 10);
    m_highway_road.add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_type_unknown.add_field("way_id", OFTString, 10);
    m_highway_type_unknown.add_field("highway", OFTString, 40);
    m_highway_type_unknown.add_field("tags", OFTString, MAX_FIELD_LENGTH);

    // register checks
    register_check(lanes_ok, "lanes", &m_highway_lanes);
    register_check(name_not_fixme, "name", &m_highway_name_fixme);
    register_check(oneway_ok, "oneway", &m_highway_oneway);
    register_check(maxheight_ok, "maxheight", &m_highway_maxheight);
    register_check(maxspeed_ok, "maxspeed", &m_highway_maxspeed);
    register_check(name_missing_major, "highway", &m_highway_name_missing_major);
    register_check(name_missing_minor, "highway", &m_highway_name_missing_minor);
    register_check(highway_road, "", &m_highway_road);
    register_check(highway_unknown, "highway", &m_highway_type_unknown);
}

void HighwayViewHandler::register_check(std::function<bool (const osmium::TagList&)> function, std::string key, gdalcpp::Layer* layer) {
    m_checks.push_back(function);
    m_keys.push_back(key);
    m_layers.push_back(layer);
}

bool HighwayViewHandler::is_valid_const_speed(const char* maxspeed_value) {
    if (!strcmp(maxspeed_value, "RO:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "none")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RU:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RU:rural")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RO:rural")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RU:living_street")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RO:trunk")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RU:motorway")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "AT:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "DE:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "UA:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "AT:rural")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "UA:rural")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "IT:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RO:motorway")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "DE:rural")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "CZ:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "RU:urban")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "walk")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "AT:motorway")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "IT:rural")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "DE:living_street")) {
        return true;
    }
    if (!strcmp(maxspeed_value, "DE:walk")) {
        return true;
    }
    return false;
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
    try {
        gdalcpp::Feature feature(*layer, m_factory.create_linestring(way));
        static char idbuffer[20];
        sprintf(idbuffer, "%ld", way.id());
        feature.set_field("way_id", idbuffer);
        feature.set_field("tags", other_tags.c_str());
        if (third_field_name && third_field_value) {
            feature.set_field(third_field_name, third_field_value);
        }
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_verbose_output << err.what() << "\n";
    }
}

bool HighwayViewHandler::lanes_ok(const osmium::TagList& tags) {
    const char* lanes_value = tags.get_value_by_key("lanes");
    if (!lanes_value) {
        return true;
    }
    char* rest;
    long int lanes_read = std::strtol(lanes_value, &rest, 10);
    if (*rest || lanes_read <= 0 || lanes_read > 16) {
        return false;
    }
    return true;
}

bool HighwayViewHandler::name_not_fixme(const osmium::TagList& tags) {
    const char* name_value = tags.get_value_by_key("name");
    if (!name_value) {
        return true;
    }
    if (!strcmp(name_value, "fixme") || !strcmp(name_value, "unknown")) {
        return false;
    }
    // check for question marks
    if (strchr(name_value, '?')) {
        return false;
    }
    return true;
}

bool HighwayViewHandler::oneway_ok(const osmium::TagList& tags) {
    const char* oneway_value = tags.get_value_by_key("oneway");
    if (!oneway_value) {
        return true;
    }
    if (!strcmp(oneway_value, "yes") || !strcmp(oneway_value, "no") || !strcmp(oneway_value, "-1")) {
        return true;
    }
    return false;
}

bool HighwayViewHandler::maxspeed_ok(const osmium::TagList& tags) {
    const char* maxspeed_value = tags.get_value_by_key("maxspeed");
    if (!maxspeed_value) {
        return true;
    }
    char* rest;
    long int maxspeed_read = std::strtol(maxspeed_value, &rest, 10);
    if (*rest) { // something behind the number
        if (!strcmp(rest, " mph") && maxspeed_read <= 112 && maxspeed_read > 0) {
            return true;
        }
        if (maxspeed_value == rest && (!strcmp(rest, " none") || !strcmp(rest, "signals"))) {
            return true;
        }
    } else if (maxspeed_read > 0 && maxspeed_read <= 150) {
        return true;
    }
    // check for national contants
    return is_valid_const_speed(maxspeed_value);
}

bool HighwayViewHandler::maxheight_ok(const osmium::TagList& tags) {
    const char* maxheight_value = tags.get_value_by_key("maxheight");
    if (!maxheight_value) {
        return true;
    }
    if (!strcmp(maxheight_value, "none") || !strcmp(maxheight_value, "default") || !strcmp(maxheight_value, "physical")) {
        return true;
    }
    char* rest;
    // try parsing as metric
    double maxheight_m = std::strtod(maxheight_value, &rest);
    if (maxheight_m > 0 && *rest == 0) {
        return true;
    }
    // try parsing as imperial
    long int maxheight_read = std::strtol(maxheight_value, &rest, 10);
    if (*rest) { // something behind the number
        if (*rest == '\'' && maxheight_read > 0) {
            ++rest; // move one character to the right
            if (isdigit(*rest)) {
                char* rest2;
                double maxheight_read2 = std::strtod(rest, &rest2);
                if (maxheight_read2 > 0 || *rest2 == '"') {
                    return true;
                }
                return false;
            } else if (*rest == 0) {
                // ' followed by nothing
                return true;
            } else if (*rest) {
                // ' is followed by a non-numeric character
                return false;
            }
        }
    }
    // This shouldn't happen and we consider this as an error.
    // If th integer is not followed by a ', it is no imperical unit.
    return false;
}

bool HighwayViewHandler::name_missing_major(const osmium::TagList& tags) {
    const char* name = tags.get_value_by_key("name");
    const char* ref = tags.get_value_by_key("ref");
    if (name || ref) {
        return true;
    }
    const char* highway = tags.get_value_by_key("highway");
    if (strcmp(highway, "motorway") != 0 && strcmp(highway, "trunk") != 0 && strcmp(highway, "primary") != 0
            && strcmp(highway, "secondary") != 0 && strcmp(highway, "tertiary") != 0) {
        return true;
    }
    return false;
}

bool HighwayViewHandler::name_missing_minor(const osmium::TagList& tags) {
    const char* name = tags.get_value_by_key("name");
    const char* ref = tags.get_value_by_key("ref");
    if (name || ref) {
        return true;
    }
    const char* highway = tags.get_value_by_key("highway");
    if (strcmp(highway, "residential") != 0 && strcmp(highway, "living_street") != 0 && strcmp(highway, "pedestrian") != 0) {
        return true;
    }
    return false;
}

bool HighwayViewHandler::highway_road(const osmium::TagList& tags) {
    const char* highway = tags.get_value_by_key("highway");
    if (highway && !strcmp(highway, "road")) {
        return false;
    }
    return true;
}

bool HighwayViewHandler::highway_unknown(const osmium::TagList& tags) {
    const char* highway = tags.get_value_by_key("highway");
    if (!strcmp(highway, "motorway") || !strcmp(highway, "motorway_link") || !strcmp(highway, "trunk")
            || !strcmp(highway, "trunk_link") || !strcmp(highway, "primary") || !strcmp(highway, "primary_link")
            || !strcmp(highway, "secondary") || !strcmp(highway, "secondary_link") || !strcmp(highway, "tertiary")
            || !strcmp(highway, "tertiary_link") || !strcmp(highway, "residential")
            || !strcmp(highway, "living_street") || !strcmp(highway, "pedestrian") || !strcmp(highway, "unclassified")
            || !strcmp(highway, "service") || !strcmp(highway, "track") || !strcmp(highway, "path")
            || !strcmp(highway, "footway") || !strcmp(highway, "cycleway") || !strcmp(highway, "bridleway")
            || !strcmp(highway, "steps") || !strcmp(highway, "raceway") || !strcmp(highway, "bus_guideway")
            || !strcmp(highway, "construction") || !strcmp(highway, "disused") || !strcmp(highway, "abandoned")
            || !strcmp(highway, "proposed") || !strcmp(highway, "platform") || !strcmp(highway, "road")) {
        return true;
    }
    return false;
}

void HighwayViewHandler::check_them_all(const osmium::Way& way) {
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (!m_checks.at(i)(way.tags())) {
            std::string tags_str = tags_string(way.tags(), m_keys.at(i).c_str());
            const char* value = way.get_value_by_key(m_keys.at(i).c_str());
            set_fields(m_layers.at(i), way, m_keys.at(i).c_str(), value, tags_str);
        }
    }
}

void HighwayViewHandler::way(const osmium::Way& way) {
    if (way.get_value_by_key("highway")) {
        check_them_all(way);
    }
}
