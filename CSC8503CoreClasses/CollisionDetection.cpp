#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"
#include "Debug.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray& r, const Plane& p, RayCollision& collisions) {
	float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());

	if (ln == 0.0f) {
		return false; //direction vectors are perpendicular!
	}

	Vector3 planePoint = p.GetPointOnPlane();

	Vector3 pointDir = planePoint - r.GetPosition();

	float d = Vector3::Dot(pointDir, p.GetNormal()) / ln;

	collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

	return true;
}

bool CollisionDetection::RayIntersection(const Ray& r, GameObject& object, RayCollision& collision, Layer layer) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetTransform();
	const CollisionVolume* volume = object.GetBoundingVolume();

	if (!volume) {
		return false;
	}

	switch (volume->type) {
	case VolumeType::AABB:		hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume&)*volume, collision); break;
	case VolumeType::OBB:		hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume&)*volume, collision); break;
	case VolumeType::Sphere:	hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume&)*volume, collision); break;

	case VolumeType::Capsule:	hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume&)*volume, collision); break;
	}

	return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray& r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision) {
	Vector3  boxMin = boxPos - boxSize;
	Vector3 boxMax = boxPos + boxSize;

	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();

	Vector3 tVals(-1, -1, -1);

	for (int i = 0; i < 3; ++i) {
		if (rayDir[i] > 0) {
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
		}
		else if (rayDir[i] < 0) {
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
		}
	}

	float bestT = tVals.GetMaxElement();
	if (bestT < 0.0f) {
		return false;
	}

	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f;

	for (int i = 0; i < 3; ++i) {
		if (intersection[i] + epsilon < boxMin[i] || intersection[i] - epsilon > boxMax[i]) {
			return false;
		}
	}
	collision.collidedAt = intersection;
	collision.rayDistance = bestT;

	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray& r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision) {
	Vector3 boxPos = worldTransform.GetPosition();
	Vector3 boxSize = volume.GetHalfDimensions();
	return RayBoxIntersection(r, boxPos, boxSize, collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray& r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) {

	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	Vector3 localRayPos = r.GetPosition() - position;
	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());

	bool collided = RayBoxIntersection(tempRay, Vector3(), volume.GetHalfDimensions(), collision);

	if (collided) {
		collision.collidedAt = transform * collision.collidedAt + position;
	}
	return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray& r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision) {

	Vector3 spherePos = worldTransform.GetPosition();
	float sphereRadius = volume.GetRadius();

	Vector3 dir = (spherePos - r.GetPosition());

	float sphereProj = Vector3::Dot(dir, r.GetDirection());

	if (sphereProj < 0.0f) {
		return false;
	}

	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);

	float sphereDist = (point - spherePos).Length();

	if (sphereDist > sphereRadius) {
		return false;
	}

	float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));
	collision.rayDistance = sphereProj - (offset);
	collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);

	return true;
}

bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) {
	Vector3 top(worldTransform.GetPosition() + (worldTransform.GetOrientation() * Vector3(0, 1, 0) * (volume.GetHalfHeight() - volume.GetRadius())));
	Vector3 bottom(worldTransform.GetPosition() - (worldTransform.GetOrientation() * Vector3(0, 1, 0) * (volume.GetHalfHeight() - volume.GetRadius())));

	Vector3 normal = r.GetPosition() - worldTransform.GetPosition();
	Vector3 capsuleDir = top - bottom;
	Vector3 point = Vector3::Cross(capsuleDir, normal);
	point = worldTransform.GetPosition() + (point.Normalised() * 1.0f);

	Plane capPlane = Plane::PlaneFromTri(top, bottom, point);

	if (!RayPlaneIntersection(r, capPlane, collision))
		return false;

	float capsuleLineLength = capsuleDir.Length();
	capsuleDir.Normalise();

	Vector3 rayCapDir = collision.collidedAt - bottom;
	float dot = Maths::Clamp(Vector3::Dot(rayCapDir, capsuleDir), 0.0f, capsuleLineLength);

	Vector3 spherePos = bottom + (capsuleDir * dot);

	SphereVolume sphere(volume.GetRadius());
	Transform t;
	t.SetPosition(spherePos);
	t.SetScale(Vector3(1, 1, 1) * volume.GetRadius());

	return RaySphereIntersection(r, t, sphere, collision);
}

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) {
		return false;
	}

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetTransform();
	Transform& transformB = b->GetTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);

	//Two AABBs
	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Spheres
	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	//Two OBBs
	if (pairType == VolumeType::OBB) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Capsules
	if (pairType == VolumeType::Capsule){
		CapsuleIntersection((CapsuleVolume&)*volA, transformA, (CapsuleVolume&)*volB, transformB, collisionInfo);
	}

	//AABB vs OBB
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::AABB) {
		AABBToOBBIntersection((OBBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::OBB)
	{
		collisionInfo.a = b;
		collisionInfo.b = a;
		AABBToOBBIntersection((OBBVolume&)*volB, transformB, (AABBVolume&)*volA, transformA, collisionInfo);
	}
	//AABB vs Sphere pairs
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//OBB vs sphere pairs
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
		return OBBSphereIntersection((OBBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBSphereIntersection((OBBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//Capsule vs other interactions
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
		return SphereCapsuleIntersection((CapsuleVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return SphereCapsuleIntersection((CapsuleVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::AABB) {
		return AABBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	if (volB->type == VolumeType::Capsule && volA->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (AABBVolume&)*volA, transformA, collisionInfo);
	}

	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA;
	Vector3 totalSize = halfSizeA + halfSizeB;

	if (abs(delta.x) < totalSize.x &&
		abs(delta.y) < totalSize.y &&
		abs(delta.z) < totalSize.z) {
		return true;
	}
	return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3 boxAPos = worldTransformA.GetPosition();
	Vector3 boxBPos = worldTransformB.GetPosition();

	Vector3 boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBSize = volumeB.GetHalfDimensions();

	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

	if (overlap) {
		static const Vector3 faces[6] =
		{
			Vector3(-1,0,0), Vector3(1,0,0),
			Vector3(0, -1, 0), Vector3(0, 1, 0),
			Vector3(0, 0, -1), Vector3(0, 0, 1)
		};

		Vector3 maxA = boxAPos + boxASize;
		Vector3 minA = boxAPos - boxASize;

		Vector3 maxB = boxBPos + boxBSize;
		Vector3 minB = boxBPos - boxBSize;

		float distances[6] =
		{
			(maxB.x - minA.x),
			(maxA.x - minB.x),
			(maxB.y - minA.y),
			(maxA.y - minB.y),
			(maxB.z - minA.z),
			(maxA.z - minB.z)
		};

		float penetration = FLT_MAX;
		Vector3 bestAxis;

		for (int i = 0; i < 6; i++) {
			if (distances[i] < penetration) {
				penetration = distances[i];
				bestAxis = faces[i];
			}
		}

		collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
		return true;
	}

	return false;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	float radii = volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

	float deltaLength = delta.Length();

	if (deltaLength < radii) {
		float penetration = (radii - deltaLength);
		Vector3 normal = delta.Normalised();
		Vector3 localA = normal * volumeA.GetRadius();
		Vector3 localB = -normal * volumeB.GetRadius();


		collisionInfo.AddContactPoint(localA, localB, normal, penetration);

		return true;
	}

	return false;
}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Vector3 boxSize = volumeA.GetHalfDimensions();
	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

	Vector3 closestPointOnBox = Maths::Vector3::Clamp(delta, -boxSize, boxSize);

	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	if (distance < volumeB.GetRadius()) {
		Vector3 collisionNormal = localPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);

		return true;
	}

	return false;
}

bool  CollisionDetection::OBBSphereIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Quaternion boxOri = worldTransformA.GetOrientation();
	Matrix3 boxRot = Matrix3(boxOri);
	Matrix3 invBoxRot = Matrix3(boxOri.Conjugate());

	Vector3 boxSize = volumeA.GetHalfDimensions();

	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();
	delta = invBoxRot * delta;

	Vector3 closestPointOnBox = Maths::Vector3::Clamp(delta, -boxSize, boxSize);

	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	if (distance < volumeB.GetRadius())
	{
		Vector3 collisionNormal = (boxRot * localPoint).Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = localPoint;
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

bool NCL::CollisionDetection::AABBToOBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA, const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Transform tempTransform = worldTransformB;
	tempTransform.SetPosition(worldTransformB.GetPosition());
	tempTransform.SetOrientation(Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
	tempTransform.SetScale(worldTransformB.GetScale());

	OBBVolume tempVolume = OBBVolume(volumeB.GetHalfDimensions());

	bool tempCollided = OBBIntersection(volumeA, worldTransformA, tempVolume, tempTransform, collisionInfo);
	if (tempCollided) {
		collisionInfo.point.normal = -collisionInfo.point.normal;
	}
	return tempCollided;
}

bool CollisionDetection::AABBCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3 point = Maths::Vector3::Clamp(worldTransformA.GetPosition(), worldTransformB.GetPosition() - volumeB.GetHalfDimensions(), worldTransformB.GetPosition() + volumeB.GetHalfDimensions());

	SphereVolume sphere(volumeA.GetRadius());
	Transform sphereTransform;
	sphereTransform.SetPosition(SpherePosFromCapsule(volumeA, worldTransformA, point));
	sphereTransform.SetScale(Vector3(1, 1, 1) * volumeA.GetRadius());

	bool collision = AABBSphereIntersection(volumeB, worldTransformB, sphere, sphereTransform, collisionInfo);
	collisionInfo.point.normal = -collisionInfo.point.normal;
	collisionInfo.point.localA = collisionInfo.point.localA + (sphereTransform.GetPosition() - worldTransformA.GetPosition());
	return collision;
	return false;
}

bool CollisionDetection::SphereCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	SphereVolume sphere(volumeA.GetRadius());
	Transform sphereTransform;
	sphereTransform.SetPosition(SpherePosFromCapsule(volumeA, worldTransformA, worldTransformB.GetPosition()));
	sphereTransform.SetScale(Vector3(1, 1, 1) * volumeA.GetRadius());

	bool collision = SphereIntersection(sphere, sphereTransform, volumeB, worldTransformB, collisionInfo);
	collisionInfo.point.localA = collisionInfo.point.localA + (sphereTransform.GetPosition() - worldTransformA.GetPosition());
	return collision;
}

bool CollisionDetection::OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	std::vector<Vector3> volumeAVertices = GetOBBVertices(volumeA, worldTransformA.GetPosition(), worldTransformA.GetOrientation());
	std::vector<Vector3> volumeBVertices = GetOBBVertices(volumeB, worldTransformB.GetPosition(), worldTransformB.GetOrientation());

	std::vector<Vector3> edgeNormals = GetEdgeNormals(worldTransformA.GetOrientation(), worldTransformB.GetOrientation());

	Vector3 overlapAxis;
	float overlapMagnitude = FLT_MAX;

	for (int i = 0; i < 15; i++) {
		if (edgeNormals[i] == Vector3(0, 0, 0))
			continue;
		float minA = FLT_MAX;
		float maxA = std::numeric_limits<float>::lowest();

		float minB = FLT_MAX;
		float maxB = std::numeric_limits<float>::lowest();

		for (int j = 0; j < 8; j++) {
			float projA = Vector3::Dot(edgeNormals[i], volumeAVertices[j]) / edgeNormals[i].Length();
			float projB = Vector3::Dot(edgeNormals[i], volumeBVertices[j]) / edgeNormals[i].Length();


			minA = std::min(projA, minA);
			maxA = std::max(projA, maxA);

			minB = std::min(projB, minB);
			maxB = std::max(projB, maxB);
		}

		bool isOverlaping = (minA <= minB && minB <= maxA) || (minB <= minA && minA <= maxB);
		if (!isOverlaping) {
			return false;
		}
		if (std::min(maxB, maxA) - std::max(minB, minA) < overlapMagnitude) {
			overlapMagnitude = std::min(maxB, maxA) - std::max(minB, minA);
			overlapAxis = edgeNormals[i];
		}
	}

	collisionInfo.AddContactPoint(Vector3(), Vector3(), overlapAxis, overlapMagnitude);
	return true;
}

bool NCL::CollisionDetection::CapsuleIntersection(const CapsuleVolume& volumeA, const Transform& worldTransformA, const CapsuleVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	// Step 1: Check for AABB intersection (Custom AABB approximation)
	Vector3 halfDimensionsA(volumeA.GetRadius(), volumeA.GetHalfHeight() + volumeA.GetRadius(), volumeA.GetRadius());
	Vector3 halfDimensionsB(volumeB.GetRadius(), volumeB.GetHalfHeight() + volumeB.GetRadius(), volumeB.GetRadius());

	AABBVolume aabbA(halfDimensionsA);
	AABBVolume aabbB(halfDimensionsB);

	bool aabbCollision = AABBCapsuleIntersection(volumeA, worldTransformA, aabbB, worldTransformB, collisionInfo);

	if (aabbCollision) {
		// Step 2: Check for Sphere intersection
		SphereVolume sphereA(volumeA.GetRadius());
		Transform sphereTransformA;
		sphereTransformA.SetPosition(SpherePosFromCapsule(volumeA, worldTransformA, worldTransformB.GetPosition()));
		sphereTransformA.SetScale(Vector3(1, 1, 1) * volumeA.GetRadius());

		SphereVolume sphereB(volumeB.GetRadius());
		Transform sphereTransformB;
		sphereTransformB.SetPosition(SpherePosFromCapsule(volumeB, worldTransformB, worldTransformA.GetPosition()));
		sphereTransformB.SetScale(Vector3(1, 1, 1) * volumeB.GetRadius());

		bool sphereCollision = SphereIntersection(sphereA, sphereTransformA, sphereB, sphereTransformB, collisionInfo);
		collisionInfo.point.localA = collisionInfo.point.localA + (sphereTransformA.GetPosition() - worldTransformA.GetPosition());
		collisionInfo.point.localB = collisionInfo.point.localB + (sphereTransformB.GetPosition() - worldTransformB.GetPosition());

		return sphereCollision;
	}

	return false;
}

Matrix4 GenerateInverseView(const Camera& c) {
	float pitch = c.GetPitch();
	float yaw = c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
		Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

	return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	float negDepth = nearPlane - farPlane;

	float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

	Matrix4 m;

	float h = 1.0f / tan(fov * PI_OVER_360);

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = invNegDepth;//// +PI_OVER_360;
	m.array[3][2] = -1.0f;
	m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

	return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	float aspect = Window::GetWindow()->GetScreenAspect();
	float fov = cam.GetFieldOfVision();
	float nearPlane = cam.GetNearPlane();
	float farPlane = cam.GetFarPlane();

	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

	Matrix4 proj = cam.BuildProjectionMatrix(aspect);

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
		(screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
		(screenPos.z),
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

std::vector<Vector3> NCL::CollisionDetection::GetEdgeNormals(const Quaternion& orientationA, const Quaternion& orientationB)
{
	std::vector<Vector3> edges;

	edges.push_back(orientationA * Vector3(1, 0, 0)); //Principal x asix object A
	edges.push_back(orientationA * Vector3(0, 1, 0)); //Principal y asix object A
	edges.push_back(orientationA * Vector3(0, 0, 1)); //Principal z asix object A

	edges.push_back(orientationB * Vector3(1, 0, 0)); //Principal x asix object B
	edges.push_back(orientationB * Vector3(0, 1, 0)); //Principal y asix object B
	edges.push_back(orientationB * Vector3(0, 0, 1)); //Principal z asix object B

	for (int j = 0; j < 3; ++j) {
		for (int k = 3; k < 6; ++k) {
			edges.push_back(Vector3::Cross(edges[j], edges[k]).Normalised()); //Normalixed world axis
		}
	}
	return edges;
}

std::vector<Vector3> NCL::CollisionDetection::GetOBBVertices(const OBBVolume& volume, const Vector3& position, const Quaternion& orientation) {
	std::vector<Vector3> vertices;

	Vector3 halfDimensions = volume.GetHalfDimensions();
	vertices.push_back(Vector3(-halfDimensions.x, -halfDimensions.y, -halfDimensions.z));
	vertices.push_back(Vector3(halfDimensions.x, -halfDimensions.y, -halfDimensions.z));
	vertices.push_back(Vector3(halfDimensions.x, halfDimensions.y, -halfDimensions.z));
	vertices.push_back(Vector3(-halfDimensions.x, halfDimensions.y, -halfDimensions.z));
	vertices.push_back(Vector3(-halfDimensions.x, -halfDimensions.y, halfDimensions.z));
	vertices.push_back(Vector3(halfDimensions.x, -halfDimensions.y, halfDimensions.z));
	vertices.push_back(Vector3(halfDimensions.x, halfDimensions.y, halfDimensions.z));
	vertices.push_back(Vector3(-halfDimensions.x, halfDimensions.y, halfDimensions.z));

	for (int i = 0; i < 8; i++) {
		vertices[i] = orientation * vertices[i];
	}

	for (int i = 0; i < 8; i++) {
		vertices[i] += position;
	}

	return vertices;
}

Vector3 NCL::CollisionDetection::CalculateWorldOrientation(const Transform& worldTransform, const Vector3& localDirection) {
	return worldTransform.GetOrientation() * localDirection;
}
Ray CollisionDetection::BuildRayFromMouse(const PerspectiveCamera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);
}

Ray NCL::CollisionDetection::BuidRayFromCenterOfTheCamera(const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3((screenSize.x / 2.f),
		(screenSize.y / 2.f),
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3((screenSize.x / 2.f),
		(screenSize.y / 2.f),
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);

}

Vector3 NCL::CollisionDetection::GetCameraVec(const PerspectiveCamera& cam)
{
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3((screenSize.x / 2.f),
		(screenSize.y / 2.f),
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3((screenSize.x / 2.f),
		(screenSize.y / 2.f),
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return c;
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;

	float t = tan(fov * PI_OVER_360);

	float neg_depth = nearPlane - farPlane;

	const float h = 1.0f / t;

	float c = (farPlane + nearPlane) / neg_depth;
	float e = -1.0f;
	float d = 2.0f * (nearPlane * farPlane) / neg_depth;

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = 1.0f / d;

	m.array[3][2] = 1.0f / e;
	m.array[3][3] = -c / (d * e);

	return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera& c) {
	float pitch = c.GetPitch();
	float yaw = c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
		Matrix4::Rotation(pitch, Vector3(1, 0, 0));

	return iview;
}

Vector3 NCL::CollisionDetection::SpherePosFromCapsule(const CapsuleVolume& capsule, const Transform& capTransform, const Vector3& otherObjPos) {
	Vector3 extentPosition = capTransform.GetOrientation() * Vector3(0, 1, 0) * (capsule.GetHalfHeight() - capsule.GetRadius());
	Vector3 capTop(capTransform.GetPosition() + extentPosition);
	Vector3 capBottom(capTransform.GetPosition() - extentPosition);

	Vector3 capsuleDir = capTop - capBottom;
	float capLineLength = capsuleDir.Length();
	capsuleDir.Normalise();

	Vector3 pointCapDir = otherObjPos - capBottom;
	float dot = Maths::Clamp(Vector3::Dot(pointCapDir, capsuleDir), 0.0f, capLineLength);

	return capBottom + (capsuleDir * dot);
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const PerspectiveCamera& c) {
	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());


	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(position.x / (float)screenSize.x) * 2.0f - 1.0f,
		(position.y / (float)screenSize.y) * 2.0f - 1.0f,
		(position.z) - 1.0f,
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

