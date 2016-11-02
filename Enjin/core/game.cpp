#include "game.h"

// Default Constructor
Game::Game()
{
}

// Parameter Constructor
Game::Game(const GLfloat& width, const GLfloat& height)
	: m_width(width), m_height(height)

{
	m_mainCamera = Camera(m_width, m_height);

	m_lastXPos = 400.0f;
	m_lastYPos = 300.0f;

	m_firstMouse = true;

	Initialize();
}

// Destructor
Game::~Game()
{
}

// Intialize the game
void Game::Initialize()
{
	m_teapot = Model("assets/models/teapot.obj");
	m_terrain = Model("assets/models/terrain/terrain.obj");

	CreateLampMesh(0.05f);

	m_lampShader = Shader("assets/shaders/lamp.vs", "assets/shaders/lamp.fs");
	m_terrainShader = Shader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs");
	m_explodeShader = Shader("assets/shaders/explode.vs", "assets/shaders/explode.fs", "assets/shaders/explode.gs");
	
	m_projection = m_mainCamera.GetProjectionMatrix();

	m_lightDirectional = DirectionalLight(vec3(-0.2f, -1.0f, -0.3f), vec3(0.05f), vec3(0.05f), vec3(1.0f));

	m_lightPoints.reserve(k_N_POINT_LIGHTS * sizeof(PointLight));

	vec3 pointLightPositions[k_N_POINT_LIGHTS] = {
		vec3(0.0f, 0.0f, 0.0f),
		vec3(20.3f, 1.6f, 20.0f),
		vec3(-40.3f, 2.0f, -10.0f),
		vec3(0.3f, -1.6f, -20.0f),
		vec3(15.3f, 0.0f, 30.0f)
	};

	for (unsigned int i = 0; i < k_N_POINT_LIGHTS; ++i)
		m_lightPoints.push_back(PointLight(pointLightPositions[i], 1.0f, 0.009f, 0.0032f, vec3(0.2f), vec3(0.5f), vec3(1.0f)));

	m_lightSpot = SpotLight(vec3(-20.3f, 0.0f, -50.0f), vec3(0.0f, -1.0f, 0.0f), 1.0f, 0.009f, 0.0032f, vec3(1.0f), vec3(0.8f), vec3(1.0f), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)));

	m_explodeDelta = 0.0f;
	m_angle = 0.0f;

	// Set Terrain Shader Uniforms

	m_terrainShader.Use();

	glUniform3f(glGetUniformLocation(m_terrainShader.GetProgram(), "_eyePos"),
		m_mainCamera.GetPosition().x, m_mainCamera.GetPosition().y, m_mainCamera.GetPosition().z);

	SetLightUniforms(m_terrainShader);
	
	glUniform3f(glGetUniformLocation(m_terrainShader.GetProgram(), "_material.ambient"), 0.05f, 0.05f, 0.05f);

	glUniform3f(glGetUniformLocation(m_terrainShader.GetProgram(), "_material.diffuse"), 0.151f, 0.151f, 0.151f);

	glUniform3f(glGetUniformLocation(m_terrainShader.GetProgram(), "_material.specular"), 0.297254f, 0.30829f, 0.306678f);

	glUniform1f(glGetUniformLocation(m_terrainShader.GetProgram(), "_material.shininess"), 0.1f * 128.0f);

	glUseProgram(0);

	// Set Explode Shader Uniforms

	m_explodeShader.Use();

	glUniform3f(glGetUniformLocation(m_explodeShader.GetProgram(), "_eyePos"),
		m_mainCamera.GetPosition().x, m_mainCamera.GetPosition().y, m_mainCamera.GetPosition().z);

	glUniform1f(glGetUniformLocation(m_explodeShader.GetProgram(), "_time"), float(glfwGetTime()));

	glUniform3f(glGetUniformLocation(m_explodeShader.GetProgram(), "_material.ambient"), 1.0f, 0.5f, 0.3f);

	glUniform3f(glGetUniformLocation(m_explodeShader.GetProgram(), "_material.diffuse"), 1.0f, 0.5f, 0.3f);

	glUniform3f(glGetUniformLocation(m_explodeShader.GetProgram(), "_material.specular"), 0.5f, 0.5f, 0.5f);

	glUniform1f(glGetUniformLocation(m_explodeShader.GetProgram(), "_material.shininess"), 32.0f);

	SetLightUniforms(m_explodeShader);

	glUseProgram(0);
}

