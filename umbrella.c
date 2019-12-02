#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define MAX_DEG 55.0
#define MIN_DEG 0.0

/* Globals */

/* Control points */
static GLuint number_control_points_v = 3;
static GLfloat control_points[3][3][3];
static GLUnurbsObj* nurb;
static GLboolean control_flag = GL_FALSE;

/* Degree variable */
enum Mode { ROTATION=1, TRANSLATION=3 } g_mode;
static GLfloat x_translation = 0.0;
static GLfloat y_translation = 0.0;
static GLfloat z_translation = 0.0;
static GLfloat x_rotation = -25.0;
static GLfloat y_rotation = 0.0;
static GLfloat z_rotation = 0.0;
static GLfloat scale = 1.0;
static GLfloat frame_deg = 55.0;
static GLuint frame_number = 12;

/* Fog */
static GLint fogMode;
static GLboolean fog_flag = GL_TRUE;

/* Texture */
static GLuint texture;

/* Blend */
static GLboolean blend_flag = GL_FALSE;

/* Animation */
static GLfloat move_flag = GL_FALSE;


static void calcTrans(GLfloat x, GLfloat y, GLfloat z,
		      GLfloat* point, GLfloat* result)
{
  result[0] = point[0] + x;
  result[1] = point[1] + y;
  result[2] = point[2] + z;
}

static void calcRotateY(GLfloat deg, GLfloat* point, GLfloat* result)
{
  GLfloat rad = deg/180*M_PI;
  result[0] = point[0]*cos(rad) - point[2]*sin(rad);
  result[1] = point[1];
  result[2] = point[0]*sin(rad) + point[2]*cos(rad);
}

static void calcRotateZ(GLfloat deg, GLfloat* point, GLfloat* result)
{
  GLfloat rad = deg/180*M_PI;
  result[0] = point[0]*cos(rad) + point[1]*sin(rad);
  result[1] = -point[0]*sin(rad) + point[1]*cos(rad);
  result[2] = point[2];
}

static void initControlPoints()
{
  unsigned int i, j;
  GLfloat deg, range;
  GLfloat points[3][3] = {
    {0.03, 2.0, 0.0},
    {0.4, 1.4, 0.0},
    {0.45, -0.7, 0.0}
  };
  GLfloat support_points[3][3] = {
    {0.01, 2.0, 0.0},
    {0.0, 1.4, 0.0},
    {0.0, -0.7, 0.0}
  };
  GLfloat tmp[3][3];

  /* Calclation of points (frame) */
  for(j = 0; j < 3; j++){
    calcTrans(0.0, -2.0, 0.0, points[j], points[j]);
    if(j == 1){
      range = 1.4*frame_deg/MAX_DEG;
      calcRotateZ(-frame_deg*range, points[j], tmp[j]);
    }
    else{
      calcRotateZ(-frame_deg, points[j], tmp[j]);
    }
    calcTrans(0.0, 2.0, 0.0, tmp[j], points[j]);
  }

  /* Calclation of points (surface) */
  for(j = 0; j < 3; j++){
    calcTrans(0.0, -2.0, 0.0, support_points[j], support_points[j]);
    if(j == 1){
      range = 1.6*frame_deg/MAX_DEG;
      calcRotateZ(-frame_deg*range, support_points[j], tmp[j]);
    }
    else{
      range = 1.2*frame_deg/MAX_DEG;
      calcRotateZ(-frame_deg*range, support_points[j], tmp[j]);
    }
    calcTrans(0.0, 2.0, 0.0, tmp[j], support_points[j]);
  }
  
  /* set control points */
  for(j = 0; j < 3; j++){
    calcRotateY(0, points[j], control_points[0][j]);
    calcRotateY(180/frame_number, support_points[j], control_points[1][j]);
    calcRotateY(360/frame_number, points[j], control_points[2][j]);
  }
}

static void initTexture(void)
{
  GLubyte data[64][64][3];
  int i, j, c;
  
  for (i = 0; i < 64; i++) {
    for (j = 0; j < 64; j++) {
      c = ((i/8)&1)^((j/8)&1);
      if(c == 0){ // white
	data[i][j][0] = 200;
	data[i][j][1] = 200;
	data[i][j][2] = 200;
      }
      else{ // grey
	data[i][j][0] = c * 255;
	data[i][j][1] = c * 255;
	data[i][j][2] = c * 255;
      }
    }
  }
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)data);
}

