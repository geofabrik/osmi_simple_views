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


HighwayViewHandler::HighwayViewHandler(Options& options) :
        AbstractViewHandler(options),
        m_highway_lanes(create_layer("highway_lanes", wkbLineString)),
        m_highway_maxheight(create_layer("highway_maxheight", wkbLineString)),
        m_highway_maxweight(create_layer("highway_maxweight", wkbLineString)),
        m_highway_maxlength(create_layer("highway_maxlength", wkbLineString)),
        m_highway_maxspeed(create_layer("highway_maxspeed", wkbLineString)),
        m_highway_name_fixme(create_layer("highway_name_fixme", wkbLineString)),
        m_highway_name_missing_major(create_layer("highway_name_missing_major", wkbLineString)),
        m_highway_name_missing_minor(create_layer("highway_name_missing_minor", wkbLineString)),
        m_highway_oneway(create_layer("highway_oneway", wkbLineString)),
        m_highway_road(create_layer("highway_road", wkbLineString)),
        m_highway_unknown_node(create_layer("highway_unknown_node", wkbPoint)),
        m_highway_unknown_way(create_layer("highway_unknown_way", wkbLineString)) {
    // add fields to layers
    m_highway_lanes->add_field("way_id", OFTString, 10);
    m_highway_lanes->add_field("lanes", OFTString, 40);
    m_highway_lanes->add_field("error", OFTString, 80);
    m_highway_lanes->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_maxheight->add_field("way_id", OFTString, 10);
    m_highway_maxheight->add_field("maxheight", OFTString, 40);
    m_highway_maxheight->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_maxweight->add_field("way_id", OFTString, 10);
    m_highway_maxweight->add_field("maxweight", OFTString, 40);
    m_highway_maxweight->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_maxlength->add_field("way_id", OFTString, 10);
    m_highway_maxlength->add_field("maxlength", OFTString, 40);
    m_highway_maxlength->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_maxspeed->add_field("way_id", OFTString, 10);
    m_highway_maxspeed->add_field("maxspeed", OFTString, 40);
    m_highway_maxspeed->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_name_fixme->add_field("way_id", OFTString, 10);
    m_highway_name_fixme->add_field("name", OFTString, 20);
    m_highway_name_fixme->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_name_missing_major->add_field("way_id", OFTString, 10);
    m_highway_name_missing_major->add_field("highway", OFTString, 20);
    m_highway_name_missing_major->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_name_missing_minor->add_field("way_id", OFTString, 10);
    m_highway_name_missing_minor->add_field("highway", OFTString, 20);
    m_highway_name_missing_minor->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_oneway->add_field("way_id", OFTString, 10);
    m_highway_oneway->add_field("oneway", OFTString, 40);
    m_highway_oneway->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_road->add_field("way_id", OFTString, 10);
    m_highway_road->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_unknown_node->add_field("node_id", OFTString, 10);
    m_highway_unknown_node->add_field("highway", OFTString, 40);
    m_highway_unknown_node->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_unknown_way->add_field("way_id", OFTString, 10);
    m_highway_unknown_way->add_field("highway", OFTString, 40);
    m_highway_unknown_way->add_field("tags", OFTString, MAX_FIELD_LENGTH);

    // register checks
    register_check(name_not_fixme, "name", m_highway_name_fixme.get());
    register_check(oneway_ok, "oneway", m_highway_oneway.get());
    register_check(maxheight_ok, "maxheight", m_highway_maxheight.get());
    register_check(maxweight_ok, "maxweight", m_highway_maxweight.get());
    register_check(maxlength_ok, "maxlength", m_highway_maxlength.get());
    register_check(maxspeed_ok, "maxspeed", m_highway_maxspeed.get());
    register_check(name_missing_major, "highway", m_highway_name_missing_major.get());
    register_check(name_missing_minor, "highway", m_highway_name_missing_minor.get());
    register_check(highway_road, "", m_highway_road.get());
}

void HighwayViewHandler::give_correct_name() {
    rename_output_files("highways");
}

