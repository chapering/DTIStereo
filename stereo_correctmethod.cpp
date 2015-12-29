/*
 * stereo rendering exemplifying the "correct method" implementation from 
 * hcai $Exp 06/08/2012
 *
 * Acknowledgement:
 *	this method is coded on the basis of that written by Paul Bourke in an example program
 *	for stereo rendering named "pulsar.c"
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include "GLoader.h"

//#define PI 3.141592653589793238462643
//#define DTOR            0.0174532925
#define PI	M_PI
#define DTOR            (M_PI/180.0)
#define RTOD            57.2957795
#define CROSSPROD(p1,p2,p3) \
   p3.x = p1.y*p2.z - p1.z*p2.y; \
   p3.y = p1.z*p2.x - p1.x*p2.z; \
   p3.z = p1.x*p2.y - p1.y*p2.x

// bounding volume of the brain model
GLfloat Xminmax[2] = {50.774986, 182.695862};
GLfloat Yminmax[2] = {38.447567, 203.434158};
GLfloat Zminmax[2] = {5.271503, 135.014359};
CStreamtubes tubes;

GLint rot_x = 0, rot_y = 0, rot_z = 0;
GLfloat scale=1.0;

static GLboolean fullscreen = GL_FALSE;	/* Create a single fullscreen window */
static GLboolean stereo = GL_FALSE;	/* Enable stereo.  */

typedef struct {
    double x, y, z;
} XYZ;
typedef struct {
    double r, g, b;
} COLOUR;
typedef struct {
    unsigned char r, g, b, a;
} PIXELA;
typedef struct {
    XYZ vp;			/* View position           */
    XYZ vd;			/* View direction vector   */
    XYZ vu;			/* View up direction       */
    XYZ pr;			/* Point to rotate about   */
    double focallength;		/* Focal Length along vd   */
    double aperture;		/* Camera aperture         */
    double eyesep;		/* Eye separation          */
    int screenwidth, screenheight;
} CAMERA;
double dtheta = 1;
CAMERA camera;
XYZ origin = { 0.0, 0.0, 0.0 };

static void 
Normalise(XYZ * p)
{
    double length;

    length = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
    if (length != 0) {
		p->x /= length;
		p->y /= length;
		p->z /= length;
    } else {
		p->x = 0;
		p->y = 0;
		p->z = 0;
    }
}

static void
initCamera()
{
    camera.aperture = 60;
    camera.focallength = 457;
    //camera.eyesep = camera.focallength / 20;
    camera.eyesep = 60;
    camera.pr = origin;

	/*
    camera.vp.x = 39;
    camera.vp.y = 53;
    camera.vp.z = 22;
	*/
    camera.vp.x = 0;
    camera.vp.y = 0;
    camera.vp.z = (Zminmax[0]+Zminmax[1]) * 6.0;
    camera.vd.x = -camera.vp.x;
    camera.vd.y = -camera.vp.y;
    camera.vd.z = -camera.vp.z;

    camera.vu.x = 0;
    camera.vu.y = 1;
    camera.vu.z = 0;

    camera.screenwidth = 1920;
    camera.screenheight = 1080;
}

static void 
keyFunc(unsigned char key, int x, int y)
{
    switch (key) {
		case 27:			/* Quit */
		case 'Q':
		case 'q':
			exit(0);
		case 'x':
			rot_x = (rot_x + 1) % 360;
			break;
		case 'y':
			rot_y = (rot_y + 1) % 360;
			break;
		case 'z':
			rot_z = (rot_z + 1) % 360;
			break;
		case 32: // SPACE to get home
			rot_x = rot_y = rot_z = 0;
			break;
		case 'c':
			glDumpImagePPM(0,0,1920,1080,true);
			break;
		case '=':
			scale += 0.1;
			break;
		case '-':
			scale -= 0.1;
			break;
		default:
			return;
	}
	glutPostRedisplay();
}

