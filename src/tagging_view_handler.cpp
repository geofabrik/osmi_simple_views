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

#include "tagging_view_handler.hpp"

TaggingViewHandler::TaggingViewHandler(std::string& output_filename, std::string& output_format,
        osmium::util::VerboseOutput& verbose_output, int epsg) :
        AbstractViewHandler(output_filename, output_format, verbose_output, epsg),
        m_tagging_fixmes_on_nodes(m_dataset, "tagging_fixmes_on_nodes", wkbPoint, GDAL_DEFAULT_OPTIONS),
        m_tagging_fixmes_on_ways(m_dataset, "tagging_fixmes_on_ways", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_tagging_nodes_with_empty_k(m_dataset, "tagging_nodes_with_empty_k", wkbPoint, GDAL_DEFAULT_OPTIONS),
        m_tagging_ways_with_empty_k(m_dataset, "tagging_ways_with_empty_k", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_tagging_nodes_with_empty_v(m_dataset, "tagging_nodes_with_empty_v", wkbPoint, GDAL_DEFAULT_OPTIONS),
        m_tagging_ways_with_empty_v(m_dataset, "tagging_ways_with_empty_v", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_tagging_misspelled_node_keys(m_dataset, "tagging_misspelled_node_keys", wkbPoint, GDAL_DEFAULT_OPTIONS),
        m_tagging_misspelled_way_keys(m_dataset, "tagging_misspelled_way_keys", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_tagging_nonop_confusion_nodes(m_dataset, "tagging_nonop_confusion_nodes", wkbPoint, GDAL_DEFAULT_OPTIONS),
        m_tagging_nonop_confusion_ways(m_dataset, "tagging_nonop_confusion_ways", wkbLineString, GDAL_DEFAULT_OPTIONS),
        m_tagging_no_feature_tag_nodes(m_dataset, "tagging_no_feature_tag_nodes", wkbPoint, GDAL_DEFAULT_OPTIONS),
        m_tagging_no_feature_tag_ways(m_dataset, "tagging_no_feature_tag_ways", wkbLineString, GDAL_DEFAULT_OPTIONS)
{
    m_tagging_fixmes_on_nodes.add_field("node_id", OFTString, 10);
    m_tagging_fixmes_on_nodes.add_field("tag", OFTString, MAX_STRING_LENGTH);
    m_tagging_fixmes_on_nodes.add_field("lastchange", OFTString, 21);
    m_tagging_fixmes_on_ways.add_field("way_id", OFTString, 10);
    m_tagging_fixmes_on_ways.add_field("tag", OFTString, MAX_STRING_LENGTH);
    m_tagging_fixmes_on_ways.add_field("lastchange", OFTString, 21);
    m_tagging_nodes_with_empty_k.add_field("node_id", OFTString, 10);
    m_tagging_nodes_with_empty_k.add_field("value", OFTString, MAX_STRING_LENGTH);
    m_tagging_nodes_with_empty_k.add_field("lastchange", OFTString, 21);
    m_tagging_ways_with_empty_k.add_field("way_id", OFTString, 10);
    m_tagging_ways_with_empty_k.add_field("value", OFTString, MAX_STRING_LENGTH);
    m_tagging_ways_with_empty_k.add_field("lastchange", OFTString, 21);
    m_tagging_nodes_with_empty_v.add_field("node_id", OFTString, 10);
    m_tagging_nodes_with_empty_v.add_field("key", OFTString, MAX_STRING_LENGTH);
    m_tagging_nodes_with_empty_v.add_field("lastchange", OFTString, 21);
    m_tagging_ways_with_empty_v.add_field("way_id", OFTString, 10);
    m_tagging_ways_with_empty_v.add_field("key", OFTString, MAX_STRING_LENGTH);
    m_tagging_ways_with_empty_v.add_field("lastchange", OFTString, 21);
    m_tagging_misspelled_node_keys.add_field("node_id", OFTString, 10);
    m_tagging_misspelled_node_keys.add_field("key", OFTString, MAX_STRING_LENGTH);
    m_tagging_misspelled_node_keys.add_field("error", OFTString, 20);
    m_tagging_misspelled_node_keys.add_field("otherkey", OFTString, MAX_STRING_LENGTH);
    m_tagging_misspelled_node_keys.add_field("lastchange", OFTString, 21);
    m_tagging_misspelled_way_keys.add_field("way_id", OFTString, 10);
    m_tagging_misspelled_way_keys.add_field("key", OFTString, MAX_STRING_LENGTH);
    m_tagging_misspelled_way_keys.add_field("error", OFTString, 20);
    m_tagging_misspelled_way_keys.add_field("otherkey", OFTString, MAX_STRING_LENGTH);
    m_tagging_misspelled_way_keys.add_field("lastchange", OFTString, 21);
    m_tagging_nonop_confusion_nodes.add_field("node_id", OFTString, 10);
    m_tagging_nonop_confusion_nodes.add_field("tags", OFTString, MAX_STRING_LENGTH);
    m_tagging_nonop_confusion_nodes.add_field("lastchange", OFTString, 21);
    m_tagging_nonop_confusion_ways.add_field("way_id", OFTString, 10);
    m_tagging_nonop_confusion_ways.add_field("tags", OFTString, MAX_STRING_LENGTH);
    m_tagging_nonop_confusion_ways.add_field("lastchange", OFTString, 21);
    m_tagging_no_feature_tag_nodes.add_field("node_id", OFTString, 10);
    m_tagging_no_feature_tag_nodes.add_field("tags", OFTString, MAX_STRING_LENGTH);
    m_tagging_no_feature_tag_nodes.add_field("lastchange", OFTString, 21);
    m_tagging_no_feature_tag_ways.add_field("way_id", OFTString, 10);
    m_tagging_no_feature_tag_ways.add_field("tags", OFTString, MAX_STRING_LENGTH);
    m_tagging_no_feature_tag_ways.add_field("lastchange", OFTString, 21);
}

void TaggingViewHandler::write_feature_to_simple_layer(gdalcpp::Layer* layer,
        const osmium::OSMObject& object, const char* field_name, const char* value) {
    try {
        std::unique_ptr<OGRGeometry> geometry;
        if (object.type() == osmium::item_type::way) {
            geometry =m_factory.create_linestring(static_cast<const osmium::Way&>(object));
        } else if (object.type() == osmium::item_type::node) {
            geometry = m_factory.create_point(static_cast<const osmium::Node&>(object));
        }
        gdalcpp::Feature feature(*layer, std::move(geometry));
        set_basic_fields(feature, object, field_name, value);
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_verbose_output << err.what() << "\n";
    }
}

/*static*/ void TaggingViewHandler::set_basic_fields(gdalcpp::Feature& feature, const osmium::OSMObject& object,
        const char* field_name, const char* value) {
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", object.id());
    if (object.type() == osmium::item_type::way) {
        feature.set_field("way_id", idbuffer);
    } else if (object.type() == osmium::item_type::node) {
        feature.set_field("node_id", idbuffer);
    }
    if (field_name && value) {
        // shorten value if too long
        if (strlen(value) > MAX_STRING_LENGTH) {
            char output_value[MAX_STRING_LENGTH + 5];
            strncpy(output_value, value, MAX_STRING_LENGTH);
            output_value[MAX_STRING_LENGTH] = '\0';
            feature.set_field(field_name, output_value);
        } else {
            feature.set_field(field_name, value);
        }
    }
    std::string timestamp = object.timestamp().to_iso();
    feature.set_field("lastchange", timestamp.c_str());
}


void TaggingViewHandler::check_fixme(const osmium::OSMObject& object) {
    gdalcpp::Layer* current_layer;
    if (object.type() == osmium::item_type::way) {
        current_layer = &m_tagging_fixmes_on_ways;
    } else if (object.type() == osmium::item_type::node) {
        current_layer = &m_tagging_fixmes_on_nodes;
    } else {
        return;
    }
    const char* fixme = object.tags().get_value_by_key("fixme");
    if (fixme) {
        std::string tag = "fixme=";
        tag += fixme;
        write_feature_to_simple_layer(current_layer, object, "tag", tag.c_str());
        return;
    }
    const char* fixme_uppercase = object.tags().get_value_by_key("FIXME");
    if (fixme_uppercase) {
        std::string tag = "FIXME=";
        tag += fixme_uppercase;
        write_feature_to_simple_layer(current_layer, object, "tag", tag.c_str());
        return;
    }
    const char* todo = object.tags().get_value_by_key("todo");
    if (todo) {
        std::string tag = "todo=";
        tag += todo;
        write_feature_to_simple_layer(current_layer, object, "tag", tag.c_str());
        return;
    }
    for (const osmium::Tag& t : object.tags()) {
        const char* tag_value = t.value();
        if (!tag_value) {
            continue;
        }
        if (!strcasecmp(tag_value, "FIXME")) {
            std::string tag = t.key();
            tag += "=";
            tag += tag_value;
            write_feature_to_simple_layer(current_layer, object, "tag", tag.c_str());
            return;
        }
    }
}

void TaggingViewHandler::empty_value(const osmium::OSMObject& object) {
    gdalcpp::Layer* current_layer;
    if (object.type() == osmium::item_type::way) {
        current_layer = &m_tagging_ways_with_empty_v;
    } else if (object.type() == osmium::item_type::node) {
        current_layer = &m_tagging_nodes_with_empty_v;
    } else {
        return;
    }
    for (const osmium::Tag& t : object.tags()) {
        if (!strcmp(t.value(), "")) {
            write_feature_to_simple_layer(current_layer, object, "key", t.key());
            break;
        }
    }
}

void TaggingViewHandler::key_with_space(const osmium::OSMObject& object) {
    for (const osmium::Tag& t : object.tags()) {
        for (size_t i = 0; i < strlen(t.key()); ++i) {
            if (isspace(t.key()[i])) {
                char output_value[2 * 256 + 5];
                sprintf(output_value, "'%s'='%s'", t.key(), t.value());
                write_missspelled(object, output_value, "contains_whitespace", nullptr);
                break;
            }
        }
    }
}

void TaggingViewHandler::empty_key(const osmium::OSMObject& object) {
    gdalcpp::Layer* current_layer;
    if (object.type() == osmium::item_type::way) {
        current_layer = &m_tagging_ways_with_empty_k;
    } else if (object.type() == osmium::item_type::node) {
        current_layer = &m_tagging_nodes_with_empty_k;
    } else {
        return;
    }
    for (const osmium::Tag& t : object.tags()) {
        if (!strcmp(t.key(), "")) {
            write_feature_to_simple_layer(current_layer, object, "value", t.value());
            break;
        }
    }
}

void TaggingViewHandler::unusual_character(const osmium::OSMObject& object) {
    for (const osmium::Tag& t : object.tags()) {
        if (is_a_x_key_key(t.key(), "name") || is_a_x_key_key(t.key(), "description")
                || is_a_x_key_key(t.key(), "note") || is_a_x_key_key(t.key(), "comment")
                || !strcmp(t.key(), "fixme") || !strcmp(t.key(), "FIXME")
                || !strcmp(t.key(), "todo") || !strcmp(t.key(), "website")
                || is_a_x_key_key(t.key(), "contact") || !strcmp(t.key(), "url")
                || !strcmp(t.key(), "email")) {
            continue;
        }
        for (size_t i = 0; i < strlen(t.key()); ++i) {
            if (!is_good_character(t.key()[i])) {
                if (object.type() == osmium::item_type::node) {
                    write_missspelled(object, t.key(), "node_with_unusual_char", nullptr);
                } else if (object.type() == osmium::item_type::way) {
                    write_missspelled(object, t.key(), "way_with_unusual_char", nullptr);
                }
                return;
            }
        }
    }
}

void TaggingViewHandler::check_key_length(const osmium::OSMObject& object) {
    for (const osmium::Tag& t : object.tags()) {
        if (strlen(t.key()) <= 2) {
            write_missspelled(object, t.key(), "short", nullptr);
            break;
        }
        if (strlen(t.key()) > 50) {
            write_missspelled(object, t.key(), "long", nullptr);
            break;
        }
    }
}


void TaggingViewHandler::write_missspelled(const osmium::OSMObject& object,
        const char* key, const char* error, const char* otherkey) {
    gdalcpp::Layer* current_layer;
    std::unique_ptr<OGRGeometry> geometry;
    try {
        if (object.type() == osmium::item_type::way) {
            current_layer = &m_tagging_misspelled_way_keys;
            geometry = m_factory.create_linestring(static_cast<const osmium::Way&>(object));
        } else if (object.type() == osmium::item_type::node) {
            current_layer = &m_tagging_misspelled_node_keys;
            geometry = m_factory.create_point(static_cast<const osmium::Node&>(object));
        } else {
            return;
        }
        if (object.type() == osmium::item_type::way) {
        } else if (object.type() == osmium::item_type::node) {
        }
        gdalcpp::Feature feature(*current_layer, std::move(geometry));
        set_basic_fields(feature, object, "key", key);
        feature.set_field("error", error);
        if (otherkey) {
            feature.set_field("otherkey", otherkey);
        }
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_verbose_output << err.what() << "\n";
    }
}

/*static*/ bool TaggingViewHandler::is_a_x_key_key(const char* key, const char* whitelisted_base) {
    const char* location = strstr(key, whitelisted_base);
    const size_t length_white = strlen(whitelisted_base);
    const size_t length = strlen(key);
    if (!location) {
        return false;
    }
    if ((location == key || *(location - 1) == ':' || *(location - 1) == '_')
            && ((location + length_white) == (key + length) || *(location + length_white) == ':' || *(location + length_white) == '_')) {
        return true;
    }
    return false;
}

bool TaggingViewHandler::is_good_character(const char character) {
    if (isalnum(character)) {
        return true;
    }
    switch (character) {
    case ':':
    case '_':
    case '-':
        return true;
    }
    return false;
}

void TaggingViewHandler::hidden_nonop(const osmium::OSMObject& object) {
    gdalcpp::Layer* current_layer;
    if (object.type() == osmium::item_type::way) {
        current_layer = &m_tagging_nonop_confusion_ways;
    } else if (object.type() == osmium::item_type::node) {
        current_layer = &m_tagging_nonop_confusion_nodes;
    } else {
        return;
    }
    if (!has_important_core_tag(object.tags())) {
        return;
    }
    const char* disused = object.get_value_by_key("disused");
    const char* abandoned = object.get_value_by_key("abandoned");
    const char* razed = object.get_value_by_key("razed");
    const char* dismantled = object.get_value_by_key("dismantled");
    const char* construction = object.get_value_by_key("construction");
    const char* proposed = object.get_value_by_key("proposed");
    if (!value_is_false(disused) || !value_is_false(abandoned) || !value_is_false(razed)
            || !value_is_false(dismantled) || !value_is_false(construction) || !value_is_false(proposed)) {
        write_feature_to_simple_layer(current_layer, object, "tags", tags_string(object.tags(), nullptr).c_str());
    }
}

bool TaggingViewHandler::has_important_core_tag(const osmium::TagList& tags) {
    const char* highway = tags.get_value_by_key("highway");
    if (highway && !is_nonop(highway)) {
        return true;
    }
    const char* railway = tags.get_value_by_key("railway");
    if (railway && !is_nonop(railway)) {
        return true;
    }
    const char* amenity = tags.get_value_by_key("amenity");
    if (amenity && !is_nonop(amenity)) {
        return true;
    }
    const char* shop = tags.get_value_by_key("shop");
    if (shop && !is_nonop(shop)) {
        return true;
    }
    return false;
}

bool TaggingViewHandler::value_is_false(const char* value) {
    return value == nullptr || !strcmp(value, "no") || !strcmp(value, "false");
}

bool TaggingViewHandler::is_nonop(const char* key) {
    return !strcmp(key, "disused") || !strcmp(key, "abandoned") || !strcmp(key, "razed")
            || !strcmp(key, "dismantled") || !strcmp(key, "construction") || !strcmp(key, "proposed");
}

bool TaggingViewHandler::has_feature_key(const osmium::TagList& tags) {
    for (const osmium::Tag& t : tags) {
        if (!strcmp(t.key(), "building")) {
            return true;
        } else if (!strcmp(t.key(), "landuse")) {
            return true;
        } else if (!strcmp(t.key(), "highway")) {
            return true;
        } else if (!strcmp(t.key(), "railway")) {
            return true;
        } else if (!strcmp(t.key(), "amenity")) {
            return true;
        } else if (!strcmp(t.key(), "shop")) {
            return true;
        } else if (!strcmp(t.key(), "natural")) {
            return true;
        } else if (!strcmp(t.key(), "waterway")) {
            return true;
        } else if (!strcmp(t.key(), "power")) {
            return true;
        } else if (!strcmp(t.key(), "barrier")) {
            return true;
        } else if (!strcmp(t.key(), "leisure")) {
            return true;
        } else if (!strcmp(t.key(), "man_made")) {
            return true;
        } else if (!strcmp(t.key(), "tourism")) {
            return true;
        } else if (!strcmp(t.key(), "boundary")) {
            return true;
        } else if (!strcmp(t.key(), "public_transport")) {
            return true;
        } else if (!strcmp(t.key(), "sport")) {
            return true;
        } else if (!strcmp(t.key(), "emergency")) {
            return true;
        } else if (!strcmp(t.key(), "historic")) {
            return true;
        } else if (!strcmp(t.key(), "route")) {
            return true;
        } else if (!strcmp(t.key(), "aeroway")) {
            return true;
        } else if (!strcmp(t.key(), "place")) {
            return true;
        } else if (!strcmp(t.key(), "craft")) {
            return true;
        } else if (!strcmp(t.key(), "entrance")) {
            return true;
        } else if (!strcmp(t.key(), "playground")) {
            return true;
        } else if (!strcmp(t.key(), "aerialway")) {
            return true;
        } else if (!strcmp(t.key(), "healthcare")) {
            return true;
        } else if (!strcmp(t.key(), "military")) {
            return true;
        } else if (!strcmp(t.key(), "building:part")) {
            return true;
        } else if (!strcmp(t.key(), "training")) {
            return true;
        } else if (!strcmp(t.key(), "traffic_sign")) {
            return true;
        } else if (!strcmp(t.key(), "xmas:feature")) {
            return true;
        } else if (!strcmp(t.key(), "seamark:type")) {
            return true;
        } else if (!strcmp(t.key(), "waterway:sign")) {
            return true;
        } else if (!strcmp(t.key(), "university")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "historic")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "razed")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "demolished")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "abandoned")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "disused")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "construction")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "proposed")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "temporary")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "TMC")) {
            return true;
        } else if (!strcmp(t.key(), "pipeline")) {
            return true;
        } else if (!strcmp(t.key(), "club")) {
            return true;
        } else if (!strcmp(t.key(), "golf")) {
            return true;
        } else if (!strcmp(t.key(), "junction")) {
            return true;
        } else if (!strcmp(t.key(), "office") && (strcmp(t.value(), "yes"))) {
            // office=yes is no real feature tag, "yes" is is a value for lazy users, newbies and SEO spammers.
            return true;
        } else if (!strcmp(t.key(), "piste:type")) {
            return true;
        } else if (!strcmp(t.key(), "mountain_pass")) {
            return true;
        } else if (!strcmp(t.key(), "harbour")) {
            return true;
        }
    }
    return false;
}

