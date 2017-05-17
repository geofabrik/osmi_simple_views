/*
 * osmi_places.cpp
 *
 *  Created on:  2017-04-18
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include <string>
#include <iostream>
#include <getopt.h>

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_collector.hpp>
// the indexes themselves have to be included first
#include <osmium/index/map/dense_mmap_array.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/map/dense_mem_array.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>

#include "places_handler.hpp"
#include "geometry_view_handler.hpp"
#include "highway_view_handler.hpp"
#include "tagging_view_handler.hpp"

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

void print_help(char* arg0) {
    std::cerr << "Usage: " << arg0 << " [OPTIONS] INFILE OUTFILE\n" \
              << "Options:\n" \
              << "  -h, --help           This help message.\n" \
              << "  -f, --format         Output format (default: SQlite)\n" \
              << "  -i, --index          Set index type for location index (default: sparse_mem_array)\n" \
              << "  --gdal-lco=\"K=V\"     set GDAL output driver option, this is equivalent to ogr2ogr's,\n" \
              << "                       -lco K=V\n";
#ifndef ONLYMERCATOROUTPUT
    std::cerr << "  -s EPSG, --srs=ESPG  Output projection (EPSG code) (default: 3857)\n";
#endif
    std::cerr << "  -t TYPE, --type=TYPE View to be produced\n" \
              << "  -v, --verbose        Verbose output\n";
}

int main(int argc, char* argv[]) {

    static struct option long_options[] = {
        {"help",   no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"gdal-lco", required_argument, 0, 200},
        {"index", required_argument, 0, 'i'},
        {"srs", required_argument, 0, 's'},
        {"type",   required_argument, 0, 't'},
        {"verbose",   no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    std::string view_type = "";
    std::string location_index_type = "sparse_mmap_array";
    std::vector<std::string> gdal_options;
    std::string output_format = "SQlite";
    int srs = 3857;
    bool verbose = false;

    while (true) {
        int c = getopt_long(argc, argv, "hf:i:s:t:v", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                print_help(argv[0]);
                exit(1);
            case 'f':
                if (optarg) {
                    output_format = optarg;
                } else {
                    print_help(argv[0]);
                    exit(1);
                }
                break;
            case 200:
                if (!optarg) {
                    std::cerr << "ERROR: --gdal-lco requires an argument\n";
                    print_help(argv[0]);
                    exit(1);
                }
                gdal_options.emplace_back(optarg);
                break;
            case 'i':
                if (optarg) {
                    location_index_type = optarg;
                } else {
                    print_help(argv[0]);
                    exit(1);
                }
                break;
            case 's':
#ifdef ONLYMERCATOROUTPUT
                std::cerr << "ERROR: Usage of output projections other than " \
                        "EPSG:3857 is not compiled into this binary.\n";
                print_help(argv[0]);
                exit(1);
#else
                if (optarg) {
                    srs = atoi(optarg);
                } else {
                    print_help(argv[0]);
                    exit(1);
                }
#endif
                break;
            case 't':
                view_type = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            default:
                print_help(argv[0]);
                exit(1);
        }
    }

    std::string input_filename;
    std::string output_filename;
    int remaining_args = argc - optind;
    if (remaining_args == 2) {
        input_filename =  argv[optind];
        output_filename = argv[optind+1];
    } else if (remaining_args == 1) {
        input_filename =  argv[optind];
    } else {
        input_filename = "-";
    }

    if (view_type == "") {
        std::cerr << "ERROR: Argument -t VIEW is mandatory.\n";
        print_help(argv[0]);
        exit(1);
    }


    const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
    auto location_index = map_factory.create_map(location_index_type);
    location_handler_type location_handler(*location_index);

    osmium::util::VerboseOutput verbose_output(verbose);

    osmium::area::Assembler::config_type assembler_config;
    osmium::area::MultipolygonCollector<osmium::area::Assembler> collector(assembler_config);

    if (view_type == "places") {
        verbose_output.print("Pass 1 (Multipolygons) ...\n");
        osmium::io::Reader reader1(input_filename, osmium::osm_entity_bits::relation);
        collector.read_relations(reader1);
        reader1.close();

        verbose_output.print("Pass 1 done\n");
    }
    verbose_output.print("Pass 2 ...\n");

    osmium::io::Reader reader2(input_filename, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
    if (view_type == "places") {
        PlacesHandler places_handler(output_filename, output_format, gdal_options, verbose_output, srs);
        osmium::apply(reader2, location_handler, places_handler,
                collector.handler([&places_handler](const osmium::memory::Buffer& area_buffer) {
                osmium::apply(area_buffer, places_handler);
                }));
    } else if (view_type == "geometry") {
        GeometryViewHandler geom_view_handler(output_filename, output_format, gdal_options, verbose_output, srs);
        osmium::apply(reader2, location_handler, geom_view_handler);
    } else if (view_type == "highways") {
        HighwayViewHandler highway_view_handler(output_filename, output_format, gdal_options, verbose_output, srs);
        osmium::apply(reader2, location_handler, highway_view_handler);
    } else if (view_type == "tagging") {
        TaggingViewHandler tagging_view_handler(output_filename, output_format, gdal_options, verbose_output, srs);
        osmium::apply(reader2, location_handler, tagging_view_handler);
    }
    reader2.close();
    verbose_output.print("Pass 2 done\n");
}
