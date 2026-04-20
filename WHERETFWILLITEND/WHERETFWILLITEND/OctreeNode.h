#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <DirectXCollision.h>
using namespace DirectX;
class OctreeNode {
	std::unique_ptr<OctreeNode>children_[8];
	std::vector<int> submesh_indices_;
	BoundingBox bounding_box_;
	bool leaf_ = false;
public:
	OctreeNode(int current_depth, BoundingBox& box, bool is_root, std::vector<int>& parent_indices, std::vector<BoundingSphere>& all_spheres);
	void GetIndeciesToDraw(std::vector<int>& indecies, BoundingFrustum& frustum);
};