static void
do_draw(void)
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glClearColor(0,0,0,0);

   glScalef(scale, scale, scale);
   glRotatef(rot_x, 1,0,0);
   glRotatef(rot_y, 0,1,0);
   glRotatef(rot_z, 0,0,1);

   // three axes
   glBegin(GL_LINES);
	  glColor3f(1,0,0);
	  glVertex3f(0,0,0);
	  glVertex3f(200,0,0);

	  glColor3f(0,1,0);
	  glVertex3f(0,0,0);
	  glVertex3f(0,200,0);

	  glColor3f(0,0,1);
	  glVertex3f(0,0,0);
	  glVertex3f(0,0,200);
   glEnd();

   glColor3f(1.0, 1.0, 1.0);
   glPushMatrix();
   glTranslatef( - (Xminmax[0] + Xminmax[1])/2.0, 
		   - (Yminmax[0] + Yminmax[1])/2.0, 
		   - (Zminmax[0] + Zminmax[1])/2.0 );

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   glBegin(GL_POLYGON);
	   glVertex3f(Xminmax[1], Yminmax[1], Zminmax[0]); // 1
	   glVertex3f(Xminmax[1], Yminmax[0], Zminmax[0]); // 2
	   glVertex3f(Xminmax[0], Yminmax[0], Zminmax[0]); // 3 
	   glVertex3f(Xminmax[0], Yminmax[1], Zminmax[0]); // 4
   glEnd();

   glBegin(GL_POLYGON);
	   glVertex3f(Xminmax[1], Yminmax[1], Zminmax[0]); // 1
	   glVertex3f(Xminmax[1], Yminmax[1], Zminmax[1]); // 8
	   glVertex3f(Xminmax[1], Yminmax[0], Zminmax[1]); // 7 
	   glVertex3f(Xminmax[1], Yminmax[0], Zminmax[0]); // 2
   glEnd();

   glBegin(GL_POLYGON);
	   glVertex3f(Xminmax[1], Yminmax[1], Zminmax[0]); // 1
	   glVertex3f(Xminmax[0], Yminmax[1], Zminmax[0]); // 4
	   glVertex3f(Xminmax[0], Yminmax[1], Zminmax[1]); // 5
	   glVertex3f(Xminmax[1], Yminmax[1], Zminmax[1]); // 8
   glEnd();

   glBegin(GL_POLYGON);
	   glVertex3f(Xminmax[0], Yminmax[1], Zminmax[0]); // 4
	   glVertex3f(Xminmax[0], Yminmax[0], Zminmax[0]); // 3 
	   glVertex3f(Xminmax[0], Yminmax[0], Zminmax[1]); // 6
	   glVertex3f(Xminmax[0], Yminmax[1], Zminmax[1]); // 5
   glEnd();

   glBegin(GL_POLYGON);
	   glVertex3f(Xminmax[1], Yminmax[0], Zminmax[0]); // 2
	   glVertex3f(Xminmax[1], Yminmax[0], Zminmax[1]); // 7 
	   glVertex3f(Xminmax[0], Yminmax[0], Zminmax[1]); // 6
	   glVertex3f(Xminmax[0], Yminmax[0], Zminmax[0]); // 3 
   glEnd();

   glBegin(GL_POLYGON);
	   glVertex3f(Xminmax[0], Yminmax[1], Zminmax[1]); // 5
	   glVertex3f(Xminmax[0], Yminmax[0], Zminmax[1]); // 6
	   glVertex3f(Xminmax[1], Yminmax[0], Zminmax[1]); // 7 
	   glVertex3f(Xminmax[1], Yminmax[1], Zminmax[1]); // 8
   glEnd();
   glPopMatrix();

   tubes.draw();
   glutSwapBuffers();
}

