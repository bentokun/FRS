#include "Game.h"

namespace FRS {

	void Game::DisableConsoleCallback() {
		DestroyDebugReportCallbackEXT(instance, reporter, nullptr);
	}

	
	FRS_STATE Game::EnabledConsoleCallback(PFN_vkDebugReportCallbackEXT func) {

		FRS_STATE state = FRS::vkCheckValidation();

		if (state != SUCCESS) {
			if (FRS::vkEnabledConsoleReport(instance,
				func, &reporter)) {
				return SUCCESS;
			}
			return FAILURE;
		}

		else {
			FRS_MESSAGE("Can't enable console callback");
			return FAILURE;
		}
	}


	Game::Game(std::string appName, bool debugMessageEnabled, 
		FRSWindowState state, 
		int windowWidth,
		int windowHeight) :

		EnableValidation(debugMessageEnabled),
		windowWidth(windowWidth),
		windowHeight(windowHeight){
		
		quit = false;
		CreateInstance(appName, EnableValidation, &instance);
		this->state = state;

	}

	void Game::Run() {

		Start();

		uint32_t frame = 0;

		while (!quit) {

			PollEvents();
			AdjustCurrentThreadFPS(60.0f);
			Update();

			//printf("Time: %f \n", (duration + sleep_time).count());
		}

		CleanUp();

	};

	struct Vertex {
		FRSML::vec2 pos;
		FRSML::vec3 color;
		FRSML::vec2 normal;
	};

