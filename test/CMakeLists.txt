enable_testing()

message(STATUS "Configuring unit tests")

include(CMakeParseArguments)
include_directories(include)
include_directories(../src)

add_library(testlib STATIC test_main.cpp)

set(ALL_TESTS "")

# Otherwise GCC throws a lot of warnings for REQUIRE(...) from Catch v.1.2.1
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wno-parentheses)
endif()


add_executable(test_tagging_view t/test_tagging_view.cpp ../src/tagging_view_handler.cpp ../src/abstract_view_handler.cpp ../src/ogr_output_base.cpp ../src/any_relation_collector.cpp)
target_link_libraries(test_tagging_view testlib ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES})
add_test(NAME test_tagging_view
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND test_tagging_view)