static void init (void)
{
  /* Initialization of light */
  GLfloat light_ambient[]={0.3, 0.3, 0.3, 1.0};
  GLfloat light_diffuse[]={1.0, 1.0, 1.0, 1.0};
  GLfloat light_specular[]={1.0, 1.0, 1.0, 1.0};
  GLfloat scene_ambient[]={0.2f, 0.2f, 0.2f};
  
  glClearColor(1.0, 1.0, 1.0, 1.0);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE );
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, scene_ambient);

  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_NORMALIZE);

  /* Initialization of NURBS surface */
  nurb = gluNewNurbsRenderer();
  gluNurbsProperty(nurb, GLU_SAMPLING_TOLERANCE, 25.0);
  gluNurbsProperty(nurb, GLU_DISPLAY_MODE, GLU_FILL);

  // Initialization of Texture
  initTexture();
  
  /* Initialization of fog */
  { 
    GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
    fogMode = GL_EXP;
    glFogi(GL_FOG_MODE, fogMode);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.3);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 1.0);
    glFogf(GL_FOG_END, 20.0); 
  }
}

static void plotPlane(void)
{
  GLfloat planeColor[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat planeSpecular[] = {0.3, 0.3, 0.3, 0.0};
  GLfloat planeAmbient[]= {0.3, 0.3, 0.3, 1.0};
  GLfloat planeShininess= {30.0};
  
  GLboolean texture2d;
  
  texture2d = glIsEnabled(GL_TEXTURE_2D);  
  if (texture2d != GL_TRUE) glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, planeColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, planeSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, planeAmbient);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, planeShininess);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glBegin(GL_QUADS);
  glNormal3f(0.0, 1.0, 0.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-4.0, -4.0, -4.0);
  glTexCoord2f(0.0, 1.5); glVertex3f(-4.0, -4.0,  4.0);
  glTexCoord2f(1.5, 1.5); glVertex3f( 4.0, -4.0,  4.0);
  glTexCoord2f(1.5, 0.0); glVertex3f( 4.0, -4.0, -4.0);
  glEnd();
  
  glBegin(GL_QUADS);
  glNormal3f(1.0, 0.0, 0.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-4.0, -4.0, -4.0);
  glTexCoord2f(0.0, 1.5); glVertex3f(-4.0,  4.0, -4.0);
  glTexCoord2f(1.5, 1.5); glVertex3f(-4.0,  4.0,  4.0);
  glTexCoord2f(1.5, 0.0); glVertex3f(-4.0, -4.0,  4.0);
  glEnd();
  
  glBegin(GL_QUADS);
  glNormal3f(0.0, 0.0, 1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-4.0, -4.0, -4.0);
  glTexCoord2f(0.0, 1.5); glVertex3f( 4.0, -4.0, -4.0);
  glTexCoord2f(1.5, 1.5); glVertex3f( 4.0,  4.0, -4.0);
  glTexCoord2f(1.5, 0.0); glVertex3f(-4.0,  4.0, -4.0);
  glEnd();
  
  if (texture2d != GL_TRUE) glDisable(GL_TEXTURE_2D);
}

static void plotControlPoints(void)
{
  unsigned int i, j;
  GLfloat deg = 360/frame_number;
  
  glDisable(GL_LIGHTING);
  
  glPointSize(5.0f);
  glColor3f(0.0, 1.0, 0.0);
  for ( j = 0; j < frame_number; ++j){
    glRotatef(deg, 0.0, 1.0, 0.0);
    glBegin(GL_POINTS);
    for (i = 0; i < number_control_points_v; ++i) {
      glVertex3fv(&control_points[0][i][0]);
    }
    glEnd();
  }
  
  glEnable(GL_LIGHTING);
}

static void plotControlPolygon(void)
{
  unsigned int i, j;
  GLfloat deg = 360/frame_number;

  glDisable(GL_LIGHTING);
  
  glLineWidth(4.0f);
  glColor3f(0.0, 0.0, 1.0);
  for ( j = 0; j < frame_number; ++j){
    glRotatef(deg, 0.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < number_control_points_v; ++i) {
      glVertex3fv(&control_points[0][i][0]);
    }
    glEnd();
  }

  for ( j = 0; j < 3; ++j){
    glRotatef(deg, 0.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < 3; ++i) {
      glVertex3fv(&control_points[j][i][0]);
    }
    glEnd();
  }

  glEnable(GL_LIGHTING);
}

/* Draw a canopy of umbrella */
static void plotNURBSSurface(void)
{
  GLfloat specular[] = {1.0, 1.0, 0.5, 0.7};
  GLfloat diffuse[] = {0.5, 0.5, 0.1, 0.7};
  GLfloat ambient[] = {0.8, 0.8, 0.4, 0.7};
  GLfloat shininess = {70.0};
  GLfloat deg = 360/frame_number;
  unsigned int i;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  GLfloat knots[] = {0.0,0.0,0.0,1.0,1.0,1.0};
  
  glPushMatrix();
  for(i = 0; i < frame_number; i++){
    glRotatef(deg, 0.0, 1.0, 0.0);
    gluBeginSurface(nurb);
    gluNurbsSurface(nurb, 6, knots, 6, knots,
		    9, 3, &control_points[0][0][0], 3, 3,
		    GL_MAP2_VERTEX_3);
    gluEndSurface(nurb);
  }
  glPopMatrix();
  
  glFlush();
}

