#include "Renderer/RenderGraph/GraphBuilder.h"

GraphBuilder::GraphBuilder(RenderGraph* render_graph, std::string identifier)
    : _renderGraph(render_graph)
    , _graphIdentifier(identifier) {
}
