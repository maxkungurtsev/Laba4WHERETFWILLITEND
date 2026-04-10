#include "OctreeNode.h"

OctreeNode::OctreeNode(int current_depth, BoundingBox& box, bool is_root, std::vector<int>& parent_indices, std::shared_ptr<Model> mesh) {
	bounding_box_ = box;
	std::vector<int> new_parent_indices;
	for (int i = 0; i < parent_indices.size(); i++) {
		if (box.Contains(mesh->GetSubMeshes()[parent_indices[i]].bounding_sphere_)) {
			submesh_indices_.push_back(parent_indices[i]);
			if (!is_root) {
				//OutputDebugStringA(" Putting submesh into child\n");
			}
		}
		else {
			new_parent_indices.push_back(parent_indices[i]);
		}
	}
	
	parent_indices = new_parent_indices;
/*
	OutputDebugStringA(("\nOctree node is being created, depth:" + std::to_string(current_depth)+'\n'+
						"is root:"+ std::to_string(is_root)+
						"submeshes in this node:" + std::to_string(submesh_indices_.size())+
						"submeshes in parent"+ std::to_string(parent_indices.size())).c_str());

*/
	

	if (current_depth < 3 and submesh_indices_.size()>20){
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
