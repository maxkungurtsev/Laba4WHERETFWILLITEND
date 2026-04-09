#pragma once
#include "Model.h"

class OctreeNode {
	std::unique_ptr<OctreeNode>children_[8];
	std::vector<int> submesh_indices;
	BoundingBox bounding_box_;
	bool leaf_ = false;
public:
	OctreeNode(int current_depth, BoundingBox& box);
	void AddSubmeshToTree(SubMesh& sub_mesh_, int index);
	void GetIndeciesToDraw(std::vector<int>& indecies, BoundingFrustum& frustum);
};