void HighwayViewHandler::close() {
    m_highway_lanes.reset();
    m_highway_maxheight.reset();
    m_highway_maxspeed.reset();
    m_highway_name_fixme.reset();
    m_highway_name_missing_major.reset();
    m_highway_name_missing_minor.reset();
    m_highway_oneway.reset();
    m_highway_road.reset();
    m_highway_unknown_node.reset();
    m_highway_unknown_way.reset();
    close_datasets();
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

void HighwayViewHandler::set_fields(gdalcpp::Layer* layer, const osmium::Way& way, const char* third_field_name,
        const char* third_field_value, std::string& other_tags) {
    set_fields<osmium::Way>(
            layer, way, third_field_name, third_field_value, other_tags,
            [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
            way.id(), "way_id"
    );
}

int HighwayViewHandler::check_lanes_value_and_write_error(const osmium::Way& way, const char* key) {
    const char* lanes_value = way.get_value_by_key(key);
    if (!lanes_value) {
        return 0;
    }
    char* rest;
    long int lanes_read = std::strtol(lanes_value, &rest, 10);
    if (*rest || lanes_read <= 0 || lanes_read > 16) {
        std::string tags_str = tags_string(way.tags(), key);
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", lanes_value, tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "invalid number"
        );
        return -1;
    }
    return lanes_read;
}

bool HighwayViewHandler::all_oneway(const osmium::TagList& tags) {
    const char* oneway = tags.get_value_by_key("oneway");
    if (!oneway || strcmp(oneway, "yes")) {
        return false;
    }
    // check for keys starting with "oneway:"
    for (const auto& t : tags) {
        if (!strncmp(t.key(), "oneway:", 7) && !strcmp(t.value(), "no")) {
            return false;
        }
    }
    return true;
}

int HighwayViewHandler::pipe_separated_items_count(const char* value) {
    if (!value) {
        return 0;
    }
    int count = 1;
    for (const char* ptr = value; *ptr != 0; ++ptr) {
        if (*ptr == '|') {
            ++count;
        }
    }
    return count;
}

bool HighwayViewHandler::check_valid_turns(const char* turns) {
    const std::vector<std::pair<const std::string, const size_t>> valid_turns = {
            {"left", 4},
            {"through", 7},
            {"right", 5},
            {"slight_left", 11},
            {"slight_right", 12},
            {"sharp_left", 10},
            {"sharp_right", 11},
            {"reverse", 7},
            {"merge_to_left", 13},
            {"merge_to_right", 14},
            {"none", 4}
    };
    if (!turns) {
        return true;
    }
    const char* start = turns;
    const char* end = strchr(start, '\0');
    if (end - start < 4) {
        return false;
    }
    while (start < end) {
        const char* sep = strchr(start, '|');
        const char* sep_semicolon = strchr(start, ';');
        if (!sep && !sep_semicolon) {
            sep = end;
        } else if ((sep && sep_semicolon && sep_semicolon < sep) || !sep) {
            sep = sep_semicolon;
        }
        bool found = false;
        size_t item_length = sep - start;
        for (auto vp : valid_turns) {
            if (item_length == vp.second && !strncmp(start, vp.first.c_str(), vp.second)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
        if (sep == end) {
            return true;
        }
        start = sep + 1;
    }
    // The last character in the string is a separator.
    if (start >= end) {
        return false;
    }
    return true;
}

void HighwayViewHandler::check_lanes_tags(const osmium::Way& way) {
    int lanes = check_lanes_value_and_write_error(way, "lanes");
    if (lanes == -1) {
        return;
    }
    // If any of these checks fail, further checks don't make sense.
    int lanes_fwd = check_lanes_value_and_write_error(way, "lanes:forward");
    if (lanes_fwd == -1) {
        return;
    }
    int lanes_bkwd = check_lanes_value_and_write_error(way, "lanes:backward");
    if (lanes_bkwd == -1) {
        return;
    }
    // lanes:forward=* and lanes:backward=* without lanes=*
    if (lanes_fwd > 0 && lanes_bkwd > 0 && lanes == 0) {
        std::string tags_str = selective_tags_str<2>(way.tags(), '|', {"lanes:forward", "lanes:backward"});
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", "NOT SET", tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "lanes:forward=* and lanes:backward=* without lanes=*"
        );
        return;
    }
    // check if the values make sense at all
    if (lanes_fwd > 0 && lanes_bkwd > 0 && lanes != lanes_fwd + lanes_bkwd) {
        std::string tags_str = selective_tags_str<2>(way.tags(), '|', {"lanes:forward", "lanes:backward"});
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "forward+backward != both"
        );
        return;
    }
    // direction values on oneways
    bool pure_oneway = all_oneway(way.tags());
    if ((lanes_fwd > 0 || lanes_bkwd > 0) && pure_oneway) {
        std::string tags_str = selective_tags_str<3>(way.tags(), '|', {"lanes:forward", "lanes:backward", "oneway"});
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "direction dependent value given although road is oneway"
        );
        return;
    }
    // check if turn:lanes is present on bidirectional ways
    std::string all_tags_str = tags_string(way.tags(), "highway");
    if (way.tags().has_key("turn:lanes") && !pure_oneway) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes on bidirectional way"
        );
        return;
    }
    if ((way.tags().has_key("turn:lanes:forward") || way.tags().has_key("turn:lanes:forward")) && pure_oneway) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "unneccessary direction-dependent turn:lanes on oneway"
        );
        return;
    }
    // number of lanes vs. turn:lanes
    const char* turn_lanes_value = way.get_value_by_key("turn:lanes");
    int turn_lanes_count = pipe_separated_items_count(turn_lanes_value);
    if (turn_lanes_count > 0 && lanes == 0) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes without lanes=*"
        );
        return;
    }
    if (turn_lanes_count > 0 && turn_lanes_count < lanes) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes contains too few lanes"
        );
        return;
    }
    if (!check_valid_turns(turn_lanes_value)) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes contains invalid directions"
        );
        return;
    }
    // forward
    const char* turn_lanes_value_fwd = way.get_value_by_key("turn:lanes:forward");
    int turn_lanes_count_fwd = pipe_separated_items_count(turn_lanes_value_fwd);
    if (turn_lanes_count_fwd > 0 && lanes_fwd == 0) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes:forward without lanes:forward=*"
        );
        return;
    }
    if (turn_lanes_count_fwd > 0 && turn_lanes_count_fwd < lanes_fwd) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes:forward contains too few lanes"
        );
        return;
    }
    if (!check_valid_turns(turn_lanes_value_fwd)) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes:forward contains invalid directions"
        );
        return;
    }
    // backward
    const char* turn_lanes_value_bkwd = way.get_value_by_key("turn:lanes:backward");
    int turn_lanes_count_bkwd = pipe_separated_items_count(turn_lanes_value_bkwd);
    if (turn_lanes_count_bkwd > 0 && lanes_bkwd == 0) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes:backward without lanes:backward=*"
        );
        return;
    }
    if (turn_lanes_count_bkwd > 0 && turn_lanes_count_bkwd < lanes_bkwd) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes:backward contains too few lanes"
        );
        return;
    }
    if (!check_valid_turns(turn_lanes_value_bkwd)) {
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), all_tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "turn:lanes:backward contains invalid directions"
        );
        return;
    }
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

