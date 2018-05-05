//Include all libraries needed for the program
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

//General include directives template
#if _WIN32
#   include <Windows.h>
#endif
#if __APPLE__
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <GLUT/glut.h>
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glut.h>
#endif

//Define maximum number of missiles
#define maxNumber 15

//Declaring all structures and enums needed for this program
typedef struct { float x, y; } vec2f;

//Structure for missiles
typedef struct { vec2f r, v; bool launch; int type; int damage; float projSize; } state;
state missileOne[maxNumber];
state missileTwo[maxNumber];
state islandMissile[maxNumber];

//Structure for items, i.e. island, left & right boat 
typedef struct { bool status; int RemainLife; } status;
status islandStatus = { true, 10 };
status shipOne = { true, 10 };
status shipTwo = { true, 10 };

//enum for island properties
enum { iCannon, islands } island;
float islandAngle[islands] = { 90.0f };
vec2f islandPosition[islands] = { 0.0f, 0.25f };
float islandCannonIncrement[islands] = { 1.0f };

//enum for left ship properties
enum { body, cannon, shipPart } ShipOne;
float shipOneAngle[shipPart] = { 0.0f, 40.0f };
vec2f shipOnePosition[shipPart] = { { -0.5f, (0.25 * sin(-M_PI)) },{ 0.0f, -0.05f } };
float incrementO[shipPart] = { 0.1f, 1.0f };

//enum for right ship properties
enum { bodyT, cannonT, shipPart2 } ShipTwo;
float shipTwoAngle[shipPart2] = { 0.0f, 140.0f };
vec2f shipTwoPosition[shipPart2] = { { 0.5f, (0.25 * sin(M_PI)) },{ 0.0f, -0.05f } };
float incrementT[shipPart2] = { 0.1f, 1.0f };

//Structure for water wave
typedef struct {
	float amplitude;
	float wavelength;
	float w;
	float k;
}water;

//Initializing values for water structure
water wave = { 0.25, 2, M_PI / 4, M_PI };

//Structure containing global properties
typedef struct {
	float seaW;
	bool water;
	bool debug;
	bool go;
	bool displayFrame;
	bool showTangent;
	bool showNormal;
	bool wireframe;
	float startTime;
	int frames;
	float frameRate;
	float frameRateInterval;
	float lastFrameRateT;
	float t;
	float lastT;
	float dt;
	const float gravity;
	const int milli;
	int segments;
	int tesselation;
}global;

//Initializing values for all properties in global structure
global g =
{ 0.0, false, true, true, false, false, false, false, 0.0, 0, 0.0, 0.02, 0.0, 0.0, -1.0, 0.0, -9.8, 1000, 64, 64 };

//Calculate k value for sine waves function
//--> y = A * sin(k *x) <--
float calculateK(float wavelength)
{
	float k;
	return k = 2 * M_PI / wavelength;
}

//Function to initialize all properties for the "state" structure that contains all missile properties
void initMissile()
{
	for (int i = 0; i < maxNumber; i++)
	{
		missileOne[i].r.x = 0.0;
		missileOne[i].r.y = 0.0;
		missileOne[i].v.x = 0.0;
		missileOne[i].v.y = 0.0;
		missileOne[i].launch = false;
		missileOne[i].type = 1;
		missileOne[i].damage = 0;
		missileOne[i].projSize = 0.005;
	}
	for (int a = 0; a < maxNumber; a++)
	{
		missileTwo[a].r.x = 0.0;
		missileTwo[a].r.y = 0.0;
		missileTwo[a].v.x = 0.0;
		missileTwo[a].v.y = 0.0;
		missileTwo[a].launch = false;
		missileTwo[a].type = 2;
		missileTwo[a].damage = 0;
		missileTwo[a].projSize = 0.005;
	}
	for (int b = 0; b < maxNumber; b++)
	{
		islandMissile[b].r.x = 0.0;
		islandMissile[b].r.y = 0.0;
		islandMissile[b].v.x = 0.0;
		islandMissile[b].v.y = 0.0;
		islandMissile[b].launch = false;
		islandMissile[b].type = 0;
		islandMissile[b].damage = 0;
		islandMissile[b].projSize = 0.01;
	}
}

