#include <GLTools.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>


GLuint shader;
float scale = 0.25f;

void changeSize(int w, int h) {
   glViewport(0, 0, w, h);
}

void setupRC() {
   glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
   shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
}

void renderScene (void) {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glUseProgram(shader);
   glBegin(GL_TRIANGLES);
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 1.0f, 0.0f);
      glVertex3f(scale * -1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * 0.0f, scale * -.5f, scale * 1.0f);
      glVertex3f(scale * 1.0f, scale * 0.0f, scale * 0.0f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 0.0f, 0.0f);
      glVertex3f(scale * 1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * 0.0f, scale * -.5f, scale * 1.0f);
      glVertex3f(scale * 2.0f, scale * -.5f, scale * 1.0f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f);
      glVertex3f(scale * -1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * 1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * .5f, scale * 1.5f, scale * .5f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f);
      glVertex3f(scale * -1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * 1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * .5f, scale * 1.5f, scale * .5f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 0.0f, 1.0f);
      glVertex3f(scale * 1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * 2.0f, scale * -.5f, scale * 1.0f);
      glVertex3f(scale * .5f, scale * 1.5f, scale * .5f);
      //--
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 1.0f, 1.0f);
      glVertex3f(scale * -1.0f, scale * 0.0f, scale * 0.0f);
      glVertex3f(scale * 0.0f, scale * -.5f, scale * 1.0f);
      glVertex3f(scale * 0.5f, scale * 1.5f, scale * 0.5f);
      //przedni trójkąt, który można zakomentować, aby zobaczyć inne ścianki
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
      glVertex3f(scale * 0.0f, scale * -.5f, scale * 1.0f);
      glVertex3f(scale * 2.0f, scale * -.5f, scale * 1.0f);
      glVertex3f(scale * .5f, scale * 1.5f, scale * .5f);
      //--
   glEnd();
   glutSwapBuffers();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("Pyramid");
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
