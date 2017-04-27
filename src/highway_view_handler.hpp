/*
 * highway_view_handler.hpp
 *
 *  Created on:  2017-04-27
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_HIGHWAY_VIEW_HANDLER_HPP_
#define SRC_HIGHWAY_VIEW_HANDLER_HPP_

#include <functional>
#include <string>
#include <vector>

#include "abstract_view_handler.hpp"

class HighwayViewHandler : public AbstractViewHandler {
    /// layer for ways with lanes=* value which is not an unsigned integer
    gdalcpp::Layer m_highway_lanes;
    /// layer for ways with strang maxheight values
    gdalcpp::Layer m_highway_maxheight;
    gdalcpp::Layer m_highway_maxspeed;
    gdalcpp::Layer m_highway_name_fixme;
    gdalcpp::Layer m_highway_name_missing_major;
    gdalcpp::Layer m_highway_name_missing_minor;
    gdalcpp::Layer m_highway_oneway;
    gdalcpp::Layer m_highway_type_deprecated;
    gdalcpp::Layer m_highway_road;
    gdalcpp::Layer m_highway_type_unknown;


    /// param vector of functions returning false if a tag is malformed.
    std::vector<std::function<bool (const char*)>> m_checks;

    /**
     * vector with keys of the OSM tags to be checked. The n-th element of this vector
     * is the argument for the n-th function in checks.
     */
    std::vector<std::string> m_keys;

    /// output layer for errorenous objects if the check fails
    std::vector<gdalcpp::Layer*> m_layers;

    /**
     * Build a string containing tags (length of key and value below 48 characters)
     * to be inserted into a "tag" column. The returned string is shorter than
     * MAX_FIELD_LENGTH characters. No keys or values will be truncated.
     *
     * \param tags TagList of the OSM object
     * \param not_include key whose value should not be included in the tags' string
     *
     * \returns string with the tags
     */
    std::string tags_string(const osmium::TagList& tags, const char* not_include);

    /**
     * Create a feature, set its field and add it to its layer.
     *
     * This method sets `way_id` and `tags` automatically.
     *
     * \param layer layer the feature should be added to
     * \param way reference to OSM object
     * \param third_field_name name of the third field to be set (nullptr if it does
     * not exist or should not be set)
     * \param third_field_value value of the third field to be set (nullptr if it does
     * not exist of should not be set)
     * \param other_tags string containing concatenated tags to be written into the field
     * `tags`
     */
    void set_fields(gdalcpp::Layer& layer, const osmium::Way& way, const char* third_field_name,
            const char* third_field_value, std::string& other_tags);
    void set_fields(gdalcpp::Layer* layer, const osmium::Way& way, const char* third_field_name,
            const char* third_field_value, std::string& other_tags);

    /**
     * Check if a name is not a fixme placeholder, e.g. "fixme" or "unknown".
     *
     * \param name_value value of the name tag
     *
     * \returns true if the name is a valid name
     */
    static bool name_not_fixme(const char* name_value);

    static bool lanes_ok(const char* name_value);

    /**
     * Run all checks on an OSM object.
     *
     * \param way reference to the OSM way to be checked
     */
    void check_them_all(const osmium::Way& way);

    /**
     * Register a check to be run for each object
     *
     * \param function function to be run. It has to return false if the value is malformed.
     * \param key OSM key whose value has to be checked
     * \param layer layer which the errorenous OSM object should be added to
     */
    void register_check(std::function<bool (const char*)> function, std::string key, gdalcpp::Layer* layer);

public:
    HighwayViewHandler(std::string& output_filename, std::string& output_format, std::vector<std::string>& gdal_options,
            osmium::util::VerboseOutput& verbose_output, int epsg = 3857);

    void way(const osmium::Way& way);
};



#endif /* SRC_HIGHWAY_VIEW_HANDLER_HPP_ */
