#include "Game.h"

namespace FRS {

	void Game::CreateGameDevice(Device* device, Window window) {

		*device = FRS::Device(instance, &window, EnableValidation);

	}


	void Game::DisableConsoleCallback() {

		DestroyDebugReportCallbackEXT(instance, reporter, nullptr);

	}

	
	FRS_STATE Game::EnabledConsoleCallback(PFN_vkDebugReportCallbackEXT func) {

		FRS_STATE state = FRS::vkCheckValidation();

		if (state != SUCCESS) {

			//std::cout << "Enable Console!" << std::endl;

			if (FRS::vkEnabledConsoleReport(instance,
				func, &reporter)) {
				return SUCCESS;
			}

			return FAILURE;
		}

		else {

			std::cout << "Cant enable console callback!" <<" "<<state<< std::endl;

			return FAILURE;
		}
			

	}

	FRS_STATE Game::CreateSwapChain(Swapchain* swapChain, 
		Device* device,
		FRSImageComponent rbgacomponent[4], Window* window) {

		*swapChain = Swapchain{ device, window, rbgacomponent };

		return SUCCESS;

	}

	void Game::DestroySwapChain(Device device,
		Swapchain swapChain) {

		DestroyTheSwapchain(swapChain);

	}

	Game::Game(std::string appName, bool debugMessageEnabled) {

		quit = false;

		EnableValidation = debugMessageEnabled;

		VkApplicationInfo appInfo;

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_0;
		appInfo.pNext = NULL;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "FRSEngine";
		appInfo.pApplicationName = appName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		
		VkInstanceCreateInfo instanceInfo;
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.pNext = NULL;
		instanceInfo.flags = 0;

		std::vector<std::string> extensionsName = FRS::vkInstanceExtensions();
		std::vector<const char*> realExtensions;

		for (int i = 0; i < extensionsName.size(); i++) {
			realExtensions.push_back(extensionsName[i].c_str());
		}

		if (EnableValidation) {
			realExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		 
		instanceInfo.enabledExtensionCount = (uint32_t)realExtensions.size();
		
		std::cout << "Total Extensions: " << std::endl;

		for (auto& extensionName : extensionsName) {
			std::cout << "\t"<<"\t"<< extensionName << std::endl;
		}

		std::cout << "\n";

		instanceInfo.ppEnabledExtensionNames = realExtensions.data();

		const std::vector<const char*> layerCheck{
			"VK_LAYER_LUNARG_standard_validation"
		};

		if (EnableValidation) {
			instanceInfo.enabledLayerCount = layerCheck.size();
			instanceInfo.ppEnabledLayerNames = layerCheck.data();
		}
		else {
			instanceInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

		if (result != VK_SUCCESS) {
			std::cout << result << std::endl;
			FRS_FATAL_ERROR("Cant create VK_INSTANCE!");
		}

	}

	void Game::Run() {

		Start();

		do {
			Update();
		} while (!quit);

		CleanUp();
	};

	struct Pos {
		float x, y;
	};

	struct Color {
		float x, y, z;
	};

	struct Vertex {

		FRSML::vec2 pos;
		FRSML::vec3 color;

	};

	const std::vector<Vertex> vertices = {
		{ { 0.0f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } }
	};


	//Set Data should only be used once.
	void Game::Start() {
		
		CreateVulkanWindow(&window, instance, "Ha", 800, 600);
		EnabledConsoleCallback(debugCallbackFunc);
		CreateGameDevice(&device, window);

		FRSImageComponent components[4] = { VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };
		
	 	manager = FRS::ContentManager(device);

		Load();

		swapChain = FRS::Swapchain(&device, &window, components);

		allocator = FRS::DeviceAllocator{ device, 1 << 20 };

		CreateBuffer(buffer1,device,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			sizeof(vertices[0]) * vertices.size(), true ,allocator);
		
		buffer1.Bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(Vertex));

		buffer1.Attribute(0, offsetof(Vertex, pos), VK_FORMAT_R32G32_SFLOAT);
		buffer1.Attribute(1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT);
		

		
		std::vector<Buffer> buffers;

		buffers.push_back(buffer1);

		CreateGraphicPipeline(&graphPipeline, device, swapChain, shader, buffers);

		commander = FRS::Commander(swapChain, graphPipeline, device, allocator);

		commander.SetData(buffer1, 0, sizeof(vertices[0])* vertices.size(), (void*)vertices.data());

		commander.ReadDrawingCommand(std::bind(&Game::Draw, this));

		using namespace std::placeholders;
		//lambda is not my friend, but will try latez
		window.SetResizeCallback(std::bind(&Game::ResizingHandler, this, _1, _2));
		
	};

	void Game::Recreate() {

		vkDeviceWaitIdle(device.logicalDevice);

		FRSImageComponent components[4] = { VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };

		RecreateTheSwapchain(&swapChain, &window, components);

		DestroyGraphicPipeline(graphPipeline);
		DestroyCommander(commander);

		std::vector<Buffer> buffers;
		buffers.push_back(buffer1);

		CreateGraphicPipeline(&graphPipeline, device, swapChain, shader, buffers);
		commander = FRS::Commander(swapChain, graphPipeline, device, allocator);

		commander.ReadDrawingCommand(std::bind(&Game::Draw, this));
	
	}

	void Game::ResizingHandler(int width, int height) {
		Recreate();
	}

	void Game::Load() {
		shader = manager.Load("vert.spv", "frag.spv");
	}

	void Game::Unload() {
		manager.Unload(shader);
	}

	void Game::Update() {

		FRSPollEvents();

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

		Buffer buffers[1] = { buffer1 };
		VkDeviceSize offset[1] = { 0 };

		commander.Start();
			commander.BindVertexBuffers(0, 1, buffers, offset);
			commander.Draw(3, 1, 0, 0);
		commander.End();

	}

	void Game::CleanUp() {


		Unload();

		vkDeviceWaitIdle(device.logicalDevice);
		
		DestroyBuffer(device, &buffer1);
		DestroyCommander(commander);
		DestroyGraphicPipeline(graphPipeline);
		DestroySwapChain(device, swapChain);
		DestroyWindow(window);
		DestroyDevice(device);

		vkDestroyInstance(instance, nullptr);
		

	}

	Game::~Game() {}


}
