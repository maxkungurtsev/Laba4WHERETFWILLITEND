#include "OctreeNode.h"

OctreeNode::OctreeNode(int current_depth, BoundingBox& box, bool is_root, std::vector<int>& parent_indices, Model &mesh) {
	bounding_box_ = box;

	for (int i = 0; i < parent_indices.size(); i++) {
		if (box.Contains(mesh.GetSubMeshes()[parent_indices[i]].bounding_sphere_)) {
			submesh_indices_.push_back(parent_indices[i]);
		}
	}
	for (int i = 0; i < submesh_indices_.size(); i++) {
		parent_indices.erase(std::remove(parent_indices.begin(), parent_indices.end(), 2), parent_indices.end());
	}
	if (current_depth < 4 and submesh_indices_.size()>20){
		for (int i = 0; i < 8; i++) {
			XMFLOAT3 center = bounding_box_.Center;
			XMFLOAT3 ext = bounding_box_.Extents;
			ext = XMFLOAT3(ext.x * 0.5, ext.y * 0.5, ext.z * 0.5);
			int x_offset = (i & 1) ? 1 : -1;
			int y_offset = (i & 2) ? 1 : -1;
			int z_offset = (i & 4) ? 1 : -1;
			center = XMFLOAT3(center.x + x_offset * ext.x, center.y + y_offset * ext.y, center.z + z_offset * ext.z);
			BoundingBox box(center, ext);
			children_[i] = std::make_unique<OctreeNode>(current_depth + 1, box, false, submesh_indices_, mesh);
		}
	}
	else {
		leaf_ = true;
	}
};

void OctreeNode::GetIndeciesToDraw(std::vector<int>& indicies, BoundingFrustum& frustum) {
	if (frustum.Intersects(bounding_box_)) {
		indicies.insert(indicies.end(), submesh_indices_.begin(), submesh_indices_.end());
		if (!leaf_){
			for (int i = 0; i < 8; i++) {
				children_[i]->GetIndeciesToDraw(indicies, frustum);
			}
		}
	}
}
