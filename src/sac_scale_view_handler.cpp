/*
 * highway_properties_view.cpp
 *
 *  Created on: 2022-08-30
 *      Author: Michael Reichert
 */

#include "sac_scale_view_handler.hpp"

const std::array<const char*, 6> valid_sac_scales = {"hiking", "mountain_hiking",
        "demanding_mountain_hiking", "alpine_hiking", "demanding_alpine_hiking",
        "difficult_alpine_hiking"};

const std::array<const char*, 4> valid_highways = {"path", "footway", "track", "steps"};

/// Surfaces which are unlikely with SAC scale T1 and T2
const std::array<const char*, 18> good_surface_values = {"paved", "asphalt", "concrete",
        "concrete:lanes", "concrete:plates", "paving_stones", "sett", "cobblestone", "compacted",
        "grass_paver", "metal", "tartan", "clay", "artificial_turf", "unhewn_cobblestone",
        "woodchips", "brick", "chipseal"};

/// Surfaces which can be accepted without a sac_scale=* tag.
const std::array<const char*, 4> medium_surface_values = {"gravel", "fine_gravel", "sand",
        "pebblestone"};

/// Surfaces requiring a sac_scale=* tag.
const std::array<const char*, 4> bad_surface_values = {"rock", "rocks", "stone", "ground"};

/// Values forbidding access.
const std::array<const char*, 2> no_values = {"no", "private"};

SacScaleViewHandler::SacScaleViewHandler(Options& options) :
		AbstractViewHandler(options),
		m_sac_scale(create_layer("sac_scale", wkbLineString)),
		m_sac_scale_warnings(create_layer("sac_scale_warnings", wkbLineString)),
		m_sac_scale_errors(create_layer("sac_scale_errors", wkbLineString)) {
	m_sac_scale->add_field("way_id", OFTString, 10);
	m_sac_scale->add_field("highway", OFTString, 40);
	m_sac_scale->add_field("sac_scale", OFTString, 40);
	m_sac_scale->add_field("surface", OFTString, 40);
	m_sac_scale->add_field("width", OFTString, 40);
	m_sac_scale->add_field("tags", OFTString, MAX_FIELD_LENGTH);
	m_sac_scale_warnings->add_field("way_id", OFTString, 10);
	m_sac_scale_warnings->add_field("warning", OFTString, 50);
	m_sac_scale_warnings->add_field("highway", OFTString, 40);
	m_sac_scale_warnings->add_field("sac_scale", OFTString, 40);
	m_sac_scale_warnings->add_field("surface", OFTString, 40);
	m_sac_scale_warnings->add_field("width", OFTString, 40);
	m_sac_scale_warnings->add_field("tags", OFTString, MAX_FIELD_LENGTH);
	m_sac_scale_errors->add_field("way_id", OFTString, 10);
	m_sac_scale_errors->add_field("error", OFTString, 50);
	m_sac_scale_errors->add_field("highway", OFTString, 40);
	m_sac_scale_errors->add_field("sac_scale", OFTString, 40);
	m_sac_scale_errors->add_field("surface", OFTString, 40);
	m_sac_scale_errors->add_field("width", OFTString, 40);
	m_sac_scale_errors->add_field("tags", OFTString, MAX_FIELD_LENGTH);
}

void SacScaleViewHandler::give_correct_name() {
    rename_output_files("highway_properties");
}

void SacScaleViewHandler::close() {
	m_sac_scale.reset();
    m_sac_scale_warnings.reset();
    m_sac_scale_errors.reset();
    close_datasets();
}

bool SacScaleViewHandler::surface_matches_sac_scale(const osmium::Way& way,
        const char* sac_scale) {
    const char* surface = way.get_value_by_key("surface");
    if (!surface) {
        return true;
    }
    size_t sac_num = 1;
    for (auto&& s : valid_sac_scales) {
        if (!strcmp(s, sac_scale)) {
            break;
        }
        ++sac_num;
    }
    return !(sac_num > 2 && value_in_array(surface, good_surface_values));
}

void SacScaleViewHandler::process_sac_scale(const osmium::Way& way) {
	const char* highway = way.get_value_by_key("highway");
    const char* abandoned_highway = way.get_value_by_key("abandoned:highway");
    const char* disused_highway = way.get_value_by_key("disused:highway");
	if (!highway && !abandoned_highway && !disused_highway) {
	    add_to_layer(*m_sac_scale_errors, way, highway, nullptr, "error",
	            "sac_scale without highway");
	}
	const char* sac_scale = way.get_value_by_key("sac_scale");
	bool valid_sac = value_in_array(sac_scale, valid_sac_scales);
	bool highway_valid_for_sac = value_in_array(highway, valid_highways)
	        || value_in_array(abandoned_highway, valid_highways) || value_in_array(disused_highway, valid_highways);
	if (!highway_valid_for_sac) {
	    add_to_layer(*m_sac_scale_warnings, way, highway, sac_scale, "warning",
	            "sac_scale on highway!=path/footway/track");
	} else if (!valid_sac) {
        add_to_layer(*m_sac_scale_errors, way, highway, sac_scale, "error",
                "invalid sac_scale value");
	} else {
        add_to_layer(*m_sac_scale, way, highway, sac_scale);
        if (!surface_matches_sac_scale(way, sac_scale)) {
            add_to_layer(*m_sac_scale_warnings, way, highway, sac_scale, "warning",
                    "sac_scale/surface mismatch");
        }
	}
}