//Function to plot the coordinates for the center island body
void islandBody()
{
	glBegin(GL_POLYGON);
	glColor4f(1.0, 1.0, 0.0, 1.0);
	glVertex3f(0.25, 0.25, 0);
	glVertex3f(-0.25, 0.25, 0);
	glVertex3f(-0.25, -1.0, 0);
	glVertex3f(0.25, -1.0, 0);
	glEnd();
}

//Function to plot the coordinates for the cannon that sits above the center island body
void islandCannon()
{
	glBegin(GL_POLYGON);
	glColor4f(1.0, 1.0, 0.0, 1.0);
	glVertex2f(0, -0.025);
	glVertex2f(0.25, -0.025);
	glVertex2f(0.25, 0.025);
	glVertex2f(0, 0.025);
	glEnd();
}

//Function to calculate the degree value based on given radian parameter
float calcDegree(float radians)
{
	float degree = radians * 180 / M_PI;
	return degree;
}

//Function to calculate the radian value based on given degree parameter
float calcRadians(float degree)
{
	float radians = degree * (M_PI / 180);
	return radians;
}

//Function to draw the world axes x, y, and z
void drawAxes(float length)
{
	//X axis
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(length, 0.0f, 0.0f);
	glEnd();

	//Y axis
	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, length, 0.0f);
	glEnd();

	//Z axis
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, length);
	glEnd();
}

//Function to print the game over screen when any item has 0 lives / when the game ends
void gameOver()
{
	char buffer[30];
	char *bufferP;
	int w, h;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	w = glutGet(GLUT_WINDOW_WIDTH);
	h = glutGet(GLUT_WINDOW_HEIGHT);
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	/* Show Game Over*/
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(160, 270);
	snprintf(buffer, sizeof buffer, "GAME OVER");
	for (bufferP = buffer; *bufferP; bufferP++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufferP);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopAttrib();
}

//Function to calculate the slop angle of the cannon on the boat
float slope(float x)
{
	return 0.25 * 2 * M_PI * (cos(2 * M_PI * x + g.t));
}

//Function that returns the value of PI
float calK()
{
	return M_PI;
}

//Function to calculate the value of y axis based on the given x axis parameter
float calY(float x)
{
	float y;
	float k = calK();

	y = 0.25 * sin(2 * k * x + g.seaW);
	return y;
}

//Function to calculate the degree of the boat based on the given x axis parameter
float boatDegree(float x)
{
	float degree, newX, newY, y, Slope;

	newX = x;
	newX += 0.01;
	newY = slope(x) * (newX - x) + calY(x);
	newY = newY - calY(x);
	newX = newX - x;
	degree = atan2(newY, newX);
	return degree;
}

//Function to draw the vector based on  given parameters 
void drawVector(float x, float y, float a, float b, float s, bool normalize, float rd, float gr, float bl)
{
	glBegin(GL_LINES);
	glColor3f(rd, gr, bl);
	glVertex3f(x, y, 1);
	if (normalize) {
		// --> |V| = sqrt(a^2 + b^2) , V/|V| = <a/|V|, b/|V|> <--
		float normalV = sqrt(a*a + b*b);
		a = a / normalV;
		b = b / normalV;
		glVertex3f(x + (a * s), y + (b * s), 0);
	}
	else
	{
		glVertex3f(x + a * s, y + b * s, 0);
	}
}

//Function to calculate the values to draw a tangent vector
void drawTangent(float x, float y, float s, float rd, float gr, float bl)
{
	glBegin(GL_LINES);
	glColor3f(rd, gr, bl);
	float newX, newY;
	float d = slope(x);
	newX = x;
	newX = newX + s;
	newY = d * (newX - x) + calY(x);
	drawVector(x, y, newX - x, newY - y, 1, false, rd, gr, bl);
	glEnd();
}

