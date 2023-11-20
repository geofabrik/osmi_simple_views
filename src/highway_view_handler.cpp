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
#include <limits>


HighwayViewHandler::HighwayViewHandler(Options& options) :
        AbstractViewHandler(options),
        m_highway_abandoned(create_layer("highway_abandoned", wkbLineString)),
        m_highway_disused(create_layer("highway_disused", wkbLineString)),
        m_highway_construction(create_layer("highway_construction", wkbLineString)),
        m_highway_proposed(create_layer("highway_proposed", wkbLineString)),
        m_highway_multiple_lifecycle_states(create_layer("highway_multiple_lifecycles", wkbLineString)),
        m_highway_incomplete_nonop(create_layer("highway_incomplete_nonop", wkbLineString)),
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
        m_highway_long_ref(create_layer("highway_long_ref", wkbLineString)),
        m_highway_unknown_node(create_layer("highway_unknown_node", wkbPoint)),
        m_highway_unknown_way(create_layer("highway_unknown_way", wkbLineString)) {
    // add fields to layers
    m_highway_abandoned->add_field("way_id", OFTString, 10);
    m_highway_abandoned->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_abandoned->add_field("abandoned:highway", OFTString, 60);
    m_highway_disused->add_field("way_id", OFTString, 10);
    m_highway_disused->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_disused->add_field("disused:highway", OFTString, 60);
    m_highway_construction->add_field("way_id", OFTString, 10);
    m_highway_construction->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_construction->add_field("construction:highway", OFTString, 60);
    m_highway_proposed->add_field("way_id", OFTString, 10);
    m_highway_proposed->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_proposed->add_field("proposed:highway", OFTString, 60);
    m_highway_multiple_lifecycle_states->add_field("way_id", OFTString, 10);
    m_highway_multiple_lifecycle_states->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_multiple_lifecycle_states->add_field("error", OFTString, 60);
    m_highway_incomplete_nonop->add_field("way_id", OFTString, 10);
    m_highway_incomplete_nonop->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_highway_incomplete_nonop->add_field("error", OFTString, 60);
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
    m_highway_long_ref->add_field("way_id", OFTString, 10);
    m_highway_long_ref->add_field("ref", OFTString, MAX_FIELD_LENGTH);
    m_highway_long_ref->add_field("tags", OFTString, MAX_FIELD_LENGTH);
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
    register_check(highway_long_ref, "ref", m_highway_long_ref.get());
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
    m_highway_long_ref.reset();
    m_highway_unknown_node.reset();
    m_highway_unknown_way.reset();
    m_highway_multiple_lifecycle_states.reset();
    m_highway_incomplete_nonop.reset();
    m_highway_abandoned.reset();
    m_highway_disused.reset();
    m_highway_construction.reset();
    m_highway_proposed.reset();
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
        std::string tags_str = tags_string(way.tags(), "lanes");
        std::string error_msg = "invalid number ";
        error_msg += key;
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", error_msg.c_str()
        );
        return -1;
    }
    return lanes_read;
}

