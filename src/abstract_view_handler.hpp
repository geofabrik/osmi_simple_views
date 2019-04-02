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

#include <gdalcpp.hpp>
#include <osmium/handler.hpp>
#include <osmium/osm/way.hpp>
#include "ogr_output_base.hpp"

class AbstractViewHandler : public osmium::handler::Handler, public OGROutputBase {

    /**
     * Get filename suffix by output format with a leading dot.
     *
     * If the format is not found, an empty string is returned.
     *
     * This method is required because GDAL does not provide such a method.
     */
    std::string filename_suffix();

    /**
     * Return capability to create multiple layers with one data source.
     *
     * By default, this method returns false.
     */
    bool one_layer_per_datasource_only();

protected:
    std::string m_output_directory;
    std::string& m_output_format;
    int m_epsg;
    /// ORG dataset
    // This has to be a vector of unique_ptr because a vector of objects themselves fails to
    // compile due to "copy constructor of 'Dataset' is implicitly deleted because field
    // 'm_options' has a deleted copy constructor".
    std::vector<std::unique_ptr<gdalcpp::Dataset>> m_datasets;

    /**
     * Check if all nodes of the way are valid.
     */
    bool all_nodes_valid(const osmium::WayNodeList& wnl);

    void rename_output_files(const std::string& view_name);

public:
    AbstractViewHandler() = delete;

    AbstractViewHandler(std::string& output_directory, std::string& output_format,
            osmium::util::VerboseOutput& verbose_output, int epsg);

    /**
     * Add proper file name suffix to the output files. If there is one output dataset only,
     * give it the name of the view.
     *
     * @arg view_name name of the view.
     */
    virtual void give_correct_name() = 0;

    virtual ~AbstractViewHandler();

    virtual void node(const osmium::Node&) = 0;

    virtual void way(const osmium::Way&) = 0;

    virtual void area(const osmium::Area&) = 0;

    /**
     * Add a new dataset to the vector if the last one cannot be use for multiple layers
     */
    void ensure_writeable_dataset(const char* layer_name);

    /**
     * Get a pointer to the dataset being used. The ownership will stay at AbstractViewHandler.
     */
    gdalcpp::Dataset* get_dataset_pointer(const char* layer_name);

    gdalcpp::Layer create_layer(const char* layer_name, OGRwkbGeometryType type, const std::vector<std::string>& options = {});

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

};



#endif /* SRC_ABSTRACT_VIEW_HANDLER_HPP_ */