bool HighwayViewHandler::check_length_value(const char* value) {
    if (!value) {
        return true;
    }
    if (!strcmp(value, "none") || !strcmp(value, "default") || !strcmp(value, "physical")) {
        return true;
    }
    char* rest;
    // try parsing as metric
    double maxheight_m = std::strtod(value, &rest);
    if (maxheight_m > 0 && *rest == 0) {
        return true;
    }
    // try parsing as imperial
    long int maxheight_read = std::strtol(value, &rest, 10);
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

bool HighwayViewHandler::maxheight_ok(const osmium::TagList& tags) {
    const char* maxheight_value = tags.get_value_by_key("maxheight");
    return check_length_value(maxheight_value);
}

bool HighwayViewHandler::maxlength_ok(const osmium::TagList& tags) {
    const char* maxlength_value = tags.get_value_by_key("maxlength");
    return check_length_value(maxlength_value);
}

bool HighwayViewHandler::maxweight_ok(const osmium::TagList& tags) {
    const char* maxweight_value = tags.get_value_by_key("maxweight");
    if (!maxweight_value || !strcmp(maxweight_value, "unsigned")) {
        return true;
    }
    char* rest;
    // try parsing as metric
    double maxweight_metric = std::strtod(maxweight_value, &rest);
    // check for too large values
    if (maxweight_metric > 0 && *rest == 0 && maxweight_metric < 90) {
        return true;
    }
    // try parsing as imperial
    long int maxweight_read = std::strtol(maxweight_value, &rest, 10);
    if (*rest) { // something behind the number
        if (*rest == ' ' && maxweight_read > 0) {
            ++rest; // move one character to the right
            if (!strcmp(rest, "t") && maxweight_read < 90) { // metric tonne
                return true;
            }
            if (!strcmp(rest, "st") && maxweight_read < 90 * 0.9071847) {
                return true;
            }
            if (!strcmp(rest, "lt") && maxweight_read < 90 * 1.016047) {
                return true;
            }
            if (!strcmp(rest, "kg") && maxweight_read < 90000) {
                return true;
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
    const char* tiger_reviewed = tags.get_value_by_key("tiger:reviewed");
    const char* name = tags.get_value_by_key("name");
    const char* ref = tags.get_value_by_key("ref");
    if (name || ref) {
        return true;
    }
    // return true (i.e. don't write to output file) if they are unreviewed ways from TIGER import
    if (tiger_reviewed && !strcmp(tiger_reviewed, "no")) {
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

void HighwayViewHandler::highway_unknown_node(const osmium::Node& node) {
    const char* highway = node.get_value_by_key("highway");
    if (!highway) {
        return;
    }
    if (!strcmp(highway, "bus_stop") || !strcmp(highway, "motorway_junction")
            || !strcmp(highway, "services") || !strcmp(highway, "checkpoint")
            || !strcmp(highway, "construction") || !strcmp(highway, "turning_circle")
            || !strcmp(highway, "rest_area") || !strcmp(highway, "crossing")
            || !strcmp(highway, "traffic_signals") || !strcmp(highway, "street_lamp")
            || !strcmp(highway, "stop") || !strcmp(highway, "give_way")
            || !strcmp(highway, "milestone") || !strcmp(highway, "turning_loop")
            || !strcmp(highway, "mini_roundabout") || !strcmp(highway, "speed_camera")
            || !strcmp(highway, "emergency_access_point") || !strcmp(highway, "elevator")
            || !strcmp(highway, "passing_place") || !strcmp(highway, "traffic_mirror")
            || !strcmp(highway, "emergency_bay") || !strcmp(highway, "ford")
            || !strcmp(highway, "speed_display") || !strcmp(highway, "proposed")) {
        return;
    }
    std::string tags_str = tags_string(node.tags(), "highway");
    set_fields<osmium::Node>(m_highway_unknown_node.get(), node, "highway", highway, tags_str,
            [](const osmium::Node& node, ogr_factory_type& factory) {return factory.create_point(node);},
            node.id(), "node_id");
}

void HighwayViewHandler::highway_unknown_way(const osmium::Way& way) {
    const char* highway = way.get_value_by_key("highway");
    if (!highway) {
        return;
    }
    if (!strcmp(highway, "motorway") || !strcmp(highway, "motorway_link") || !strcmp(highway, "trunk")
            || !strcmp(highway, "trunk_link") || !strcmp(highway, "primary") || !strcmp(highway, "primary_link")
            || !strcmp(highway, "secondary") || !strcmp(highway, "secondary_link") || !strcmp(highway, "tertiary")
            || !strcmp(highway, "tertiary_link") || !strcmp(highway, "residential")
            || !strcmp(highway, "living_street") || !strcmp(highway, "pedestrian") || !strcmp(highway, "unclassified")
            || !strcmp(highway, "service") || !strcmp(highway, "track") || !strcmp(highway, "path")
            || !strcmp(highway, "footway") || !strcmp(highway, "cycleway") || !strcmp(highway, "bridleway")
            || !strcmp(highway, "steps") || !strcmp(highway, "raceway") || !strcmp(highway, "bus_guideway")
            || !strcmp(highway, "construction") || !strcmp(highway, "disused") || !strcmp(highway, "abandoned")
            || !strcmp(highway, "proposed") || !strcmp(highway, "platform") || !strcmp(highway, "road")
            || !strcmp(highway, "elevator") || !strcmp(highway, "corridor") || !strcmp(highway, "no")) {
        return;
    }
    if (way.is_closed() && (!strcmp(highway, "services") || !strcmp(highway, "rest_area") || !strcmp(highway, "traffic_island"))) {
        return;
    }
    std::string tags_str = tags_string(way.tags(), "highway");
    set_fields<osmium::Way>(m_highway_unknown_way.get(), way, "highway", highway, tags_str,
            [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
            way.id(), "way_id");
}

void HighwayViewHandler::check_them_all(const osmium::Way& way) {
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (!m_checks.at(i)(way.tags())) {
            if (!all_nodes_valid(way.nodes())) {
                return;
            }
            std::string tags_str = tags_string(way.tags(), m_keys.at(i).c_str());
            const char* value = way.get_value_by_key(m_keys.at(i).c_str());
            set_fields(m_layers.at(i), way, m_keys.at(i).c_str(), value, tags_str);
        }
    }
}

void HighwayViewHandler::way(const osmium::Way& way) {
    if (way.get_value_by_key("highway")) {
        check_them_all(way);
        highway_unknown_way(way);
        check_lanes_tags(way);
    }
}

void HighwayViewHandler::node(const osmium::Node& node) {
    highway_unknown_node(node);
}
