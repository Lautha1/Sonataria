#include "OpenGLShader.h"

#include <string>
#include "Logger.h"
#include <sstream>
using namespace std;

OpenGLShader::OpenGLShader(int vShaderResID, int fShaderResID) {
	this->shaderInitialized = false;
	this->vertShader = 0;
	this->fragShader = 0;
	this->shaderProgram = 0;

	this->vShaderResID = vShaderResID;
	this->fShaderResID = fShaderResID;
}

OpenGLShader::~OpenGLShader() {
	glDeleteShader(this->vertShader);
	glDeleteShader(this->fragShader);
	glDeleteProgram(this->shaderProgram);
}

bool OpenGLShader::initShader() {
	// Vertex Shader
	char* vertex_shader = LoadShader(vShaderResID);
	if (vertex_shader == nullptr) {
		logger.logError(L"Error Loading Vertex Shader");
		return false;
	}
	logger.log(L"Vertex shader code loaded");

	// Fragment Shader
	char* fragment_shader = LoadShader(fShaderResID);
	if (fragment_shader == nullptr) {
		logger.logError(L"Error Loading Fragment Shader");
		return false;
	}
	logger.log(L"Fragment shader code loaded");

	// Compile Shaders
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertex_shader, NULL);
	glCompileShader(vertShader);
	checkShaderLog(vertShader);

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragment_shader, NULL);
	glCompileShader(fragShader);
	checkShaderLog(fragShader);

	// Prepare to link the compiled shaders into a program
	this->shaderProgram = glCreateProgram();
	glAttachShader(this->shaderProgram, fragShader);
	glAttachShader(this->shaderProgram, vertShader);

	// Bind attribute index values (must happen before linking)
	this->initAttributes();

	// Link the shaders into a usable program
	glLinkProgram(this->shaderProgram);
	checkProgramLog(this->shaderProgram);
	glUseProgram(this->shaderProgram);

	// Free the shader resources that were malloc'd
	free(vertex_shader);
	free(fragment_shader);

	// Setup shader uniform variables
	this->initUniforms();

	// Finish and indicate success
	this->shaderInitialized = true;
	return true;
}

void OpenGLShader::checkShaderLog(GLuint shader)
{
	GLsizei logLength;
	GLchar infoLog[1024];
	glGetShaderInfoLog(shader, 1024, &logLength, infoLog);
	wstringstream outputBuilder;
	outputBuilder << L" > Shader Log  " << (const char*)infoLog;
	logger.log(outputBuilder.str());
}

void OpenGLShader::checkProgramLog(GLuint shader)
{
	GLsizei logLength;
	GLchar infoLog[1024];
	glGetProgramInfoLog(shader, 1024, &logLength, infoLog);
	wstringstream outputBuilder;
	outputBuilder << L" > Program Log  " << (const char*)infoLog;
	logger.log(outputBuilder.str());
}

// Utility function to log the most recent WIN32 error
void logLastWin32Error() {
	LPTSTR lpMsgBuf = nullptr;
	DWORD lastError = GetLastError();
	if (lastError != 0) {
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			lpMsgBuf,
			0, NULL
		);
	}

	if (lpMsgBuf != 0) {
		logger.logError(lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
}

/**
 * Load in a resource saved as a Win32 Resource.
 *
 * @param name ID to the resource
 * @param size DWORD nullptr
 * @param data char* nullptr to save the returned data to
 * @param type string as resource type
 * @return true if successfully loaded
 */
bool OpenGLShader::LoadShaderInResource(int name, DWORD& size, const char*& data, const char* type) // *& is passing the pointer by reference and not by val.
{
	HMODULE handle = ::GetModuleHandleA(NULL);
	HRSRC rc = ::FindResourceA(handle, MAKEINTRESOURCEA(name), type);
	if (rc == 0) {
		logger.logError(L"Failed to find resource");
		logLastWin32Error();
		return false;
	}

	HGLOBAL rcData = ::LoadResource(handle, rc);
	if (rcData == 0) {
		logger.logError(L"Failed to load resource");
		logLastWin32Error();
		return false;
	}

	size = ::SizeofResource(handle, rc);
	data = static_cast<const char*>(::LockResource(rcData));
	return true;
}

/**
 * Load a shader with a given resouce id.
 *
 * @param id of the shader resource to load
 * @return the char* of data of the shader
 */
char* OpenGLShader::LoadShader(int id) {
	DWORD vshader_size;
	const char* vertex_shader_ptr = nullptr;
	char* vertex_shader = nullptr;
	if (LoadShaderInResource(id, vshader_size, vertex_shader_ptr, "GLSLShader")) {
		size_t size = vshader_size + 1;
		vertex_shader = (char*)malloc(size);
		memcpy_s((void*)vertex_shader, vshader_size, vertex_shader_ptr, vshader_size);
		vertex_shader[vshader_size] = '\0';

		return vertex_shader;
	}
	else
	{
		return nullptr;
	}
}