//Function to calculate the values to draw a normal vector
void drawNormal(float x, float y, float s, float rd, float gr, float bl)
{
	glBegin(GL_LINES);
	glColor3f(rd, gr, bl);
	float newX, newY;
	float deri = slope(x);
	newX = x;
	newX = newX + s;
	newY = deri * (newX - x) + calY(x);
	drawVector(x, y, -(newY - y), (newX - x), 0.1, true, rd, gr, bl);
	glEnd();
}

//Function to plot the coordinates and draws the water waves
void drawWater()
{
	float x, y;
	float left = -1.0;
	float right = 1.0;
	float range = right - left;
	float stepSize = range / g.segments;
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= g.segments; i++)
	{
		x = (float)i * stepSize + left;
		y = calY(x);
		glColor4f(0.0f, 1.0f, 1.0f, 0.5f);
		glVertex3f(x, y, 0);
		glVertex3f(x, -1.0, 0);
	}
	glEnd();
}

//Function to plot the coordinates for the bottom half of the boat
void boatTrapezium()
{
	glBegin(GL_POLYGON);
	glVertex2f(0.1f, 0.025f);
	glVertex2f(0.05f, -0.025f);
	glVertex2f(-0.05f, -0.025f);
	glVertex2f(-0.1f, 0.025f);
	glEnd();
}

//Function to plot the coordinates for the top half of the boat
void boatRectangle()
{
	glBegin(GL_POLYGON);
	glVertex2f(-0.025f, -0.025f);
	glVertex2f(0.025f, -0.025f);
	glVertex2f(0.025f, 0.025f);
	glVertex2f(-0.025f, 0.025f);
	glEnd();
}

//Function to plot the coordinates for the cannon that sits above the top of the boat
void boatCannon()
{
	glBegin(GL_POLYGON);
	glVertex2f(0.00f, 0.005f);
	glVertex2f(0.12f, 0.005f);
	glVertex2f(0.12f, -0.005f);
	glVertex2f(0.00f, -0.005f);
	glEnd();
}

//Function the draw the health bar based on the given remaining life parameter
void healthBar(float lifeLeft)
{
	glBegin(GL_POLYGON);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(lifeLeft, 0.0f);
	glVertex2f(lifeLeft, 0.1f);
	glVertex2f(0.0f, 0.1f);
	glEnd();
}

//Function to draw the life bar for island
void drawIslandHealth(float life)
{
	if (life == 0)
		islandStatus.status = false;
	else
		islandStatus.status = true;

	if (islandStatus.status == true)
	{
		drawAxes(1);
		glPushMatrix();
		glTranslatef(-0.9f, 0.78f, 0.0f);
		glScalef(0.5f, 0.5f, 0.0f);
		glColor3f(1.0f, 1.0f, 0.0f);
		if (life > 0)
		{
			healthBar(life / 10);
		}
		glPopMatrix();
	}
	else
	{
		g.go = false;
	}
}

