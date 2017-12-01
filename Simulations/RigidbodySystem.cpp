#include "RigidbodySystem.h"



RigidbodySystem::RigidbodySystem()
{

}

RigidbodySystem::RigidbodySystem(Vec3 size, Vec3 position, float mass) : size(size), m_position(position), mass(mass)
{
	// 45 deg. 
	// need to be removed
	// ------------------------
	//orientation = sqrt(2) / 2;
	// ------------------------
	orientation = Quat(0, 0.0, 0.0, 1);
	rotMat.initRotationZ(0);
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
	Vec3 armVector = loc - m_position;
	//Watch out should be += not =
	torque += GamePhysics::cross(armVector,f);

}

void RigidbodySystem::updateStep(float elapsedTime)
{
	float h = elapsedTime;
	Mat4 rotMatTranspose = Mat4(rotMat);
	rotMatTranspose.transpose();

	m_position += h * velocity;
	velocity += h * (force / mass);

	orientation += h / 2.0f * Quat(angluarvelocity.x, angluarvelocity.y, angluarvelocity.z, 0) * orientation;
	orientation.unit();
	angularMomentum += h * torque;
	Mat4 tempInteriatensor = rotMat * interiatensor * rotMatTranspose;
	angluarvelocity = tempInteriatensor.transformVector(angularMomentum);

	transMat.initTranslation(m_position.x, m_position.y, m_position.z);
	rotMat = orientation.getRotMat();

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

void RigidbodySystem::calculateInteriaTensor() {	
	//Rotation around z-achse
	//float a = size.x;
	//float b = size.y;
	//interiatensor = mass*(a*a + b*b) / 12.0f;

	float w = size.x;
	float h = size.y;
	float d = size.z;

	interiatensor = Mat4(12.0f / mass*(h*h + d*d), 0.0f, 0.0f, 0.0f,
						 0.0f, 12.0f / (mass*(w*w + d*d)), 0.0f, 0.0f,
					     0.0f, 0.0f, 12.0f / (mass*(w*w + h*h)), 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f);
};