/*
 * places_handler.cpp
 *
 *  Created on:  2017-04-18
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "places_handler.hpp"
#include <iostream>
#include <osmium/index/index.hpp>
#include <osmium/osm/item_type.hpp>

PlacesHandler::PlacesHandler(std::string& output_filename, std::string& output_format,
        std::vector<std::string>& gdal_options, osmium::util::VerboseOutput& verbose_output,
        int epsg /*= 3857*/) :
#ifndef ONLYMERCATOROUTPUT
        m_factory(osmium::geom::Projection(epsg)),
#endif
        m_verbose_output(verbose_output),
        m_dataset(output_format, output_filename, gdalcpp::SRS(epsg)),
        m_points(m_dataset, "points", wkbPoint),
        m_polygons(m_dataset, "polygons", wkbMultiPolygon),
        m_errors_points(m_dataset, "errors_points", wkbPoint),
        m_errors_polygons(m_dataset, "errors_polygons", wkbMultiPolygon)
        {
    m_dataset.enable_auto_transactions(10000);
    // default layer creation options
    if (output_format == "SQlite") {
        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "OFF");
        CPLSetConfigOption("OGR_SQLITE_PRAGMA", "journal_mode=OFF,TEMP_STORE=MEMORY,temp_store=memory,LOCKING_MODE=EXCLUSIVE");
        CPLSetConfigOption("OGR_SQLITE_CACHE", "600");
        CPLSetConfigOption("OGR_SQLITE_JOURNAL", "OFF");
        CPLSetConfigOption("SPATIAL_INDEX", "NO");
        CPLSetConfigOption("COMPRESS_GEOM", "NO");
        CPLSetConfigOption("SPATIALITE", "YES");
    } else if (output_format == "ESRI Shapefile") {
        CPLSetConfigOption("SHAPE_ENCODING", "UTF8");
    }
    // apply layer creation options
    for (std::string& option : gdal_options) {
        size_t equal_sign_pos = option.find("=");
        if (equal_sign_pos != std::string::npos) {
            std::string key = option.substr(0, equal_sign_pos);
            std::string value = option.substr(equal_sign_pos+1, option.size() - 1 - equal_sign_pos);
            CPLSetConfigOption(key.c_str(), value.c_str());
        }
    }
    // add fields to layers
    m_points.add_field("node_id", OFTString, 10);
    m_points.add_field("place", OFTString, 20);
    m_points.add_field("type", OFTString, 20);
    m_points.add_field("popstr", OFTString, 20);
    m_points.add_field("population", OFTInteger, 10);
    m_points.add_field("capitalstr", OFTString, 20);
    m_points.add_field("capital", OFTInteger, 2);
    m_points.add_field("admlvl", OFTInteger, 2);
    m_points.add_field("name", OFTString, 100);
    m_points.add_field("lastchange", OFTString, 21);
    // and the same for the polygons layer
    m_polygons.add_field("node_id", OFTString, 10);
    m_polygons.add_field("place", OFTString, 20);
    m_polygons.add_field("type", OFTString, 20);
    m_polygons.add_field("popstr", OFTString, 20);
    m_polygons.add_field("population", OFTInteger, 10);
    m_polygons.add_field("capitalstr", OFTString, 20);
    m_polygons.add_field("capital", OFTInteger, 2);
    m_polygons.add_field("admlvl", OFTInteger, 2);
    m_polygons.add_field("name", OFTString, 100);
    m_polygons.add_field("lastchange", OFTString, 21);
    // errors layer
    m_errors_points.add_field("node_id", OFTString, 10);
    m_errors_points.add_field("geomtype", OFTString, 1);
    m_errors_points.add_field("error", OFTString, 60);
    m_errors_points.add_field("value", OFTString, 100);
    m_errors_points.add_field("lastchange", OFTString, 21);
    m_errors_polygons.add_field("node_id", OFTString, 10);
    m_errors_polygons.add_field("geomtype", OFTString, 1);
    m_errors_polygons.add_field("error", OFTString, 60);
    m_errors_polygons.add_field("value", OFTString, 100);
    m_errors_polygons.add_field("lastchange", OFTString, 21);
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

void PlacesHandler::add_feature(std::unique_ptr<OGRGeometry>&& geometry, const osmium::OSMObject& osm_object,
        const char* geomtype, const osmium::object_id_type id) {
    gdalcpp::Layer* current_layer = &m_points;
    if (osm_object.type() == osmium::item_type::area) {
        current_layer = &m_polygons;
    }
    gdalcpp::Feature feature(*current_layer, std::move(geometry));
    set_basic_fields(feature, osm_object, id);

    const char* place = osm_object.get_value_by_key("place", "");
    feature.set_field("place", place);
    if (place_value_ok(place)) {
        feature.set_field("type", osm_object.get_value_by_key("place", ""));
    } else {
        add_error(osm_object, id, geomtype, "unknown place value");
    }
    const char* popstr = osm_object.get_value_by_key("population");
    if (popstr) {
        feature.set_field("popstr", popstr);
        char* rest;
        long int value_int = std::strtol(popstr, &rest, 10);
        if (*rest) {
            add_error(osm_object, id, geomtype, "characters after population number");
        } else if (value_int < 100000000 && value_int > 0) {
            feature.set_field("population", static_cast<int>(value_int));
        } else {
            feature.set_field("population", 0);
            add_error(osm_object, id, geomtype, "population number beyond usual range");
        }
    } else {
        feature.set_field("population", 0);
    }
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
    feature.set_field("name", osm_object.get_value_by_key("name", ""));
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
        const char* geomtype, std::string error) {
    std::unique_ptr<OGRGeometry> geometry;
    gdalcpp::Layer* error_layer;
    switch (osm_object.type()) {
    case osmium::item_type::node:
        geometry = m_factory.create_point(static_cast<const osmium::Node&>(osm_object));
        error_layer = &m_errors_points;
        break;
    case osmium::item_type::area:
        geometry = m_factory.create_multipolygon(static_cast<const osmium::Area&>(osm_object));
        error_layer = &m_errors_polygons;
        break;
    default:
        return;
    }
    gdalcpp::Feature the_feature(*error_layer, std::move(geometry));
    set_basic_fields(the_feature, osm_object, id);
    the_feature.set_field("error", error.c_str());
    the_feature.set_field("value", osm_object.get_value_by_key("place", ""));
    the_feature.set_field("geomtype", geomtype);
    the_feature.add_to_layer();
}

void PlacesHandler::node(const osmium::Node& node) {
    const char* place = node.get_value_by_key("place");
    if (place) {
        add_feature(m_factory.create_point(node), node, "n", node.id());
    }
}

void PlacesHandler::area(const osmium::Area& area) {
    const char* place = area.get_value_by_key("place");
    try {
        if (place) {
            if (area.from_way()) {
                add_feature(m_factory.create_multipolygon(area), area, "w", area.orig_id());
            } else {
                add_feature(m_factory.create_multipolygon(area), area, "r", area.orig_id());
            }
        }
    } catch (osmium::geometry_error& err) {
        m_verbose_output << err.what();
    } catch (osmium::not_found& err) {
        m_verbose_output << err.what();
    }
}