	const std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
		{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
		{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	void Game::InputHandler(int code, FRSKeyState state) {
		if (code == VK_ESCAPE) {
			quit = true;
		}
	}

	//Set Data should only be used once.
	void Game::Start() {

		VkFormat format;
		int width, height, mipLevel;
		char message[256];

		CreateVulkanWindow(&window, instance,
			"VulkanTest (No Texture)", 
			windowWidth, 
			windowHeight,
			0, 0, state);

		//Meaningless, the allocation thing is still seperate 
		using namespace std::placeholders;
		//lambda is not my friend, but will try latez
		window.SetResizeCallback(std::bind(&Game::ResizingHandler, this, _1, _2));
		window.SetKeyboardCallback(std::bind(&Game::InputHandler, this, _1, _2));

		if (EnableValidation)
			EnabledConsoleCallback(debugCallbackFunc);
		CreateDevice(&device, instance, window, true);
		CreateDeviceAllocator(&allocator, device, 1 << 20);

		FRSImageComponent components[4] = { VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };

		CreateContentManager(&manager, device, &allocator);
		Load();

		CreateSwapchain(&swapChain, device, window, components, false);

		shader.VertexInput.VertexBindings[0].InputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		shader.VertexInput.VertexBindings[0].Stride = sizeof(Vertex);
		shader.VertexInput.BindingSize[0] = sizeof(vertices[0]) * vertices.size();
		shader.VertexInput.BindingDatas[0] = (void*)vertices.data();

		shader.VertexInput.VertexBindings[0].Location[0].Format = VK_FORMAT_R32G32_SFLOAT;
		shader.VertexInput.VertexBindings[0].Location[0].Offset = offsetof(Vertex, pos);
		shader.VertexInput.VertexBindings[0].Location[1].Offset = offsetof(Vertex, color);
		shader.VertexInput.VertexBindings[0].Location[1].Format = VK_FORMAT_R32G32B32_SFLOAT;
		shader.VertexInput.VertexBindings[0].Location[2].Offset = offsetof(Vertex, normal);
		shader.VertexInput.VertexBindings[0].Location[2].Format = VK_FORMAT_R32G32_SFLOAT;

		shader.IndexInput.IndexDatas[0] = (void*)(indices.data());
		shader.IndexInput.IndexSize[0] = sizeof(indices[0]) * indices.size();

		shader.UniformSets[0].UniformBindings[0].dataArrayLength = 1;
		shader.UniformSets[0].UniformBindings[1].dataArrayLength = 1;
		shader.UniformSets[0].UniformBindings[0].stage = VERTEX_SHADER_STAGE;
		shader.UniformSets[0].UniformBindings[1].stage = VERTEX_SHADER_STAGE;

		shader.UniformSets[0].UniformBindings[2].dataArrayLength = 3;
		shader.UniformSets[0].UniformBindings[2].stage = FRAGMENT_SHADER_STAGE;
		shader.UniformSets[0].UniformBindings[2].Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		shader.UniformSets[0].BindingDatas[2] = &tex;

		shader.UniformSets[0].BindingSize[0] = sizeof(UniformBufferObject);
		shader.UniformSets[0].BindingSize[1] = sizeof(Color);
		
		shader.UniformSets[0].UniformBindings[0].Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		shader.UniformSets[0].UniformBindings[1].Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		
		shader.UniformSets[0].UniformBindings[0].Range[0] = sizeof(UniformBufferObject);
		shader.UniformSets[0].UniformBindings[0].OffSet[0] = 0;
		shader.UniformSets[0].UniformBindings[0].Size[0] = sizeof(UniformBufferObject);

		shader.UniformSets[0].UniformBindings[1].Range[0] = sizeof(Color);
		shader.UniformSets[0].UniformBindings[1].OffSet[0] = 0;
		shader.UniformSets[0].UniformBindings[1].Size[0] = sizeof(Color);

		CreateGraphicPipeline(&graphPipeline, device, &allocator,
			swapChain, shader);

		CreateCommander(&commander, swapChain, graphPipeline, device,
			&allocator);

		commander.ReadDrawingCommand(std::bind(&Game::Draw, this));


	}

	void Game::Recreate() {

		FRSImageComponent components[4] = { VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };

		RecreateSwapchain(&swapChain, window, components);
		vkDeviceWaitIdle(device.logicalDevice);

		CreateGraphicPipeline(&graphPipeline, device,
			&allocator, swapChain, shader);
		CreateCommander(&commander, swapChain, graphPipeline, device,
			&allocator);

		commander.ReadDrawingCommand(std::bind(&Game::Draw, this));
	
	}

	void Game::ResizingHandler(int width, int height) {
		Recreate();
	}

	void Game::Load() {
		shader = manager.Load("vert.spv", "frag.spv");
		tex = manager.Load("teacher_test.dds");
	}

	void Game::Unload() {
		manager.Unload(shader);
	}


	void Game::Update() {

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
	
		ubo.model = FRSML::Rotate(FRSML::Identity, time * FRSML::ToRadians(90.0f), FRSML::vec3(0, 0, 1));
		ubo.view = FRSML::LookAt(FRSML::vec3(2.0f, 2.0f, 2.0f), FRSML::vec3(0.0f, 0.0f, 0.0f),
			FRSML::vec3(0,0,1));
		ubo.proj = FRSML::CreatePerspectiveMatrix(FRSML::ToRadians(45.0f),
			FRSML::vec2(window.GetWindowExtent().width, window.GetWindowExtent().height), 0.1, 10);

	    ubo.proj[1][1] *= -1;

		color.sinTime = 1 - FRSML::Sin(time);

		shader.UniformSets[0].BindingDatas[0] = (void*)(&ubo);
		shader.UniformSets[0].BindingDatas[1] = (void*)(&color);

		commander.UpdateData(shader);

		controller.GetState();

		if (controller.buttonB) {
			std::cout << "Button B pressed" << std::endl;
			controller.Vibrate(32000, 32000);
		}

		commander.Submit();
		commander.Render();
		commander.Wait();

		if (window.ShouldQuit) {
			std::cout << "Quit" << std::endl;
			quit = true;
		}
		
		

	}

	void Game::Draw() {

		commander.Clear(1, 1, 1, 1);

		VkDeviceSize offset[1] = { 0 };

		commander.Start();
			commander.BindVertexBuffers(0, 1, &graphPipeline.GetStaticBuffer()[0], offset);
			commander.BindIndexBuffers(graphPipeline.GetIndexBuffer()[0], 0, VK_INDEX_TYPE_UINT16);
			commander.BindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr);
			commander.DrawIndexed(indices.size(), 1, 0, 0, 0);
		commander.End();

	}

	void Game::CleanUp() {


		Unload();

		vkDeviceWaitIdle(device.logicalDevice);
		
		DestroyDeviceAllocator(allocator);
		DestroyCommander(&commander);
		DestroyGraphicPipeline(&graphPipeline);
		DestroySwapchain(swapChain);
		DestroyWindow(window);
		DestroyDevice(device);

		vkDestroyInstance(instance, nullptr);
		

	}

	Game::~Game() {}


}
