#include <GLAD/glad.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <vector>

#include "Player.h"
#include "ShaderProgram.h"


void Player::Setup(int vertices, float positionAttribute[], const char* vrtxShaderPath, const char* frgmtShaderPath, float radius) {
	circleRadius = radius;
	numOfVertices = vertices;
	playerPosition.y = -0.4f;
	highestPoint = 1.0f - circleRadius;
	lowestPoint = groundUpperline + circleRadius;


	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);

	unsigned int positionVBO;
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * sizeof(float), positionAttribute, GL_DYNAMIC_DRAW); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	shaderProgram.Setup(vrtxShaderPath, frgmtShaderPath);

	glBindVertexArray(0);
}

void Player::Draw(float deltaTime, int numOfPlatforms, std::vector<Platform>& platforms) {
	glBindVertexArray(vaoId);
	shaderProgram.activate();

	playerPosition.x = glm::max(-1.0f + circleRadius, glm::min(1.0f - circleRadius, playerPosition.x));
	playerPosition.y = glm::min(highestPoint, glm::max(playerPosition.y + velocityY, lowestPoint));

	onGround = false;
	if (playerPosition.y == lowestPoint) onGround = true;

	if (speedup > 0.1f) {
		timer += deltaTime;
		shaderProgram.setBoolUniform("hyper", true);
		if (timer > hyperTime) tired = true, speedup = 0.0f, timer = 0.0f;
	}
	else {
		shaderProgram.setBoolUniform("hyper", false);
	}

	if (tired) {
		timer += deltaTime;
		if (timer > cooldownTime) tired = false, timer = 0.0f;
	}

	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, playerPosition);
	shaderProgram.setMat4Uniform("modelMat", modelMat);

	for (int i = 0; i < numOfPlatforms; i++)
		if (DetectCollision(platforms[i])) Collide(platforms[i]);

	glDrawArrays(GL_TRIANGLE_FAN, 0, numOfVertices);


	if (onGround)                              velocityY = 0.0f;
	else if (playerPosition.y == highestPoint) velocityY = glm::min(0.0f, velocityY - gravity * deltaTime);
	else                                       velocityY -= gravity * deltaTime;


	shaderProgram.deactivate();
	glBindVertexArray(0);
}


void Player::Move(Player_Movement key, float deltaTime) {
	if (key == UP && onGround) velocityY = kickoff + (speedup / 2000);

	if (key == RIGHT) playerPosition.x += (velocityX + speedup) * deltaTime; 
	if (key == LEFT)  playerPosition.x -= (velocityX + speedup) * deltaTime; 
}


void Player::GetHyper() { if (!tired) speedup = 0.5f; }
void Player::BeNormal() { speedup = 0.0f; timer = 0.0f; }


void Player::DeleteVAO() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &vaoId);
}




// Private Functions:

bool Player::DetectCollision(Platform platform) {
	closestX = glm::max(platform.leftSide, glm::min(playerPosition.x, platform.rightSide));
	closestY = glm::max(platform.lowerSide, glm::min(playerPosition.y, platform.upperSide));

	distanceX = playerPosition.x - closestX;
	distanceY = playerPosition.y - closestY;
	float distanceSquare = (distanceX * distanceX) + (distanceY * distanceY);


	return (distanceSquare < (circleRadius * circleRadius));
}


void Player::Collide(Platform platform) {
	float radiusSquare = circleRadius * circleRadius;

	if (closestY == platform.upperSide && glm::abs(distanceX) < (0.5f * circleRadius)) { 

		if (closestX == platform.leftSide) { 
			float firstTerm = (playerPosition.x - platform.leftSide) * (playerPosition.x - platform.leftSide);
			playerPosition.y = glm::sqrt(radiusSquare - firstTerm) + platform.upperSide;

		}
		else if (closestX == platform.rightSide) { 
			float firstTerm = (playerPosition.x - platform.rightSide) * (playerPosition.x - platform.rightSide);
			playerPosition.y = glm::sqrt(radiusSquare - firstTerm) + platform.upperSide;

		}
		else {
			playerPosition.y = glm::max(playerPosition.y, platform.upperSide + circleRadius);
		}

		onGround = true;


	}
	else if (closestY == platform.lowerSide) { 
		velocityY = glm::min(velocityY, 0.0f);

		if (closestX == platform.leftSide) { 
			float firstTerm = (playerPosition.x - platform.leftSide) * (playerPosition.x - platform.leftSide);
			playerPosition.y = -glm::sqrt(radiusSquare - firstTerm) + platform.lowerSide;

		}
		else if (closestX == platform.rightSide) { 
			float firstTerm = (playerPosition.x - platform.rightSide) * (playerPosition.x - platform.rightSide);
			playerPosition.y = -glm::sqrt(radiusSquare - firstTerm) + platform.lowerSide;

		}
		else {
			playerPosition.y = glm::min(playerPosition.y, platform.lowerSide - circleRadius);
		}


	}
	else if (closestX == platform.leftSide) { 
		playerPosition.x = platform.leftSide - circleRadius;

	}
	else if (closestX == platform.rightSide) { 
		playerPosition.x = platform.rightSide + circleRadius;

	}
}