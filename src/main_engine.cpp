/// MIT License
/// 
/// Copyright (c) 2020 Konstantin Rolf
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
/// 
/// Written by Konstantin Rolf (konstantin.rolf@gmail.com)
/// July 2020

#include "main.hpp"
#include <spdlog/spdlog.h>

#ifdef LT_GLFW_BACKEND

#include <memory>
#include <vector>

#include "mapcanvas.h"

#include "traffic/osm.h"
#include "traffic/osm_graph.h"
#include "traffic/osm_mesh.h"
#include "traffic/parser.hpp"
#include "traffic/render.hpp"
#include "traffic/agent.h"

#include "engine/window.hpp"

using namespace lt;
using namespace traffic;

int main_engine(int argc, char** argv)
{
    spdlog::info("Starting Engine Backend");
    Engine eng;
    eng.init("Window", 800, 600);

    auto manager = std::make_shared<ConcurrencyManager>();
	auto world = std::make_shared<World>(manager.get());

    SizedObject size;
	size.setWidth(800);
	size.setHeight(600);

	auto m_canvas = std::make_shared<MapCanvas>(world->getMap(), &size);
	m_canvas->setActive(true);

	// Loads the default map
	bool loadDefault = true;
	if (loadDefault) {
		world->loadMap("maps/warendorf.xmlmap");
		m_canvas->loadMap(world->getMap());
		m_canvas->loadHighwayMap(world->getHighwayMap());
		m_canvas->setActive(true);
	}

    eng.setPipeline(m_canvas.get());
    eng.mainloop();
    eng.exit();

    return 0;
}
#else
int main_engine(int argc, char** argv)
{
    spdlog::error("Could not find Engine implementation!");
    return 0;
}
#endif
