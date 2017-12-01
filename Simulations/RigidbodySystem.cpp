#include "RigidbodySystem.h"



RigidbodySystem::RigidbodySystem()
{

}

RigidbodySystem::RigidbodySystem(Vec3 size, Vec3 position, int mass) : size(size), m_position(position), mass(mass)
{
	// 45 deg. 
	// need to be removed
	// ------------------------
	orientation = sqrt(2) / 2;
	// ------------------------
	rotMat.initRotationZ(orientation);
	transMat.initTranslation(position.x, position.y, position.z);
	scaleMat.initScaling(size.x, size.y, size.z);
	angluarvelocity = 0;
	calculateInteriaTensor();
}

RigidbodySystem::~RigidbodySystem()
{
}

void RigidbodySystem::applyForce(Vec3& loc, Vec3& f)
{
	//for 3D vector
	// F += f;
	// xi = loc - center
	// torque += xi x force
	
	force += f;
	//loc is probably in world space as m_position
	// armvector = x - loc
	Vec3 armVector = m_position - loc;
	//Watch out should be += not =
	torque += GamePhysics::cross(armVector,f);

}

void RigidbodySystem::updateStep(float elapsedTime)
{
	float h = elapsedTime;

	m_position += h * velocity;
	velocity += h * (force / mass);

	orientation += h * angluarvelocity;
	angluarvelocity += h * (torque.z / interiatensor);

	transMat.initTranslation(m_position.x, m_position.y, m_position.z);
	rotMat.initRotationZ(orientation);

	clearForce();
}

void RigidbodySystem::clearForce() {
	force.x = 0;
	force.y = 0;
	force.z = 0;

	torque.x = 0;
	torque.y = 0;
	torque.z = 0;
}