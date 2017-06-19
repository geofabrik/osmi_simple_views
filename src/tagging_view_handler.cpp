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
        m_tagging_misspelled_way_keys(m_dataset, "tagging_misspelled_way_keys", wkbLineString, GDAL_DEFAULT_OPTIONS)
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
        if (!strcmp(tag_value, "FIXME") || !strcmp(tag_value, "fixme")) {
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
                || !strcmp(t.key(), "todo")) {
            continue;
        }
        for (size_t i = 0; i < strlen(t.key()); ++i) {
            if (!is_good_character(t.key()[i])) {
                if (object.type() == osmium::item_type::node) {
                    write_missspelled(object, t.key(), "node_with_unusual_char", nullptr);
                } else if (object.type() == osmium::item_type::node) {
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

void TaggingViewHandler::handle_object(const osmium::OSMObject& object) {
    empty_value(object);
    check_fixme(object);
    empty_key(object);
    unusual_character(object);
    check_key_length(object);
}

void TaggingViewHandler::node(const osmium::Node& node) {
    handle_object(node);
}

void TaggingViewHandler::way(const osmium::Way& way) {
    handle_object(way);
}
