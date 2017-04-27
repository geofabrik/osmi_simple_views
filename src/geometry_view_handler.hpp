/*
 * geometrie_view_handler.hpp
 *
 *  Created on:  2017-04-24
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#ifndef SRC_GEOMETRY_VIEW_HANDLER_HPP_
#define SRC_GEOMETRY_VIEW_HANDLER_HPP_

#include <gdalcpp.hpp>

#include <osmium/handler.hpp>
#include <osmium/geom/ogr.hpp>

#ifdef ONLYMERCATOROUTPUT
    #include <osmium/geom/mercator_projection.hpp>
#else
    #include <osmium/geom/projection.hpp>
#endif

#include <osmium/osm/node.hpp>
#include <osmium/osm/undirected_segment.hpp>

#include "abstract_view_handler.hpp"

class GeometryViewHandler : public AbstractViewHandler {
    /// layer for ways which have many nodes
    gdalcpp::Layer m_geometry_long_ways;
    /// layer for segments which are very long
    gdalcpp::Layer m_geometry_long_seg_seg;
    /// layer for ways which have very long segments
    gdalcpp::Layer m_geometry_long_seg_way;
    /// layer for ways which have only a single node
    gdalcpp::Layer m_geometry_single_node_in_way;
    /// layer for ways which have a duplicated node
    gdalcpp::Layer m_geometry_duplicate_node_in_way_way;
    /// layer for duplicated nodes in a way
    gdalcpp::Layer m_geometry_duplicate_node_in_way_node;
    /// layer for ways which intersect themselves
    gdalcpp::Layer m_geometry_self_intersection_ways;
    /// layer for intersection points of self intersecting ways
    gdalcpp::Layer m_geometry_self_intersection_points;

    /// maximum length of a string field
    static constexpr size_t MAX_FIELD_LENGTH = 254;

    /**
     * Add a feature to the output layers.
     *
     * This method will select on its own which layer to use.
     *
     * \param geometry geometry to be written
     * \param osm_object OSM object to be written
     * \param geomtype char representing the type of the OSM object (n = node, w = way, r = relation)
     * \param id ID of the OSM object
     */
    void add_feature(std::unique_ptr<OGRGeometry>&& geometry, const osmium::OSMObject& osm_object,
            const char* geomtype, const osmium::object_id_type id);

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
     */
    void add_error(const osmium::OSMObject& osm_object, const osmium::object_id_type id,
            const char* geomtype, std::string error);

    /**
     * Build a string containing tags (length of key and value below 48 characters)
     * to be inserted into a "tag" column. The returned string is shorter than
     * MAX_FIELD_LENGTH characters. No keys or values will be truncated.
     */
    std::string tags_string(const osmium::TagList& tags);

    /**
     * Build a linestring from a part of a WayNodeList.
     */
    std::unique_ptr<OGRGeometry> build_linestring_from_segment(osmium::WayNodeList::const_iterator start,
            osmium::WayNodeList::const_iterator end);

    /**
     * Check if a way has segments longer than 2000 metres. If yes, write them to the output layer for
     * overlong segments.
     */
    bool check_segments_length(const osmium::Way& way);

    /**
     * Check if a way has segments longer than 2000 metres. If yes, write the whole way to the output layer for
     * overlong segments.
     *
     * This method calls check_segments_length(const osmium::Way&).
     */
    void handle_way_many_nodes(const osmium::Way& way);

    void handle_long_segments(const osmium::Way& way);

    /**
     * Write a node to the output layer for ways which have only a single node.
     */
    void single_node_in_way(const osmium::Way& way);

    /**
     * Check if two consecutive nodes are the same (equality of ID) and write the error to the output layers.
     */
    void duplicated_node_in_way(const osmium::Way& way);

    /**
     * Get intersection point of two segments.
     */
    static osmium::Location intersection(const osmium::Segment& s1, const osmium::Segment&s2);

    /**
     * Check if the intervals of two segments on the x axis overlap. If not, an intersection is impossible.
     */
    static bool outside_x_range(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2);

    /**
     * Check if the intervals of two segments on the y axis overlap. If not, an intersection is impossible.
     */
    static bool y_range_overlap(const osmium::UndirectedSegment& s1, const osmium::UndirectedSegment& s2);

    /**
     * Write a whole way which has a self intersection to the output layer.
     */
    void add_self_intersection_way(const osmium::Way& way, bool already_flagged);

    /**
     * Write an intersection point to the output layer.
     */
    void add_self_intersection_point(const osmium::Location& location, const osmium::object_id_type way_id,
            const osmium::object_id_type node_id = 0);

    /**
     * Check if the way has a self intersection and write the intersection points to the output layer.
     */
    void check_self_intersection(const osmium::Way& way);

    /**
     * Check if a way is degenerated.
     *
     * A way is degenerated if it has only one node or if it lacks of a pair of two consecutive nodes with
     * different ID and different location.
     */
    bool way_is_degenerated(const osmium::WayNodeList& nodes);

public:
    GeometryViewHandler() = delete;

    GeometryViewHandler(std::string& output_filename, std::string& output_format, std::vector<std::string>& gdal_options,
            osmium::util::VerboseOutput& verbose_output, int epsg = 3857);

    void way(const osmium::Way& way);
};



#endif /* SRC_GEOMETRY_VIEW_HANDLER_HPP_ */
