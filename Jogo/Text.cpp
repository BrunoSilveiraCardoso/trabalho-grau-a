#include <iostream>

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Text.h"
#include "ShaderProgram.h"



Text::Text(unsigned int width, unsigned int height, const char* vrtxShaderPath, const char* frgmtShaderPath, unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) {
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shaderProgramId.Setup(vrtxShaderPath, frgmtShaderPath);
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	shaderProgramId.activate();
	shaderProgramId.setMat4Uniform("projection", projection);


	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		std::cin.get();
		return;
	}

	FT_Face face;
	if (FT_New_Face(ft, "Resources/fonts/arial.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		std::cin.get();
		return;
	}

	FT_Set_Pixel_Sizes(face, width, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	for (GLubyte c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}

		GLuint textureId;
		GenerateCharTexture(face, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		AddToMap(face, textureId, c);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);

	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}




void Text::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	shaderProgramId.activate();
	glBindVertexArray(vaoId);

	shaderProgramId.setVec3Uniform("textColor", color.x, color.y, color.z);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = CharactersMap[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		GLfloat vertices[6][4] = {
		{xpos,     ypos + h, 0.0, 0.0},
		{xpos,     ypos,     0.0, 1.0},
		{xpos + w, ypos,     1.0, 1.0},

		{xpos,     ypos + h, 0.0, 0.0},
		{xpos + w, ypos,     1.0, 1.0},
		{xpos + w, ypos + h, 1.0, 0.0}
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureId);

		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale; 
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}




void Text::AddToMap(FT_Face face, GLuint textureId, char c) {
	Character character = {
		textureId,
		glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		(GLuint)(face->glyph->advance.x)
	};
	CharactersMap.insert(std::pair<GLchar, Character>(c, character));
}

void Text::GenerateCharTexture(FT_Face face, GLuint& textureId) {
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);
}
