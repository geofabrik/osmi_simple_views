/*
 *  © 2017 Geofabrik GmbH
 *  © 2012-2016 Jochen Topf <jochen@topf.org>
 * *
 *  This file is part of osmi_simple_views.
 *
 *  This file contains code from OSMCoastline by Jochen Topf, see
 *  http://osmcode.org/osmcoastline/ for more informations.
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


#include "geometry_view_handler.hpp"

#include <vector>
#include <osmium/geom/haversine.hpp>

GeometryViewHandler::GeometryViewHandler(Options& options) :
        AbstractViewHandler(options),
        m_geometry_long_ways(create_layer("geometry_long_ways", wkbLineString, get_gdal_default_layer_options())),
        m_geometry_long_seg_seg(create_layer("geometry_long_seg_seg", wkbLineString, get_gdal_default_layer_options())),
        m_geometry_long_seg_way(create_layer("geometry_long_seg_way", wkbLineString, get_gdal_default_layer_options())),
        m_geometry_single_node_in_way(create_layer("geometry_single_node_in_way", wkbPoint, get_gdal_default_layer_options())),
        m_geometry_duplicate_node_in_way_way(create_layer("geometry_duplicate_node_in_way_way", wkbLineString, get_gdal_default_layer_options())),
        m_geometry_duplicate_node_in_way_node(create_layer("geometry_duplicate_node_in_way_node", wkbPoint, get_gdal_default_layer_options())),
        m_geometry_self_intersection_ways(create_layer("geometry_self_intersection_ways", wkbLineString, get_gdal_default_layer_options())),
        m_geometry_self_intersection_points(create_layer("geometry_self_intersection_points", wkbPoint, get_gdal_default_layer_options())) {
    // add fields to layers
    m_geometry_long_ways->add_field("way_id", OFTString, 10);
    m_geometry_long_ways->add_field("lastchange", OFTString, 21);
    m_geometry_long_ways->add_field("length", OFTInteger, 5);
    m_geometry_long_ways->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    // segments of long ways
    m_geometry_long_seg_seg->add_field("way_id", OFTString, 10);
    m_geometry_long_seg_seg->add_field("lastchange", OFTString, 21);
    m_geometry_long_seg_seg->add_field("length", OFTInteger, 9);
    // ways with long segments
    m_geometry_long_seg_way->add_field("way_id", OFTString, 10);
    m_geometry_long_seg_way->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_geometry_long_seg_way->add_field("lastchange", OFTString, 21);
    // ways with a single node
    m_geometry_single_node_in_way->add_field("way_id", OFTString, 10);
    m_geometry_single_node_in_way->add_field("node_id", OFTString, 10);
    m_geometry_single_node_in_way->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_geometry_single_node_in_way->add_field("lastchange", OFTString, 21);
    // ways with a duplicated node
    m_geometry_duplicate_node_in_way_way->add_field("way_id", OFTString, 10);
    m_geometry_duplicate_node_in_way_way->add_field("node_id", OFTString, 10);
    m_geometry_duplicate_node_in_way_way->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    m_geometry_duplicate_node_in_way_way->add_field("lastchange", OFTString, 21);
    // ways with a duplicated node
    m_geometry_duplicate_node_in_way_node->add_field("way_id", OFTString, 10);
    m_geometry_duplicate_node_in_way_node->add_field("node_id", OFTString, 10);
    m_geometry_duplicate_node_in_way_node->add_field("lastchange", OFTString, 21);
    // ways with self intersection
    m_geometry_self_intersection_ways->add_field("way_id", OFTString, 10);
    m_geometry_self_intersection_ways->add_field("tags", OFTString, MAX_FIELD_LENGTH);
    // self intersections
    m_geometry_self_intersection_points->add_field("node_id", OFTString, 10);
    m_geometry_self_intersection_points->add_field("way_id", OFTString, 10);
    m_geometry_self_intersection_points->add_field("rel_id", OFTString, 10); // TODO why?
}

void GeometryViewHandler::give_correct_name() {
    rename_output_files("geometry");
}

std::string GeometryViewHandler::tags_string(const osmium::TagList& tags) {
    std::string tag_str;
    // only add tags to the tags string if their key and value are shorter than 50 characters
    for (const osmium::Tag& t : tags) {
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

void GeometryViewHandler::handle_way_many_nodes(const osmium::Way& way) {
    gdalcpp::Feature feature(*m_geometry_long_ways, m_factory.create_linestring(way));
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", way.id());
    feature.set_field("way_id", idbuffer);
    feature.set_field("length", static_cast<int>(way.nodes().size()));
    std::string the_timestamp (way.timestamp().to_iso());
    feature.set_field("lastchange", the_timestamp.c_str());
    feature.set_field("tags", tags_string(way.tags()).c_str());
    feature.add_to_layer();
}

std::unique_ptr<OGRGeometry> GeometryViewHandler::build_linestring_from_segment(osmium::WayNodeList::const_iterator start,
        osmium::WayNodeList::const_iterator end) {
    m_factory.linestring_start();
    size_t linestring_length = m_factory.fill_linestring(start, end);
    return m_factory.linestring_finish(linestring_length);
}

bool GeometryViewHandler::check_segments_length(const osmium::Way& way) {
    bool long_segment = false;
    for (osmium::WayNodeList::const_iterator it = way.nodes().cbegin();
            it != (way.nodes().cend() - 1); ++it) {
        if (!(it->location().valid()) || !((it + 1)->location().valid())) {
            continue;
        }
        double length = osmium::geom::haversine::distance(it->location(), (it + 1)->location());
        // 0.3 degree is about 21 km near 49.0° N
        if (length > 20000) {
            long_segment = true;
            // build_linestring_from_segment(osmium::WayNodeList::const_iterator, osmium::WayNodeList::const_iterator)
            // has to be called with it+2 as second argument because this will be used as it != end in a for loop.
            gdalcpp::Feature feature(*m_geometry_long_seg_seg, build_linestring_from_segment(it, (it + 2)));
            static char idbuffer[20];
            sprintf(idbuffer, "%ld", way.id());
            feature.set_field("way_id", idbuffer);
            feature.set_field("length", static_cast<int>(length));
            std::string the_timestamp (way.timestamp().to_iso());
            feature.set_field("lastchange", the_timestamp.c_str());
            feature.add_to_layer();
        }
    }
    return long_segment;
}

void GeometryViewHandler::handle_long_segments(const osmium::Way& way) {
    if (check_segments_length(way)) {
        gdalcpp::Feature feature(*m_geometry_long_seg_way, m_factory.create_linestring(way));
        static char idbuffer[20];
        sprintf(idbuffer, "%ld", way.id());
        feature.set_field("way_id", idbuffer);
        feature.set_field("tags", tags_string(way.tags()).c_str());
        std::string the_timestamp (way.timestamp().to_iso());
        feature.set_field("lastchange", the_timestamp.c_str());
        feature.add_to_layer();
    }
}

void GeometryViewHandler::single_node_in_way(const osmium::Way& way) {
    gdalcpp::Feature feature(*m_geometry_single_node_in_way, m_factory.create_point(way.nodes().front()));
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", way.id());
    feature.set_field("way_id", idbuffer);
    static char idbuffer2[20];
    sprintf(idbuffer2, "%ld", way.nodes().front().ref());
    feature.set_field("node_id", idbuffer2);
    feature.set_field("tags", tags_string(way.tags()).c_str());
    std::string the_timestamp (way.timestamp().to_iso());
    feature.set_field("lastchange", the_timestamp.c_str());
    feature.add_to_layer();
}

void GeometryViewHandler::duplicated_node_in_way(const osmium::Way& way) {
    // prevent that a way with duplicates is written twice
    bool multiple_errors = false;
    for (osmium::WayNodeList::const_iterator it = way.nodes().cbegin(); it != way.nodes().cend() - 1;
            ++it) {
        osmium::WayNodeList::const_iterator next = it + 1;
        if (!(it->location().valid()) || !(next->location().valid())) {
            continue;
        }
        if (it->ref() == next->ref() || (it->lat() == next->lat() && it->lon() == next->lon())) {
            gdalcpp::Feature feature(*m_geometry_duplicate_node_in_way_node, m_factory.create_point(*it));
            static char idbuffer[20];
            sprintf(idbuffer, "%ld", way.id());
            feature.set_field("way_id", idbuffer);
            static char idbuffer2[20];
            sprintf(idbuffer2, "%ld", it->ref());
            feature.set_field("node_id", idbuffer2);
            std::string the_timestamp (way.timestamp().to_iso());
            feature.set_field("lastchange", the_timestamp.c_str());
            feature.add_to_layer();
            if (!multiple_errors) {
                gdalcpp::Feature way_feature(*m_geometry_duplicate_node_in_way_way, m_factory.create_linestring(way));
                way_feature.set_field("way_id", idbuffer);
                way_feature.set_field("node_id", idbuffer2);
                way_feature.set_field("tags", tags_string(way.tags()).c_str());
                std::string the_timestamp (way.timestamp().to_iso());
                way_feature.set_field("lastchange", the_timestamp.c_str());
                way_feature.add_to_layer();
            }
            multiple_errors = true;
        }
    }
}

/**
 * This method was copied from
 * [OSMCoastline](https://github.com/osmcode/osmcoastline/blob/master/src/coastline_ring_collection.cpp)
 */
