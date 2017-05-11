#pragma once

#ifdef _WIN32

#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif

#endif

#include "Vulkan\ContentManager.h"
#include "Vulkan\VkExtensions.h"

class TFSAPI Behavior {
public:
	Behavior() {};

	virtual void Start() {};
	virtual void CleanUp() {};
	virtual void Update() {};
	virtual void Draw() {};
	virtual void Load() {};
	virtual void Unload() {};

};

class TFSAPI GameBehavior : Behavior {
public:

	GameBehavior() :
		Behavior() {};

private:
	FRS::ContentManager contentManager;
	
};