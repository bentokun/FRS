- Attrib command: (Eg: VulkanTest.exe -w -s1920x1080)
 	+ -f: fullscreen.
 	+ -s1920x1080: change size. Default size is 1920x1080. You can replace 1920 and 1080 by your width and height.
	+ -wf: windowed with screen size.
	+ -w: windowed mode.

- Require dll:
	+ FRSML.dll: Math.
	+ FRS.dll: Vulkan library.
	
- Require resources:
	+ vert.spv: SPIR-V Vertex Shader.
	+ frag.spv: SPIR-V Fragment Shader.
	+ teacher-test.dds: Teacher Kawakami from Persona 5

- Debugging material:
	+ log.rdc: RenderDoc file for viewing what happens (helpful with figure out what the fuck is wrong with my texture)