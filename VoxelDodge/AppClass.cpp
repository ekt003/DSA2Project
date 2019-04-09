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

	m_pEntityMngr->AddEntity("Minecraft\\Steve.obj", "Steve");
	m_pEntityMngr->UsePhysicsSolver();
	
	m_Ship = m_pEntityMngr->GetEntity(0);

	/*
	for (int i = 0; i < 100; i++)
	{
		m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + std::to_string(i));
		
		vector3 v3Position = vector3(glm::sphericalRand(12.0f));
		v3Position.y = 0.0f;
		matrix4 m4Position = glm::translate(v3Position);
		m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));
		m_pEntityMngr->UsePhysicsSolver();
		//m_pEntityMngr->SetMass(2);

		//m_pEntityMngr->SetMass(i+1);
	}*/
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
			m_fDelta -= 1.0f;
		}
		//If we're tilting to the left, roll right
		else if (m_fDelta < 0)
		{
			m_fDelta += 1.0f;
		}
	}

	//Calculate rotation matrix of the up vector for the camera
	glm::mat4 rot = glm::rotate(IDENTITY_M4, glm::radians(m_fDelta), AXIS_Z);

	//apply rotation to the camera's up vector (the Y-AXIS)
	vector3 newUp = vector3(rot * vector4(AXIS_Y, 0));

	//Set the camera's position, target, and up vector
	m_pCameraMngr->SetPositionTargetAndUpward(m_v3CameraPos, m_Ship->GetPosition(), newUp);
	//SPAWN CUBES
	if (timer == 10) { //creates one entity every 10 update loops
		m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_");

		//sets x position based on random value centered around player
		//sets y position as zero always
		//sets z position as 100 past the camera position. Eventually the camera will render less than that so it will look like the cubes fade into existence
		vector3 v3Position = vector3(m_v3CameraPos.x + glm::linearRand(-30, 30), 0.0f, m_v3CameraPos.z + 100); 
		matrix4 m4Position = glm::translate(v3Position);
		//setting position of cube
		m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));
		//reseting timer
		timer = 0;
	}
	//cube timer, to be done better later
	timer++;


	//Set the model matrix for the main object
	//m_pEntityMngr->SetModelMatrix(m_m4Steve, "Steve");

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
	//m_pEntityMngr->AddEntityToRenderList(-1, true);
	vector3 shipPos = m_Ship->GetPosition();
	shipPos.z += 0.5f;
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