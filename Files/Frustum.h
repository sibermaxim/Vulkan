#pragma once
#include <array>
#include <glm/glm.hpp>

namespace vks
{
	class Frustum
	{
	public:

		enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5};
		std::array<glm::vec4, 6> planes;

		void Update(glm::mat4 matrix);
		bool CheckSphere(glm::vec3 pos, float radius);
	};
}
