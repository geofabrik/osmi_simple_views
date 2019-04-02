/*
 *  © 2017 Geofabrik GmbH
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

#include "any_relation_collector.hpp"

#include "handler_collection.hpp"

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

void print_help(char* arg0) {
    std::cerr << "Usage: " << arg0 << " [OPTIONS] INPUT_FILE OUTPUT_DIRECTORY\n" \
              << "Options:\n" \
              << "  -h, --help           This help message.\n" \
              << "  -f, --format         Output format (default: SQlite)\n" \
              << "  -i, --index          Set index type for location index (default: sparse_mem_array)\n";
#ifndef ONLYMERCATOROUTPUT
    std::cerr << "  -s EPSG, --srs=ESPG  Output projection (EPSG code) (default: 3857)\n";
#endif
    std::cerr << "  -t TYPE, --type=TYPE View to be produced (tagging, highways, places, geometry).\n" \
              << "                       Use `-t view1 -t view2` if you want to produce files of\n" \
              << "                       multiple views.\n" \
              << "  -v, --verbose        Verbose output\n";
}

int main(int argc, char* argv[]) {

    static struct option long_options[] = {
        {"help",   no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"index", required_argument, 0, 'i'},
        {"srs", required_argument, 0, 's'},
        {"type",   required_argument, 0, 't'},
        {"verbose",   no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    std::vector<ViewType> views;
    std::string location_index_type = "sparse_mem_array";
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
                if (!strcmp(optarg, "tagging")) {
                    views.push_back(ViewType::tagging);
                } else if (!strcmp(optarg, "geometry")) {
                    views.push_back(ViewType::geometry);
                } else if (!strcmp(optarg, "highways")) {
                    views.push_back(ViewType::highways);
                } else if (!strcmp(optarg, "places")) {
                    views.push_back(ViewType::places);
                } else {
                    std::cerr << "ERROR: -t must be one of tagging, geometry, highways, places\n";
                    print_help(argv[0]);
                    exit(1);
                }
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

    if (views.size() == 0) {
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
    AnyRelationCollector any_collector(verbose_output, output_format, srs);

    HandlerCollection handlers;
    int pass_count = 1;

    // additional passes for views which use relations
    for (auto vt : views) {
        if (vt == ViewType::places) {
            verbose_output << "Pass " << pass_count << " (Multipolygons) ...\n";
            osmium::io::Reader reader1(input_filename, osmium::osm_entity_bits::relation);
            reader1.close();
            verbose_output << "Pass " << pass_count << " done\n";
            ++pass_count;
        } else if (vt == ViewType::tagging) {
            verbose_output << "Pass " << pass_count << " (Relations) ...\n";
            osmium::io::Reader reader1(input_filename, osmium::osm_entity_bits::relation);
            any_collector.read_relations(reader1);
            reader1.close();
            verbose_output << "Pass " << pass_count << " done\n";
            ++pass_count;
        }
    }
    verbose_output << "Pass " << pass_count << " ...\n";

    osmium::io::Reader reader2(input_filename, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);
    for (auto vt : views) {
        if (vt == ViewType::tagging) {
            any_collector.create_layer(handlers.add_handler(vt, output_filename, output_format, verbose_output,
                    "tagging_ways_without_tags", srs));
        } else {
            handlers.add_handler(vt, output_filename, output_format, verbose_output, nullptr, srs);
        }
        if (vt == ViewType::places) {
            handlers.add_multipolygon_collector(collector);
        }
    }

    osmium::apply(reader2, location_handler, handlers, any_collector.handler());
    reader2.close();
    handlers.give_correct_name();
    verbose_output << "Pass " << pass_count << " done\n";

}
