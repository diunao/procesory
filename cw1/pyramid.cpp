#include <GLTools.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>


GLuint shader;
float scale = 0.25f;

void changeSize (int w, int h) {
   glViewport (0, 0, w, h);
}

void setupRC () {
   glClearColor (0.0f, 0.0f, 1.0f, 1.0f);
   shader = gltLoadShaderPairWithAttributes ("shaders/pass_thru_shader.vp", "shaders/pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
}

void drawTriangle (float x0, float y0, float z0,
                   float x1, float y1, float z1, 
                   float x2, float y2, float z2, 
                   float r, float g, float b) {
   glVertexAttrib3f (GLT_ATTRIBUTE_COLOR, r, g, b);
   glVertex3f (x0, y0, z0);
   glVertex3f (x1, y1, z1);
   glVertex3f (x2, y2, z2);
}

void renderScene (void) {
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glUseProgram (shader);
   glBegin (GL_TRIANGLES);
   drawTriangle (scale * -1.0f, scale * 0.0f, scale * 0.0f,
                 scale * 0.0f, scale * -.5f, scale * 1.0f,
                 scale * 1.0f, scale * 0.0f, scale * 0.0f,
                 0.0f, 1.0f, 0.0f);
   drawTriangle (scale * 1.0f, scale * 0.0f, scale * 0.0f,
                 scale * 0.0f, scale * -.5f, scale * 1.0f,
                 scale * 2.0f, scale * -.5f, scale * 1.0f,
                 1.0f, 0.0f, 0.0f);
   drawTriangle (scale * -1.0f, scale * 0.0f, scale * 0.0f,
                 scale * 1.0f, scale * 0.0f, scale * 0.0f,
                 scale * .5f, scale * 1.5f, scale * .5f,
                 1.0f, 1.0f, 0.0f);
   drawTriangle (scale * 1.0f, scale * 0.0f, scale * 0.0f,
                 scale * 2.0f, scale * -.5f, scale * 1.0f,
                 scale * .5f, scale * 1.5f, scale * .5f,
                 1.0f, 0.0f, 1.0f);
   drawTriangle (scale * -1.0f, scale * 0.0f, scale * 0.0f,
                 scale * 0.0f, scale * -.5f, scale * 1.0f,
                 scale * 0.5f, scale * 1.5f, scale * 0.5f,
                 0.0f, 1.0f, 1.0f);
   // przedni trójk¹t, który mo¿na zakomentowaæ, aby zobaczyæ inne œcianki
   drawTriangle (scale * 0.0f, scale * -.5f, scale * 1.0f,
                 scale * 2.0f, scale * -.5f, scale * 1.0f,
                 scale * .5f, scale * 1.5f, scale * .5f,
                 1.0f, 1.0f, 1.0f);
   glEnd ();
   glutSwapBuffers ();
}

int main (int argc, char* argv[]) {
   glutInit (&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize (600, 600);
   glutCreateWindow ("Pyramid");
   glutReshapeFunc (changeSize);
   glutDisplayFunc (renderScene);
   GLenum err = glewInit ();
   if (GLEW_OK != err) {
      return 1;
   }
   setupRC ();
   glutMainLoop ();
   return 0;
}
