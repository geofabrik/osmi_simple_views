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

TEST_CASE("test detection of long strings") {

    SECTION("ASCII") {
        REQUIRE(TaggingViewHandler::char_length_utf8("abcdefghijkl") == 12);
        REQUIRE(TaggingViewHandler::char_length_utf8("abcdef01ijkl") == 12);
        REQUIRE(TaggingViewHandler::char_length_utf8("abef01ijkl") == 10);
    }

    SECTION("Umlauts") {
        REQUIRE(TaggingViewHandler::char_length_utf8("äbcdefghijkl") == 12);
        REQUIRE(TaggingViewHandler::char_length_utf8("lmnöpqrstvwx") == 12);
        REQUIRE(TaggingViewHandler::char_length_utf8("nöpqrstvwx") == 10);
        REQUIRE(TaggingViewHandler::char_length_utf8("nö") == 2);
    }

    SECTION("Encodings with more than two bytes") {
        REQUIRE(TaggingViewHandler::char_length_utf8("カールスルーエ") == 7);
    }
}

TEST_CASE("is_a_x_key_key") {
    const char* whitelist_base = "name";

    SECTION("short_name") {
        const char* key = "short_name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }

    SECTION("official_name") {
        const char* key = "official_name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }

    SECTION("short_name:ru") {
        const char* key = "short_name:ru";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }

    SECTION("named_entity") {
        const char* key = "named_entity";
        REQUIRE_FALSE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }

    SECTION("railway:name") {
        const char* key = "railway:name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }

    SECTION("name") {
        const char* key = "name";
        REQUIRE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }

    SECTION("highway") {
        const char* key = "highway";
        REQUIRE_FALSE(TaggingViewHandler::is_a_x_key_key(key, whitelist_base));
    }
}
