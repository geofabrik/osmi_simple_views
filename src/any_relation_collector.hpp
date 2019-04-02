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


#ifndef SRC_ANY_RELATION_COLLECTOR_HPP_
#define SRC_ANY_RELATION_COLLECTOR_HPP_

#include <gdalcpp.hpp>
#include <osmium/relations/collector.hpp>
#include "ogr_output_base.hpp"

class AnyRelationCollector : public osmium::relations::Collector<AnyRelationCollector,
true, true, true>, public OGROutputBase {

    std::unique_ptr<gdalcpp::Layer> m_tagging_ways_without_tags;

public:
    AnyRelationCollector() = delete;

    AnyRelationCollector(osmium::util::VerboseOutput& verbose_output, std::string& output_format,
            int epsg = 3857);

    /**
     * This method decides which relations we're interested in, and
     * instructs Osmium to collect their members for us.
     *
     * Only multipolygons and boundary relations may have way members without any tags.
     * All other types of relations must have members with tags.
     */
    bool keep_relation(const osmium::Relation& relation) const;

    /**
     * Tells Osmium which members to keep for a relation of interest.
     */
    bool keep_member(const osmium::relations::RelationMeta&, const osmium::RelationMember& member) const;

    /**
     * This method is called for all ways that are not a member of
     * any relation.
     *
     * Overwrite this method in a child class if you are interested
     * in this.
     */
    void way_not_in_any_relation(const osmium::Way& way);

    void complete_relation(osmium::relations::RelationMeta&);

    /**
     * Assign the pointer pointing to a dataset.
     */
    void create_layer(gdalcpp::Dataset* dataset);

};

#endif /* SRC_ANY_RELATION_COLLECTOR_HPP_ */
