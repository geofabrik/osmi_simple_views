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
#include "catch.hpp"
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/node.hpp>
#include <turn_restrictions_manager.hpp>

using tagmap = std::map<std::string, std::string>;

void assert_validation_result(const ValidationResult& v, const bool expected_valid) {
    CHECK(v.failed() != expected_valid);
}

void set_dummy_osm_object_attributes(osmium::OSMObject& object) {
    object.set_version("1");
    object.set_changeset("5");
    object.set_uid("140");
    object.set_timestamp(osmium::Timestamp("2016-01-05T01:22:45Z"));
}

void add_tags(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder, const tagmap& tags) {
    osmium::builder::TagListBuilder tl_builder(buffer, builder);
    for (tagmap::const_iterator it = tags.begin(); it != tags.end(); it++) {
        tl_builder.add_tag(it->first, it->second);
    }
}

void add_relation_members(osmium::memory::Buffer& buffer, osmium::builder::Builder* builder,
        std::vector<osmium::object_id_type>& member_ids, std::vector<osmium::item_type>& member_types,
        std::vector<std::string>& member_roles) {
    assert(member_ids.size() == member_types.size());
    assert(member_types.size() == member_roles.size());
    osmium::builder::RelationMemberListBuilder rml_builder(buffer, builder);
    for (size_t i = 0; i < member_ids.size(); i++) {
        rml_builder.add_member(member_types.at(i), member_ids.at(i), member_roles.at(i).c_str());
    }
}

template<bool clear_buffer>
osmium::Relation& create_relation(osmium::memory::Buffer& buffer, const osmium::object_id_type id, const tagmap& tags,
        std::vector<osmium::object_id_type>& member_ids, std::vector<osmium::item_type>& member_types,
        std::vector<std::string>& member_roles) {
    if (clear_buffer) {
        buffer.clear();
    }
    osmium::builder::RelationBuilder relation_builder(buffer);
    osmium::Relation& relation = static_cast<osmium::Relation&>(relation_builder.object());
    relation.set_id(id);
    set_dummy_osm_object_attributes(relation);
    relation_builder.set_user("");
    add_tags(buffer, &relation_builder, tags);
    add_relation_members(buffer, &relation_builder, member_ids, member_types, member_roles);
    return relation_builder.object();
}

TEST_CASE("test keys") {
    using VSC = TurnRestrictionsManager::VehicleSuperclass;
    Options opts;
    TurnRestrictionsManager trm {opts};
    CHECK(trm.is_valid_restriction_key("restriction:conditional") == VSC::conditional);
    CHECK(trm.is_valid_restriction_key("restriction:motorcar") == VSC::other);
    CHECK(trm.is_valid_restriction_key("restriction:motorcar:conditional") == VSC::conditional);
    CHECK(trm.is_valid_restriction_key("restriction:bicycle") == VSC::bicycle);
    CHECK(trm.is_valid_restriction_key("restriction:invalid_vehicle") == VSC::invalid);
}

TEST_CASE("test tags") {
    Options opts;
    TurnRestrictionsManager trm {opts};
    static constexpr int buffer_size = 10 * 1000 * 1000;
    osmium::memory::Buffer buffer(buffer_size);
    TurnRestrictionsManager::Restriction restriction;
    tagmap tags {
        {"type", "restriction"},
        {"restriction:hgv", "no_right_turn"}
    };
    std::vector<osmium::object_id_type> member_ids = {2, 3, 4};
    std::vector<osmium::item_type> member_types = {osmium::item_type::way, osmium::item_type::node, osmium::item_type::way};
    std::vector<std::string> member_roles = {"from", "via", "to"};
    SECTION("restriction:hgv") {
        osmium::Relation& rel1 = create_relation<false>(buffer, 1, tags, member_ids, member_types, member_roles);
        assert_validation_result(trm.check_tagging(rel1, restriction), true);
    }
    SECTION("restriction:hgv=* + note=*") {
        tags.emplace("note", "test");
        osmium::Relation& rel1 = create_relation<false>(buffer, 1, tags, member_ids, member_types, member_roles);
        assert_validation_result(trm.check_tagging(rel1, restriction), true);
        tags.erase("restriction:hgv");
    }
    SECTION("restriction:hgv_articulated") {
        tags.emplace("restriction:hgv_articulated", "only_right_turn");
        osmium::Relation& rel2 = create_relation<true>(buffer, 1, tags, member_ids, member_types, member_roles);
        assert_validation_result(trm.check_tagging(rel2, restriction), true);
        tags.erase("hgv_articulated");
    }
    SECTION("restriction=xxINVALIDxx") {
        tags.emplace("restriction", "xxINVALIDxx");
        osmium::Relation& rel3 = create_relation<true>(buffer, 1, tags, member_ids, member_types, member_roles);
        assert_validation_result(trm.check_tagging(rel3, restriction), false);
        tags.erase("restriction");
    }
    SECTION("restriction:conditional=no_right_turn @ (length > 10)") {
        tags.emplace("restriction:conditional", "no_right_turn @ (length > 10)");
        osmium::Relation& rel3 = create_relation<true>(buffer, 1, tags, member_ids, member_types, member_roles);
        assert_validation_result(trm.check_tagging(rel3, restriction), true);
        tags.erase("restriction:conditional");
    }
    SECTION("restriction:hgv=only_straight_on") {
        tags.emplace("restriction:hgv", "only_straight_on");
        osmium::Relation& rel3 = create_relation<true>(buffer, 1, tags, member_ids, member_types, member_roles);
        assert_validation_result(trm.check_tagging(rel3, restriction), true);
    }
}

