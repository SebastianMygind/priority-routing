#include <gtest/gtest.h>
#include "../src/osm/graph.h"

class GraphTest : public ::testing::Test
{
protected:
    OSMGraph graph;

    void SetUp() override
    {
        bool loaded_graph = graph.load("../data/TestingData.osm");
        ASSERT_TRUE(loaded_graph);

        ASSERT_TRUE(graph.BuildAdjList());
        ASSERT_TRUE(graph.BuildRoadNodes());
    }
};

TEST_F(GraphTest, StringToNode_FindAddress_Frøhaven17)
{
    OSMNodeID result = graph.StringToNode("Frøhaven 17");

    EXPECT_NE(result, 0xFFFFFFFF);

    EXPECT_TRUE(result == 13770599694);

}

TEST_F(GraphTest, StringToNode_FindAddress_Frødalen1)
{
    OSMNodeID result = graph.StringToNode("Frødalen 1");

    EXPECT_NE(result, 0xFFFFFFFF);

    EXPECT_TRUE(result == 6323092520);
}

TEST_F(GraphTest, StringToNode_GraceFullyFail)
{
    OSMNodeID result = graph.StringToNode("Gracefully Fail");

    EXPECT_EQ(result, 0xFFFFFFFF);
}

TEST_F(GraphTest, StringToNode_DirectNodeID)
{
    OSMNodeID result = graph.StringToNode("1612703763");

    EXPECT_NE(result, 0xFFFFFFFF);

    EXPECT_TRUE(result == 1612703763);
}