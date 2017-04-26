#-----------------------------------------------------------------------------
#
#  CMake Config
#
#-----------------------------------------------------------------------------

add_executable(osmi_places osmi_places.cpp places_handler.cpp geometry_view_handler.cpp)
target_link_libraries(osmi_places ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osmi_places DESTINATION bin)

add_executable(osmi_places_merc osmi_places.cpp places_handler.cpp geometry_view_handler.cpp)
target_compile_options(osmi_places_merc PUBLIC "-DONLYMERCATOROUTPUT")
target_link_libraries(osmi_places_merc ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS osmi_places_merc DESTINATION bin)
