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


#include "places_handler.hpp"
#include <iostream>
#include <osmium/index/index.hpp>
#include <osmium/osm/item_type.hpp>

PlacesHandler::PlacesHandler(std::string& output_filename, std::string& output_format,
        osmium::util::VerboseOutput& verbose_output, int epsg /*= 3857*/) :
        AbstractViewHandler(output_filename, output_format, verbose_output, epsg),
        m_points(create_layer("points", wkbPoint, GDAL_DEFAULT_LAYER_OPTIONS)),
        m_polygons(create_layer("polygons", wkbMultiPolygon, GDAL_DEFAULT_LAYER_OPTIONS)),
        m_errors_points(create_layer("errors_points", wkbPoint, GDAL_DEFAULT_LAYER_OPTIONS)),
        m_errors_polygons(create_layer("errors_polygons", wkbMultiPolygon, GDAL_DEFAULT_LAYER_OPTIONS)),
        m_cities(create_layer("cities", wkbPoint, GDAL_DEFAULT_LAYER_OPTIONS)) {
    // add fields to layers
    m_points->add_field("node_id", OFTString, 10);
    m_points->add_field("place", OFTString, 20);
    m_points->add_field("type", OFTString, 20);
    m_points->add_field("popstr", OFTString, 20);
    m_points->add_field("population", OFTInteger, 10);
    m_points->add_field("capitalstr", OFTString, 20);
    m_points->add_field("capital", OFTInteger, 2);
    m_points->add_field("admlvl", OFTInteger, 2);
    m_points->add_field("name", OFTString, 100);
    m_points->add_field("lastchange", OFTString, 21);
    // and the same for the polygons layer
    m_polygons->add_field("node_id", OFTString, 10);
    m_polygons->add_field("place", OFTString, 20);
    m_polygons->add_field("type", OFTString, 20);
    m_polygons->add_field("popstr", OFTString, 20);
    m_polygons->add_field("population", OFTInteger, 10);
    m_polygons->add_field("capitalstr", OFTString, 20);
    m_polygons->add_field("capital", OFTInteger, 2);
    m_polygons->add_field("admlvl", OFTInteger, 2);
    m_polygons->add_field("name", OFTString, 100);
    m_polygons->add_field("lastchange", OFTString, 21);
    // errors layer
    m_errors_points->add_field("node_id", OFTString, 10);
    m_errors_points->add_field("geomtype", OFTString, 1);
    m_errors_points->add_field("error", OFTString, 60);
    m_errors_points->add_field("value", OFTString, 100);
    m_errors_points->add_field("lastchange", OFTString, 21);
    m_errors_polygons->add_field("node_id", OFTString, 10);
    m_errors_polygons->add_field("geomtype", OFTString, 1);
    m_errors_polygons->add_field("error", OFTString, 60);
    m_errors_polygons->add_field("value", OFTString, 100);
    m_errors_polygons->add_field("lastchange", OFTString, 21);
    // cities layer
    m_cities->add_field("node_id", OFTString, 10);
    m_cities->add_field("popstr", OFTString, 20);
    m_cities->add_field("population", OFTInteger, 10);
    m_cities->add_field("capitalstr", OFTString, 20);
    m_cities->add_field("capital", OFTInteger, 2);
    m_cities->add_field("admlvl", OFTInteger, 2);
    m_cities->add_field("name", OFTString, 100);
    m_cities->add_field("lastchange", OFTString, 21);

}

void PlacesHandler::give_correct_name() {
    rename_output_files("places");
}

void PlacesHandler::close() {
    m_points.reset();
    m_polygons.reset();
    m_errors_points.reset();
    m_errors_polygons.reset();
    m_cities.reset();
    close_datasets();
}