void SacScaleViewHandler::process_missing_sac_scale(const osmium::Way& way,
        const char* highway) {
    const char* footway = way.get_value_by_key("footway");
    if (!strcmp(highway, "footway") && footway
            && (!strcmp(footway, "sidewalk") || !strcmp(footway, "crossing"))) {
        return;
    }
    // mtb:scale:imba is used in bike parks, those ways are dedicated for mountainbikers.
    if (way.tags().has_key("mtb:scale:imba") || way.tags().has_tag("tunnel", "building_passage")
            || way.tags().has_tag("tunnel", "yes") || way.tags().has_tag("indoor", "yes")
            || way.tags().has_tag("tactile_paving", "yes")
            || way.tags().has_tag("man_made", "pier") || way.tags().has_key("level")
            || way.tags().has_tag("lit", "yes")) {
        return;
    }
    const char* foot = way.get_value_by_key("foot");
    const char* osm_access = way.get_value_by_key("access");
    if ((foot && value_in_array(foot, no_values))
            || (!foot && osm_access && value_in_array(osm_access, no_values))) {
        // foot=no/private
        // or foot no set and access=no/private
        // In those cases, it makes limited sense to map the sac_scale=*.
        // At least, it becomes difficult to survey the way legally.
        return;
    }
    const char* surface = way.get_value_by_key("surface");
    if (way.tags().has_tag("segregated", "yes")
            || value_in_array(surface, good_surface_values)
            || value_in_array(surface, medium_surface_values)) {
        return;
    }
    if (surface && (value_in_array(surface, good_surface_values)
            || value_in_array(surface, medium_surface_values))) {
        return;
    }
    const char* mtb_scale_uphill = way.get_value_by_key("mtb:scale:uphill");
    if (mtb_scale_uphill && (!strcmp(mtb_scale_uphill, "0") || !strcmp(mtb_scale_uphill, "1"))) {
        return;
    }
    const char* mtb_scale = way.get_value_by_key("mtb:scale");
    if (mtb_scale && (!strcmp(mtb_scale, "0"))) {
        return;
    }
    if (surface && value_in_array(surface, bad_surface_values)) {
        add_to_layer(*m_sac_scale_warnings, way, way.get_value_by_key("highway"), nullptr,
                "warning", "sac_scale missing");
    } else {
        add_to_layer(*m_sac_scale_warnings, way, way.get_value_by_key("highway"), nullptr,
                "warning", "sac_scale or surface recommended");
    }
}

void SacScaleViewHandler::add_to_layer(gdalcpp::Layer& layer, const osmium::Way& way,
        const char* highway, const char* sac_scale, const char* extra_field,
        const char* extra_value) {
    try {
        gdalcpp::Feature feature(layer, m_factory.create_linestring(way));
        static char idbuffer[20];
        sprintf(idbuffer, "%ld", way.id());
        feature.set_field("way_id", idbuffer);
        if (highway) {
            feature.set_field("highway", way.get_value_by_key("highway"));
        }
        if (sac_scale) {
            feature.set_field("sac_scale", sac_scale);
        }
        const char* surface = way.get_value_by_key("surface");
        if (surface) {
            feature.set_field("surface", surface);
        }
        const char* width = way.get_value_by_key("width");
        if (width) {
            feature.set_field("width", width);
        }
        if (extra_field && extra_value) {
            feature.set_field(extra_field, extra_value);
        }
        std::string tags_str = tags_string<4>(way.tags(), '|', {"highway", "sac_scale",
                "surface", "width"});
        if (!tags_str.empty()) {
            feature.set_field("tags", tags_str.c_str());
        }
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_options.verbose_output << err.what() << "\n";
    }
}

void SacScaleViewHandler::way(const osmium::Way& way) {
	if (way.tags().has_key("sac_scale")) {
		process_sac_scale(way);
		return;
	};
	const char* highway = way.get_value_by_key("highway");
	if (highway && (!strcmp(highway, "footway") || !strcmp(highway, "path"))) {
	    process_missing_sac_scale(way, highway);
	}
}

void SacScaleViewHandler::node(const osmium::Node&) {}

void SacScaleViewHandler::area(const osmium::Area&) {}
