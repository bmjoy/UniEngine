#pragma once
#include "Core.h"
#include "Transforms.h"
#include "UniEngineAPI.h"
namespace UniEngine
{
	struct UNIENGINE_API Ray : ComponentBase {
		glm::vec3 Start;
		glm::vec3 Direction;
		float Length;
		Ray() = default;
		Ray(glm::vec3 start, glm::vec3 end);
		Ray(glm::vec3 start, glm::vec3 direction, float length);
		bool Intersect(const glm::vec3& position, float radius) const;
		bool Intersect(const glm::mat4& transform, const Bound& bound) const;
		glm::vec3 GetEnd() const;
	};

	inline Ray::Ray(glm::vec3 start, glm::vec3 end)
	{
		Start = start;
		Direction = glm::normalize(end - start);
		Length = glm::distance(start, end);
	}

	inline Ray::Ray(glm::vec3 start, glm::vec3 direction, float length)
	{
		Start = start;
		Direction = direction;
		Length = length;
	}

	inline bool Ray::Intersect(const glm::vec3& position, float radius) const
	{
		const glm::vec3 rayEnd = Start + Direction * Length;
		auto cp = glm::closestPointOnLine(position, Start, rayEnd);
		if (cp == Start || cp == rayEnd) return false;
		return glm::distance(cp, position) <= radius;
	}

	inline bool Ray::Intersect(const glm::mat4& model, const Bound& bound) const
	{
		float tMin = 0.0f;
		float tMax = 100000.0f;
		GlobalTransform t;
		t.Value = model;
		glm::vec3 scale = t.GetScale();
		t.SetScale(glm::vec3(1.0f));
		glm::mat4 transform = t.Value;
		
		glm::vec3 OBBWorldSpace(transform[3].x, transform[3].y, transform[3].z);
		
		
		glm::vec3 delta = OBBWorldSpace - Start;
		glm::vec3 AABBMin = scale * (bound.Min);
		glm::vec3 AABBMax = scale * (bound.Max);
		// Test intersection with the 2 planes perpendicular to the OBB's X axis
		{
			glm::vec3 xAxis(transform[0].x, transform[0].y, transform[0].z);
			
			float e = glm::dot(xAxis, delta);
			float f = glm::dot(Direction, xAxis);

			if (fabs(f) > 0.001f) { // Standard case

				float t1 = (e + AABBMin.x) / f; // Intersection with the "left" plane
				float t2 = (e + AABBMax.x) / f; // Intersection with the "right" plane
				// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

				// We want t1 to represent the nearest intersection, 
				// so if it's not the case, invert t1 and t2
				if (t1 > t2) {
					float w = t1; t1 = t2; t2 = w; // swap t1 and t2
				}

				// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
				if (t2 < tMax)
					tMax = t2;
				// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
				if (t1 > tMin)
					tMin = t1;

				// And here's the trick :
				// If "far" is closer than "near", then there is NO intersection.
				// See the images in the tutorials for the visual explanation.
				if (tMax < tMin)
					return false;

			}
			else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
				if (-e + AABBMin.x > 0.0f || -e + AABBMax.x < 0.0f)
					return false;
			}
		}


		// Test intersection with the 2 planes perpendicular to the OBB's Y axis
		// Exactly the same thing than above.
		{
			glm::vec3 yAxis(transform[1].x, transform[1].y, transform[1].z);
			float e = glm::dot(yAxis, delta);
			float f = glm::dot(Direction, yAxis);

			if (fabs(f) > 0.001f) {

				float t1 = (e + AABBMin.y) / f;
				float t2 = (e + AABBMax.y) / f;

				if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

				if (t2 < tMax)
					tMax = t2;
				if (t1 > tMin)
					tMin = t1;
				if (tMin > tMax)
					return false;

			}
			else {
				if (-e + AABBMin.y > 0.0f || -e + AABBMax.y < 0.0f)
					return false;
			}
		}


		// Test intersection with the 2 planes perpendicular to the OBB's Z axis
		// Exactly the same thing than above.
		{
			glm::vec3 zAxis(transform[2].x, transform[2].y, transform[2].z);
			float e = glm::dot(zAxis, delta);
			float f = glm::dot(Direction, zAxis);

			if (fabs(f) > 0.001f) {

				float t1 = (e + AABBMin.z) / f;
				float t2 = (e + AABBMax.z) / f;

				if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

				if (t2 < tMax)
					tMax = t2;
				if (t1 > tMin)
					tMin = t1;
				if (tMin > tMax)
					return false;

			}
			else {
				if (-e + AABBMin.z > 0.0f || -e + AABBMax.z < 0.0f)
					return false;
			}
		}
		return true;
	}

	inline glm::vec3 Ray::GetEnd() const
	{
		return Start + Direction * Length;
	}
}