// Update the game
void Game::Update()
{
	m_mainCamera.Update();

	m_angle += 0.5f * 0.01f;

	if (m_angle > 360.0f)
		m_angle = 0.0f;
}

// Render the game
void Game::Render()
{
	m_view = m_mainCamera.GetViewMatrix();

	// Clear the colorbuffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderLamps();
	RenderTerrain();
	RenderExplodingTeapot();
}

void Game::CreateLampMesh(float size)
{
	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] = {
		// Positions          // Normals           // Texture Coords
		-size, -size, -size,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		size, -size, -size,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		size,  size, -size,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		size,  size, -size,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-size,  size, -size,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-size, -size, -size,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-size, -size,  size,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		size, -size,  size,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		size,  size,  size,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		size,  size,  size,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-size,  size,  size,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-size, -size,  size,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-size,  size,  size, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-size,  size, -size, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-size, -size, -size, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-size, -size, -size, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-size, -size,  size, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-size,  size,  size, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		size,  size,  size,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		size,  size, -size,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		size, -size, -size,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		size, -size, -size,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		size, -size,  size,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		size,  size,  size,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-size, -size, -size,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		size, -size, -size,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		size, -size,  size,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		size, -size,  size,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-size, -size,  size,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-size, -size, -size,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-size,  size, -size,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		size,  size, -size,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		size,  size,  size,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		size,  size,  size,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-size,  size,  size,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-size,  size, -size,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	// Create buffers/arrays
	glGenVertexArrays(1, &m_lampVAO);
	glGenBuffers(1, &m_lampVBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_lampVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(m_lampVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void Game::RenderLamps()
{
	m_lampShader.Use();

	for (unsigned int i = 0; i < k_N_POINT_LIGHTS; ++i)
	{
		// Load Identity Matrix
		m_model = mat4();

		// Translate to origin
		m_model = glm::translate(m_model, m_lightPoints[i].GetPosition());

		// Calculate Model View Projection Matrix mvp => projetction * view * model
		m_mvp = m_projection * m_view * m_model;
		glUniformMatrix4fv(glGetUniformLocation(m_lampShader.GetProgram(), "_mvpMat"), 1, GL_FALSE, glm::value_ptr(m_mvp));

		// Render Lamp

		glBindVertexArray(m_lampVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	glUseProgram(0);
}

void Game::RenderTerrain()
{
	// Terrain

	m_terrainShader.Use();

	vec3 lightColor;
	lightColor.x = sin(float(glfwGetTime()) * 3.0f);
	lightColor.y = sin(float(glfwGetTime()) * 1.7f);
	lightColor.z = sin(float(glfwGetTime()) * 2.3f);

	vec3 diffuseColor = lightColor   * glm::vec3(0.75f); // Decrease the influence
	vec3 ambientColor = diffuseColor * glm::vec3(0.25f); // Low influence

	m_lightSpot.SetAmbient(ambientColor);
	m_lightSpot.SetDiffuse(diffuseColor);

	glUniform3f(glGetUniformLocation(m_terrainShader.GetProgram(), "_spotLight.ambeint"),
		m_lightSpot.GetAmbient().x, m_lightSpot.GetAmbient().y, m_lightSpot.GetAmbient().z);

	glUniform3f(glGetUniformLocation(m_terrainShader.GetProgram(), "_spotLight.diffuse"),
		m_lightSpot.GetDiffuse().x, m_lightSpot.GetDiffuse().y, m_lightSpot.GetDiffuse().z);

	// Load Identity Matrix
	m_model = mat4();

	// Translate to origin
	m_model = glm::translate(m_model, vec3(0.0f, -10.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(m_terrainShader.GetProgram(), "_modelMat"), 1, GL_FALSE, glm::value_ptr(m_model));

	// Calculate Model View Projection Matrix mvp => projetction * view * model
	m_mvp = m_projection * m_view * m_model;
	glUniformMatrix4fv(glGetUniformLocation(m_terrainShader.GetProgram(), "_mvpMat"), 1, GL_FALSE, glm::value_ptr(m_mvp));

	// Calculate Normal Matrix for models Normal => mat3(transpose(inverse(model)))
	m_normal = glm::inverse(m_model);
	m_normal = glm::transpose(m_normal);
	glUniformMatrix4fv(glGetUniformLocation(m_terrainShader.GetProgram(), "_normalMat"), 1, GL_FALSE, glm::value_ptr(m_normal));
	m_terrain.Render(m_terrainShader);

	glUseProgram(0);
}

void Game::RenderExplodingTeapot()
{
	m_explodeShader.Use();

	glUniform1f(glGetUniformLocation(m_explodeShader.GetProgram(), "_explodeBias"), m_explodeDelta);

	vec3 lightColor;
	lightColor.x = sin(float(glfwGetTime()) * 2.0f);
	lightColor.y = sin(float(glfwGetTime()) * 0.7f);
	lightColor.z = sin(float(glfwGetTime()) * 1.3f);

	vec3 diffuseColor = lightColor   * glm::vec3(0.5f); // Decrease the influence
	vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // Low influence

	m_lightPoints[0].SetAmbient(ambientColor);
	m_lightPoints[0].SetDiffuse(diffuseColor);

	glUniform3f(glGetUniformLocation(m_explodeShader.GetProgram(), "_pointLights[0].ambient"),
		m_lightPoints[0].GetAmbient().x, m_lightPoints[0].GetAmbient().y, m_lightPoints[0].GetAmbient().z);

	glUniform3f(glGetUniformLocation(m_explodeShader.GetProgram(), "_pointLights[0].diffuse"),
		m_lightPoints[0].GetDiffuse().x, m_lightPoints[0].GetDiffuse().y, m_lightPoints[0].GetDiffuse().z);

	// Load Identity Matrix
	m_model = mat4();

	// Translate to origin
	m_model = glm::translate(m_model, vec3(0.0f, 0.5f, 0.0f));
	m_model = glm::scale(m_model, vec3(2.0f, 2.0f, 2.0f));
	m_model = glm::rotate(m_model, m_angle, vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(m_explodeShader.GetProgram(), "_modelMat"), 1, GL_FALSE, glm::value_ptr(m_model));

	// Calculate Model View Projection Matrix mvp => projetction * view * model
	m_mvp = m_projection * m_view * m_model;
	glUniformMatrix4fv(glGetUniformLocation(m_explodeShader.GetProgram(), "_mvpMat"), 1, GL_FALSE, glm::value_ptr(m_mvp));

	// Calculate Normal Matrix for models Normal => mat3(transpose(inverse(model)))
	m_normal = glm::inverse(m_model);
	m_normal = glm::transpose(m_normal);
	glUniformMatrix4fv(glGetUniformLocation(m_explodeShader.GetProgram(), "_normalMat"), 1, GL_FALSE, glm::value_ptr(m_normal));
	m_teapot.Render(m_explodeShader);

	glUseProgram(0);
}

void Game::SetLightUniforms(Shader shader)
{
	string pointLightUniformName = "";

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_directionalLight.direction"),
		m_lightDirectional.GetDirection().x, m_lightDirectional.GetDirection().y, m_lightDirectional.GetDirection().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_directionalLight.ambient"),
		m_lightDirectional.GetAmbient().x, m_lightDirectional.GetAmbient().y, m_lightDirectional.GetAmbient().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_directionalLight.diffuse"),
		m_lightDirectional.GetDiffuse().x, m_lightDirectional.GetDiffuse().y, m_lightDirectional.GetDiffuse().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_directionalLight.specular"),
		m_lightDirectional.GetSpecular().x, m_lightDirectional.GetSpecular().y, m_lightDirectional.GetSpecular().z);

	for (unsigned int i = 0; i < k_N_POINT_LIGHTS; ++i)
	{
		pointLightUniformName = "_pointLights[" + std::to_string(i) + "]";

		glUniform3f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".position").c_str()),
			m_lightPoints[i].GetPosition().x, m_lightPoints[i].GetPosition().y, m_lightPoints[i].GetPosition().z);

		glUniform3f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".ambient").c_str()),
			m_lightPoints[i].GetAmbient().x, m_lightPoints[i].GetAmbient().y, m_lightPoints[i].GetAmbient().z);

		glUniform3f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".diffuse").c_str()),
			m_lightPoints[i].GetDiffuse().x, m_lightPoints[i].GetDiffuse().y, m_lightPoints[i].GetDiffuse().z);

		glUniform3f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".specular").c_str()),
			m_lightPoints[i].GetSpecular().x, m_lightPoints[i].GetSpecular().y, m_lightPoints[i].GetSpecular().z);

		glUniform1f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".constant").c_str()),
			m_lightPoints[i].GetConstant());

		glUniform1f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".linear").c_str()),
			m_lightPoints[i].GetLinear());

		glUniform1f(glGetUniformLocation(shader.GetProgram(), (pointLightUniformName + ".quadratic").c_str()),
			m_lightPoints[i].GetQuadratic());
	}

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_spotLight.position"),
		m_lightSpot.GetPosition().x, m_lightSpot.GetPosition().y, m_lightSpot.GetPosition().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_spotLight.direction"),
		m_lightSpot.GetDirection().x, m_lightSpot.GetDirection().y, m_lightSpot.GetDirection().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_spotLight.ambeint"),
		m_lightSpot.GetAmbient().x, m_lightSpot.GetAmbient().y, m_lightSpot.GetAmbient().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_spotLight.diffuse"),
		m_lightSpot.GetDiffuse().x, m_lightSpot.GetDiffuse().y, m_lightSpot.GetDiffuse().z);

	glUniform3f(glGetUniformLocation(shader.GetProgram(), "_spotLight.specular"),
		m_lightSpot.GetSpecular().x, m_lightSpot.GetSpecular().y, m_lightSpot.GetSpecular().z);

	glUniform1f(glGetUniformLocation(shader.GetProgram(), "_spotLight.constant"),
		m_lightSpot.GetConstant());

	glUniform1f(glGetUniformLocation(shader.GetProgram(), "_spotLight.linear"),
		m_lightSpot.GetLinear());

	glUniform1f(glGetUniformLocation(shader.GetProgram(), "_spotLight.quadratic"),
		m_lightSpot.GetQuadratic());

	glUniform1f(glGetUniformLocation(shader.GetProgram(), "_spotLight.cutOff"),
		m_lightSpot.GetCutoff());

	glUniform1f(glGetUniformLocation(shader.GetProgram(), "_spotLight.outerCutOff"),
		m_lightSpot.GetOuterCutoff());
}

