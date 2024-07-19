#pragma once

#include "Hittable.h"
#include "Material.h"
#include "BVHNode.h"
#include "Sphere.h"
#include <vector>
#include <memory>

struct Scene
{
	// list of hittable primitives -> thus far ONLY spheres 
	std::vector<Hittable*> HittableObjs;
	// indices for hittableObjects
	std::vector<int> HittableObjsIndxs;
	// list of materials referenced by their index by the hittables
	std::vector<Material> Materials;

	// list of BVHNodes -> size max [N * 2 - 1] -> maybe list of pointers of bvhs better?!
	std::vector<BVHNode> BVHNodes;
	// used for creation -> maybe later change and delete
	uint32_t NodesUsed = 1; 
	uint32_t RootNodeIndex = 0;


	void BuildBVH() {

		// intialize triangle index array
		for (int i = 0; i < HittableObjs.size(); i++) {
			HittableObjsIndxs.emplace_back(i);
		}

		std::cout << HittableObjsIndxs[0] << std::endl;
		std::cout << HittableObjsIndxs[1] << std::endl;
		std::cout << HittableObjsIndxs.at(2) << std::endl;
		std::cout << HittableObjsIndxs.at(3) << std::endl;


		// TODO: later calculate ceneters for each primitive -> triangles etc. (now not needed, only spheres

		BVHNode& RootNode = BVHNodes.emplace_back();
		RootNode.m_LeftChildIndx = RootNode.m_RightChildIndx = 0;
		RootNode.m_FirstPrimIndx = 0;
		RootNode.m_PrimCount = HittableObjs.size();

		
		UpdateBVHNodeBounds(RootNodeIndex);

		// BVHNodes.emplace_back(RootNode);

		SubdivideBVHNode(RootNodeIndex);
	}

	void UpdateBVHNodeBounds(uint32_t NodeIndx) {

		BVHNode& node = BVHNodes.at(NodeIndx);

		// just for testing atm but is needed
		node.m_BoundingBox.m_X = Interval::m_Empty;
		node.m_BoundingBox.m_Y = Interval::m_Empty;
		node.m_BoundingBox.m_Z = Interval::m_Empty;

		
		for (uint32_t i = node.m_FirstPrimIndx; i < node.m_FirstPrimIndx + node.m_PrimCount; i++) {

			Sphere* sphere = dynamic_cast<Sphere*>(HittableObjs[i]);
			if (sphere) {
				// NOTE: node intervals already initialized to min = max and max = min -> otherwise not working
				node.m_BoundingBox.m_X.m_Min = fminf(node.m_BoundingBox.m_X.m_Min, sphere->Position.x - sphere->Radius);
				node.m_BoundingBox.m_X.m_Max = fmaxf(node.m_BoundingBox.m_X.m_Max, sphere->Position.x + sphere->Radius);

				node.m_BoundingBox.m_Y.m_Min = fminf(node.m_BoundingBox.m_Y.m_Min, sphere->Position.y - sphere->Radius);
				node.m_BoundingBox.m_Y.m_Max = fmaxf(node.m_BoundingBox.m_Y.m_Max, sphere->Position.y + sphere->Radius);

				node.m_BoundingBox.m_Z.m_Min = fminf(node.m_BoundingBox.m_Z.m_Min, sphere->Position.z - sphere->Radius);
				node.m_BoundingBox.m_Z.m_Max = fmaxf(node.m_BoundingBox.m_Z.m_Max, sphere->Position.z + sphere->Radius);
			} // NOTE: add here when adding new Hittable objects
		}
	}

	void SubdivideBVHNode(uint32_t NodeIndx) {

		// TODO: maybe add a max depth or min size of hittables in the list -> interesting later when adding triangles

		BVHNode& node = BVHNodes.at(NodeIndx);
		// find longest axis 
		int axis = node.m_BoundingBox.BiggestIntervalAxis();
		float splitPos;
		if (axis == 0) {
			splitPos = node.m_BoundingBox.m_X.m_Min + node.m_BoundingBox.m_X.Size() * 0.5f;
		}
		else if (axis == 1) {
			std::cout << "splitAxis y" << std::endl;
			splitPos = node.m_BoundingBox.m_Y.m_Min + node.m_BoundingBox.m_Y.Size() * 0.5f;
		}
		else {
			splitPos = node.m_BoundingBox.m_Z.m_Min + node.m_BoundingBox.m_Z.Size() * 0.5f;
		}

		// split groups in two halfes
		int i = node.m_FirstPrimIndx;
		int j = node.m_FirstPrimIndx + node.m_PrimCount - 1;
		while (i <= j) {
			if (HittableObjs[i]->Position[axis] < splitPos) {
				i++;
			}
			else {
				// TODO: fix -> no seems to be right
				std::cout << "test" << std::endl;
				std::cout << HittableObjsIndxs.at(i);
				std::cout << HittableObjsIndxs.at(j);
				
				std::swap(HittableObjsIndxs[i], HittableObjsIndxs[j--]);
				


				/*
				uint32_t one = HittableObjsIndxs.at(i);
				uint32_t two = HittableObjsIndxs.at(j);
				HittableObjsIndxs.at(j) = one;
				HittableObjsIndxs.at(i) = two;
				j = j - 1;
				*/
				

				std::cout << HittableObjsIndxs.at(i);
				std::cout << HittableObjsIndxs.at(j+1);

				std::cout << "test" << std::endl;
				
			}
		}

		std::cout << HittableObjsIndxs[0] << std::endl;
		std::cout << HittableObjsIndxs[1] << std::endl;
		std::cout << HittableObjsIndxs[2] << std::endl;
		std::cout << HittableObjsIndxs[3] << std::endl;
		std::cout << HittableObjsIndxs[4] << std::endl;
		std::cout << "----" << std::endl;


		int leftCount = i - node.m_FirstPrimIndx;

		// std::cout << j;

		if (leftCount == 0 || leftCount == node.m_PrimCount) {
			return;
		}

		std::cout << "will split";
		
		int leftChildIndx = NodesUsed++;
		int rightChildIndx = NodesUsed++;

		BVHNode& LeftNode = BVHNodes.emplace_back();
		LeftNode.m_FirstPrimIndx = node.m_FirstPrimIndx;
		LeftNode.m_PrimCount = leftCount;

		BVHNode& RightNode = BVHNodes.emplace_back();
		RightNode.m_FirstPrimIndx = i;
		RightNode.m_PrimCount = node.m_PrimCount - leftCount;

		node.m_LeftChildIndx = leftChildIndx;
		node.m_RightChildIndx = rightChildIndx;
		node.m_PrimCount = 0;

		//BVHNodes.emplace_back(LeftNode);
		//BVHNodes.emplace_back(RightNode);

		UpdateBVHNodeBounds(leftChildIndx);
		UpdateBVHNodeBounds(rightChildIndx);

		SubdivideBVHNode(leftChildIndx);
		SubdivideBVHNode(rightChildIndx);

	}
};