bool PlacesHandler::place_value_ok(const char* value) {
    if (!strcmp(value, "continent")) {
        return true;
    }
    if (!strcmp(value, "country")) {
        return true;
    }
    if (!strcmp(value, "state")) {
        return true;
    }
    if (!strcmp(value, "region")) {
        return true;
    }
    if (!strcmp(value, "county")) {
        return true;
    }
    if (!strcmp(value, "city")) {
        return true;
    }
    if (!strcmp(value, "town")) {
        return true;
    }
    if (!strcmp(value, "village")) {
        return true;
    }
    if (!strcmp(value, "hamlet")) {
        return true;
    }
    if (!strcmp(value, "municipality")) {
        return true;
    }
    if (!strcmp(value, "suburb")) {
        return true;
    }
    if (!strcmp(value, "locality")) {
        return true;
    }
    if (!strcmp(value, "island")) {
        return true;
    }
    if (!strcmp(value, "islet")) {
        return true;
    }
    if (!strcmp(value, "farm")) {
        return true;
    }
    if (!strcmp(value, "subdivision")) {
        return true;
    }
    if (!strcmp(value, "sea")) {
        return true;
    }
    if (!strcmp(value, "ocean")) {
        return true;
    }
    if (!strcmp(value, "neighbourhood")) {
        return true;
    }
    if (!strcmp(value, "quarter")) {
        return true;
    }
    if (!strcmp(value, "isolated_dwelling")) {
        return true;
    }
    return false;
}

bool PlacesHandler::is_capital(const osmium::TagList& tags) {
    const char* is_capital = tags.get_value_by_key("is_capital", "");
    if (!strcmp(is_capital, "country")) {
        return true;
    }
    const char* capital = tags.get_value_by_key("capital", "");
    int admin_level = std::atoi(tags.get_value_by_key("admin_level", ""));
    if (!strcmp(capital, "yes") && (admin_level < 3) && (admin_level > 0)) {
        return true;
    } else if (!strcmp(capital, "2")) {
        return true;
    }
    return false;
}

void PlacesHandler::check_population(const osmium::OSMObject& osm_object, const osmium::object_id_type id,
        const char* geomtype, const char* place_value, long int population) {
    if (!place_value || population == 0) {
        return;
    }
    if (!strcmp(place_value, "city") && population < 10000) {
        add_error(osm_object, id, geomtype, "population too small for city", std::to_string(population));
    } else if (!strcmp(place_value, "town") && population > 200000) {
        add_error(osm_object, id, geomtype, "population too large for town", std::to_string(population));
    } else if (!strcmp(place_value, "town") && population < 500) {
        add_error(osm_object, id, geomtype, "population too small for town", std::to_string(population));
    } else if (!strcmp(place_value, "village") && population > 25000) {
        add_error(osm_object, id, geomtype, "population too large for village", std::to_string(population));
    } else if (!strcmp(place_value, "hamlet") && population > 1000) {
        add_error(osm_object, id, geomtype, "population too large for hamlet", std::to_string(population));
    } else if (!strcmp(place_value, "suburb") && population > 1000000) {
        add_error(osm_object, id, geomtype, "population too large for suburb", std::to_string(population));
    } else if (!strcmp(place_value, "isolated_dwelling") && population > 500) {
        add_error(osm_object, id, geomtype, "population too large for isolated_dwelling", std::to_string(population));
    } else if (!strcmp(place_value, "city") && population > 60000000) {
        add_error(osm_object, id, geomtype, "population too large for city", std::to_string(population));
    } else if (population > 12000000000) {
        add_error(osm_object, id, geomtype, "population too large for planet", std::to_string(population));
    }
}

void PlacesHandler::add_feature(std::unique_ptr<OGRGeometry>&& geometry, const osmium::OSMObject& osm_object,
        const char* geomtype, const osmium::object_id_type id, const char* place_value, bool city_layer /*= false*/) {
    gdalcpp::Layer* current_layer = m_points.get();
    if (osm_object.type() == osmium::item_type::area) {
        current_layer = m_polygons.get();
    }
    if (city_layer) {
        current_layer = m_cities.get();
    }
    gdalcpp::Feature feature(*current_layer, std::move(geometry));
    set_basic_fields(feature, osm_object, id);

    // place and type field
    if (!city_layer) {
        feature.set_field("place", place_value);
        if (place_value_ok(place_value)) {
            feature.set_field("type", place_value);
        } else {
            add_error(osm_object, id, geomtype, "unknown place value");
        }
    }

    // population
    const char* popstr = osm_object.get_value_by_key("population");
    if (popstr) {
        feature.set_field("popstr", popstr);
        char* rest;
        long int population = std::strtol(popstr, &rest, 10);
        if (*rest) {
            add_error(osm_object, id, geomtype, "characters after population number", popstr);
        } else if (population < 20000000000 && population > 0) {
            feature.set_field("population", static_cast<int>(population));
            check_population(osm_object, id, geomtype, place_value, population);
        } else {
            feature.set_field("population", 0);
            add_error(osm_object, id, geomtype, "population number beyond usual range", popstr);
        }
    } else {
        feature.set_field("population", 0);
    }

    // capital
    const char* capitalstr = osm_object.get_value_by_key("capital", "");
    feature.set_field("capitalstr", capitalstr);
    if (is_capital(osm_object.tags())) {
        feature.set_field("capital", 1);
    } else {
        feature.set_field("capital", 0);
    }

    const char* admin_level = osm_object.get_value_by_key("admin_level", "");
    char* rest;
    long int admlvl_int = std::strtol(admin_level, &rest, 10);
    if (*rest) {
        add_error(osm_object, id, geomtype, "characters after admin_level number");
    } else if (admlvl_int < 0 || admlvl_int > 11) {
        add_error(osm_object, id, geomtype, "admin_level number beyond usual range");
    } else {
        feature.set_field("admlvl", static_cast<int>(admlvl_int));
    }

    // name
    const char* name = osm_object.get_value_by_key("name");
    if (name) {
        feature.set_field("name", name);
    } else {
        add_error(osm_object, id, geomtype, "place_without_name");
    }
    feature.add_to_layer();
}

