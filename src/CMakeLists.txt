#-----------------------------------------------------------------------------
#
#  CMake Config
#
#-----------------------------------------------------------------------------

set(SOURCES
	osmi_simple_views.cpp
	options.hpp
	places_handler.cpp
	places_handler.hpp
	geometry_view_handler.cpp
	geometry_view_handler.hpp
	abstract_view_handler.cpp
	abstract_view_handler.hpp
	highway_view_handler.cpp
	highway_view_handler.hpp
	highway_relation_manager.cpp
	highway_relation_manager.hpp
	tagging_view_handler.cpp
	tagging_view_handler.hpp
	ogr_output_base.cpp
	ogr_output_base.hpp
	any_relation_collector.cpp
	any_relation_collector.hpp
	handler_collection.cpp
	handler_collection.hpp
)

add_executable(osmi_simple_views ${SOURCES})
target_link_libraries(osmi_simple_views ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osmi_simple_views DESTINATION bin)

add_executable(osmi_simple_views_merc ${SOURCES})
target_compile_options(osmi_simple_views_merc PUBLIC "-DONLYMERCATOROUTPUT")
target_link_libraries(osmi_simple_views_merc ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osmi_simple_views_merc DESTINATION bin)