//Function to draw the life bar for the left boat
void drawShipOneHealth(float life)
{
	if (life == 0)
		shipOne.status = false;
	else
		shipOne.status = true;

	if (shipOne.status == true)
	{
		glPushMatrix();
		glTranslatef(-0.9f, 0.9f, 0.0f);
		glScalef(0.5f, 0.5f, 0.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		if (life > 0)
		{
			healthBar(life / 10);
		}
		glPopMatrix();
	}
	else
	{
		g.go = false;
	}

}

//Function to draw the life bar for the right boat
void drawShipTwoHealth(float life)
{
	if (life == 0)
		shipTwo.status = false;
	else
		shipTwo.status = true;

	if (shipTwo.status == true)
	{
		glPushMatrix();
		glTranslatef(-0.9f, 0.84f, 0.0f);
		glScalef(0.5f, 0.5f, 0.0f);
		glColor3f(0.0f, 0.0f, 1.0f);
		if (life > 0)
		{
			healthBar(life / 10);
		}
		glPopMatrix();
	}
	else
	{
		g.go = false;
	}
}

//Function to draw the missile projectile based on the given missiles object parameter
void drawProjectile(state *obj)
{
	for (int i = 0; i < maxNumber; i++)
	{
		if (obj[i].launch)
		{
			glPushMatrix();
			glTranslatef(obj[i].r.x, obj[i].r.y, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			glutWireSphere(obj[i].projSize, 16, 8);
			glPopMatrix();
		}
	}
}

//Function to calculate the parabola values
void calculateParabola(state *obj, float dt)
{
	obj->r.x += obj->v.x * dt;
	obj->r.y += obj->v.y * dt;
	obj->v.y += g.gravity * dt;
}

//Function to calculate the distance based on given parameters
float calPythagoras(float a, float b, float c, float d)
{
	float totDistance = sqrt(((a - b)*(a - b)) + ((c - d)*(c - d)));
	return totDistance;
}

//Function to check whether has any collision being made based on the given missile object
bool checkCollision(state obj)
{
	if (obj.launch)
	{
		float lx = shipOnePosition[body].x;
		float ly = calY(lx);
		float rx = shipTwoPosition[bodyT].x;
		float ry = calY(rx);
		float rightDistance = calPythagoras(rx, obj.r.x, ry, obj.r.y);
		float leftDistance = calPythagoras(lx, obj.r.x, ly, obj.r.y);

		//For left & right boat shotting to center tower
		if (obj.r.y <= 0.25 && obj.r.x < 0.25 && obj.r.x > -0.25)
		{
			if (obj.type != 0 && obj.r.y <= 0.25 && obj.r.y > 0.24)	
			{
				islandStatus.RemainLife -= obj.damage;
			}
			return false;
		}
		//For center tower and right boat shooting to left boat
		if (leftDistance < 0.1 && obj.r.x < 0)
		{
			if (obj.type != 1)
			{
				shipOne.RemainLife -= obj.damage;
			}
			return false;
		}
		//For center tower and left boat shooting to right boat
		if (rightDistance < 0.1 && obj.r.x > 0)
		{
			if (obj.type != 2)
			{
				shipTwo.RemainLife -= obj.damage;
			}
			return false;
		}
		if (obj.r.y < calY(obj.r.x))
		{
			return false;
		}
		return obj.launch;
	}
	else
	{
		return obj.launch;
	}
}

//Function to calculate the accurate position of the missile projectile
//as the cannons are constantly being rotated
void updateProjectile(state *obj)
{
	float x;

	//For left boat missile
	if (obj[0].type == 1)
	{
		for (int i = 0; i < maxNumber; i++)
		{
			//If missile is not being shot / launched yet
			if (!(obj[i].launch))
			{
				x = shipOnePosition[body].x;
				obj[i].r.x = x + cos(boatDegree(x) + calcRadians(shipOneAngle[cannon])) * 0.12;
				obj[i].r.y = calY(x) + sin(boatDegree(x) + calcRadians(shipOneAngle[cannon])) * 0.12;
				obj[i].v.x = 30.0 * (-x + obj[i].r.x);
				obj[i].v.y = 30.0 * (-calY(x) + obj[i].r.y);
			}
			//If missile has been launched and check collision has been made
			if ((obj[i].launch = checkCollision(obj[i])) == true)
			{
				obj[i].damage = 1;
			}
		}
	}
	//For right boat missile
	if (obj[0].type == 2)
	{
		for (int i = 0; i < maxNumber; i++)
		{
			if (!(obj[i].launch))
			{
				x = shipTwoPosition[bodyT].x;
				obj[i].r.x = x + cos(boatDegree(x) + calcRadians(shipTwoAngle[cannonT])) * 0.12;
				obj[i].r.y = calY(x) + sin(boatDegree(x) + calcRadians(shipTwoAngle[cannonT])) * 0.12;
				obj[i].v.x = 30.0 * (-x + obj[i].r.x);
				obj[i].v.y = 30.0 * (-calY(x) + obj[i].r.y);
			}
			if ((obj[i].launch = checkCollision(obj[i])) == true)
			{
				obj[i].damage = 1;
			}
		}
	}
	//For center tower missile
	if (obj[0].type == 0)
	{
		for (int i = 0; i < maxNumber; i++)
		{
			if (!(obj[i].launch))
			{
				//--> x(x0, v0, t, ?) = x0 + v0 t cos? <--
				obj[i].r.x = islandPosition[iCannon].x + 0.15 * cos(calcRadians(islandAngle[iCannon]));
				//--> y(y0, v0, t, ?) = y0 + v0 t sin? <--
				obj[i].r.y = islandPosition[iCannon].y + 0.15 * sin(calcRadians(islandAngle[iCannon]));
				obj[i].v.x = 20.0 * (obj[i].r.x);
				obj[i].v.y = 20.0 * (obj[i].r.y - 0.25);
			}
			if ((obj[i].launch = checkCollision(obj[i])) == true)
			{
				obj[i].damage = 1;
			}
		}
	}
}

//Function to draw the prediction line of the missile
void missileLine(state obj)
{
	if (obj.launch)
	{
		float t = 0;

		glBegin(GL_LINE_STRIP);
		glColor3f(1, 1, 1);
		obj.damage = 0;
		while (checkCollision(obj))
		{
			glVertex2f(obj.r.x, obj.r.y);
			calculateParabola(&obj, t);
			t += 0.0001;
		}
		glEnd();
	}
}

//Function that constantly updates the missile
void updateMissile(state *obj, float dt)
{
	//Center tower
	if (obj[0].type == 0)
	{
		for (int i = 0; i < maxNumber; i++)
		{
			calculateParabola(&(obj[i]), dt);
		}
	}
	//Left boat
	if (obj[0].type == 1)
	{
		for (int i = 0; i < maxNumber; i++)
		{
			calculateParabola(&(obj[i]), dt);
		}
	}
	//Right boat
	if (obj[0].type == 2)
	{
		for (int i = 0; i < maxNumber; i++)
		{
			calculateParabola(&(obj[i]), dt);
		}
	}
}

//Function to initiate when a shoot missile key has been pressed
void shootMissile(state *obj)
{
	for (int i = 0; i < maxNumber; i++)
	{
		if (!(obj[i].launch))
		{
			g.startTime = glutGet(GLUT_ELAPSED_TIME) / (float)g.milli;
			obj[i].launch = true;
			return;
		}
	}
}

//Function that invokes the "islandBody" & "islandCannon" function
//and draws the entire tower together
void drawIsland()
{
	glPushMatrix();
	islandBody();
	glTranslatef(islandPosition[iCannon].x, islandPosition[iCannon].y, 0.0f);
	glRotatef(islandAngle[iCannon], 0.0f, 0.0f, 1.0f);
	glPushMatrix();
	islandCannon();
	glPopMatrix();
	glPopMatrix();
	updateProjectile(islandMissile);
}

//Function that draws the left boat
void drawBoat()
{
	float x = shipOnePosition[body].x;
	float y = wave.amplitude * sin(2 * wave.k * x + g.seaW);
	shipOnePosition[body].y = y;
	float dy = wave.amplitude * 2 * wave.k * cos(2 * wave.k * x + g.seaW);
	shipOneAngle[body] = calcDegree(atanf(dy));

	glPushMatrix();
	glTranslatef(shipOnePosition[body].x, shipOnePosition[body].y, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);
	glRotatef(shipOneAngle[body], 0.0f, 0.0f, 1.0f);
	drawAxes(0.1);
	glColor3f(1.0f, 0.0f, 0.0f);
	boatTrapezium();

	glColor3f(1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.05f, 0.0f);
	boatRectangle();

	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glTranslatef(shipOnePosition[cannon].x, shipOnePosition[cannon].y, 0.0f);
	glRotatef(shipOneAngle[cannon], 0.0f, 0.0f, 1.0f);
	boatCannon();

	glPopMatrix();
	glPopMatrix();

	updateProjectile(missileOne);
}

//Function that draws the right boat
void drawBoat2()
{
	float x = shipTwoPosition[bodyT].x;
	float y = wave.amplitude * sin(2 * wave.k * x + g.seaW);
	shipTwoPosition[bodyT].y = y;
	float dy = wave.amplitude * 2 * wave.k * cos(2 * wave.k * x + g.seaW);
	shipTwoAngle[bodyT] = calcDegree(atanf(dy));

	glPushMatrix();
	glTranslatef(shipTwoPosition[bodyT].x, shipTwoPosition[bodyT].y, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glRotatef(shipTwoAngle[bodyT], 0.0f, 0.0f, 1.0f);
	drawAxes(0.1);
	glColor3f(0.0f, 0.0f, 1.0f);
	boatTrapezium();

	glPushMatrix();
	glColor3f(0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, 0.05f, 0.0f);
	boatRectangle();

	glColor3f(0.0f, 0.0f, 1.0f);
	glTranslatef(shipTwoPosition[cannonT].x, shipTwoPosition[cannonT].y, 0.0f);
	glRotatef(shipTwoAngle[cannonT], 0.0f, 0.0f, 1.0f);
	boatCannon();

	glPopMatrix();
	glPopMatrix();

	updateProjectile(missileTwo);
}

//Function that constantly calculates the frame rate, time per frame, and tesselation
void displayOSD()
{
	char buffer[30];
	char *bufferP;
	int w, h;

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	w = glutGet(GLUT_WINDOW_WIDTH);
	h = glutGet(GLUT_WINDOW_HEIGHT);
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	/* getting frame rate*/
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(260, 370);
	snprintf(buffer, sizeof buffer, "Fr (f/s): %4.0f", g.frameRate);
	for (bufferP = buffer; *bufferP; bufferP++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufferP);

	/*Calculator time per frame*/
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(260, 350);
	snprintf(buffer, sizeof buffer, "Ft (ms/f): %3.0f", 1.0 / g.frameRate * 1000.0);
	for (bufferP = buffer; *bufferP; bufferP++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufferP);

	/* Calculate tesselation */
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(260, 330);
	snprintf(buffer, sizeof buffer, "Tess: %8.0i", g.tesselation);
	for (bufferP = buffer; *bufferP; bufferP++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufferP);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopAttrib();
}

//Function that constantly updates throughout the entire program
void idle()
{
	float time;
	g.t = glutGet(GLUT_ELAPSED_TIME) / (float)1000 - g.startTime;

	if (g.lastT < 0.0f)
	{
		g.lastT = g.t;
		return;
	}

	g.dt = g.t - g.lastT;

	if (g.dt < 0)
	{
		g.lastT = g.t;
		return;
	}

	if (g.go)
	{
		g.seaW += 1.0*g.dt;
	}

	updateMissile(islandMissile, g.dt);
	updateMissile(missileOne, g.dt);
	updateMissile(missileTwo, g.dt);

	g.lastT = g.t;

	/*for the frame rate calculation*/
	time = g.t - g.lastFrameRateT;
	if (time > g.frameRateInterval) {
		g.frameRate = g.frames / time;
		g.lastFrameRateT = g.t;
		g.frames = 0;
	}

	/* Ask glut to schedule call to display function */
	glutPostRedisplay();
}

//Function to enable and disable wire frame mode
void wireFrame()
{
	if (g.wireframe == true)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (g.wireframe == false)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

//Function to enable and disable to display of tangent and normal vectors
void showTangentNormal()
{
	float x, y;
	float a, b;
	float left = -1.0;
	float right = 1.0;
	float range = right - left;
	float stepSize = range / g.segments;
	if (g.showTangent)
	{
		for (x = left; x <= right; x += 0.1) {
			y = calY(x);
			drawTangent(x, y, 0.1, 1, 0, 0);
		}
	}
	if (g.showNormal)
	{
		for (x = left; x <= right; x += 0.1) {
			y = calY(x);
			drawNormal(x, y, 0.1, 0, 1, 0);
		}
	}
}

//Function that invokes all drawing functions for this program
void display()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	/* Put drawing code here */
	drawIsland();
	drawWater();
	drawAxes(1);
	drawProjectile(islandMissile);
	drawProjectile(missileOne);
	drawProjectile(missileTwo);
	drawBoat();
	drawShipOneHealth(shipOne.RemainLife);
	drawIslandHealth(islandStatus.RemainLife);
	drawBoat2();
	drawShipTwoHealth(shipTwo.RemainLife);

	for (int i = 0; i < maxNumber; i++)
	{
		missileLine(missileOne[i]);
	}
	for (int i = 0; i < maxNumber; i++)
	{
		missileLine(missileTwo[i]);
	}
	for (int i = 0; i < maxNumber; i++)
	{
		missileLine(islandMissile[i]);
	}

	wireFrame();
	showTangentNormal();
	displayOSD();

	//If any of the object (tower / left boat / right boat) has 0 lives left
	if (!islandStatus.status || !shipOne.status || !shipTwo.status)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(1, 1, 1);
		gameOver();
	}

	GLenum err = glGetError();
	// Check for errors
	while ((err = glGetError()) != GL_NO_ERROR)
		printf("%s\n", gluErrorString(err));

	glutSwapBuffers();
	g.frames++;
}

/* shortcut key which determine what to do */
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		//Toggle wireframe mode on/off
	case 'w':
		if (g.wireframe == false)
			g.wireframe = true;
		else
			g.wireframe = false;
		break;
		//Toggle normal visualisation on/off
	case 'n':
		if (g.showNormal == false)
			g.showNormal = true;
		else
			g.showNormal = false;
		break;
		//Toggle tangent visualisation on/off
	case 't':
		if (g.showTangent == false)
			g.showTangent = true;
		else
			g.showTangent = false;
		break;
		//Left cannon rotate right (down)
	case 'q':
		if (shipOneAngle[cannon] > 0.0)
			shipOneAngle[cannon] -= incrementO[cannon];
		break;
		//Left cannon rotate left (up)
	case 'Q':
		if (shipOneAngle[cannon] < 180.0)
			shipOneAngle[cannon] += incrementO[cannon];
		break;
		//Left boat move left
	case 'a':
		if (shipOnePosition[body].x > -0.895)
			shipOnePosition[body].x -= 0.01f;
		break;
		//Left boat move right
	case 'd':
		if (shipOnePosition[body].x < -0.38)
			shipOnePosition[body].x += 0.01f;
		break;
		//Right cannon rotate right (down)
	case 'o':
		if (shipTwoAngle[cannonT] > 0.0)
			shipTwoAngle[cannonT] -= incrementT[cannonT];
		break;
		//Right cannon rotate left (up)
	case 'O':
		if (shipTwoAngle[cannonT] < 180.0)
			shipTwoAngle[cannonT] += incrementT[cannonT];
		break;
		//Right boat move left
	case 'j':
		if (shipTwoPosition[bodyT].x > 0.38)
			shipTwoPosition[bodyT].x -= 0.01f;
		break;
		//Right boat move right
	case 'l':
		if (shipTwoPosition[bodyT].x < 0.855)
			shipTwoPosition[bodyT].x += 0.01f;
		break;
		//Left boat fire missile
	case 'e':
		shootMissile(missileOne);
		break;
		//Right boat fire missile
	case 'i':
		shootMissile(missileTwo);
		break;
		//Toggle all animation on/off
	case 'g':
		if (g.go == false)
			g.go = true;
		else
			g.go = false;
		break;
		//Double wave tesselation
	case '+':
		g.segments *= 2;
		g.tesselation *= 2;
		break;
		//Halve wave tesselation
	case '-':
		if (g.segments > 4)
		{
			g.segments /= 2;
			g.tesselation /= 2;
		}
		break;
		//Rotate island cannon down (right)
	case 'h':
		if (islandAngle[iCannon] > 0)
			islandAngle[iCannon] -= islandCannonIncrement[iCannon];
		break;
		//Rotate island cannon up (left)
	case 'f':
		if (islandAngle[iCannon] < 180)
			islandAngle[iCannon] += islandCannonIncrement[iCannon];
		break;
		//(space) Fire island cannon
	case ' ':
		shootMissile(islandMissile);
		break;
		// ESC - quit the program
	case 27:
		exit(EXIT_SUCCESS);
		break;
		//Default
	default:
		break;
	}
}

//Main function of program
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutInitWindowPosition(300, 300);
	glutCreateWindow("Assignment 1");
	initMissile();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMainLoop();
	return EXIT_SUCCESS;
}