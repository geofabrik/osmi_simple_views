# OSMI Simple Views

This repository contains the tools to generated the data which powers the view
[Tagging](http://tools.geofabrik.de/osmi/?view=tagging&lon=-70.32779&lat=41.68389&zoom=11),
[Highways](http://tools.geofabrik.de/osmi/?view=highways&lon=-4.20863&lat=40.85298&zoom=8),
[Places](http://tools.geofabrik.de/osmi/?view=places&lon=-64.55104&lat=9.07186&zoom=8&overlays=megacities,largecities,cities,towns,villages,hamlets,islands,suburbs,farms,localities,municipalities,errors_unknown_place_type,errors_population_format,errors_place_without_name,errors_population_number_format,errors_pop_type_mismatch,population)
and
[Geometry](http://tools.geofabrik.de/osmi/?view=geometry&lon=9.77490&lat=45.91982&zoom=8)
of the OpenStreetMap Inspector (OSMI) by [Geofabrik](http://www.geofabrik.de/).

This software uses the [Osmium library](https://github.com/osmcode/libosmium) by Jochen Topf for everything related with
reading OSM data and the [GDAL](http://gdal.org/) library (via Jochen Topf's C++ wrapper
[gdalcpp](https://github.com/joto/gdalcpp)) to write the output data.


## License and Authors

This software was developed by Geofabrik GmbH. See the Git history for a full
list of contributors.

This software is licensed under the terms of GNU General Public License version
3 or newer. See [LICENSE.md](LICENSE.md) for the full legal text of the license.

The Catch unit test framework is available under the terms of [Boost Software
License](test/include/LICENSE_1_0.txt).

The geometry view contains code to detect self intersections of ways which is a
modified copy of a part of [OSMCoastline](osmcode.org/osmcoastline/) by Jochen
Topf, available GNU General Public License 3 or newer.


## How it works

This software reads an OpenStreetMap planet dump (or one of its smaller extracts) and
produces an Spatialite database which contains all errorenous objects which were
found in the OpenStreetMap data. Other output formats than Spatialite are
possible but not as well tested. You can open the output files using QGIS.

The Spatialite database is used as the data source of the [WMS
service](https://wiki.openstreetmap.org/wiki/OSM_Inspector/WxS) by the OSMI
backend. This service provides the map and a GetFeatureInfo API call used by
the frontend.


## Dependencies

* C++11 compiler
* libosmium (`libosmium-dev`) and all its [important dependencies](http://osmcode.org/libosmium/manual.html#dependencies)
* GDAL library (`libgdal-dev`)
* proj.4 (`libproj4-dev`)
* CMake (`cmake`)

You can install libosmium either using your package manager or just cloned from
its Github repository to any location on your disk. Add a symlink libosmium in
the top level directory of this repository to the location of
`libosmium/include` if you use the latter variant. Take care to use libosmium
v2.x not the old Osmium v1.x!


## Building

```sh
mkdir build
cd build
cmake ..
make
```

## Usage

Run `./osmi_simple_views -h` to see the available options.

There are two binaries. `osmi_simple_views_merc` can only produce output files
in Web Mercator projection (EPSG:3857) but is faster than `osmi_simple_views`
because it uses a faster coordinate transformation engine provided by libosmium
while `osmi_simple_views` calls Proj4.

