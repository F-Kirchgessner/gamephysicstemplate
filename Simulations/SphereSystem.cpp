#include "SphereSystem.h"



SphereSystem::SphereSystem()
{
	boxSize = 0.5f * 2;
}


SphereSystem::~SphereSystem()
{
}

void SphereSystem::addSphere(float radius, float mass, Vec3 position, Vec3 color)
{
	Sphere s(radius, mass, position);
	s.red = color.x;
	s.green = color.y;
	s.blue = color.z;
	spheres.push_back(s);
}

void SphereSystem::updateStep(float elapsedTime, float damping, int accelerator)
{
	if (spheres.size() == 0)
		return;

	float h = elapsedTime;

	//add Gravity
	//add damping force
	for (auto& sphere : spheres) {
		sphere.addGravity(0.50f);
		sphere.addDamping(damping);
	}

	//check for collision and add penalty-force
	if (accelerator == 0) {
		naiveCollision();
	}
	else {
		uniformGridCollision();
	}

	//LeapFrog
	for (auto& sphere : spheres) {
		if (sphere.init) {
			sphere.v.x += (sphere.f.x / sphere.m) * h / 2.0f;
			sphere.v.y += (sphere.f.y / sphere.m) * h / 2.0f;
			sphere.v.z += (sphere.f.z / sphere.m) * h / 2.0f;
			
		}
		sphere.init = false;
	}
	
	for (auto& sphere : spheres) {
		sphere.v.x += (sphere.f.x / sphere.m) * h;
		sphere.v.y += (sphere.f.y / sphere.m) * h;
		sphere.v.z += (sphere.f.z / sphere.m) * h;
		
		sphere.pos.x += sphere.v.x * h;
		sphere.pos.y += sphere.v.y * h;
		sphere.pos.z += sphere.v.z * h;
	}

	for (auto& sphere : spheres) {
		sphere.clearForce();
	}

	checkBox();
}


void SphereSystem::naiveCollision() {
	for (int i = 0; i < spheres.size(); i++) {
		for (int j = i + 1; j < spheres.size(); j++) {
			checkForCollision(spheres[i], spheres[j]);
		}
	}
}


void SphereSystem::uniformGridCollision() {
	
	Sphere *grid[numCells * numCells * numCells][maxSpheres] = {{ NULL }};
	float cellSize = boxSize / numCells;

	for (auto& sphere : spheres) {
		int x = int((sphere.pos.x + (boxSize / 2)) / (boxSize / numCells));
		int y = int((sphere.pos.y + (boxSize / 2)) / (boxSize / numCells));
		int z = int((sphere.pos.z + (boxSize / 2)) / (boxSize / numCells));
		int gridPos = x + y * numCells + z * numCells * numCells;
		int i = 0;
		for (; grid[gridPos] != NULL && i < maxSpheres - 1 && grid[gridPos][i] != NULL; i++) {}

		grid[gridPos][i] = &sphere;
	}

	for (int x = 0; x < numCells; x++) {
		for (int y = 0; y < numCells; y++) {
			for (int z = 0; z < numCells; z++) {
				int cell = x + y * numCells + z * numCells * numCells;
				if (grid[cell] == NULL ||grid[cell][0] == NULL)
					continue;

				checkCells(grid[cell], grid[cell]);
				if (y < numCells - 1) 
					checkCells(grid[cell], grid[cell + numCells]);
				if (x < numCells - 1) 
					checkCells(grid[cell], grid[cell + 1]);
				if (x < numCells - 1 && y < numCells - 1) 
					checkCells(grid[cell], grid[cell + 1 + numCells]);
				if (z < numCells - 1) 
					checkCells(grid[cell], grid[cell + numCells * numCells]);
				if (x < numCells - 1 && z < numCells - 1) 
					checkCells(grid[cell], grid[cell + 1 + numCells * numCells]);
				if (y < numCells - 1 && z < numCells - 1) 
					checkCells(grid[cell], grid[cell + numCells + numCells * numCells]);
				if (x < numCells - 1 && y < numCells - 1 && z < numCells - 1) 
					checkCells(grid[cell], grid[cell + 1 + numCells + numCells * numCells]);
			}
		}
	}
}


void SphereSystem::checkCells(Sphere **cell1, Sphere **cell2) {
	if (cell1 == NULL || cell2 == NULL)
		return;
	for (int i = 0; i < maxSpheres && cell1[i] != NULL; i++) {
		int j = 0;
		if (cell1 == cell2)
			j = i + 1;
		for (; j < maxSpheres && cell2[j] != NULL; j++) {
			checkForCollision(*cell1[i], *cell2[j]);
		}
	}
}


void SphereSystem::checkBox() {
	float boxSizeHalf = boxSize / 2;
	for (auto& sphere : spheres) {
		float radius = sphere.r;

		if (sphere.pos.x - radius < -boxSizeHalf) {
			sphere.pos.x = -boxSizeHalf + radius;
			sphere.v.x = -sphere.v.x;
		}
		if (sphere.pos.y - radius < -0.5) {
			sphere.pos.y = -boxSizeHalf + radius;
			sphere.v.y = -sphere.v.y;
		}
		if (sphere.pos.z - radius < -boxSizeHalf) {
			sphere.pos.z = -boxSizeHalf + radius;
			sphere.v.z = -sphere.v.z;
		}

		if (sphere.pos.x + radius > boxSizeHalf) {
			sphere.pos.x = boxSizeHalf - radius;
			sphere.v.x = -sphere.v.x;
		}
		if (sphere.pos.y + radius > boxSizeHalf) {
			sphere.pos.y = boxSizeHalf - radius;
			sphere.v.y = -sphere.v.y;
		}
		if (sphere.pos.z + radius > boxSizeHalf) {
			sphere.pos.z = boxSizeHalf - radius;
			sphere.v.z = -sphere.v.z;
		}
	}
}

void SphereSystem::resolveCollision(Sphere &a, Sphere &b) {
	float lam = 10;
	Vec3 penForce;
	penForce.x = lam*(1 - (b.pos.x - a.pos.x) / (2 * a.r));
	penForce.y = lam*(1 - (b.pos.y - a.pos.y) / (2 * a.r));
	penForce.z = lam*(1 - (b.pos.z - a.pos.z) / (2 * a.r));
	//std::cout << penForce.x << ' ' << penForce.y << ' ' << penForce.z << '\n';

	a.addPenaltyForce(penForce);
	b.addPenaltyForce(-penForce);
}

void SphereSystem::checkForCollision(Sphere &a, Sphere &b) {
	float distance = FLT_MAX;
	float dx = a.pos.x - b.pos.x;
	float dy = a.pos.y - b.pos.y;
	float dz = a.pos.z - b.pos.z;
	distance = dx*dx + dy * dy + dz * dz;
	if (distance <= (a.r + b.r) * (a.r + b.r)) {
		resolveCollision(a, b);

		Vec3 penForce = Vec3(0.0f, 0.0f, 0.0f);
		float lam = 0.5f;
		penForce.x = lam*(1 - (b.pos.x - a.pos.x) / (2 * a.r));
		penForce.y = lam*(1 - (b.pos.y - a.pos.y) / (2 * a.r));
		penForce.z = lam*(1 - (b.pos.z - a.pos.z) / (2 * a.r));
		a.addPenaltyForce(penForce);
		b.addPenaltyForce(-penForce);
	}
}