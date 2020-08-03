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
The project uses the cross platform compiler [CMake](https://cmake.org/) in combination with various open source frameworks.

Building on Linux
```
git clone --recursive https://github.com/KonstantinRr/Traffic-Sim
cd Traffic-Sim

# Building the Project
mkdir -p build/
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
make -C build/ -j 8 # Amount of Threads
```

## Dependencies
The project uses some open source dependencies which can be found below. It relies on nanogui as the main OpenGL and Window framework.

- [fmtlib](https://github.com/fmtlib/fmt/)
- [nanogui](https://github.com/mitsuba-renderer/nanogui)
- [GLM](https://github.com/g-truc/glm)
- [RapidXML](http://rapidxml.sourceforge.net/)
