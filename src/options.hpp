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
    tagging= 4
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
};



#endif /* SRC_OPTIONS_HPP_ */
