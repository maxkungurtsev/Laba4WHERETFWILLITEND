#include "OctreeNode.h"


OctreeNode::OctreeNode(int current_depth, BoundingBox& box) {
	bounding_box_ = box;
	if (current_depth > 0) {
		for (int i = 0; i < 8; i++) {
			XMFLOAT3 center = bounding_box_.Center;
			XMFLOAT3 ext = bounding_box_.Extents;
			ext = XMFLOAT3(ext.x * 0.5, ext.y * 0.5, ext.z * 0.5);
			int x_offset = (i & 1) ? 1 : -1;
			int y_offset = (i & 2) ? 1 : -1;
			int z_offset = (i & 4) ? 1 : -1;
			center = XMFLOAT3(center.x + x_offset * ext.x, center.y + y_offset * ext.y, center.z + z_offset * ext.z);
			BoundingBox box(center, ext);
			children_[i] = std::make_unique<OctreeNode>(current_depth - 1, box);
		}
	}
	else {
		leaf_ = true;
	}
};
void OctreeNode::AddSubmeshToTree(SubMesh& sub_mesh_, int index) {
	bool in_child = false;
	if (leaf_) {
		submesh_indices.push_back(index);
		return;
	}
	for (int i = 0; i < 8; i++) {
		if (children_[i]->bounding_box_.Contains(sub_mesh_.bounding_sphere_)) {
			children_[i]->AddSubmeshToTree(sub_mesh_, index);
			in_child = true;
			break;
		};
	}
	if (!in_child) {
		submesh_indices.push_back(index);
	}
	return;
};


void OctreeNode::GetIndeciesToDraw(std::vector<int>& indecies, BoundingFrustum& frustum) {
	if (frustum.Intersects(bounding_box_)) {
		indecies.insert(indecies.end(), submesh_indices.begin(), submesh_indices.end());
		if (!leaf_){
			for (int i = 0; i < 8; i++) {
				children_[i]->GetIndeciesToDraw(indecies, frustum);
			}
		}
	}
}
