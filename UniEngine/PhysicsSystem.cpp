#include "pch.h"
#include "PhysicsSystem.h"
using namespace physx;

PxRigidDynamic* UniEngine::PhysicsSystem::createDynamic(const PxTransform& t, const PxGeometry& geometry,
	const PxVec3& velocity)
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*_Physics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	_PhysicsScene->addActor(*dynamic);
	return dynamic;
}

void UniEngine::PhysicsSystem::createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = _Physics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = _Physics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			_PhysicsScene->addActor(*body);
		}
	}
	shape->release();
}

void UniEngine::PhysicsSystem::OnCreate()
{
	_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, _Allocator, _ErrorCallback);

	_PhysVisDebugger = PxCreatePvd(*_PhysicsFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	_PhysVisDebugger->connect(*transport, PxPvdInstrumentationFlag::eALL);

	_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_PhysicsFoundation, PxTolerancesScale(), true, _PhysVisDebugger);

	PxSceneDesc sceneDesc(_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	_Dispatcher = PxDefaultCpuDispatcherCreate(_ThreadPool->Size());
	sceneDesc.cpuDispatcher = _Dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	_PhysicsScene = _Physics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = _PhysicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = _Physics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*_Physics, PxPlane(0, 1, 0, 0), *gMaterial);
	_PhysicsScene->addActor(*groundPlane);

	for (PxU32 i = 0; i < 5; i++)
		createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f);

	if (true)
		createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));
	Enable();
}

void UniEngine::PhysicsSystem::OnDestroy()
{
	PX_RELEASE(_PhysicsScene);
	PX_RELEASE(_Dispatcher);
	PX_RELEASE(_Physics);
	if (_PhysVisDebugger)
	{
		PxPvdTransport* transport = _PhysVisDebugger->getTransport();
		_PhysVisDebugger->release();	_PhysVisDebugger = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(_PhysicsFoundation);
}

void UniEngine::PhysicsSystem::FixedUpdate()
{
	_PhysicsScene->simulate(_Time->FixedDeltaTime());
	_PhysicsScene->fetchResults(true);
}
