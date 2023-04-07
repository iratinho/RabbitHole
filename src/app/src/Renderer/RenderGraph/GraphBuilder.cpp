#include "Renderer/RenderGraph/GraphBuilder.h"

GraphBuilder::GraphBuilder(RenderGraph* render_graph, std::string identifier)
    : render_graph_(render_graph)
    , graph_identifier_(identifier) {
}
