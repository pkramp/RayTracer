#include "GL\glew.h"
#include "GL\freeglut.h"

#include "BMP\Bitmap.h"
#include "BMP\BMPHandler.h"

#include <iostream>

#include "Vector.h"

using namespace std;

class Ray{
public:
	Vector origin;
	Vector direction;
	Ray(Vector orig, Vector dir){
		origin = orig;
		direction = dir;
	}
};

class Sphere{
	Vector center;
	double radius;
	Sphere(Vector c, double r){
		center = c;
		radius = r;
	}

	bool intersect(Ray ray, double){
		Vector m = center;
		Vector o = ray.origin;
		Vector om = o - m;


	}
};





void init(void)
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	//GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Vector* lightSource = new Vector();


	//glutSolidSphere(1.0, 20, 16);
	for (int height = 0; height < 500; height++)
	{
		for (int width = 0; width < 500; width++)
		{
			Ray ray(Vector(height, width, 0), Vector(0, 0, 1));
		}
	}








	BYTE * Buffer = new BYTE[3 * 500 * 500];
	glReadPixels(0, 0, 500, 500, GL_RGB, GL_UNSIGNED_BYTE, Buffer);
	const char* imagesource = "image.bmp";
	BMPHandler::getInstance().Load24bitBMP(imagesource, 1, "image");
	Bitmap * image = BMPHandler::getInstance().findBitmapByKey("image");

	image->Buffer = Buffer;
	BMPHandler::getInstance().SaveBMP(image, "newimage.bmp", 500, 500, "image");
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-1.5, 1.5, -1.5*(GLfloat)h / (GLfloat)w,
		1.5*(GLfloat)h / (GLfloat)w, -10.0, 10.0);
	else
		glOrtho(-1.5*(GLfloat)w / (GLfloat)h,
		1.5*(GLfloat)w / (GLfloat)h, -1.5, 1.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}