bool TaggingViewHandler::has_non_feature_key(const osmium::TagList& tags) {
    for (const osmium::Tag& t : tags) {
        if (is_a_x_key_key(t.key(), "name")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "description")) {
            return true;
//        } else if (is_a_x_key_key(t.key(), "note")) {
//            return true;
        } else if (is_a_x_key_key(t.key(), "comment")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "website")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "url")) {
            return true;
        } else if (is_a_x_key_key(t.key(), "contact:website")) {
            return true;
        }
    }
    return false;
}

void TaggingViewHandler::no_main_tags(const osmium::OSMObject& object) {
    if (has_feature_key(object.tags())) {
        return;
    }
    gdalcpp::Layer* current_layer;
    if (object.type() == osmium::item_type::way) {
        current_layer = &m_tagging_no_feature_tag_ways;
    } else if (object.type() == osmium::item_type::node) {
        current_layer = &m_tagging_no_feature_tag_nodes;
    } else {
        return;
    }
    if (has_non_feature_key(object.tags())) {
        write_feature_to_simple_layer(current_layer, object, "tags", tags_string(object.tags(), nullptr).c_str());
    }
}

void TaggingViewHandler::handle_object(const osmium::OSMObject& object) {
    empty_value(object);
    check_fixme(object);
    empty_key(object);
    unusual_character(object);
    check_key_length(object);
    hidden_nonop(object);
    no_main_tags(object);
}

void TaggingViewHandler::node(const osmium::Node& node) {
    handle_object(node);
}

void TaggingViewHandler::way(const osmium::Way& way) {
    handle_object(way);
}
