#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Camera
{
	float fov;
	float znear, zfar;

	void UpdateViewMatrix();		//обновление матрицы вида

public:

	enum CameraType{lookat, firstperson};
	CameraType type = lookat;

	vec3 rotation = vec3();
	vec3 position = vec3();
	vec4 viewPos = vec4();

	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;
	
	bool updated = false;			//флаг обновления матриц камеры
	bool flipY = false;
	
	struct 
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	}keys;

	struct
	{
		mat4 perspective;
		mat4 view;
	}matrices;
	
	bool moving();

	void SetPosition(vec3 position);
	void SetRotation(vec3 rotation);
	void SetPerspective(float fov, float aspect, float znear, float zfar);
	void SetTranslation(glm::vec3 translation);
	
	void Update(float deltaTime);

	void Rotate(vec3 delta);
	void Translate(vec3 delta);
	void UpdateAspectRatio(float aspect);
};