/*static*/ osmium::Location GeometryViewHandler::intersection(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment&s2) {
    if (s1.first()  == s2.first()  ||
        s1.first()  == s2.second() ||
        s1.second() == s2.first()  ||
        s1.second() == s2.second()) {
        return osmium::Location();
    }

    const double denom = ((s2.second().lat() - s2.first().lat())*(s1.second().lon() - s1.first().lon())) -
                   ((s2.second().lon() - s2.first().lon())*(s1.second().lat() - s1.first().lat()));

    if (denom != 0) {
        const double nume_a = ((s2.second().lon() - s2.first().lon())*(s1.first().lat() - s2.first().lat())) -
                              ((s2.second().lat() - s2.first().lat())*(s1.first().lon() - s2.first().lon()));

        const double nume_b = ((s1.second().lon() - s1.first().lon())*(s1.first().lat() - s2.first().lat())) -
                              ((s1.second().lat() - s1.first().lat())*(s1.first().lon() - s2.first().lon()));

        if ((denom > 0 && nume_a >= 0 && nume_a <= denom && nume_b >= 0 && nume_b <= denom) ||
            (denom < 0 && nume_a <= 0 && nume_a >= denom && nume_b <= 0 && nume_b >= denom)) {
            const double ua = nume_a / denom;
            const double ix = s1.first().lon() + ua*(s1.second().lon() - s1.first().lon());
            const double iy = s1.first().lat() + ua*(s1.second().lat() - s1.first().lat());
            return osmium::Location(ix, iy);
        }
    }

    return osmium::Location();
}

