#include <iostream>
#include "gtest/gtest.h"
#include "Core/Utils.hpp"

TEST(DAG, NoEdges) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);

    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1, 2, 3 };
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, SingleVertex) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);

    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1 };
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, LinearGraph) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);
    dagGraph.MakeEdge({1, 2});
    dagGraph.MakeEdge({2, 3});

    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1, 2, 3 };
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, ComplexGrap_0) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);
    dagGraph.MakeVertex(4);
    dagGraph.MakeVertex(5);
    dagGraph.MakeEdge({1, 3});
    dagGraph.MakeEdge({1, 4});
    dagGraph.MakeEdge({2, 4});
    dagGraph.MakeEdge({2, 5});
    dagGraph.MakeEdge({3, 5});

    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1, 2, 3, 4, 5 };
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, ComplexGrap_1) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);
    dagGraph.MakeEdge({1, 3});
    dagGraph.MakeEdge({2, 3});
    
    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1, 2, 3};
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, ComplexGrap_3) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);
    dagGraph.MakeVertex(4);
    
    dagGraph.MakeEdge({1, 3});
    dagGraph.MakeEdge({1, 4});
    dagGraph.MakeEdge({2, 4});
    
    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1, 2, 3, 4 };
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, ComplexGrap_4) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);
    dagGraph.MakeVertex(4);
    dagGraph.MakeVertex(5);
    
    dagGraph.MakeEdge({1, 3});
    dagGraph.MakeEdge({1, 4});
    dagGraph.MakeEdge({2, 4});
    dagGraph.MakeEdge({2, 5});

    dagGraph.Sort();
    
    std::vector<DAG::Vertex> wantedRes { 1, 2, 3, 4, 5 };
    std::vector<DAG::Vertex> res;
    dagGraph.ForEachSorted([&res](DAG::Vertex vertex) {
        res.push_back(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}

TEST(DAG, VertexDegrees) {
    DAG dagGraph;
    
    dagGraph.MakeVertex(1);
    dagGraph.MakeVertex(2);
    dagGraph.MakeVertex(3);
    dagGraph.MakeVertex(4);
    
    dagGraph.MakeEdge({1, 3});
    dagGraph.MakeEdge({1, 4});
    dagGraph.MakeEdge({2, 4});

    dagGraph.Sort();
    
    std::unordered_map<DAG::Vertex, int> wantedRes { {4, 2}, {3 ,1}, {2, 0}, {1, 0} };
    std::unordered_map<DAG::Vertex, int> res;
    dagGraph.ForEachSorted([&res, &dagGraph](DAG::Vertex vertex) {
        res[vertex] = dagGraph.GetVertexDegree(vertex);
    });
    
    EXPECT_TRUE(wantedRes == res);
}