/* Draw a frame of umbrella */
static void plotFramework(void)
{
  GLfloat mat_specular[]={0.5, 0.5, 0.5, 1.0};
  GLfloat mat_diffuse[]= {0.5, 0.5, 0.5, 1.0};
  GLfloat mat_ambient[]= {0.5, 0.5, 0.5, 1.0};
  GLfloat mat_shininess= {51.2};
  
  GLfloat mat_specular2[]={0.5, 0.5, 0.5, 1.0};
  GLfloat mat_diffuse2[]= {0.3, 0.3, 0.3, 1.0};
  GLfloat mat_ambient2[]= {0.3, 0.3, 0.3, 1.0};
  GLfloat mat_shininess2= {38.4};
  
  GLfloat deg = 360/frame_number, range;
  unsigned int i, j;
  GLUquadricObj *cylinder;

  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  /* Shank */
  glPushMatrix();
  glTranslatef(0.0, 2.0, 0.0);
  glRotatef(90, 1.0, 0.0, 0.0);
  cylinder = gluNewQuadric();
  gluQuadricDrawStyle(cylinder, GLU_FILL);
  gluCylinder(cylinder, 0.05, 0.05, 3, 10, 10);
  glPopMatrix();

  /* Handle */
  glPushMatrix();
  glTranslatef(0.0, -0.8, 0.0);
  glRotatef(90, 1.0, 0.0, 0.0);
  cylinder = gluNewQuadric();
  gluQuadricDrawStyle(cylinder, GLU_FILL);
  gluCylinder(cylinder, 0.08, 0.08, 0.3, 10, 10);
  glPopMatrix();
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular2);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient2);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse2);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess2);

  /* Top */
  glPushMatrix();
  glRotatef(-90, 1.0, 0.0, 0.0);
  glTranslatef(0.0, 0.0, 2.0);
  glutSolidCone(0.05, 0.2, 10, 10);
  glPopMatrix();

  /* Ring */
  glPushMatrix();
  range = frame_deg/MAX_DEG;
  glTranslatef(0.0, -0.4+1.8*range, 0.0);
  glRotatef(90, 1.0, 0.0, 0.0);
  cylinder = gluNewQuadric();
  gluQuadricDrawStyle(cylinder, GLU_FILL);
  gluCylinder(cylinder, 0.07, 0.07, 0.3, 10, 10); 
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -0.05, 0.0);
  for(i = 0; i < frame_number; i++){
    glRotatef(deg, 0.0, 1.0, 0.0);
    
    /* Tip */
    glPushMatrix();    
    glTranslatef(control_points[0][2][0],
		 control_points[0][2][1],
                 control_points[0][2][2]);
    glutSolidSphere(0.02,16,16);
    glPopMatrix();
  }
  
  glLineWidth(2.0f);
  glDisable(GL_LIGHTING);
  glColor3f(0.45,0.45,0.45);
  for(i = 0; i < frame_number; i++){
    glRotatef(deg, 0.0, 1.0, 0.0);
    
    /* Rib */
    glPushMatrix();
    glTranslatef(-0.02, 0.0, 0.0);
    glBegin (GL_LINE_STRIP);
    glEvalCoord1f(0.0);
    glEvalCoord1f(0.6);
    glEvalCoord1f(1.0);
    glEnd();
    glPopMatrix();
    
    /* Spreader */
    glBegin (GL_LINE_STRIP);
    glVertex3d(0.04, -0.2+1.8*range, 0.0);
    glEvalCoord1f(0.6);
    glEnd();
  }
  glPopMatrix();
  glEnable(GL_LIGHTING);
  
  gluDeleteQuadric(cylinder);
  
  glFlush();
}

