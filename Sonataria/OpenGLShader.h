#pragma once
#include <windows.h>
#include <GL/glew.h>

class OpenGLShader
{
public:
	OpenGLShader(int vShaderResID, int fShaderResID);
	~OpenGLShader();

	bool initShader();

	bool isValid() const { return shaderInitialized;  }
	GLuint getProgram() const { return shaderProgram;  }

protected:
	bool shaderInitialized;
	int vShaderResID, fShaderResID;
	GLuint vertShader, fragShader, shaderProgram;

	// Override in children
	virtual void initAttributes() = 0;
	virtual void initUniforms() = 0;

	// General Shader helper functions
	static void checkShaderLog(GLuint shader);
	static void checkProgramLog(GLuint shader);
	static char* LoadShader(int id);
	static bool LoadShaderInResource(int name, DWORD & size, const char*& data, const char* type);
};
