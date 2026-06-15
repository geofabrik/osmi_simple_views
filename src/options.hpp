/*
 * options.hpp
 *
 *  Created on:  2019-04-12
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_OPTIONS_HPP_
#define SRC_OPTIONS_HPP_

/**
 * Available views
 */
enum class ViewType : char {
    none= 0,
    places= 1,
    geometry= 2,
    highways= 3,
    tagging = 4,
    sac_scale = 5,
    turn_restrictions = 6
};

/**
 * options for program execution
 */
struct Options {
    std::vector<ViewType> views;
    std::string location_index_type = "sparse_mem_array";
    std::string output_format = "SQlite";
    std::string output_directory = "";
#ifdef ONLYMERCATOROUTPUT
    const int srs = 3857;
#else
    const int srs = 4326;
#endif
    osmium::util::VerboseOutput verbose_output {false};

    /**
     * Return capability to create multiple layers with one data source.
     *
     * By default, this method returns false.
     */
    bool one_layer_per_datasource_only() {
        return case_insensitive_comp_left(output_format, "geojson")
            || case_insensitive_comp_left(output_format, "esri shapefile");
    }

    /**
     * Do a case-insensitive string comparison assuming that the right side is lower case.
     */
    static bool case_insensitive_comp_left(const std::string& a, const std::string& b) {
        return (
            a.size() == b.size()
            && std::equal(
                a.begin(), a.end(), b.begin(),
                [](const char c, const char d) {
                    return c == d || std::tolower(c) == d;
                })
        );
    }
};



#endif /* SRC_OPTIONS_HPP_ */
