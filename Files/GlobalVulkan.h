#pragma once
#include <array>

struct UISettings
{
	bool displayModels = true;
	bool displayLogos = true;
	bool displayBackground = true;
	bool animateLight = false;
	float lightSpeed = 0.25f;
	std::array<float, 50>frameTimes{};
	float frameTimeMin = 999.0f, frameTimeMax = 0.0f;
	float lightTimer = 0.0f;
};

//typedef _UISettings UISettings;

extern UISettings uiSettings;