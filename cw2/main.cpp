#include <GLFrame.h>
#include <GLTools.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math3d.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>

GLuint shader;
GLuint MVPMatrixLocation;
GLFrame cameraFrame;
GLFrustum viewFrustum;
float location[] = {0.0f, 0.0f, 0.0f};
float target[] = {0.0f, 0.0f, 0.0f};
float upDir[] = {0.0f, 0.0f, 1.0f};
float MVPMatrix[16];
float cameraMatrix[16];
CStopWatch timer;

void setUpFrame(GLFrame & frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
   frame.SetOrigin(origin);
   frame.SetForwardVector(forward);
   M3DVector3f side,oUp;
   m3dCrossProduct3(side,forward,up);
   m3dCrossProduct3(oUp,side,forward);
   frame.SetUpVector(oUp);
   frame.Normalize();
}

void lookAt(GLFrame & frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
   M3DVector3f forward;
   m3dSubtractVectors3(forward, at, eye);
   setUpFrame(frame, eye, forward, up);
}

void changeSize(int w, int h) {
   viewFrustum.SetPerspective(90.0f, (float)w / h, 1.0f, 200.0f);
   glViewport(0, 0, w, h);
}

void setupRC() {
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
   MVPMatrixLocation = glGetUniformLocation(shader, "MVPMatrix");
   glEnable(GL_CULL_FACE);
}

void renderScene(void) {
   float angle = timer.GetElapsedSeconds() * 3.14f;
   location[0] = 6.0f * cos(angle);
   location[1] = 6.0f * sin(angle);
   location[2] = 5.0f;
   lookAt(cameraFrame, location, target, upDir);
   cameraFrame.GetCameraMatrix(cameraMatrix);
   m3dMatrixMultiply44(MVPMatrix, viewFrustum.GetProjectionMatrix(), cameraMatrix);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glUseProgram(shader);
   glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, MVPMatrix);
   glBegin(GL_QUADS);
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 0.0f, 0.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
      glVertex3f(-1.0f, 1.0f, 0.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glVertex3f(1.0f, -1.0f, 0.0f);
   glEnd();
   glBegin(GL_TRIANGLES);
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f);
      glVertex3f(1.0f, -1.0f, 0.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glVertex3f(0.0f, 0.0f, 2.0f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glVertex3f(-1.0f, 1.0f, 0.0f);
      glVertex3f(0.0f, 0.0f, 2.0f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 1.0f, 1.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
      glVertex3f(0.0f, 0.0f, 2.0f);
      glVertex3f(-1.0f, 1.0f, 0.0f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
      glVertex3f(1.0f, -1.0f, 0.0f);
      glVertex3f(0.0f, 0.0f, 2.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
   glEnd();
   glutSwapBuffers();
   glutPostRedisplay();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("Rotating pyramid");
   glutReshapeFunc(changeSize);
   glutDisplayFunc(renderScene);
   GLenum err = glewInit();
   if(GLEW_OK != err) {
      return 1;
   }
   setupRC();
   glutMainLoop();
   return 0;
}
