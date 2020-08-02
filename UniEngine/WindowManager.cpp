#include "pch.h"
#include "WindowManager.h"
#include "InputManager.h"
#include "RenderTarget.h"
#include "Default.h"
using namespace UniEngine;

std::vector<GLFWmonitor*> WindowManager::_Monitors;
GLFWmonitor* WindowManager::_PrimaryMonitor;
GLFWwindow* WindowManager::_Window;
unsigned WindowManager::_Width;
unsigned WindowManager::_Height;


void WindowManager::ResizeCallback(GLFWwindow* window, int width, int height) {
	_Width = width;
	_Height = height;
}

void UniEngine::WindowManager::SetMonitorCallback(GLFWmonitor* monitor, int event)
{
	if (event == GLFW_CONNECTED)
	{
		// The monitor was connected
		for (auto i : _Monitors) if (i == monitor) return;
		_Monitors.push_back(monitor);
	}
	else if (event == GLFW_DISCONNECTED)
	{
		// The monitor was disconnected
		for (auto i = 0; i < _Monitors.size(); i++) {
			if (monitor == _Monitors[i]) {
				_Monitors.erase(_Monitors.begin() + i);
			}
		}
	}
	_PrimaryMonitor = glfwGetPrimaryMonitor();
}

void UniEngine::WindowManager::Init(unsigned width, unsigned height, std::string name, bool fullScreen)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
	int size;
	auto monitors = glfwGetMonitors(&size);
	for (auto i = 0; i < size; i++) {
		_Monitors.push_back(monitors[i]);
	}
	_PrimaryMonitor = glfwGetPrimaryMonitor();
	glfwSetMonitorCallback(SetMonitorCallback);

	_Width = width;
	_Height = height;
	// glfw window creation
	// --------------------
	_Window = glfwCreateWindow(width, height, name.c_str(), fullScreen ? _PrimaryMonitor : nullptr, NULL);
	glfwSetFramebufferSizeCallback(_Window, WindowManager::ResizeCallback);
	glfwSetCursorPosCallback(_Window, InputManager::CursorPositionCallback);
	glfwSetScrollCallback(_Window, InputManager::MouseScrollCallback);
	glfwSetKeyCallback(_Window, InputManager::KeyCallback);
	glfwSetMouseButtonCallback(_Window, InputManager::MouseButtonCallback);
	if (_Window == NULL)
	{
		Debug::Error("Failed to create GLFW window");
	}
	glfwMakeContextCurrent(_Window);
}

GLFWwindow* UniEngine::WindowManager::GetWindow()
{
	return _Window;
}

GLFWmonitor* UniEngine::WindowManager::PrimaryMonitor()
{
	return _PrimaryMonitor;
}

void UniEngine::WindowManager::Start()
{
	RenderTarget::BindDefault();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void UniEngine::WindowManager::Update()
{
	glfwSwapBuffers(_Window);
}

void UniEngine::WindowManager::DrawTexture(GLTexture2D* texture)
{
	RenderTarget::BindDefault();
	/* Make the window's context current */
	glViewport(0, 0, _Width, _Height);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	/* Render here */
	glDisable(GL_DEPTH_TEST);
	glDrawBuffer(GL_BACK);
	auto program = Default::GLPrograms::ScreenProgram;
	program->Bind();
	program->SetFloat("depth", 0);
	Default::GLPrograms::ScreenVAO->Bind();
	//Default::Textures::UV->Texture()->Bind(GL_TEXTURE_2D);
	texture->Bind(0);
	program->SetInt("screenTexture", 0);
	program->SetFloat2("center", glm::vec2(0));
	program->SetFloat2("size", glm::vec2(1.0));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}


