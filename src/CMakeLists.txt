#-----------------------------------------------------------------------------
#
#  CMake Config
#
#-----------------------------------------------------------------------------

add_executable(osmi_simple_views osmi_simple_views.cpp places_handler.cpp geometry_view_handler.cpp abstract_view_handler.cpp highway_view_handler.cpp tagging_view_handler.cpp ogr_output_base.cpp any_relation_collector.cpp)
target_link_libraries(osmi_simple_views ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osmi_simple_views DESTINATION bin)

add_executable(osmi_simple_views_merc osmi_simple_views.cpp places_handler.cpp geometry_view_handler.cpp abstract_view_handler.cpp highway_view_handler.cpp tagging_view_handler.cpp ogr_output_base.cpp any_relation_collector.cpp)
target_compile_options(osmi_simple_views_merc PUBLIC "-DONLYMERCATOROUTPUT")
target_link_libraries(osmi_simple_views_merc ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osmi_simple_views_merc DESTINATION bin)
