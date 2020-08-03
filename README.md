UNIVERSITY OF GRONINGEN - Honours College - Deepening Project

# Traffic Controller
Optimizing traffic flow is a key component of modern city planning. Commuters in the US alone spend around 54 hours yearly being stuck in traffic ([Urban Mobility Report 2019](https://static.tti.tamu.edu/tti.tamu.edu/documents/mobility-report-2019.pdf)). Modern approaches use a multi agent-based modelling approach to predict and optimize the traffic flow in cities. The research ranges from micro-simulations modelling only traffic junctions up to urban traffic research encapsulating a whole urban area. This project implements a large scale agent-based simulation model that operates on [OpenStreetMap](https://www.openstreetmap.org) data.

## Project Outline
The project is currently in active development which means that most features are not working at the moment. The general structure of the main components is listed below.

1. Simulation Software
   1. Parsing OSM data => Rendering a customized map version
   3. Modelling drivers and vehicles
   4. Route Planning
   5. Simulation physics
   6. Measuring the Traffic Flow

## OpenStreetMap data
The simulation is based on the [OpenStreetMap](https://www.openstreetmap.org) [OSM](https://wiki.openstreetmap.org/wiki/OSM_XML) format that is already used in many projects. The map content is represented in three different categories: **Nodes** represent single points on the map ranging from street corners to abstract concepts like bus stops. **Ways** are a collection of points that can form streets or other structures like routes. **Relations** are collection of ways forming even larger structures and networks. The application makes use of this data to form a graph network that is used to generate routes.

<p align="center">
  <img src="example/data.png" width="400">
</p>

## Optimizations
The project tries to implement a list of optimizations to achieve a better traffic flow:

***ENVIRONMENT-BASED OPTIMIZATION***: Includes a set of configurations to improve the traffic flow independently from the agents. This includes static optimizations like traffic lights, speed limits or street blocks

***AGENT-BASED OPTIMIZATION***: Includes all optimizations that can be performed by the agents themselves. This requires the use of informed agents that know their environment very well. It is mostly about adaptive route planning in different contexts (e.g. choose a different route if this one is already full).

## Building from Source
The project uses the cross platform compiler [CMake](https://cmake.org/) in combination with the [Cinder](https://libcinder.org/docs/index.html) framework that allows building on all major platforms. The currently supported native ports are Windows, OS X and Linux. However, there is a good probability that it might work without problems on other plattforms as well. See the example below on how to build the project in combination with Cinder with CMake. You can find more information about [building Cinder from Source here](https://www.libcinder.org/docs/guides/cmake/cmake.html). Cinder offers also pre-packaged versions for Windows and OS X available. See this guide on how to setup [Cinder on Windows](https://www.libcinder.org/docs/guides/windows-setup/index.html) or setup [Cinder on OS X](https://www.libcinder.org/docs/guides/mac-setup/index.html). Take a look at the platform notes to see the requirements for your specific platform: [Windows](https://www.libcinder.org/docs/guides/windows-notes/index.html), [Linux](https://www.libcinder.org/docs/guides/linux-notes/index.html), [OS X](https://www.libcinder.org/docs/guides/osx-notes/index.html).

It is recommended to build the debug and release version of Cinder because the app might not be able to find the cinderConfig.cmake otherwise. 

Building on Linux
```
git clone --recursive https://github.com/KonstantinRr/Traffic-Controller
cd Traffic-Controller

# Build Cinder from Source (Debug)
mkdir -p Cinder/build_debug/
cmake -B Cinder/build_debug/ -S Cinder/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17
make -C Cinder/build_debug/ -j 8 # Amount of Threads

# Build Cinder from Source (Release)
mkdir -p Cinder/build_release/
cmake -B Cinder/build_release/ -S Cinder/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
make -C Cinder/build_release/ -j 8 # Amount of Threads

# Building the Project
mkdir -p build/
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
make -C build/ -j 8 # Amount of Threads
```

Building from source on Windows
```
git clone --recursive https://github.com/KonstantinRr/Traffic-Controller
cd Traffic-Controller

# Build Cinder from Source (Debug)
mkdir build_debug\
cmake -B build_debug\ -S Cinder\ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17
msbuild build_debug\cinder.sln

# Build Cinder from Source (Release)
mkdir build_release\
cmake -B build_release\ -S Cinder\ -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
msbuild build_release\cinder.sln

# Building the Project
mkdir build\
cmake -S . -B build\ -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
msbuild build\TrafficController.sln
```

## Dependencies
The project uses some open source dependencies which can be found below. It relies on Cinder as the main OpenGL and Window framework.

- [Cinder](https://libcinder.org/docs/index.html)
- [RapidXML](http://rapidxml.sourceforge.net/)
