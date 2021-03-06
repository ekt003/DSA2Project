#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	gameActive = true;
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 5.0f, 25.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

	Simplex::TextureManager::GetInstance()->LoadTexture("logo-v3.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("fastboi.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("lifeboi.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("enter.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("button-o.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("button-u.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("arrows.png");
	Simplex::TextureManager::GetInstance()->LoadTexture("gameover.png");

	m_pEntityMngr->AddEntity("Minecraft\\Spaceship.obj", "Spaceship");
	m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube");
	m_pEntityMngr->RemoveEntity(m_pEntityMngr->GetUniqueID(1));
	m_pEntityMngr->UsePhysicsSolver();
	
	m_Ship = m_pEntityMngr->GetEntity(0);

	m_pEntityMngr->GetEntity(0)->SetMass(1.0f);
	//Generate first Oct Tree
	m_pRoot = new MyOctant(m_iOctLevel, 5);
	speedStep = 0;
	timer = 2000;
	currentState = Title;
	//gameActive = false;
	//gameActive = true;

	//loading in high scores from a file
	std::vector<uint> highScores = std::vector<uint>();
	LoadHighScores();

}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();
	if(currentState == Game){
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
	

	//Oct tree go here.
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
	if ((timer) == 2000) {

		//resets timer
		timer = 0;
		//loads appropriate file based on random number generation
		spawnPhase = glm::linearRand(1, 5);
		//spawnThread = std::thread(&this::LoadEntity, spawnPhase);
		LoadEntity(spawnPhase);		
			
		//speed up
		m_fSpeed += 0.05f;
		speedStep++;
	}

	//Draw Oct Tree
	if (timer % (int)(100/(m_fSpeed+4)) == 0) {
		if (octEnabled) {
			m_pRoot->Release();
			SafeDelete(m_pRoot);
			m_pRoot = new MyOctant(m_iOctLevel, 5); //Redraw Oct Tree
		}
	}

	

	//speeding up text timer
	if (timer < 100)
	{
		speedup = true;
	}
	else
	{
		speedup = false;
	}

	//Checking collisions with ship
	if (lifeTimer == 0)
	{
		if (m_pEntityMngr->GetEntity(0)->GetRigidBody()->GetIBeCollide() == true)
		{
			lifeTimer = 10;
			m_uLives--;
		}
	}
	
	if (lifeTimer != 0)
	{
		lifeTimer--;
	}
	if (m_uLives <= 0 && godMode == false)
	{
		currentState = End;
	}
	static int cubesSpawned = 0;
	if (!octMode)
	{
		if ((timer % (6 / ((int)m_fSpeed + 1))) == 0 && (timer > 600)) { //creates one entity every 10 update loops

			m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + m_nCubeCount);
			m_nCubeCount++;

			//sets x position based on random value centered around player
			//sets y position as zero always
			//sets z position as 100 past the camera position. Eventually the camera will render less than that so it will look like the cubes fade into existence
			vector3 v3Position = vector3(m_v3CameraPos.x + glm::linearRand((-5 * (m_fSpeed * 20)), (10 * (m_fSpeed * 20))), 0.0f, m_v3CameraPos.z + 100);
			matrix4 m4Position = glm::translate(v3Position);
			//setting position of cube
			m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));

			cubesSpawned++;

			if (cubesSpawned > 15)
			{
				if (octEnabled) {
					m_pRoot->Release();
					SafeDelete(m_pRoot);
					m_pRoot = new MyOctant(m_iOctLevel, 5); //Redraw Oct Tree
				}
				cubesSpawned = 0;
			}

		}
	}
	else
	{
		OctreeMode(cubesSpawned);
		cubesSpawned += 5;
	}
	//cube timer, to be done better later
	timer++;
	m_uScore++;

	//Set the model matrix for the main object
	//m_pEntityMngr->SetModelMatrix(m_m4Steve, "Steve");

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);

	//moves ship
	vector3 velocity = m_Ship->GetVelocity();
	m_Ship->SetVelocity(vector3(velocity.x, velocity.y, m_fSpeed));

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
	//spawnThread.join();
	
	//Display Oct Tree
	if(octEnabled)
		m_pRoot->Display();

	}//gameActive

}

