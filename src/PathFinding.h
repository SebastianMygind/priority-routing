#pragma once
#include "Graph.h"

bool Djikstra(Graph& graph, uint64_t start_node, uint64_t end_node, std::vector<uint64_t>& out_path);