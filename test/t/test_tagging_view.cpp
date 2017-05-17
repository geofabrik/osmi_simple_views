/*
 * test_intersection.cpp
 *
 *  Created on:  2017-05-09
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
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
