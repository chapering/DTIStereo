/*
 * stereo rendering exemplifying the current implementation for the display study
 * hcai $Exp 06/08/2012
 *
 *
 * Acknowledgement:
 *	this method is coded on the basis of that written by Brian Paul for "glxgears" 
 *	and copyright info is:
 *		"Copyright (C) 1999-2001  Brian Paul   All Rights Reserved."
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include "GLoader.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif

static GLboolean fullscreen = GL_FALSE;	/* Create a single fullscreen window */
static GLboolean stereo = GL_FALSE;	/* Enable stereo.  */
static GLfloat eyesep = 1.0;		/* Eye separation. */
static GLfloat fix_point = 800;		/* Fixation point distance.  */
static GLfloat left, right, asp;	/* Stereo frustum params.  */

// bounding volume of the brain model
GLfloat Xminmax[2] = {50.774986, 182.695862};
GLfloat Yminmax[2] = {38.447567, 203.434158};
GLfloat Zminmax[2] = {5.271503, 135.014359};
CStreamtubes tubes;
int datafile = GL_FALSE;

GLint rot_x = 0, rot_y = 0, rot_z = 0;
GLfloat scale=2.0;

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

   if ( datafile ) tubes.draw();
   glutSwapBuffers();
}

static void
draw(void)
{
   if (stereo) {
	  float near = Zminmax[0]/4.0, far = (Zminmax[0]+Zminmax[1])/2.0 * 16.0;

      /* First left eye.  */
      glDrawBuffer(GL_BACK_LEFT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glFrustum(left, right, -asp, asp, near*2, far);

	  //printf("left=%f right=%f bottom=%f top=%f\n", left, right, -asp, asp);

      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glTranslated(+0.5 * eyesep, 0.0, 0.0);
      do_draw();
      glPopMatrix();

      /* Then right eye.  */
      glDrawBuffer(GL_BACK_RIGHT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glFrustum(-right, -left, -asp, asp, near*2, far);

	  //printf("left=%f right=%f bottom=%f top=%f\n", -right, -left, -asp, asp);

      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glTranslated(-0.5 * eyesep, 0.0, 0.0);
      do_draw();
      glPopMatrix();
   } else {
      do_draw();
   }
}

/* new window size or exposure */
static void
reshape(int width, int height)
{
   glViewport(0, 0, (GLint) width, (GLint) height);

   float near = Zminmax[0]/4.0, far = (Zminmax[0]+Zminmax[1])/2.0 * 16.0;

   if (stereo) {
      GLfloat w;
	  GLfloat wf = 50.0;
	  w = fix_point * (1.0 / wf);

      asp = (GLfloat) height / (GLfloat) width;

      left = -wf * ((w - 0.5 * eyesep) / fix_point);
      right = wf * ((w + 0.5 * eyesep) / fix_point);
   } else {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
	  gluPerspective(18, (GLfloat) width / (GLfloat) height,near, far);
   }
   
   int bFirstReshape = 1;
   if ( bFirstReshape ) {
	   glMatrixMode(GL_MODELVIEW);
	   glLoadIdentity();
	   gluLookAt(0, 0, (Zminmax[0]+Zminmax[1]) * 6.0,
			   0, 0, 0,
			   0, 1, 0);
	   bFirstReshape = 0;
   }
}

int
main(int argc, char *argv[])
{
   int i;

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

   if ( datafile ) {
	   if ( 0 != tubes.loadGeometry( argv[argc-1] ) ) {
		   return 1;
	   }
	   tubes.getExtent( Xminmax, Yminmax, Zminmax );
   }
   Zminmax[0]=6.27;

   glutInit (&argc, argv);
   glutInitWindowPosition ( 0, 0 );
   glutInitWindowSize ( 1920, 1080 );
   if ( stereo ) {
	   glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO );
   }
   else {
	   glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
   }
   glutCreateWindow ( "stereo rendering - following Brain Paul's implementation in glxgears" );
   if ( fullscreen ) {
	   glutFullScreen();
   }
   glutReshapeFunc(reshape);
   glutDisplayFunc(draw);
   glutKeyboardFunc(keyFunc);
   glutMainLoop() ;

   return 0;
}

