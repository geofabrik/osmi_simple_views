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

#include <tagging_view_handler.hpp>

TEST_CASE("is_a_x_key_key") {
    const char* whitelist_base = "name";

    SECTION("short_name") {
        const char* key = "short_name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == true);
    }

    SECTION("official_name") {
        const char* key = "official_name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == true);
    }

    SECTION("short_name:ru") {
        const char* key = "short_name:ru";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == true);
    }

    SECTION("named_entity") {
        const char* key = "named_entity";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == false);
    }

    SECTION("railway:name") {
        const char* key = "railway:name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == true);
    }

    SECTION("highway") {
        const char* key = "short_name:ru";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == true);
    }

    SECTION("highway") {
        const char* key = "short_named:ru";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base) == false);
    }
}
