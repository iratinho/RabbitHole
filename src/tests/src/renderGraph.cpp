#include "gtest/gtest.h"
#include "Renderer/GraphBuilder.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/TextureResource.hpp"

namespace RenderGraphTest {
    class GraphBuilderTester : public GraphBuilder {
    public:
        GraphBuilderTester()   {
            auto context = GraphicsContext::Create(nullptr);
            GraphBuilder(context.get());
        };
        
        std::vector<RenderGraphNode>& GetGraphNodes_Mutable() {
            return _nodes;
        };
    };
    
    class RenderGraphTestNode : public RenderGraphNode {
    public:
        void SetContext(RasterNodeContext context) {
            _ctx = context;
        }
    };
};

TEST(RenderGraph, PassOrdering_1) {
    RenderGraphTest::GraphBuilderTester graphBuilder;

    std::shared_ptr<TextureResource> textureResource = TextureResource::MakeResource(nullptr, nullptr, true);
    
    // Pass A
    {
        PassResources passResourceReads;
        PassResources passResourceWrites;
        passResourceWrites._textureResources.push_back(textureResource);
        
        RasterNodeContext context;
        context._readResources = std::move(passResourceReads);
        context._writeResources = std::move(passResourceWrites);
        context._passName = "Pass A";
        
        RenderGraphTest::RenderGraphTestNode node;
        node.SetContext(context);
        
        graphBuilder.GetGraphNodes_Mutable().push_back(node);
    }
    
    // Pass B
    {
        PassResources passResourceReads;
        PassResources passResourceWrites;
        passResourceWrites._textureResources.push_back(textureResource);
        
        RasterNodeContext context;
        context._readResources = std::move(passResourceReads);
        context._writeResources = std::move(passResourceWrites);
        context._passName = "Pass B";

        RenderGraphTest::RenderGraphTestNode node;
        node.SetContext(context);

        graphBuilder.GetGraphNodes_Mutable().push_back(node);
    }
    
    std::vector<std::string> wantedRes = {"Pass A", "Pass B"};
    std::vector<std::string> res;
    
    graphBuilder.Exectue([&res](RenderGraphNode node) {
        auto context = node.GetContext<RasterNodeContext>();
        res.push_back(context._passName);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(RenderGraph, PassOrdering_2) {
    RenderGraphTest::GraphBuilderTester graphBuilder;

    std::shared_ptr<TextureResource> textureResource1 = TextureResource::MakeResource(nullptr, nullptr, true);
    std::shared_ptr<TextureResource> textureResource2 = TextureResource::MakeResource(nullptr, nullptr, true);

    // Pass A
    {
        PassResources passResourceReads;
        passResourceReads._textureResources.push_back(textureResource1);
        
        PassResources passResourceWrites;
        passResourceWrites._textureResources.push_back(textureResource2);
        
        RasterNodeContext context;
        context._readResources = std::move(passResourceReads);
        context._writeResources = std::move(passResourceWrites);
        context._passName = "Pass A";
        
        RenderGraphTest::RenderGraphTestNode node;
        node.SetContext(context);
        
        graphBuilder.GetGraphNodes_Mutable().push_back(node);
    }
    
    // Pass B
    {
        PassResources passResourceReads;
        PassResources passResourceWrites;
        passResourceWrites._textureResources.push_back(textureResource1);
        
        RasterNodeContext context;
        context._readResources = std::move(passResourceReads);
        context._writeResources = std::move(passResourceWrites);
        context._passName = "Pass B";

        RenderGraphTest::RenderGraphTestNode node;
        node.SetContext(context);

        graphBuilder.GetGraphNodes_Mutable().push_back(node);
    }
    
    std::vector<std::string> wantedRes = {"Pass B", "Pass A"};
    std::vector<std::string> res;
    
    graphBuilder.Exectue([&res](RenderGraphNode node) {
        auto context = node.GetContext<RasterNodeContext>();
        res.push_back(context._passName);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

