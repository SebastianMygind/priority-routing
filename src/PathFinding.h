#pragma once
#include "Graph.h"

bool Djikstra(Graph& graph, uint32_t start_node, uint32_t end_node, std::vector<uint32_t>& out_path);