// Handle keyboard callbacks
void Game::HandleKeyboardCallblack(GLFWwindow * window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
		m_mainCamera.m_position += 0.25f * m_mainCamera.m_fowardDirection;
	if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
		m_mainCamera.m_position -= 0.25f * m_mainCamera.m_fowardDirection;
	if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)
		m_mainCamera.m_position -= 0.25f * m_mainCamera.GetRight();
	if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT)
		m_mainCamera.m_position += 0.25f * m_mainCamera.GetRight();

	if (key == GLFW_KEY_Z)
	{
		if (m_explodeDelta < 3.5f)
			m_explodeDelta += 0.01f;
	}

	if (key == GLFW_KEY_X)
	{
		if(m_explodeDelta > 0.0f)
			m_explodeDelta -= 0.01f;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			m_keys[key] = true;
		else if (action == GLFW_RELEASE)
			m_keys[key] = false;
	}
}

void Game::HandleMouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (m_firstMouse)
	{
		m_lastXPos = float(xpos);
		m_lastYPos = float(ypos);
		
		m_firstMouse = false;
	}

	GLfloat xoffset = float(xpos) - m_lastXPos;
	GLfloat yoffset = m_lastYPos - float(ypos);  // Reversed since y-coordinates go from bottom to left

	m_lastXPos = float(xpos);
	m_lastYPos = float(ypos);

	m_mainCamera.ProcessMouseMovement(xoffset, yoffset);
}