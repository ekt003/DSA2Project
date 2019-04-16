#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 5.0f, 25.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

	Simplex::TextureManager::GetInstance()->LoadTexture("ShipUVs.jpg");
	Simplex::TextureManager::GetInstance()->LoadTexture("logo-v3.png");



	m_pEntityMngr->AddEntity("Minecraft\\Spaceship.obj", "Spaceship");
	m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube");
	m_pEntityMngr->RemoveEntity(m_pEntityMngr->GetUniqueID(1));
	m_pEntityMngr->UsePhysicsSolver();
	
	m_Ship = m_pEntityMngr->GetEntity(0);

}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager
	m_pEntityMngr->Update();

	//Update camera
	m_v3CameraPos = m_Ship->GetPosition(); //Get position of steve

	//Set camera to be up and behind steve
	m_v3CameraPos.z -= 10.0f;
	m_v3CameraPos.y += 3.0f;

	//Handle Collisions
	m_pEntityMngr->UsePhysicsSolver();

	//Handling counter rotation
	if (!isRotating)
	{
		//If we're tilting to the right, roll left
		if (m_fDelta > 0)
		{
			m_fDelta -= .5f;
		}
		//If we're tilting to the left, roll right
		else if (m_fDelta < 0)
		{
			m_fDelta += .5f;
		}
	}

	//Calculate rotation matrix of the up vector for the camera
	glm::mat4 rot = glm::rotate(IDENTITY_M4, glm::radians(m_fDelta), AXIS_Z);

	//apply rotation to the camera's up vector (the Y-AXIS)
	vector3 newUp = vector3(rot * vector4(AXIS_Y, 0));

	//Set the camera's position, target, and up vector
	m_pCameraMngr->SetPositionTargetAndUpward(m_v3CameraPos, m_Ship->GetPosition(), newUp);
	//SPAWN CUBES

	//decide spawn patterns
	if ((timer) == 1000) {
		//std::cout << "HERE";
		//loads appropriate file based on random number generation
		spawnPhase = glm::linearRand(1, 5);
		spawnPhase = 5;
		LoadEntity(spawnPhase);
		//speed up
		m_fSpeed += 0.05f;
		//resets timer
		timer = 0;
	}

	if (timer < 100)
	{
		speedup = true;
	}
	else
	{
		speedup = false;
	}

	/*
	if (timer % 10 == 0) { //creates one entity every 10 update loops
		m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + m_nCubeCount);
		m_nCubeCount++;

		//sets x position based on random value centered around player
		//sets y position as zero always
		//sets z position as 100 past the camera position. Eventually the camera will render less than that so it will look like the cubes fade into existence
		vector3 v3Position = vector3(m_v3CameraPos.x + glm::linearRand(-50, 50), 0.0f, m_v3CameraPos.z + 100);
		matrix4 m4Position = glm::translate(v3Position);
		//setting position of cube
		m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));

	}*/
	//cube timer, to be done better later
	timer++;
	m_uScore++;

	//Set the model matrix for the main object
	//m_pEntityMngr->SetModelMatrix(m_m4Steve, "Steve");

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
	//m_pEntityMngr->AddEntityToRenderList(-1, true);
	vector3 shipPos = m_Ship->GetPosition();
	shipPos.z += m_fSpeed;
	m_Ship->SetPosition(shipPos);

	/*float fDelta = m_pSystem->GetDeltaTime(0);
	m_pEntityMngr->ApplyForce(vector3(0.0f, 0.0f, 2.0 * fDelta), "Steve");*/

	//destroy entities that are beyond the camera
	for (size_t i = 1; i < m_pEntityMngr->GetEntityCount(); i++)
	{
		vector3 blockPosition = m_pEntityMngr->GetEntity(i)->GetPosition();
		//if the object is beyond the near value of the camera
		//std::cout << "block pos z: " << blockPosition.z << "    camera z" << m_v3CameraPos.z << std::endl;
		if (blockPosition.z < m_v3CameraPos.z)
		{
			//std::cout << "DELETED CUBE" << std::endl;
			m_pEntityMngr->RemoveEntity(m_pEntityMngr->GetUniqueID(i));
		}
	}
}

void Application::LoadEntity(int a_spawnPhase) {
	switch (a_spawnPhase) {
	case 1: //Left Turn Spawn
		std::cout << "Phase1" << std::endl;
		fileReader.open("SpawnFiles/LeftTurn.txt");
		FillMap();
		break;
	case 2: //Right Turn Spawn
		std::cout << "Phase2" << std::endl;
		fileReader.open("SpawnFiles/RightTurn.txt");
		FillMap();
		break;
	case 3: //X Spawn
		std::cout << "Phase3" << std::endl;
		fileReader.open("SpawnFiles/XSpawn.txt");
		FillMap();
		break;
	case 4: //S Spawn
		std::cout << "Phase4" << std::endl;
		fileReader.open("SpawnFiles/SSpawn.txt");
		FillMap();
		break;
	case 5: //Diamond Spawn
		std::cout << "Phase5" << std::endl;
		fileReader.open("SpawnFiles/BinaryField.txt");
		FillMap();
		break;
	}
}

void Application::FillMap(void) {
	
	uint input = 0;
	uint row = 0;
	uint col = 0;
	//if file reader is not open, don't run anything
	if (!fileReader.is_open())
		return;
	
	char c = fileReader.get(); //gets next file value
	while (fileReader.good()) { //keeps going until at the end of the file
		input = c - '0'; //converts character input into an int
		if (c == 10) { //this is a new line
			row++; //increments row
			col = 0;
			std::cout << std::endl;
		}
		else {
			if (input == 0 || input == 1) { //gets rid of garbage non binary values
				spawnMap[row][col] = input; //sets map to read in value
				std::cout << spawnMap[row][col];
				col++; //increments column
			}
		}
		c = fileReader.get(); //gets next input
	}
	fileReader.close(); //closes file reader at the end of the method call
	SpawnEntity(); //spawns entities based on newly loaded array
}


void Application::SpawnEntity(void) {
	vector3 startingPoint = m_Ship->GetPosition();
	startingPoint.x -= 60;
	//iterating through spawn map
	for (int i = 0; i < 24; i++) {
		for (int j = 0; j < 90; j++) {
			if (spawnMap[i][j] == 1) {
				m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_");
				vector3 position = startingPoint;
				position.x += (i*5);
				position.z += ((j*5) + 100);

				matrix4 m4Position = glm::translate(position);

				m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));
			}


		}
	}

}

void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui,
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//Release MyEntityManager
	MyEntityManager::ReleaseInstance();

	//release GUI
	ShutdownGUI();
}