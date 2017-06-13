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


#ifndef SRC_PLACES_HANDLER_HPP_
#define SRC_PLACES_HANDLER_HPP_

#include <memory>

#include "abstract_view_handler.hpp"

class PlacesHandler : public AbstractViewHandler {

    gdalcpp::Layer m_points;
    gdalcpp::Layer m_polygons;
    gdalcpp::Layer m_errors_points;
    gdalcpp::Layer m_errors_polygons;
    gdalcpp::Layer m_cities;

    /**
     * Check if value of the place tag is well-known.
     *
     * \param value value of the place key
     */
    bool place_value_ok(const char* value);

    /**
     * Check if the place node is a capital of an admin_level-2 country.
     */
    bool is_capital(const osmium::TagList& tags);

    /**
     * Add a feature to the output layers.
     *
     * This method will select on its own which layer to use.
     *
     * \param geometry geometry to be written
     * \param osm_object OSM object to be written
     * \param geomtype char representing the type of the OSM object (n = node, w = way, r = relation)
     * \param id ID of the OSM object
     * \param place_value value of the place key of the OSM object
     * \param should the object be added to the cities layer instead of a normal layer?
     */
    void add_feature(std::unique_ptr<OGRGeometry>&& geometry, const osmium::OSMObject& osm_object,
            const char* geomtype, const osmium::object_id_type id, const char* place_value,
            bool city_layer = false);

    /**
     * Set some basic fields needed by all layers
     *
     * \param feature OGR feature whose fields should be set
     * \param osm_object OSM object to be written
     * \param id ID of the OSM object
     */
    void set_basic_fields(gdalcpp::Feature& feature, const osmium::OSMObject& osm_object,
            const osmium::object_id_type id);

    /**
     * Add a feature to the errors layer.
     *
     * This method will select on its own which layer to use.
     *
     * \param osm_object OSM object to be written
     * \param geomtype char representing the type of the OSM object (n = node, w = way, r = relation)
     * \param id ID of the OSM object
     * \param error type of error
     * \param different_value string to be to the `value` column if it should not be the value of the place key
     */
    void add_error(const osmium::OSMObject& osm_object, const osmium::object_id_type id,
            const char* geomtype, std::string error, std::string different_value = "");

    /**
     * Check if the population of a settlement is within resonable bounds.
     */
    void check_population(const osmium::OSMObject& osm_object, const osmium::object_id_type id,
            const char* geomtype, const char* place_value, long int population);

public:
    PlacesHandler() = delete;

    PlacesHandler(std::string& output_filename, std::string& output_format,
            osmium::util::VerboseOutput& verbose_output, int epsg = 3857);

    void node(const osmium::Node& node);

    void area(const osmium::Area& area);
};



#endif /* SRC_PLACES_HANDLER_HPP_ */