/**
 * This method was copied from
 * [OSMCoastline](https://github.com/osmcode/osmcoastline/blob/master/src/coastline_ring_collection.cpp)
 */
/*static*/ bool GeometryViewHandler::outside_x_range(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2) {
    if (s1.first().x() > s2.second().x()) {
        return true;
    }
    return false;
}

/**
 * This method was copied from
 * [OSMCoastline](https://github.com/osmcode/osmcoastline/blob/master/src/coastline_ring_collection.cpp)
 */
/*static*/ bool GeometryViewHandler::y_range_overlap(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2) {
    const int tmin = s1.first().y() < s1.second().y() ? s1.first().y( ) : s1.second().y();
    const int tmax = s1.first().y() < s1.second().y() ? s1.second().y() : s1.first().y();
    const int omin = s2.first().y() < s2.second().y() ? s2.first().y()  : s2.second().y();
    const int omax = s2.first().y() < s2.second().y() ? s2.second().y() : s2.first().y();
    if (tmin > omax || omin > tmax) {
        return false;
    }
    return true;
}

void GeometryViewHandler::add_self_intersection_way(const osmium::Way& way, bool already_flagged) {
    if (already_flagged) {
        return;
    }
    gdalcpp::Feature feature(*m_geometry_self_intersection_ways, m_factory.create_linestring(way));
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", way.id());
    feature.set_field("way_id", idbuffer);
    feature.set_field("tags", tags_string(way.tags()).c_str());
    feature.add_to_layer();
}

