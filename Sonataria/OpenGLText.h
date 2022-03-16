#pragma once
#include "QuadSprite.h"
#include "OpenGLFont.h"
#include <string>

enum ALIGNMENT {
	LEFT,
	CENTERED,
	RIGHT
};

class OpenGLText : public QuadSprite {
	public:
		OpenGLText(const std::wstring& newName, const OpenGLFont& font);
		virtual void render(PROJECTION projection) const override;

		void render(PROJECTION projection, const std::string& text, ALIGNMENT align = LEFT,
			GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f,
			GLfloat pack = 1.0f, GLfloat scale = 1.5f);
		void render(PROJECTION projection, const std::wstring& text, ALIGNMENT align = LEFT,
			GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f,
			GLfloat pack = 1.0f, GLfloat scale = 1.5f);

		float computeTextWidth(const std::string& text, GLfloat pack = 1.0f) const;
		float computeTextWidth(const std::wstring& text, GLfloat pack = 1.0f) const;

	protected:
		const OpenGLFont& font;
};