static void display (void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  GLfloat light_position[]={10.0, 10.0, 5.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  
  initControlPoints();
  glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 3, &control_points[0][0][0]);
  glEnable(GL_MAP1_VERTEX_3);
  
  /* Draw planes */
  plotPlane();
  
  /* Screen adjustment */
  glPushMatrix();
  glScalef(scale, scale, scale);
  glTranslatef(x_translation,  y_translation, z_translation);
  glRotatef(x_rotation,1.0,0.0,0.0);
  glRotatef(y_rotation,0.0,1.0,0.0);
  glRotatef(z_rotation,0.0,0.0,1.0);
  
  /* Draw surface (canopy) */
  if(blend_flag){
    /* draw semitransparent */
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    plotNURBSSurface();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
  else{
    /* Draw standard */
    plotNURBSSurface();
  }

  /* Draw control points and polygon */
  if(control_flag){
    plotControlPoints();
    plotControlPolygon();
  }
  
  /* Draw a frame of umbrella */
  plotFramework();
  
  glPopMatrix();
  
  glutSwapBuffers();
}

static void reshape (int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, (GLfloat)w/(GLfloat)h, 1.0, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(7.0, 7.0, 7.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void timer(int value)
{
  static GLboolean isUp = GL_TRUE;

  if(move_flag){
    if(frame_deg <= MIN_DEG) isUp = GL_TRUE;
    else if(frame_deg >= MAX_DEG) isUp = GL_FALSE;
    frame_deg += (isUp == GL_TRUE ? 0.5 : -0.5);
    
    glutPostRedisplay();
    glutTimerFunc(50, timer, 0);
  }
}

static void handleKey(unsigned char key, int x, int y)
{
  switch(key) {
  case 27:
    exit(0);
    break;

    /* Change translation mode or rotation mode*/
  case 't':
    g_mode = TRANSLATION;
    break;

  case 'r':
    g_mode = ROTATION;
    break;

    /* Translation or rotation */
  case 'x':
    if (g_mode == TRANSLATION) {
      x_translation += 0.1f;
    }
    if (g_mode == ROTATION) {
      x_rotation += 5.0f;
    }
    glutPostRedisplay();
    break;
      
  case 'X':
    if (g_mode == TRANSLATION) {
      x_translation -= 0.1f;
    }
    if (g_mode == ROTATION) {
      x_rotation -= 5.0f;
    }
    glutPostRedisplay();
    break;
    
  case 'y':
    if (g_mode == TRANSLATION) {
      y_translation += 0.1f;
    }
    if (g_mode == ROTATION) {
      y_rotation += 5.0f;
    }
    glutPostRedisplay();
    break;

  case 'Y':
    if (g_mode == TRANSLATION) {
      y_translation -= 0.1f;
    }
    if (g_mode == ROTATION) {
      y_rotation -= 5.0f;
    }
    glutPostRedisplay();
    break;

  case 'z':
    if (g_mode == TRANSLATION) {
      z_translation += 0.1f;
    }
    if (g_mode == ROTATION) {
      z_rotation += 5.0f;
    }
    glutPostRedisplay();
    break;

  case 'Z':
    if (g_mode == TRANSLATION) {
      z_translation -= 0.1f;
    }
    if (g_mode == ROTATION) {
      z_rotation -= 5.0f;
    }
    glutPostRedisplay();
    break;

    /* Change degree of frame  */
  case 'o':
    if (frame_deg < MAX_DEG ) {
      frame_deg += 5.0f;
    }
    glutPostRedisplay();
    break;

  case 'O':
    if (frame_deg > MIN_DEG) {
      frame_deg -= 5.0f;
    }
    glutPostRedisplay();
    break;

    /* On or off animation mode */
  case 'm':
    move_flag = !move_flag;
    glutTimerFunc(50, timer, 0);
    break;

    /* change scale */
  case 's':
    scale += 0.1f;
    glutPostRedisplay();
    break;
    
  case 'S':
    scale -= 0.1f;
    glutPostRedisplay();
    break;

    /* On or off blend mode */
  case 'b':
    blend_flag = !blend_flag;
    glutPostRedisplay();
    break;

    /* Show or hide control points and polygons */
  case 'c':
    control_flag = !control_flag;
    glutPostRedisplay();
    break;

    /* Change fog mode */
  case 'f':
    if(fog_flag){
      glEnable(GL_FOG);
      fog_flag = GL_FALSE;
      fogMode = GL_EXP;
      glFogi(GL_FOG_MODE, fogMode);
    }
    else if(fogMode == GL_EXP){
      fogMode = GL_EXP2;
      glFogi(GL_FOG_MODE, fogMode);
    }  
    else if(fogMode == GL_EXP2){
      fogMode = GL_LINEAR;
      glFogi(GL_FOG_MODE, fogMode);
    }
    else{
      glDisable(GL_FOG);
      fog_flag = GL_TRUE;
    }
    glutPostRedisplay();
    break;
  }
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv); 
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
  glutInitWindowSize (480, 480);                
  glutCreateWindow("Umbrella"); 

  glutReshapeFunc(reshape); 
  glutDisplayFunc(display);
  glutKeyboardFunc(handleKey);

  init();
  
  glutTimerFunc(100, timer, 0);
  
  glutMainLoop(); 

  return EXIT_SUCCESS;
}

