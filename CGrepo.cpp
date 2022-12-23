#define _USE_MATH_DEFINES
#include <math.h>

#include "GL/freeglut.h"
#include "glm/glm.hpp"


#define WINDOW_WIDTH	(512)
#define WINDOW_HEIGHT	(640)

#define PLAYER_MAX	(4)
#define PLAYER_RADIUS	(24.0f)

#define CIRCLE_EGDE_MAX	(32)


enum {
	PHASE_IDLE,
	PHASE_HOLD,
	PHASE_FIRE,
	PHASE_MONSTER,
	PHASE_OVER,
	PHASE_CLEAR,
	PHASE_MAX,
};

typedef struct {
	const glm::vec4 color;
	glm::vec2 position;
	glm::vec2 velocity;
}PLAYER;

PLAYER players[PLAYER_MAX] = {
	{{1,0,0,1},},
	{{0,0.5,0,1},},
	{{0,1,1,1},},
	{{0.5,0,1,1},},
};
unsigned int frame;
int phase;
int currentPlayer;

glm::vec2 mousePosition;
float firePower;

void DrowShadow(float _radius, glm::vec2 _position, glm::vec4 _color) {
	_position.y += _radius;
	glPushMatrix();
	{
		glTranslatef(_position.x, _position.y, 0);
		glScaled(1, 0.5, 0);
		glBegin(GL_TRIANGLE_FAN);
		{
			glColor4f(0, 0, 0, _color.a * 0.5f);
			glVertex2f(0, 0);
			for (int i = 0; i <= CIRCLE_EGDE_MAX; i++) {
				float theta = float(M_PI * 2 * i / CIRCLE_EGDE_MAX);
				glVertex2f(cosf(theta) * _radius, -sinf(theta) * _radius);

			}
		}
		glEnd();
	}
	glPopMatrix();
}

void DrowBall(float _radius,glm::vec2 _position, glm::vec4 _color) {
	_position.y += sinf(float(frame * M_PI * 2 / (120))) * _radius / 10;
glPushMatrix();
{
	glTranslatef(_position.x, _position.y, 0);

	glBegin(GL_TRIANGLE_FAN);
	{
		glColor4f(1, 1, 1, _color.a);
		glVertex2f(0, 0);
		glColor4fv((const GLfloat*)&_color);
		for (int i = 0; i <= CIRCLE_EGDE_MAX; i++) {
			float theta = float(M_PI * 2 * i / CIRCLE_EGDE_MAX);
			glVertex2f(cosf(theta) * _radius, -sinf(theta) * _radius);

		}
	}
	glEnd();
}
glPopMatrix();
}

void Init(void) {
	frame = 0;
	currentPlayer = 0;
	phase = PHASE_IDLE;

	for (int i = 0; i < PLAYER_MAX; i++)
		players[i].position = { (1 + i) * (WINDOW_WIDTH / float(PLAYER_MAX + 1)),WINDOW_HEIGHT - PLAYER_RADIUS * 3 };
}

void Display(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.5, 0.75, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	for (int i = 0; i < PLAYER_MAX; i++)
		DrowShadow(PLAYER_RADIUS, players[i].position, players[i].color);

	for (int i = 0; i < PLAYER_MAX; i++)
		DrowBall(PLAYER_RADIUS, players[i].position, players[i].color);
	if (phase == PHASE_HOLD) {
		glPushMatrix();
		{
			glTranslatef(players[currentPlayer].position.x, players[currentPlayer].position.y, 0);
			glm::vec2 playerToMouse = mousePosition - players[currentPlayer].position;
			glRotatef(GLfloat(atan2f(playerToMouse.y, playerToMouse.x) * 180 / M_PI), 0, 0, 1);
			glScalef(firePower * 3, firePower, 0);
			glBegin(GL_POLYGON);
			{
				const glm::vec2 v[] = {
					{-1,0},
					{0,1},
					{-0.25,0.5},
					{1,0},
					{-0.25,-0.5},
					{0,-1} };
				for (int i = 0; i < sizeof(v) / sizeof(glm::vec2); i++) {
					glColor4f(1, (1 + v[i].x) / 2, 0, 0.75);
					glVertex2fv((const GLfloat*)&v[i]);
				}
			}
			glEnd();
		}
		glPopMatrix();
	}

	{
		const char* phaseNames[] = {
			"PHASE_IDLE,"
			"PHASE_HOLD",
			"PHASE_FIRE",
			"PHASE_MONSTER",
			"PHASE_OVER",
			"PHASE_CLEAR",
		};
	}

	glutSwapBuffers();
}

void Idle(void) {
	frame++;
	switch (phase) {
	case PHASE_FIRE: {
		for (int i = 0; i < PLAYER_MAX; i++) {
			if (glm::length(players[i].velocity) > 0) {
				{
					glm::vec2 lastVelocity = players[i].velocity;
					players[i].velocity += -glm::normalize(players[i].velocity) * 0.1f;
					if (glm::dot(players[i].velocity, lastVelocity) <= 0)
						players[i].velocity = {};
				}
				glm::vec2 lastPosition = players[i].position;
				players[i].position += players[i].velocity;
				if (players[i].position.x < PLAYER_RADIUS) {
					players[i].position = lastPosition;
					players[i].velocity = glm::reflect(players[i].velocity, { 1,0 });
				}

				if (players[i].position.x > WINDOW_WIDTH - PLAYER_RADIUS) {
					players[i].position = lastPosition;
					players[i].velocity = glm::reflect(players[i].velocity, { -1,0 });
				}
				if (players[i].position.y < PLAYER_RADIUS) {
					players[i].position = lastPosition;
					players[i].velocity = glm::reflect(players[i].velocity, { 0,1 });
				}
				if(players[i].position.y > WINDOW_HEIGHT - PLAYER_RADIUS) {
					players[i].position = lastPosition;
					players[i].velocity = glm::reflect(players[i].velocity, { 0,-1 });
				}
			}
		}
		break;
	}
	}
	
	glutPostRedisplay();
}
void UpdataMouse(int _x, int _y) {
	mousePosition = {_x, _y};
	firePower = glm::distance(players[currentPlayer].position, mousePosition);
	firePower = glm::min(firePower, 130.0f);
}

void Mouse(int, int _state, int _x, int _y) {
	UpdataMouse(_x, _y);

	switch (_state) {
	case GLUT_DOWN:
		switch (phase) {
		case PHASE_IDLE:
			phase = PHASE_HOLD;
			break;
		}
		break;
	case GLUT_UP:
		switch (phase) {
		case PHASE_HOLD: {
			phase = PHASE_FIRE;
			glm::vec2 v = glm::normalize(players[currentPlayer].position - mousePosition);
			v *= firePower;
			players[currentPlayer].velocity = v * 0.5f;
			break;
		}
		}
		break;
	}
}

void Motion(int _x, int _y) {
	UpdataMouse(_x, _y);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowPosition(640, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("");
	Init();
	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop();
}
