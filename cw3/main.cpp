#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLTools.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math3d.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>

GLuint shader, mvp_matrix_location;
//GLMatrixStack matrix_stack;
GLGeometryTransform geometry_pipeline;
GLMatrixStack p_stack;
GLMatrixStack mv_stack;
GLFrame camera_frame;
GLFrustum view_frustum;
float location[] = {0.0f, 0.0f, 0.0f}, target[] = {0.0f, 0.0f, 0.0f}, up_dir[] = {0.0f, 0.0f, 1.0f},
                    //mvp_matrix[16],
                    //vp_matrix[16],
                    //model_matrix[16],
                    camera_matrix[16];
CStopWatch timer;

void set_up_frame(GLFrame & frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
   frame.SetOrigin(origin);
   frame.SetForwardVector(forward);
   M3DVector3f side, o_up;
   m3dCrossProduct3(side, forward, up);
   m3dCrossProduct3(o_up, side, forward);
   frame.SetUpVector(o_up);
   frame.Normalize();
}

void look_at(GLFrame & frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
   M3DVector3f forward;
   m3dSubtractVectors3(forward, at, eye);
   set_up_frame(frame, eye, forward, up);
}

void change_size(int w, int h) {
   view_frustum.SetPerspective(90.0f, (float)w / h, 1.0f, 200.0f);
   glViewport(0, 0, w, h);
}

void setup_rc() {
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vertex_position", GLT_ATTRIBUTE_COLOR, "vertex_color");
   mvp_matrix_location = glGetUniformLocation(shader, "mvp_matrix");
   geometry_pipeline.SetMatrixStacks(mv_stack, p_stack);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
}

void draw_pyramid() {
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
}

void draw_grid() {
   for(int i = -10; i <= 10; ++i) {
      glBegin(GL_LINES);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
		glVertex3f(-10.0f, (float)i, 0.0f);
		glVertex3f(10.0f, (float)i, 0.0f);
	  glEnd();
	  glBegin(GL_LINES);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
		glVertex3f((float)i, -10.0f, 0.0f);
		glVertex3f((float)i, 10.0f, 0.0f);
	  glEnd();
   }
}

void render_scene(void) {
   float angle = timer.GetElapsedSeconds() * 3.14f / 6.0f;
   location[0] = -20.0f * cos(angle);
   location[1] = -20.0f * sin(angle);
   location[2] = 5.0f;
   look_at(camera_frame, location, target, up_dir);
   camera_frame.GetCameraMatrix(camera_matrix);
   //--
   //m3dMatrixMultiply44(vp_matrix, view_frustum.GetProjectionMatrix(), camera_matrix);
   //matrix_stack.LoadMatrix(view_frustum.GetProjectionMatrix());
   //matrix_stack.MultMatrix(camera_matrix);
   p_stack.LoadMatrix(view_frustum.GetProjectionMatrix());
   mv_stack.LoadMatrix(camera_matrix);
   //--
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glUseProgram(shader);
   //--
   glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   draw_grid();
   //--
   //m3dTranslationMatrix44(model_matrix, 0.0f, 0.0, -1.0f);
   //float scale[] = {1.5f, 1.5f, 1.5f};
   //m3dScaleMatrix44(model_matrix, scale);
   //m3dRotationMatrix44(model_matrix, -angle * 2.0f, 1.0f, 0.0f, 0.0f);
   //m3dMatrixMultiply44(mvp_matrix, vp_matrix, model_matrix);
   //matrix_stack.PushMatrix();
   //matrix_stack.Rotate(m3dRadToDeg(-angle), 1.0f, 0.0f, 0.0f);
   mv_stack.PushMatrix();
   mv_stack.Rotate(m3dRadToDeg(-angle), 1.0f, 0.0f, 0.0f);
   glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   draw_pyramid();
   //matrix_stack.PopMatrix();
   mv_stack.PopMatrix();
   //--
   //m3dTranslationMatrix44(model_matrix, 5.0f, 0.0, 0.0f);
   //m3dMatrixMultiply44(mvp_matrix, vp_matrix, model_matrix);
   //matrix_stack.Translate(5.0f, 0.0f, 0.0f);
   mv_stack.Translate(5.0f, 0.0f, 0.0f);
   glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   draw_pyramid();
   //--
   glutSwapBuffers();
   glutPostRedisplay();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("cw3");
   glutReshapeFunc(change_size);
   glutDisplayFunc(render_scene);
   GLenum err = glewInit();
   if(GLEW_OK != err) {
      return 1;
   }
   setup_rc();
   glutMainLoop();
   return 0;
}
