/*
 *  © 2019 Geofabrik GmbH
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

#include <highway_view_handler.hpp>

bool check_turn(const char* value) {
    return HighwayViewHandler::check_valid_turns(value);
}

bool check_length(const char* value) {
    return HighwayViewHandler::check_length_value(value);
}

bool check_maxweight(const char* value) {
    return HighwayViewHandler::check_maxweight(value);
}

TEST_CASE("test maxweight parsing") {
    REQUIRE(check_maxweight(nullptr));
    REQUIRE(check_maxweight(""));
    REQUIRE(check_maxweight("4"));
    REQUIRE(check_maxweight("48"));
    REQUIRE(check_maxweight("7.5"));
    REQUIRE(check_maxweight("4 t"));
    REQUIRE(check_maxweight("48 t"));
    REQUIRE(check_maxweight("7.5 t"));
    REQUIRE_FALSE(check_maxweight("4t"));
    REQUIRE_FALSE(check_maxweight("48t"));
    REQUIRE_FALSE(check_maxweight("7.5t"));
    REQUIRE(check_maxweight("48 st"));
    REQUIRE(check_maxweight("7.5 lt"));
    REQUIRE(check_maxweight("480 kg"));
    REQUIRE_FALSE(check_maxweight("7,5 lt"));
    REQUIRE_FALSE(check_maxweight("5,5 t"));
    REQUIRE_FALSE(check_maxweight("5,5t"));
    REQUIRE_FALSE(check_maxweight("5.5 "));
    REQUIRE_FALSE(check_maxweight("5.5 9"));
    REQUIRE_FALSE(check_maxweight("5.5 9"));
    REQUIRE_FALSE(check_maxweight("7.5;16"));
}

TEST_CASE("test length parsing") {
    REQUIRE(check_length(nullptr));
    REQUIRE(check_length(""));
    REQUIRE(check_length("4"));
    REQUIRE(check_length("3.8"));
    REQUIRE_FALSE(check_length("3,8"));
    REQUIRE(check_length("4 m"));
    REQUIRE(check_length("3.8 m"));
    REQUIRE(check_length("12'5\""));
    REQUIRE_FALSE(check_length("12.3'5\""));
    REQUIRE_FALSE(check_length("12' 5\""));
    REQUIRE_FALSE(check_length("12'5''"));
    REQUIRE_FALSE(check_length("12'5"));
    REQUIRE_FALSE(check_length("12 ft"));
    REQUIRE(check_length("12'"));
}

TEST_CASE("test valid turn:lane values") {

    SECTION("single values") {
        REQUIRE(check_turn("left"));
        REQUIRE(check_turn("right"));
        REQUIRE(check_turn("through"));
        REQUIRE(check_turn("slight_left"));
        REQUIRE(check_turn("slight_right"));
    }

    SECTION("single invalid values") {
        REQUIRE_FALSE(check_turn(" left"));
        REQUIRE_FALSE(check_turn("right "));
        REQUIRE_FALSE(check_turn("thr ough"));
        REQUIRE_FALSE(check_turn(" slight_left"));
        REQUIRE_FALSE(check_turn("slidght_right"));
        REQUIRE_FALSE(check_turn(""));
        REQUIRE_FALSE(check_turn("d"));
        REQUIRE_FALSE(check_turn(" "));
        REQUIRE_FALSE(check_turn("     "));
    }

    SECTION("valid multiple values") {
        REQUIRE(check_turn("left|through"));
        REQUIRE(check_turn("through|through|right"));
        REQUIRE(check_turn("through|through|slight_right|right"));
        REQUIRE(check_turn("none|none|slight_right"));
    }

    SECTION("valid multiple values with skipped none values") {
        REQUIRE(check_turn("left|"));
        REQUIRE(check_turn("||right"));
        REQUIRE(check_turn("left||right"));
        REQUIRE_FALSE(check_turn(""));
        REQUIRE_FALSE(check_turn("||"));
    }

    SECTION("invalid multiple values") {
        REQUIRE_FALSE(check_turn("left|throuXgh"));
        REQUIRE_FALSE(check_turn("throuXgh|through|right"));
        REQUIRE_FALSE(check_turn("through|throXugh|slight_right|right"));
        REQUIRE(check_turn("none||slight_right"));
        REQUIRE(check_turn("|through|slight_right|right"));
        REQUIRE(check_turn("through|through|slight_right|"));
        REQUIRE(check_turn("through|through||"));
    }

    SECTION("values with semicolons") {
        REQUIRE(check_turn("left;through;right"));
        REQUIRE(check_turn("left;through"));
        REQUIRE(check_turn("left;through|right"));
        REQUIRE(check_turn("left|through;slight_right"));
        REQUIRE(check_turn("left|through|through;slight_right"));
        REQUIRE(check_turn("left|through;slight_right|right|right"));
        REQUIRE(check_turn("left|through;slight_right;right|right"));
        REQUIRE(check_turn("left|through;slight_right|right"));
        REQUIRE(check_turn("none|none|slight_right;right"));
    }

    SECTION("invalid values with semicolons") {
        REQUIRE_FALSE(check_turn("left;through;rEght"));
        REQUIRE_FALSE(check_turn("left;throuXh"));
        REQUIRE_FALSE(check_turn("leDt;through|right"));
        REQUIRE_FALSE(check_turn("|leDt;through|right"));
        REQUIRE_FALSE(check_turn(";left;through|right"));
        REQUIRE_FALSE(check_turn("left|tDhrough|through;slight_right"));
        REQUIRE_FALSE(check_turn("left|through;back|right|right"));
        REQUIRE_FALSE(check_turn("left|through;;slight_Dright;right|right"));
        REQUIRE_FALSE(check_turn("left|through;slight_right|riDght"));
        REQUIRE(check_turn("none||slight_right;right"));
        REQUIRE_FALSE(check_turn("none|slight_right;right;;"));
        REQUIRE_FALSE(check_turn("none|;|"));
    }
}
