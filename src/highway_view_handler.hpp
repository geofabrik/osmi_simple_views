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


#ifndef SRC_HIGHWAY_VIEW_HANDLER_HPP_
#define SRC_HIGHWAY_VIEW_HANDLER_HPP_

#include <string.h>
#include <functional>
#include <string>
#include <vector>

#include "abstract_view_handler.hpp"

struct charptr_comp {
    bool operator()(const char* const a, const char* const b) const {
        return strcmp(a, b) < 0;
    }
};

class HighwayViewHandler : public AbstractViewHandler {
    /// layer for roads with abandoned:highway=*
    std::unique_ptr<gdalcpp::Layer> m_highway_abandoned;
    /// layer for roads with disused:highway=*
    std::unique_ptr<gdalcpp::Layer> m_highway_disused;
    /// layer for roads with construction:highway=*
    std::unique_ptr<gdalcpp::Layer> m_highway_construction;
    /// layer for roads with proposed:highway=*
    std::unique_ptr<gdalcpp::Layer> m_highway_proposed;
    /// layer for roads with highway=proposed/construction/disused/abandoned without tag specifying road class
    std::unique_ptr<gdalcpp::Layer> m_highway_multiple_lifecycle_states;
    /// layer for ways with lanes=* value which is not an unsigned integer
    std::unique_ptr<gdalcpp::Layer> m_highway_incomplete_nonop;
    /// layer for roads tagged with multiple lifecycle states
    std::unique_ptr<gdalcpp::Layer> m_highway_lanes;
    /// layer for ways with strang maxheight values
    std::unique_ptr<gdalcpp::Layer> m_highway_maxheight;
    std::unique_ptr<gdalcpp::Layer> m_highway_maxweight;
    std::unique_ptr<gdalcpp::Layer> m_highway_maxlength;
    std::unique_ptr<gdalcpp::Layer> m_highway_maxspeed;
    std::unique_ptr<gdalcpp::Layer> m_highway_name_fixme;
    std::unique_ptr<gdalcpp::Layer> m_highway_name_missing_major;
    std::unique_ptr<gdalcpp::Layer> m_highway_name_missing_minor;
    std::unique_ptr<gdalcpp::Layer> m_highway_oneway;
    std::unique_ptr<gdalcpp::Layer> m_highway_road;
    std::unique_ptr<gdalcpp::Layer> m_highway_long_ref;
    std::unique_ptr<gdalcpp::Layer> m_highway_unknown_node;
    std::unique_ptr<gdalcpp::Layer> m_highway_unknown_way;


    /// param vector of functions returning false if a tag is malformed.
    std::vector<std::function<bool (const osmium::TagList&)>> m_checks;

    /**
     * vector with keys of the OSM tags to be checked. The n-th element of this vector
     * is the argument for the n-th function in checks.
     */
    std::vector<std::string> m_keys;

    /// output layer for errorenous objects if the check fails
    std::vector<gdalcpp::Layer*> m_layers;

    /**
     * Check if the value of the maxspeed tag matches one of the common
     * values like RO:urban.
     */
    static bool is_valid_const_speed(const char* maxspeed_value);

    /**
     * Create a feature, set its field and add it to its layer.
     *
     * This method sets `way_id` and `tags` automatically.
     *
     * \param layer layer the feature should be added to
     * \param object reference to OSM object
     * \param third_field_name name of the third field to be set (nullptr if it does
     * not exist or should not be set)
     * \param third_field_value value of the third field to be set (nullptr if it does
     * not exist of should not be set)
     * \param other_tags string containing concatenated tags to be written into the field
     * `tags`
     * \param geom_func function returning unique pointer to geometry to be written to the output
     * layer
     *
     * \tparam class like Node or Way (from Osmium)
     */
    template <typename TOsm>
    void set_fields(gdalcpp::Layer* layer, const TOsm& object, const char* third_field_name,
            const char* third_field_value, std::string& other_tags,
            std::function<std::unique_ptr<OGRGeometry>(const TOsm&, ogr_factory_type&)> geom_func,
            const osmium::object_id_type id, const char* id_field_name, const char* key4 = nullptr,
            const char* field4 = nullptr) {
        try {
            gdalcpp::Feature feature(*layer, geom_func(object, m_factory));
            static char idbuffer[20];
            sprintf(idbuffer, "%ld", id);
            feature.set_field(id_field_name, idbuffer);
            feature.set_field("tags", other_tags.c_str());
            if (third_field_name && third_field_value) {
                feature.set_field(third_field_name, third_field_value);
            }
            if (key4 && field4) {
                feature.set_field(key4, field4);
            }
            feature.add_to_layer();
        } catch (osmium::geometry_error& err) {
            m_options.verbose_output << err.what() << "\n";
        }
    }

    void set_fields(gdalcpp::Layer* layer, const osmium::Way& way, const char* third_field_name,
            const char* third_field_value, std::string& other_tags);

    /**
     * Check if a name is not a fixme placeholder, e.g. "fixme" or "unknown".
     *
     * \param name_value value of the name tag
     *
     * \returns true if the name is a valid name
     */
    static bool name_not_fixme(const osmium::TagList& tags);

    void check_lanes_tags(const osmium::Way& way);

    static bool oneway_ok(const osmium::TagList& tags);

    static bool maxspeed_ok(const osmium::TagList& tags);

    static bool maxheight_ok(const osmium::TagList& tags);

    static bool maxweight_ok(const osmium::TagList& tags);

    static bool maxlength_ok(const osmium::TagList& tags);

    static bool name_missing_major(const osmium::TagList& tags);

    static bool name_missing_minor(const osmium::TagList& tags);

    static bool highway_road(const osmium::TagList& tags);

    static bool highway_long_ref(const osmium::TagList& tags);

    /**
     * Write way to specified layer if provided key is set.
     *
     * alternative_key is used if key is not set.
     */
    void ways_with_key(const osmium::Way& way, gdalcpp::Layer* layer, const char* key, const char* alternative_key = nullptr);

    void highway_unknown_node(const osmium::Node& node);

    void highway_unknown_way(const osmium::Way& way);

    void highway_multiple_lifecycle_states(const osmium::Way& way);


    /**
     * Run all checks on an OSM object.
     *
     * \param way reference to the OSM way to be checked
     */
    void check_them_all(const osmium::Way& way);

    /**
     * Register a check to be run for each object
     *
     * \param function function to be run. It has to return false if the object should be added to the layer.
     * \param key OSM key whose value has to be checked
     * \param layer layer which the errorenous OSM object should be added to
     */
    void register_check(std::function<bool (const osmium::TagList&)> function, std::string key, gdalcpp::Layer* layer);

    int check_lanes_value_and_write_error(const osmium::Way& way, const char* key);

    bool all_oneway(const osmium::TagList& tags);

    int pipe_separated_items_count(const char* value);

public:
    HighwayViewHandler(Options& options);

    void give_correct_name();

    void close();

    void node(const osmium::Node& node);

    void way(const osmium::Way& way);

    void relation(const osmium::Relation&) {};
    void area(const osmium::Area&) {};

    std::string name();

    static bool check_length_value(const char* value);

    static bool check_valid_turns(const char* turns);

    static bool check_maxweight(const char* maxweight_value);

    static bool check_oneway(const char* oneway_value);
};



#endif /* SRC_HIGHWAY_VIEW_HANDLER_HPP_ */
