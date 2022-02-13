#include "Camera.h"

/***********************************************
 *	�������:			UpdateViewMatrix()
 *	����������:			���������� ������� ����
 *	�������� ��������:	���
 *	��������� ��������:	���
 **********************************************/
void Camera::UpdateViewMatrix()
{
	mat4 rotM = mat4(1.0f);
	mat4 transM;

	rotM = rotate(rotM, radians(rotation.x * (flipY ? -1.0f : 1.0f)), vec3(1.0f, 0.0f, 0.0f));
	rotM = rotate(rotM, radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
	rotM = rotate(rotM, radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));

	vec3 translation = position;
	if (flipY)
		translation.y *= -1.0f;

	transM = translate(mat4(1.0f), translation);
	
	matrices.view = type == firstperson ? rotM * transM : transM * rotM;

	viewPos = vec4(position, 0.0f) * vec4(-1.0f, 1.0f, -1.0f, 1.0f);

	updated = true;
}

bool Camera::moving()
{
	return false;
}

/***********************************************
 *	�������:			SetPosition()
 *	����������:			������� ������� ������
 *	�������� ��������:	position - ������� ������
 *	��������� ��������:	���
 **********************************************/
void Camera::SetPosition(vec3 position)
{
	this->position = position;
	UpdateViewMatrix();
}

/***********************************************
 *	�������:			SetRotation()
 *	����������:			������� �������� ������
 *	�������� ��������:	rotation - �������� ������
 *	��������� ��������:	���
 **********************************************/
void Camera::SetRotation(vec3 rotation)
{
	this->rotation = rotation;
	UpdateViewMatrix();
}

/***********************************************
 *	�������:			SetPerspective()
 *	����������:			������� ������� ��������
 *	�������� ��������:	fov - ���� ������
 *						aspect - ����������� ������ � ������ ������� ���������
 *						znear - ������� ������� ������� ���������
 *						zfar - ������� ������� ������� ���������
 *	��������� ��������:	���
 **********************************************/
void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
	this->fov = fov;
	this->znear = znear;
	this->zfar = zfar;

	matrices.perspective = perspective(radians(fov), aspect, znear, zfar);
	
	if (flipY)
		matrices.perspective[1][1] *= -1.0f;
}

/***********************************************
 *	�������:			SetTranslation()
 *	����������:			������� ������� ��������
 *	�������� ��������:	translation - ������� ��������
 *	��������� ��������:	���
 **********************************************/
void Camera::SetTranslation(glm::vec3 translation)
{
	this->position = translation;
	UpdateViewMatrix();
}

void Camera::Update(float deltaTime)
{
	updated = false;
	if (type == CameraType::firstperson)
	{
		if (moving())
		{
			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			camFront.y = sin(glm::radians(rotation.x));
			camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			camFront = glm::normalize(camFront);

			float moveSpeed = deltaTime * movementSpeed;

			if (keys.up)
				position += camFront * moveSpeed;
			if (keys.down)
				position -= camFront * moveSpeed;
			if (keys.left)
				position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
			if (keys.right)
				position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

			UpdateViewMatrix();
		}
	}
}

/***********************************************
 *	�������:			Rotate()
 *	����������:			�������� ������ �� �������� ������
 *	�������� ��������:	delta - �������� ������
 *	��������� ��������:	���
 **********************************************/
void Camera::Rotate(vec3 delta)
{
	this->rotation += delta;
	UpdateViewMatrix();
}

/***********************************************
 *	�������:			Translate()
 *	����������:			������� ������ �� �������� ������
 *	�������� ��������:	delta - �������� ������
 *	��������� ��������:	���
 **********************************************/
void Camera::Translate(vec3 delta)
{
	this->position += delta;
	UpdateViewMatrix();
}

/***********************************************
 *	�������:			UpdateAspectRatio()
 *	����������:			������� ���������� ���������
 *						������ � ������
 *	�������� ��������:	aspect - ��������� ������
 *	��������� ��������:	���
 **********************************************/
void Camera::UpdateAspectRatio(float aspect)
{
	matrices.perspective = perspective(radians(fov), aspect, znear, zfar);
	if (flipY) 
		matrices.perspective[1][1] *= -1.0f;
}
