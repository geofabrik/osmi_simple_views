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

#include "abstract_view_handler.hpp"

#include <stdio.h>
#include <unistd.h>
#include <locale>

AbstractViewHandler::AbstractViewHandler(std::string& output_directory, std::string& output_format,
        osmium::util::VerboseOutput& verbose_output, int epsg) :
        m_output_directory(output_directory),
        m_output_format(output_format),
        m_epsg(epsg),
        OGROutputBase(verbose_output, output_format, epsg),
        m_datasets() {
}

AbstractViewHandler::~AbstractViewHandler() {
}

/**
 * Do a case-insensitive string comparison assuming that the right side is lower case.
 */
bool case_insensitive_comp_left(const std::string& a, const std::string& b) {
    return (
        a.size() == b.size()
        && std::equal(
            a.begin(), a.end(), b.begin(),
            [](const char c, const char d) {
                return c == d || std::tolower(c) == d;
            })
    );
}

bool AbstractViewHandler::one_layer_per_datasource_only() {
    return case_insensitive_comp_left(m_output_format, "geojson")
        || case_insensitive_comp_left(m_output_format, "esri shapefile");
}

void AbstractViewHandler::close_datasets() {
    m_datasets.clear();
}

std::string AbstractViewHandler::filename_suffix() {
    if (case_insensitive_comp_left(m_output_format, "geojson")) {
        return ".json";
    } else if (case_insensitive_comp_left(m_output_format, "sqlite")) {
        return ".db";
    }
    return "";
}

void AbstractViewHandler::rename_output_files(const std::string& view_name) {
    if (m_datasets.size() == 1 && filename_suffix().length()) {
        // rename output file if there is one output dataset only
        std::string destination_name {m_output_directory};
        destination_name += '/';
        destination_name += view_name;
        destination_name += filename_suffix();
        if (access(destination_name.c_str(), F_OK) == 0) {
            std::cerr << "ERROR: Cannot rename output file from to " << destination_name << " because file exists already.\n";
        } else {
            if (rename(m_datasets.front()->dataset_name().c_str(), destination_name.c_str())) {
                std::cerr << "ERROR: Rename from " << m_datasets.front()->dataset_name() << " to " << destination_name << "failed.\n";
            }
        }
    } else if (m_datasets.size() > 1 && filename_suffix().length()) {
        for (auto& d: m_datasets) {
            std::string destination_name = d->dataset_name();
            destination_name += filename_suffix();
            if (access(destination_name.c_str(), F_OK) == 0) {
                std::cerr << "ERROR: Cannot rename output file from to " << destination_name << " because file exists already.\n";
            } else {
                if (rename(d->dataset_name().c_str(), destination_name.c_str())) {
                    std::cerr << "ERROR: Rename from " << m_datasets.front()->dataset_name() << " to " << destination_name << "failed.\n";
                }
            }
        }
    }
}

bool AbstractViewHandler::all_nodes_valid(const osmium::WayNodeList& wnl) {
    for (const osmium::NodeRef& nd_ref : wnl) {
        if (!nd_ref.location().valid()) {
            m_verbose_output << "Invalid location for node " << nd_ref.ref() << "\n";
            return false;
        }
    }
    return true;
}

void AbstractViewHandler::ensure_writeable_dataset(const char* layer_name) {
    if (m_datasets.empty() || one_layer_per_datasource_only()) {
        std::string output_filename = m_output_directory;
        output_filename += '/';
        output_filename += layer_name;
        std::unique_ptr<gdalcpp::Dataset> ds {new gdalcpp::Dataset(m_output_format, output_filename, gdalcpp::SRS(m_epsg), get_gdal_default_dataset_options(m_output_format))};
        m_datasets.push_back(std::move(ds));
        m_datasets.back()->enable_auto_transactions(10000);
    }
}

gdalcpp::Dataset* AbstractViewHandler::get_dataset_pointer(const char* layer_name) {
    ensure_writeable_dataset(layer_name);
    return m_datasets.back().get();
}

std::unique_ptr<gdalcpp::Layer> AbstractViewHandler::create_layer(const char* layer_name, OGRwkbGeometryType type,
        const std::vector<std::string>& options /*= {}*/) {
    ensure_writeable_dataset(layer_name);
    return std::unique_ptr<gdalcpp::Layer>{new gdalcpp::Layer(*(m_datasets.back()), layer_name, type, options)};
}

std::string AbstractViewHandler::tags_string(const osmium::TagList& tags, const char* not_include) {
    std::string tag_str;
    // only add tags to the tags string if their key and value are shorter than 50 characters
    for (const osmium::Tag& t : tags) {
        if (not_include && !strcmp(t.key(), not_include)) {
            continue;
        }
        size_t add_length = strlen(t.key()) + strlen(t.value()) + 2;
        if (add_length < 50 && tag_str.length() + add_length < MAX_FIELD_LENGTH) {
            tag_str += t.key();
            tag_str += '=';
            tag_str += t.value();
            tag_str += '|';
        }
    }
    // remove last | from tag_str
    tag_str.pop_back();
    return tag_str;
}