void Application::LoadEntity(int a_spawnPhase) {
	switch (a_spawnPhase) {
	case 1: //Left Turn Spawn
		fileReader.open("SpawnFiles/LeftTurn.txt");
		FillMap();
		break;
	case 2: //Right Turn Spawn
		fileReader.open("SpawnFiles/RightTurn.txt");
		FillMap();
		break;
	case 3: //X Spawn
		fileReader.open("SpawnFiles/XSpawn.txt");
		FillMap();
		break;
	case 4: //S Spawn
		fileReader.open("SpawnFiles/SSpawn.txt");
		FillMap();
		break;
	case 5: //Diamond Spawn
		fileReader.open("SpawnFiles/DiamondSpawn.txt");
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
		}
		else {
			if (input == 0 || input == 1) { //gets rid of garbage non binary values
				spawnMap[row][col] = input; //sets map to read in value
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
	startingPoint.x -= 36;
	//iterating through spawn map
	for (int i = 0; i < 120; i++) {
		for (int j = 0; j < 24; j++) {
			if (spawnMap[i][j] == 1) {
				m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + m_nCubeCount);
				m_nCubeCount++;
				vector3 position = startingPoint;
				position.z += (i*3) + 80;
				position.x += ((j*3));
				matrix4 m4Position = glm::translate(position);

				m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));
			}

		}
	}

	//https://stackoverflow.com/questions/2516096/fastest-way-to-zero-out-a-2d-array-in-c
	//entirely taken from here I take no credit
	memset(spawnMap, 0, sizeof spawnMap);

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

void Application::LoadHighScores(void) {
	//opens high score file
	fileReader.open("HighScores/highScores.txt");

	highScores.clear();

	//checks for successful loading
	if (fileReader.is_open()) {
		//read in each high score value

		//store it in the high scores vector



		//closes file reader
		fileReader.close();
	}

}


void Application::SaveHighScore(uint a_highScore) {
	//if first high score, store it
	if (highScores.size() == 0) {
		highScores.push_back(a_highScore);
	}
	else {
		//loops through high score vector and stores high score in proper position
		for (int i = 0; i < highScores.size(); i++) {
			//found spot where it should be added
			if (a_highScore > highScores[i]) {
				//highScores.insert(highScores.begin+i, a_highScore); //inserts high score in proper position
			}
		}
	}
	
	//opens high score file
	fileWriter.open("HighScores/highScores.txt");

	//checks for successful loading
	if (fileWriter.is_open()) {
		//writes new vector to the file


		//closes file reader
		fileWriter.close();
	}
}

void Application::OctreeMode(int cubesSpawned) {
	for (int i = 0; i < 2; i++)
	{
		m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + m_nCubeCount);
		m_nCubeCount++;

		//sets x position based on random value centered around player
		//sets y position as zero always
		//sets z position as 100 past the camera position. Eventually the camera will render less than that so it will look like the cubes fade into existence
		vector3 v3Position = vector3(m_v3CameraPos.x + glm::linearRand((-10 * (m_fSpeed * 20)), (10 * (m_fSpeed * 20))), 0.0f, m_v3CameraPos.z + 100);
		matrix4 m4Position = glm::translate(v3Position);
		//setting position of cube
		m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));

		if (cubesSpawned > 150)
		{
			if (octEnabled) {
				m_pRoot->Release();
				SafeDelete(m_pRoot);
				m_pRoot = new MyOctant(m_iOctLevel, 5); //Redraw Oct Tree
			}
				
			cubesSpawned = 0;
		}
	}
}