void GeometryViewHandler::add_self_intersection_point(const osmium::Location& location, const osmium::object_id_type way_id,
        const osmium::object_id_type node_id /*= 0*/) {
    gdalcpp::Feature feature(*m_geometry_self_intersection_points, m_factory.create_point(location));
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", way_id);
    feature.set_field("way_id", idbuffer);
    static char idbuffer2[20];
    sprintf(idbuffer2, "%ld", node_id);
    feature.set_field("node_id", idbuffer2);
    feature.add_to_layer();
}

/**
 * This method was copied and modified from
 * [OSMCoastline](https://github.com/osmcode/osmcoastline/blob/master/src/coastline_ring_collection.cpp)
 */
void GeometryViewHandler::check_self_intersection(const osmium::Way& way) {
    std::vector<osmium::UndirectedSegment> segments;
    for (size_t i = 0; i != way.nodes().size() - 1; ++i) {
        if (!way.nodes()[i].location().valid() || !way.nodes()[i+1].location().valid()) {
            continue;
        }
        segments.emplace_back(way.nodes()[i].location(), way.nodes()[i+1].location());
    }
    bool way_has_error = false;
    // sorting the segments saves about 16 seconds just for Germany
    std::sort(segments.begin(), segments.end());
    for (std::vector<osmium::UndirectedSegment>::iterator it1 = segments.begin(); it1 != segments.end()-1; ++it1) {
        osmium::UndirectedSegment& s1 = *it1;
        for (std::vector<osmium::UndirectedSegment>::iterator it2 = it1 + 1; it2 != segments.end(); ++it2) {
            osmium::UndirectedSegment& s2 = *it2;
            if (s1 == s2) {
                add_self_intersection_way(way, way_has_error);
                way_has_error = true;
                add_self_intersection_point(it1->first(), way.id(), 0);
                add_self_intersection_point(it1->second(), way.id(), 0);
            } else {
                if (outside_x_range(s2, s1)) {
                    break;
                }
                if (y_range_overlap(s1, s2)) {
                    osmium::Location i = intersection(s1, s2);
                    if (i) {
                        add_self_intersection_way(way, way_has_error);
                        way_has_error = true;
                        add_self_intersection_point(i, way.id(), 0);
                    }
                }
            }
        }
    }
}

bool GeometryViewHandler::way_is_degenerated(const osmium::WayNodeList& nodes) {
    if (nodes.size() == 1) {
        return true;
    }
    for (osmium::WayNodeList::const_iterator it = nodes.cbegin(); it != nodes.cend() - 1;
            ++it) {
        osmium::WayNodeList::const_iterator next = it + 1;
        // If we found two consecutive nodes which have different IDs and locations,
        // we are able to build a valid LineString. For most valid ways the loop will be
        // iterated only one time.
        if (next->lat() != it->lat() || next->lon() != it->lon()) {
            return false;
        }
    }
    return true;
}

void GeometryViewHandler::way(const osmium::Way& way) {
    if (!all_nodes_valid(way.nodes())) {
        return;
    }
    if (way.nodes().size() >= 1900) {
        handle_way_many_nodes(way);
    }
    if (way_is_degenerated(way.nodes())) {
        single_node_in_way(way);
        // no more checks necessary
        return;
    }
    handle_long_segments(way);
    duplicated_node_in_way(way);
    check_self_intersection(way);
}

void GeometryViewHandler::close() {
    m_geometry_long_ways.reset();
    m_geometry_long_seg_seg.reset();
    m_geometry_long_seg_way.reset();
    m_geometry_single_node_in_way.reset();
    m_geometry_duplicate_node_in_way_way.reset();
    m_geometry_duplicate_node_in_way_node.reset();
    m_geometry_self_intersection_ways.reset();
    m_geometry_self_intersection_points.reset();
    close_datasets();
}