bool HighwayViewHandler::all_oneway(const osmium::TagList& tags) {
    const char* oneway = tags.get_value_by_key("oneway");
    const char* junction = tags.get_value_by_key("junction");
    if ((!oneway || strcmp(oneway, "yes")) && (!junction || strcmp(junction, "roundabout"))) {
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
        if (item_length == 0
                && ((start > turns && *(start - 1) == '|') || start == turns)
                && (*sep == '\0' || *sep == '|')) {
            // shortcut for direction value "none"
            found = true;
        }
        if (!found) {
            return false;
        }
        if (sep == end) {
            return true;
        }
        start = sep + 1;
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
    int lanes_both = check_lanes_value_and_write_error(way, "lanes:both_ways");
    if (lanes_both == -1) {
        return;
    }
    int lanes_sum = lanes_fwd + lanes_bkwd + lanes_both;
    int lanes_tag_count = static_cast<int>(lanes_fwd > 0) + static_cast<int>(lanes_bkwd > 0) + static_cast<int>(lanes_both > 0);
    // lanes:forward=*, lanes:backward=* and lanes:both_ways=* without lanes=*
    if (lanes_tag_count > 1 && lanes == 0) {
        std::string tags_str = selective_tags_str<3>(way.tags(), '|', {"lanes:forward", "lanes:backward", "lanes:both_ways"});
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", "NOT SET", tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "More than one of lanes:forward=*, lanes:backward=* and lanes:both_ways=* but lanes=* is missing."
        );
        return;
    }
    // check if the values make sense at all
    if (lanes_tag_count > 1 && lanes != lanes_sum) {
        std::string tags_str = selective_tags_str<3>(way.tags(), '|', {"lanes:forward", "lanes:backward", "lanes:both_ways"});
        set_fields<osmium::Way>(
                m_highway_lanes.get(), way, "lanes", way.get_value_by_key("lanes", ""), tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id", "error", "forward+backward+both_ways != both"
        );
        return;
    }
    // direction values on oneways
    bool pure_oneway = all_oneway(way.tags());
    if ((lanes_tag_count > 0) && pure_oneway) {
        std::string tags_str = selective_tags_str<4>(way.tags(), '|', {"lanes:forward", "lanes:backward", "lanes:both_ways", "oneway"});
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
    if ((way.tags().has_key("turn:lanes:forward") || way.tags().has_key("turn:lanes:forward")
            || way.tags().has_key("turn:lanes:both_ways")) && pure_oneway) {
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
    return check_oneway(oneway_value);
}

bool HighwayViewHandler::check_oneway(const char* oneway_value) {
    if (!oneway_value) {
        return true;
    }
    if (!strcmp(oneway_value, "yes") || !strcmp(oneway_value, "no") || !strcmp(oneway_value, "-1") || !strcmp(oneway_value, "alternating")) {
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
    if (!value || *value == 0) {
        return true;
    }
    if (!strcmp(value, "none") || !strcmp(value, "default") || !strcmp(value, "physical")) {
        return true;
    }
    char* rest;
    // try parsing as metric
    double maxheight_m = std::strtod(value, &rest);
    if (maxheight_m > 0 && (*rest == 0 || !strcmp(rest, " m"))) {
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
                if (maxheight_read2 > 0 && *rest2 == '"') {
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
    // If the integer is not followed by a ', it is no imperical unit.
    return false;
}

bool HighwayViewHandler::maxheight_ok(const osmium::TagList& tags) {
    const char* maxheight_value = tags.get_value_by_key("maxheight");
    // There are a couple of valid non-numeric maxheight values.
    if (maxheight_value && (
        !strcmp(maxheight_value, "none")
        || !strcmp(maxheight_value, "default")
        || !strcmp(maxheight_value, "below_default"))) {
        return true;
    }
    return check_length_value(maxheight_value);
}

bool HighwayViewHandler::maxlength_ok(const osmium::TagList& tags) {
    const char* maxlength_value = tags.get_value_by_key("maxlength");
    return check_length_value(maxlength_value);
}

bool HighwayViewHandler::maxweight_ok(const osmium::TagList& tags) {
    const char* maxweight_value = tags.get_value_by_key("maxweight");
    return check_maxweight(maxweight_value);
}

bool HighwayViewHandler::check_maxweight(const char* maxweight_value) {
    if (!maxweight_value || *maxweight_value == 0 || !strcmp(maxweight_value, "unsigned")) {
        return true;
    }
    char* rest;
    // try parsing as metric
    double maxweight_read = std::strtod(maxweight_value, &rest);
    // check for too large values
    if (maxweight_read > 0 && *rest == 0 && maxweight_read < 90) {
        return true;
    }
    // try parsing as imperial
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
    const char* noname = tags.get_value_by_key("noname");
    if (name || ref || (noname && !strcmp(noname, "yes"))) {
        return true;
    }
    const char* junction = tags.get_value_by_key("junction");
    if (junction && (!strcmp(junction, "roundabout") || !strcmp(junction, "round"))) {
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
    const char* noname = tags.get_value_by_key("noname");
    if (name || ref || (noname && !strcmp(noname, "yes"))) {
        return true;
    }
    const char* junction = tags.get_value_by_key("junction");
    if (junction && (!strcmp(junction, "roundabout") || !strcmp(junction, "round"))) {
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

bool HighwayViewHandler::highway_long_ref(const osmium::TagList& tags) {
    const char* ref = tags.get_value_by_key("ref");
    if (!ref) {
        return true;
    }
    int len = 0;
    int semicola = 0;
    const char* ptr = ref;
    for (; *ptr != 0; ++ptr) {
        semicola += (*ptr == ';');
        ++len;
    }
    return len - semicola < 18;
}

void HighwayViewHandler::ways_with_key(const osmium::Way& way, gdalcpp::Layer* layer, const char* key, const char* alternative_key) {
    const char* value = way.get_value_by_key(key);
    const char* found_key = key;
    if (!value && alternative_key) {
        value = way.get_value_by_key(alternative_key);
        found_key = alternative_key;
    }
    if (value) {
        std::string tags_str = tags_string(way.tags(), found_key);
        set_fields<osmium::Way>(layer, way, key, value, tags_str,
                [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                way.id(), "way_id");
    }
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
            || !strcmp(highway, "speed_display") || !strcmp(highway, "proposed")
            || !strcmp(highway, "platform") || !strcmp(highway, "toll_gantry")
            || !strcmp(highway, "trailhead")) {
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
            || !strcmp(highway, "elevator") || !strcmp(highway, "corridor") || !strcmp(highway, "no")
            || !strcmp(highway, "emergency_bay") || !strcmp(highway, "razed") || !strcmp(highway, "busway")) {
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

void HighwayViewHandler::highway_multiple_lifecycle_states(const osmium::Way& way) {
    const char* disused = way.get_value_by_key("disused");
    const char* disused_highway = way.get_value_by_key("disused:highway");
    const char* abandoned = way.get_value_by_key("abandoned");
    const char* abandoned_highway = way.get_value_by_key("abandoned:highway");
    const char* proposed = way.get_value_by_key("proposed");
    const char* proposed_highway = way.get_value_by_key("proposed:highway");
    const char* construction = way.get_value_by_key("construction");
    // special handling for construction=minor which is a valid tag for highways in use
    if (construction && !strcmp(construction, "minor")) {
        construction = nullptr;
    }
    const char* construction_highway = way.get_value_by_key("construction:highway");
    const char* highway = way.get_value_by_key("highway");
    constexpr size_t number = 5;
    if (!disused) {
        disused = disused_highway;
    }
    if (!abandoned) {
        abandoned = abandoned_highway;
    }
    if (!proposed) {
        proposed = proposed_highway;
    }
    if (!construction) {
        construction = construction_highway;
    }
    const std::array<const char*, number> values = {construction, proposed, abandoned, disused, highway};
    const std::array<std::string, number> keys = {"construction", "proposed", "abandoned", "disused", "highway"};
    constexpr size_t key_not_found = std::numeric_limits<size_t>::max();
    size_t last_found_key_idx = key_not_found;
    constexpr size_t highway_idx = number - 1;
    for (size_t i = 0; i < number; ++i) {
        if (values.at(i)) {
            const std::string& key = keys.at(i);
            if (last_found_key_idx != key_not_found
                    && (i < highway_idx
                    || (i == highway_idx && highway
                            && strcmp(highway, keys.at(last_found_key_idx).c_str())))) {
                // On of the previous keys had a value as well. Therefore, the ways has two lifecycle states.
                // If any out-of-life (abandoned/disused/construction/proposed) and highway is set,
                // we ensure that a present highway=* is not set to the key of the previous state.
                std::string tags_str = tags_string(way.tags(), nullptr);
                std::string error_msg {keys.at(last_found_key_idx)};
                error_msg.push_back('+');
                error_msg.append(key);
                set_fields<osmium::Way>(m_highway_multiple_lifecycle_states.get(), way, "error", error_msg.c_str(), tags_str,
                        [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                        way.id(), "way_id");
                return;
            } else {
                last_found_key_idx = i;
            }
        }
    }
    // highway=construction/disused/proposed/abandoned without matching key specifying road class
    if (last_found_key_idx == highway_idx) {
        const char* missing_nonop_key = nullptr;
        for (size_t i = 0; i < highway_idx; ++i) {
            const char* key = keys.at(i).c_str();
            if (!strcmp(highway, key) && !values.at(i)) {
                missing_nonop_key = key;
            }
        }
        if (missing_nonop_key) {
            std::string tags_str = tags_string(way.tags(), "highway");
            std::string error_msg = "highway=";
            error_msg.append(missing_nonop_key);
            error_msg.append(" without ");
            error_msg.append(missing_nonop_key);
            error_msg.append("=*");
            set_fields<osmium::Way>(m_highway_incomplete_nonop.get(), way, "error", error_msg.c_str(), tags_str,
                    [](const osmium::Way& way, ogr_factory_type& factory) {return factory.create_linestring(way);},
                    way.id(), "way_id");
        }
    }
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
        highway_multiple_lifecycle_states(way);
        check_lanes_tags(way);
        ways_with_key(way, m_highway_abandoned.get(), "abandoned:highway", "abandoned");
        ways_with_key(way, m_highway_disused.get(), "disused:highway", "disused");
        ways_with_key(way, m_highway_construction.get(), "construction:highway", "construction");
        ways_with_key(way, m_highway_proposed.get(), "proposed:highway", "proposed");
    } else {
        ways_with_key(way, m_highway_abandoned.get(), "abandoned:highway");
        ways_with_key(way, m_highway_disused.get(), "disused:highway");
        ways_with_key(way, m_highway_construction.get(), "construction:highway");
        ways_with_key(way, m_highway_proposed.get(), "proposed:highway");
    }
}

void HighwayViewHandler::node(const osmium::Node& node) {
    highway_unknown_node(node);
}
