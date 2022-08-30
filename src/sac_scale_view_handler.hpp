/*
 * sac_scale_view_handler.hpp
 *
 *  Created on: 2022-08-30
 *      Author: Michael Reichert
 */

#ifndef SRC_HIGHWAY_PROPERTIES_VIEW_HPP_
#define SRC_HIGHWAY_PROPERTIES_VIEW_HPP_

#include "abstract_view_handler.hpp"

class SacScaleViewHandler : public AbstractViewHandler {
    /// layer for ways with sac_scale=* set
    std::unique_ptr<gdalcpp::Layer> m_sac_scale;
    /// layer for ways with sac_scale related warnings
    std::unique_ptr<gdalcpp::Layer> m_sac_scale_warnings;
    /// layer for ways with sac_scale related errors
    std::unique_ptr<gdalcpp::Layer> m_sac_scale_errors;

    bool surface_matches_sac_scale(const osmium::Way& way, const char* sac_scale);

    template <size_t TValueCount>
    bool value_in_array(const char* value, const std::array<const char*, TValueCount>& array) {
        return value && std::any_of(array.begin(), array.end(), [&value](const char* arr_val){return !strcmp(arr_val, value);});
    }

    void process_sac_scale(const osmium::Way& way);

    void process_missing_sac_scale(const osmium::Way& way, const char* highway);

    void add_to_layer(gdalcpp::Layer& layer, const osmium::Way& way, const char* highway,
            const char* sac_scale, const char* extra_field = nullptr, const char* extra_value = nullptr);

public:
    SacScaleViewHandler(Options& options);

    SacScaleViewHandler() = delete;

    void node(const osmium::Node& node);

    void way(const osmium::Way& way);

    void area(const osmium::Area& area);

    void give_correct_name();

    void close();
};



#endif /* SRC_HIGHWAY_PROPERTIES_VIEW_HPP_ */
