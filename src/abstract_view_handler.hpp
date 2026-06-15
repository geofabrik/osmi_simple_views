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

#ifndef SRC_ABSTRACT_VIEW_HANDLER_HPP_
#define SRC_ABSTRACT_VIEW_HANDLER_HPP_

#include <array>
#include <functional>
#include <memory>
#include <vector>
#include <gdalcpp.hpp>
#include <osmium/handler.hpp>
#include <osmium/osm/way.hpp>
#include "ogr_output_base.hpp"

class AbstractViewHandler : public osmium::handler::Handler, public OGROutputBase {

protected:

    static constexpr double UPPER_LIMIT_LATITUDE = 90.0;

    /**
     * Check if all nodes of the way are valid.
     */
    bool all_nodes_valid(const osmium::WayNodeList& wnl);

//    void close_datasets();

    inline bool coordinates_valid(const osmium::Location location) {
#ifdef ONLYMERCATOROUTPUT
        return location.lat() < UPPER_LIMIT_LATITUDE && location.lat() > -UPPER_LIMIT_LATITUDE;
#else
        return m_options.srs != 3857 ||
                (location.lat() < UPPER_LIMIT_LATITUDE && location.lat() > -UPPER_LIMIT_LATITUDE);
#endif
    }

    inline bool coordinates_valid(const osmium::Node& node) {
        return coordinates_valid(node.location());
    }

    inline bool coordinates_valid(const osmium::NodeRefList& nl) {
        for (auto& nr : nl) {
            if (!coordinates_valid(nr.location())) {
                return false;
            }
        }
        return true;
    }

    inline bool coordinates_valid(const osmium::Way& way) {
        return coordinates_valid(way.nodes());
    }

    inline bool coordinates_valid(const osmium::Area& area) {
        for (auto& outer_ring : area.outer_rings()) {
            for (auto it = outer_ring.begin(); it != outer_ring.end(); ++it) {
                if (!coordinates_valid(it->location())) {
                    return false;
                }
            }
            for (auto& r : area.inner_rings(outer_ring)) {
                for (auto it = r.begin(); it != r.end(); ++it) {
                    if (!coordinates_valid(it->location())) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

public:
    AbstractViewHandler() = delete;

    AbstractViewHandler(Options& options);

    virtual ViewType view_type() const = 0;
    virtual std::string view_name() const = 0;

    virtual ~AbstractViewHandler();

    virtual void node(const osmium::Node&) = 0;

    virtual void way(const osmium::Way&) = 0;

    virtual void area(const osmium::Area&) = 0;

    template <size_t TKeyCount>
    std::string selective_tags_str(const osmium::TagList& tags, const char separator, std::array<const char*, TKeyCount> keys) {
        std::string tag_str;
        for (auto k : keys) {
            const char* value = tags.get_value_by_key(k);
            if (!value) {
                continue;
            }
            size_t add_length = strlen(value) + strlen(value) + 2;
            if (add_length < 50 && tag_str.length() + add_length < MAX_FIELD_LENGTH) {
                tag_str += k;
                tag_str += '=';
                tag_str += value;
                tag_str += separator;
            }
        }
        // remove last | from tag_str
        if (!tag_str.empty()) {
            tag_str.pop_back();
        }
        return tag_str;
    }

    /**
     * Build a string of all tags of an object except the provided ones.
     */
    template <size_t TKeyCount>
    std::string tags_string(const osmium::TagList& tags, const char separator,
            std::array<const char*, TKeyCount> excluded_keys) {
        std::string tag_str;
        for (const auto& tag : tags) {
            const char* key = tag.key();
            if (std::any_of(excluded_keys.begin(), excluded_keys.end(), [&key](const char* arr_val){return !strcmp(arr_val, key);})) {
                continue;
            }
            const char* value = tag.value();
            size_t add_length = strlen(value) + strlen(value) + 2;
            if (add_length < 50 && tag_str.length() + add_length < MAX_FIELD_LENGTH) {
                tag_str += key;
                tag_str += '=';
                tag_str += value;
                tag_str += separator;
            }
        }
        // remove last | from tag_str
        tag_str.pop_back();
        return tag_str;
    }

    /**
     * Build a string containing tags (length of key and value below 48 characters)
     * to be inserted into a "tag" column. The returned string is shorter than
     * MAX_FIELD_LENGTH characters. No keys or values will be truncated.
     *
     * \param tags TagList of the OSM object
     * \param not_include key whose value should not be included in the string of all tags.
     * If it is a null pointer, this check is skipped.
     *
     * \returns string with the tags
     */
    std::string tags_string(const osmium::TagList& tags, const char* not_include);

    /**
     * Close all open layers and datasets.
     */
    virtual void close() = 0;

};



#endif /* SRC_ABSTRACT_VIEW_HANDLER_HPP_ */
