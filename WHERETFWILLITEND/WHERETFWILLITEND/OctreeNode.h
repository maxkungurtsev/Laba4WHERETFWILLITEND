#pragma once
#include "Model.h"
#include <algorithm>
class OctreeNode {
	std::unique_ptr<OctreeNode>children_[8];
	std::vector<int> submesh_indices_;
	BoundingBox bounding_box_;
	bool leaf_ = false;
public:
	OctreeNode(int current_depth, BoundingBox& box, bool is_root, std::vector<int>& parent_indices, Model & mesh);
	void GetIndeciesToDraw(std::vector<int>& indecies, BoundingFrustum& frustum);
};