void PlacesHandler::set_basic_fields(gdalcpp::Feature& feature, const osmium::OSMObject& osm_object,
        const osmium::object_id_type id) {
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", id);
    feature.set_field("node_id", idbuffer);
    std::string the_timestamp (osm_object.timestamp().to_iso());
    feature.set_field("lastchange", the_timestamp.c_str());
}

void PlacesHandler::add_error(const osmium::OSMObject& osm_object, const osmium::object_id_type id,
        const char* geomtype, std::string error, std::string different_value /*= ""*/) {
    std::unique_ptr<OGRGeometry> geometry;
    gdalcpp::Layer* error_layer;
    switch (osm_object.type()) {
    case osmium::item_type::node:
        geometry = m_factory.create_point(static_cast<const osmium::Node&>(osm_object));
        error_layer = m_errors_points.get();
        break;
    case osmium::item_type::area:
        geometry = m_factory.create_multipolygon(static_cast<const osmium::Area&>(osm_object));
        error_layer = m_errors_polygons.get();
        break;
    default:
        return;
    }
    gdalcpp::Feature the_feature(*error_layer, std::move(geometry));
    set_basic_fields(the_feature, osm_object, id);
    the_feature.set_field("error", error.c_str());
    if (different_value == "") {
        the_feature.set_field("value", osm_object.get_value_by_key("place", ""));
    } else {
        the_feature.set_field("value", different_value.c_str());
    }
    the_feature.set_field("geomtype", geomtype);
    the_feature.add_to_layer();
}

void PlacesHandler::node(const osmium::Node& node) {
    const char* place = node.get_value_by_key("place");
    if (place) {
        add_feature(m_factory.create_point(node), node, "n", node.id(), place);
        if (!strcmp(place, "city")) {
            add_feature(m_factory.create_point(node), node, "n", node.id(), place, true);
        }
    }
}

void PlacesHandler::area(const osmium::Area& area) {
    const char* place = area.get_value_by_key("place");
    try {
        std::string geomtype;
        if (place) {
            if (area.from_way()) {
                geomtype = "w";
                add_feature(m_factory.create_multipolygon(area), area, geomtype.c_str(), area.orig_id(), place);
            } else {
                geomtype = "r";
                add_feature(m_factory.create_multipolygon(area), area, geomtype.c_str(), area.orig_id(), place);
            }
        }
        if (place && !strcmp(place, "city")) {
            std::unique_ptr<OGRMultiPolygon> multipolygon = m_factory.create_multipolygon(area);
            auto centroid_point = std::unique_ptr<OGRPoint>(new OGRPoint());
            OGRErr centroid_error = multipolygon->Centroid(centroid_point.get());
            if (centroid_error == OGRERR_NONE) {
                add_feature(std::unique_ptr<OGRPoint>(std::move(centroid_point)), area, geomtype.c_str(), area.orig_id(), place, true);
            } else {
                m_verbose_output << "Error creating centroid for area " << area.id() << ": " << centroid_error << "\n";
            }
        }
    } catch (osmium::geometry_error& err) {
        m_verbose_output << err.what();
    } catch (osmium::not_found& err) {
        m_verbose_output << err.what();
    }
}