static void
draw(void)
{
    XYZ r;
    double ratio, radians, wd2, ndfl;
    double left, right, top, bottom;
	double near = Zminmax[0]/4.0, far = (Zminmax[0]+Zminmax[1])/2.0 * 16.0;

    /* Clip to avoid extreme stereo */
    if (stereo) {
		near = camera.focallength / 5;
	}

    /* Misc stuff */
    ratio = camera.screenwidth / (double) camera.screenheight;
    radians = DTOR * camera.aperture / 2;
    wd2 = near * tan(radians);
    ndfl = near / camera.focallength;

    /* Clear the buffers */
    glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (stereo) {
		glDrawBuffer(GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (stereo) {
		/* Derive the two eye positions */
		CROSSPROD(camera.vd, camera.vu, r);
		Normalise(&r);
		r.x *= camera.eyesep / 2.0;
		r.y *= camera.eyesep / 2.0;
		r.z *= camera.eyesep / 2.0;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -ratio * wd2 - 0.5 * camera.eyesep * ndfl;
		right = ratio * wd2 - 0.5 * camera.eyesep * ndfl;
		top = wd2;
		bottom = -wd2;
		glFrustum(left, right, bottom, top, near*2, far);

		//printf("left=%f right=%f bottom=%f top=%f\n", left, right, bottom, top);

		glMatrixMode(GL_MODELVIEW);
		glDrawBuffer(GL_BACK_RIGHT);
		glLoadIdentity();
		gluLookAt(camera.vp.x + r.x, camera.vp.y + r.y, camera.vp.z + r.z,
				camera.vp.x + r.x + camera.vd.x,
				camera.vp.y + r.y + camera.vd.y,
				camera.vp.z + r.z + camera.vd.z,
				camera.vu.x, camera.vu.y, camera.vu.z);
		do_draw();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -ratio * wd2 + 0.5 * camera.eyesep * ndfl;
		right = ratio * wd2 + 0.5 * camera.eyesep * ndfl;
		top = wd2;
		bottom = -wd2;
		glFrustum(left, right, bottom, top, near*2, far);

		//printf("left=%f right=%f bottom=%f top=%f\n", left, right, bottom, top);

		glMatrixMode(GL_MODELVIEW);
		glDrawBuffer(GL_BACK_LEFT);
		glLoadIdentity();
		gluLookAt(camera.vp.x - r.x, camera.vp.y - r.y, camera.vp.z - r.z,
				camera.vp.x - r.x + camera.vd.x,
				camera.vp.y - r.y + camera.vd.y,
				camera.vp.z - r.z + camera.vd.z,
				camera.vu.x, camera.vu.y, camera.vu.z);
		do_draw();

    } else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -ratio * wd2;
		right = ratio * wd2;
		top = wd2;
		bottom = -wd2;
		glFrustum(left, right, bottom, top, near, far);

		glMatrixMode(GL_MODELVIEW);
		glDrawBuffer(GL_BACK_LEFT);
		glLoadIdentity();
		gluLookAt(camera.vp.x, camera.vp.y, camera.vp.z,
				camera.vp.x + camera.vd.x,
				camera.vp.y + camera.vd.y,
				camera.vp.z + camera.vd.z,
				camera.vu.x, camera.vu.y, camera.vu.z);
		do_draw();
    }

    //glutSwapBuffers();
}

/* new window size or exposure */
static void
reshape(int width, int height)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
    camera.screenwidth = width;
    camera.screenheight = height;
}

int
main(int argc, char *argv[])
{
   int i;
   int datafile = GL_FALSE;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-stereo") == 0) {
         stereo = GL_TRUE;
      }
      else if (strcmp(argv[i], "-fullscreen") == 0) {
         fullscreen = GL_TRUE;
      }
      else if (strcmp(argv[i], "-f") == 0) {
		  if ( i+1 >= argc ) {
			  printf("-f: missing data file.\n");
			  return 1;
		  }
		  datafile = GL_TRUE;
		  break;
      }
      else {
		 printf("Warrning: unknown parameter: %s\n", argv[i]);
	  }
   }

   if ( 0 != tubes.loadGeometry( argv[argc-1] ) ) {
	   return 1;
   }
   tubes.getExtent( Xminmax, Yminmax, Zminmax );

   glutInit (&argc, argv);
   glutInitWindowPosition ( 0, 0 );
   glutInitWindowSize ( 1920, 1080 );
   if ( stereo ) {
	   glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO );
   }
   else {
	   glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
   }
   glutCreateWindow ( "stereo rendering - P. Bourke's correct method" );
   if ( fullscreen ) {
	   glutFullScreen();
   }
   initCamera();
   glutReshapeFunc(reshape);
   glutDisplayFunc(draw);
   glutKeyboardFunc(keyFunc);
   glutMainLoop() ;

   return 0;
}

