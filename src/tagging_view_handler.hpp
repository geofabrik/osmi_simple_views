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

#ifndef SRC_TAGGING_VIEW_HANDLER_HPP_
#define SRC_TAGGING_VIEW_HANDLER_HPP_

#include "abstract_view_handler.hpp"

class TaggingViewHandler : public AbstractViewHandler {

    static constexpr size_t MAX_STRING_LENGTH = 254;

    std::unique_ptr<gdalcpp::Layer> m_tagging_fixmes_on_nodes;
    std::unique_ptr<gdalcpp::Layer> m_tagging_fixmes_on_ways;
    std::unique_ptr<gdalcpp::Layer> m_tagging_nodes_with_empty_k;
    std::unique_ptr<gdalcpp::Layer> m_tagging_ways_with_empty_k;
    std::unique_ptr<gdalcpp::Layer> m_tagging_nodes_with_empty_v;
    std::unique_ptr<gdalcpp::Layer> m_tagging_ways_with_empty_v;
    std::unique_ptr<gdalcpp::Layer> m_tagging_misspelled_node_keys;
    std::unique_ptr<gdalcpp::Layer> m_tagging_misspelled_way_keys;
    std::unique_ptr<gdalcpp::Layer> m_tagging_nonop_confusion_nodes;
    std::unique_ptr<gdalcpp::Layer> m_tagging_nonop_confusion_ways;
    std::unique_ptr<gdalcpp::Layer> m_tagging_no_feature_tag_nodes;
    std::unique_ptr<gdalcpp::Layer> m_tagging_no_feature_tag_ways;
    std::unique_ptr<gdalcpp::Layer> m_tagging_long_text_nodes;
    std::unique_ptr<gdalcpp::Layer> m_tagging_long_text_ways;

    /**
     * Write a feature to on of the layers which only have the fields
     * way_id/node_id, tag and lastchange.
     *
     * \param layer layer to write to
     * \param object OSM object
     * \param field_name name of the field which value should be written to.
     * If field_name is a nullptr, nothing will be written. Use `field_name=nullptr`
     * for layers which do not have any additional fields.
     * \param value value of tag field If field_name is a nullptr, nothing will
     * be written. Use `field_name=nullptr` for layers which do not have any
     * additional fields.
     * \param other_field_name Another field to be set.
     * \param other_value Value to be written to "other_field_name".
     */
    void write_feature_to_simple_layer(gdalcpp::Layer* layer,
            const osmium::OSMObject& object, const char* field_name, const char* value,
            const char* other_field_name = nullptr, const char* other_value = nullptr);

    /**
     * Write an object with a found spelling error to the output file.
     */
    void write_missspelled(const osmium::OSMObject& object,
            const char* key, const char* error, const char* otherkey);

    /**
     * Check if an object has one of the following keys: fixme=*, FIXME=* or todo=*.
     *
     * \param object object to be checked
     */
    void check_fixme(const osmium::OSMObject& object);

    /**
     * Check if an object has a key which contains whitespace.
     */
    void key_with_space(const osmium::OSMObject& object);

    /**
     * Check if an object has an empty key
     */
    void empty_key(const osmium::OSMObject& object);

    /**
     * Check if an object has an empty value
     */
    void empty_value(const osmium::OSMObject& object);

    /**
     * Check if an object has a tag with an unusual character
     */
    void unusual_character(const osmium::OSMObject& object);

    /**
     * Check if the length of a key is larger than 2 and smaller or equal than 50.
     */
    void check_key_length(const osmium::OSMObject& object);

    /**
     * Check if a character is an accepted character for keys and non-name values.
     */
    bool is_good_character(const char character);

    /**
     * Search for objects with a core tag but a disused/abandoned/razed/dismanted/construction/proposed=yes.
     */
    void hidden_nonop(const osmium::OSMObject& object);

    /**
     * Check if an object has an important tag.
     *
     * Following keys are considered as important: highway, railway, amenity, shop
     *
     * man_made is not in the list of searched keys because this would cause too much
     * false positives returned by the hidden_nonop check.
     *
     * \returns instance of CoreTags
     */
    static bool has_important_core_tag(const osmium::TagList& tags);

    /**
     * Check if a value is "no" or "false".
     *
     * \return true if it is so or value is a nullptr
     */
    static bool value_is_false(const char* value);

    /**
     * Check if the given key is a key which intends to indicate non-operational use:
     *
     * disused, abandoned, razed, dismantled, proposed, construction
     */
    static bool is_nonop(const char* key);

    /**
     * Check if an object has a name or description tag but no main tag.
     */
    void no_main_tags(const osmium::OSMObject& object);

    /**
     * Check if an object has a note or description tag longer than NON_SUSPICIOUS_MAX_LENGTH characters.
     */
    void long_text(const osmium::OSMObject& object);

    /**
     * Check if a tag has a "feature" key, i.e. it has a key which describes what it is.
     */
    static bool has_feature_key(const osmium::TagList& tags);

    static bool has_non_feature_key(const osmium::TagList& tags);

    /**
     * Apply all checks on an object.
     */
    void handle_object(const osmium::OSMObject& object);

public:
    TaggingViewHandler() = delete;

    explicit TaggingViewHandler(Options& options);

    void give_correct_name();

    void close();

    void node(const osmium::Node& node);

    void way(const osmium::Way& way);

    /**
     * Check if a key is a whitelisted key, e.g. "name", "short_name", "name:ru", description, description:en, comment, ….
     *
     * The substring name must be
     * * located at the beginning or preceeded by a colon or underscore
     * * and located at the end or followed by a colon or underscore
     *
     * This method will return wrong results if whitelisted_base occurs twice in key (e.g. named_name=*). But these cases are and the
     * common whitelisted keys name, description, note, comment and fixme don't have such stranges prefixes/suffixes.
     */
    static bool is_a_x_key_key(const char* key, const char* whitelisted_base);

    /**
     * Set some basic fields of a feature: ID, lastchange and one freely selectable field
     */
    static void set_basic_fields(gdalcpp::Feature& feature, const osmium::OSMObject& object,
            const char* field_name, const char* value);

    /**
     * Get length of a string with respect to multi-byte UTF-8 characters.
     */
    static size_t char_length_utf8(const char* value);

    void relation(const osmium::Relation&) {};
    void area(const osmium::Area&) {};
};



#endif /* SRC_TAGGING_VIEW_HANDLER_HPP_ */
