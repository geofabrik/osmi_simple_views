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

TEST_CASE("test valid turn:lane values") {

    SECTION("single values") {
        REQUIRE(check_turn("left") == true);
        REQUIRE(check_turn("right") == true);
        REQUIRE(check_turn("through") == true);
        REQUIRE(check_turn("slight_left") == true);
        REQUIRE(check_turn("slight_right") == true);
    }

    SECTION("single invalid values") {
        REQUIRE(check_turn(" left") == false);
        REQUIRE(check_turn("right ") == false);
        REQUIRE(check_turn("thr ough") == false);
        REQUIRE(check_turn(" slight_left") == false);
        REQUIRE(check_turn("slidght_right") == false);
        REQUIRE(check_turn("") == false);
        REQUIRE(check_turn("d") == false);
        REQUIRE(check_turn(" ") == false);
        REQUIRE(check_turn("     ") == false);
    }

    SECTION("valid multiple values") {
        REQUIRE(check_turn("left|through") == true);
        REQUIRE(check_turn("through|through|right") == true);
        REQUIRE(check_turn("through|through|slight_right|right") == true);
        REQUIRE(check_turn("none|none|slight_right") == true);
    }

    SECTION("invalid multiple values") {
        REQUIRE(check_turn("left|throuXgh") == false);
        REQUIRE(check_turn("throuXgh|through|right") == false);
        REQUIRE(check_turn("through|throXugh|slight_right|right") == false);
        REQUIRE(check_turn("none||slight_right") == false);
        REQUIRE(check_turn("|through|slight_right|right") == false);
        REQUIRE(check_turn("through|through|slight_right|") == false);
        REQUIRE(check_turn("through|through||") == false);
    }

    SECTION("values with semicolons") {
        REQUIRE(check_turn("left;through;right") == true);
        REQUIRE(check_turn("left;through") == true);
        REQUIRE(check_turn("left;through|right") == true);
        REQUIRE(check_turn("left|through;slight_right") == true);
        REQUIRE(check_turn("left|through|through;slight_right") == true);
        REQUIRE(check_turn("left|through;slight_right|right|right") == true);
        REQUIRE(check_turn("left|through;slight_right;right|right") == true);
        REQUIRE(check_turn("left|through;slight_right|right") == true);
        REQUIRE(check_turn("none|none|slight_right;right") == true);
    }

    SECTION("invalid values with semicolons") {
        REQUIRE(check_turn("left;through;rEght") == false);
        REQUIRE(check_turn("left;throuXh") == false);
        REQUIRE(check_turn("leDt;through|right") == false);
        REQUIRE(check_turn("|leDt;through|right") == false);
        REQUIRE(check_turn(";left;through|right") == false);
        REQUIRE(check_turn("left|tDhrough|through;slight_right") == false);
        REQUIRE(check_turn("left|through;back|right|right") == false);
        REQUIRE(check_turn("left|through;;slight_Dright;right|right") == false);
        REQUIRE(check_turn("left|through;slight_right|riDght") == false);
        REQUIRE(check_turn("none||slight_right;right") == false);
        REQUIRE(check_turn("none|slight_right;right;;") == false);
        REQUIRE(check_turn("none|;|") == false);
    }
}
