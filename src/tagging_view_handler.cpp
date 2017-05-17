/*
 * tagging_view_handler.cpp
 *
 *  Created on:  2017-05-16
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

//#include <cctype>
#include "tagging_view_handler.hpp"

TaggingViewHandler::TaggingViewHandler(std::string& output_filename, std::string& output_format,
        std::vector<std::string>& gdal_options, osmium::util::VerboseOutput& verbose_output,
        int epsg) :
        AbstractViewHandler(output_filename, output_format, gdal_options, verbose_output, epsg),
        m_tagging_fixmes_on_nodes(m_dataset, "tagging_fixmes_on_nodes", wkbPoint),
        m_tagging_fixmes_on_ways(m_dataset, "tagging_fixmes_on_ways", wkbLineString),
        m_tagging_nodes_with_empty_k(m_dataset, "tagging_nodes_with_empty_k", wkbPoint),
        m_tagging_ways_with_empty_k(m_dataset, "tagging_ways_with_empty_k", wkbLineString),
        m_tagging_nodes_with_empty_v(m_dataset, "tagging_nodes_with_empty_v", wkbPoint),
        m_tagging_ways_with_empty_v(m_dataset, "tagging_ways_with_empty_v", wkbLineString),
        m_tagging_misspelled_node_keys(m_dataset, "tagging_misspelled_node_keys", wkbPoint),
        m_tagging_misspelled_way_keys(m_dataset, "tagging_misspelled_way_keys", wkbLineString)
{
    m_tagging_fixmes_on_nodes.add_field("node_id", OFTString, 10);
    m_tagging_fixmes_on_nodes.add_field("tag", OFTString, 200);
    m_tagging_fixmes_on_nodes.add_field("lastchange", OFTString, 21);
    m_tagging_fixmes_on_ways.add_field("way_id", OFTString, 10);
    m_tagging_fixmes_on_ways.add_field("tag", OFTString, 200);
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
    m_tagging_misspelled_node_keys.add_field("key", OFTString, 100);
    m_tagging_misspelled_node_keys.add_field("error", OFTString, 20);
    m_tagging_misspelled_node_keys.add_field("otherkey", OFTString, 100);
    m_tagging_misspelled_node_keys.add_field("lastchange", OFTString, 21);
    m_tagging_misspelled_way_keys.add_field("way_id", OFTString, 10);
    m_tagging_misspelled_way_keys.add_field("key", OFTString, 100);
    m_tagging_misspelled_way_keys.add_field("error", OFTString, 20);
    m_tagging_misspelled_way_keys.add_field("otherkey", OFTString, 100);
    m_tagging_misspelled_way_keys.add_field("lastchange", OFTString, 21);
}

void TaggingViewHandler::write_feature_to_simple_layer(gdalcpp::Layer* layer,
        const osmium::OSMObject& object, const char* field_name, const char* value) {
    std::unique_ptr<OGRGeometry> geometry;
    if (object.type() == osmium::item_type::way) {
        geometry =m_factory.create_linestring(static_cast<const osmium::Way&>(object));
    } else if (object.type() == osmium::item_type::node) {
        geometry = m_factory.create_point(static_cast<const osmium::Node&>(object));
    }
    gdalcpp::Feature feature(*layer, std::move(geometry));
    set_basic_fields(feature, object, field_name, value);
    feature.add_to_layer();
}

void TaggingViewHandler::set_basic_fields(gdalcpp::Feature& feature, const osmium::OSMObject& object,
        const char* field_name, const char* value) {
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", object.id());
    if (object.type() == osmium::item_type::way) {
        feature.set_field("way_id", idbuffer);
    } else if (object.type() == osmium::item_type::node) {
        feature.set_field("node_id", idbuffer);
    }
    feature.set_field(field_name, value);
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
        if (!strcmp(t.key(), "") || strchr(t.key(), ' ') != nullptr) {
            char output_value[2 * 256 + 5];
            sprintf(output_value, "'%s'='%s'", t.key(), t.value());
            // shorten to MAX_STRING_LENGTH
            output_value[MAX_STRING_LENGTH] = '\0';
            write_feature_to_simple_layer(current_layer, object, "value", output_value);
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
                write_missspelled(object, t.key(), "unusual_char", nullptr);
                return;
            }
        }
    }
}

void TaggingViewHandler::write_missspelled(const osmium::OSMObject& object,
        const char* key, const char* error, const char* otherkey) {
    gdalcpp::Layer* current_layer;
    if (object.type() == osmium::item_type::way) {
        current_layer = &m_tagging_misspelled_way_keys;
    } else if (object.type() == osmium::item_type::node) {
        current_layer = &m_tagging_misspelled_node_keys;
    } else {
        return;
    }
    std::unique_ptr<OGRGeometry> geometry;
    if (object.type() == osmium::item_type::way) {
        geometry = m_factory.create_linestring(static_cast<const osmium::Way&>(object));
    } else if (object.type() == osmium::item_type::node) {
        geometry = m_factory.create_point(static_cast<const osmium::Node&>(object));
    }
    gdalcpp::Feature feature(*current_layer, std::move(geometry));
    set_basic_fields(feature, object, "key", key);
    feature.set_field("error", error);
    if (otherkey) {
        feature.set_field("otherkey", otherkey);
    }
    feature.add_to_layer();
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
    empty_value(object);
    empty_key(object);
    unusual_character(object);
}

void TaggingViewHandler::node(const osmium::Node& node) {
    handle_object(node);
}

void TaggingViewHandler::way(const osmium::Way& way) {
    handle_object(way);
}
