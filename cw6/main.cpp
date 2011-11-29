#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLTools.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math3d.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>
#include <vector>

struct PointLight {
   float position[3];
   float intensity_diffuse[3];
   float intensity_specular[3];
   float attenuation[3];

   void set_position(float x, float y, float z) {
      position[0] = x;
      position[1] = y;
      position[2] = z;
   }

   void set_intensity_diffuse(float r, float g, float b) {
      intensity_diffuse[0] = r;
      intensity_diffuse[1] = g;
      intensity_diffuse[2] = b;
   }

   void set_intensity_specular(float r, float g, float b) {
      intensity_specular[0] = r;
      intensity_specular[1] = g;
      intensity_specular[2] = b;
   }

   // attenuation = 1 / (attenuation[0] + attenuation[1] * distance_to_light + attenuation[2] * pow(distance_to_light, 2))
   void set_attenuation(float attenuation_0, float attenuation_1, float attenuation_2) {
      attenuation[0] = attenuation_0;
      attenuation[1] = attenuation_1;
      attenuation[2] = attenuation_2;
   }
};

struct Material {
   float ka;    // the ratio of reflection of the ambient term present in all points in the scene rendered, [0, 1]
   float kd;    // the ratio of reflection of the diffuse term of incoming light, [0, 1]
   float ks;    // the ratio of reflection of the specular term of incoming light, [0, 1]
   float alpha; // shininess constant, which is larger for surfaces that are smoother and more mirror-like. When this constant is large the specular highlight is small

   void set_parameters(float ka, float kd, float ks, float alpha) {
      this->ka = ka;
      this->kd = kd;
      this->ks = ks;
      this->alpha = alpha;
   }
};

GLuint shader_light,
       shader_color,
       mvp_matrix_location_shader_color,
       mvp_matrix_location,
       mv_matrix_location,
       normal_matrix_location,
       intensity_ambient_component_location,
       light_0_position_location,
       light_0_intensity_diffuse_location,
       light_0_intensity_specular_location,
       light_0_attenuation_location,
       material_0_ka_location,
       material_0_kd_location,
       material_0_ks_location,
       material_0_alpha_location;
GLGeometryTransform geometry_pipeline;
GLMatrixStack p_stack;
GLMatrixStack mv_stack;
GLFrame camera_frame;
GLFrustum view_frustum;
float location[] = {0.0f, 0.0f, 0.0f}, target[] = {0.0f, 0.0f, 0.0f}, up_dir[] = {0.0f, 0.0f, 1.0f}, camera_matrix[16], intensity_ambient_component[] = {0.0f, 0.0f, 0.0f}, light_position_eye_space[3];
CStopWatch timer;
PointLight light_0;
Material material_0;
std::vector<float> vertices;
std::vector<unsigned int> faces;
GLuint vertex_buffer;
GLuint faces_buffer;

void load_vertices(const char * filename, std::vector<float> & out_vertices) {
   FILE * fvertices = fopen(filename, "r");
   char line[120];
   float x, y, z, nx, ny, nz;
   while (fgets(line, 120, fvertices) != NULL) {
      sscanf(line,"%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz);
      out_vertices.push_back(x);
      out_vertices.push_back(y);
      out_vertices.push_back(z);
      out_vertices.push_back(nx);
      out_vertices.push_back(ny);
      out_vertices.push_back(nz);
   }
}

void load_faces(const char * filename, std::vector<unsigned int> & out_faces) {
   FILE * ffaces = fopen(filename, "r");
   char line[120];
   int trash, i, j, k;
   while (fgets(line, 120, ffaces) != NULL) {
      sscanf(line, "%u %u %u %u", &trash, &i, &j, &k);
      out_faces.push_back(i);
      out_faces.push_back(j);
      out_faces.push_back(k);
   }
}

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
   load_vertices("dragon_vertices.dat", vertices);
   load_faces("dragon_faces.dat", faces);
   glGenBuffers(1, &vertex_buffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
   glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const GLvoid *)0);
   glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const GLvoid *)(3 * sizeof(float)));
   glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
   glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
   //---
   glGenBuffers(1, &faces_buffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_buffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLuint), &faces[0], GL_STATIC_DRAW);
   //---
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   shader_light = gltLoadShaderPairWithAttributes("phong_shading.vp", "phong_shading.fp", 3, GLT_ATTRIBUTE_VERTEX, "vertex_position", GLT_ATTRIBUTE_NORMAL, "vertex_normal");
   shader_color = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vertex_position", GLT_ATTRIBUTE_COLOR, "vertex_color");
   mvp_matrix_location_shader_color = glGetUniformLocation(shader_color, "mvp_matrix");
   mvp_matrix_location = glGetUniformLocation(shader_light, "mvp_matrix");
   mv_matrix_location = glGetUniformLocation(shader_light, "mv_matrix");
   normal_matrix_location = glGetUniformLocation(shader_light, "normal_matrix");
   intensity_ambient_component_location = glGetUniformLocation(shader_light, "intensity_ambient_component");
   light_0_position_location = glGetUniformLocation(shader_light, "light_0.position");
   light_0_intensity_diffuse_location = glGetUniformLocation(shader_light, "light_0.intensity_diffuse");
   light_0_intensity_specular_location = glGetUniformLocation(shader_light, "light_0.intensity_specular");
   light_0_attenuation_location = glGetUniformLocation(shader_light, "light_0.attenuation");
   material_0_ka_location = glGetUniformLocation(shader_light, "material_0.ka");
   material_0_kd_location = glGetUniformLocation(shader_light, "material_0.kd");
   material_0_ks_location = glGetUniformLocation(shader_light, "material_0.ks");
   material_0_alpha_location = glGetUniformLocation(shader_light, "material_0.alpha");
   light_0.set_position(0.0f, 0.0f, 0.0f);
   light_0.set_intensity_diffuse(1.0f, 1.0f, 1.0f);
   light_0.set_intensity_specular(1.0f, 0.913f, 0.8f);
   light_0.set_attenuation(0.0f, 0.1f, 0.0f);
   material_0.set_parameters(1.0f, 1.0f, 1.0f, 200.0f);
   geometry_pipeline.SetMatrixStacks(mv_stack, p_stack);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
   glFrontFace(GL_CCW);
}

void draw_trinagle_face_no_normal(const float A[], const float B[], const float C[], float r = 1.0f, float g = 1.0f, float b = 1.0f) {
   glBegin(GL_TRIANGLES);
      glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
      glVertex3fv(A);
      glVertex3fv(B);
      glVertex3fv(C);
   glEnd();
}

void fill_array_3(float out_array[], float a0, float a1,float a2) {
   out_array[0] = a0;
   out_array[1] = a1;
   out_array[2] = a2;
}

void draw_light() {
   float A[3], B[3], C[3], normal[3];
   // bottom
   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, -1.0f, -1.0f, 0.0f);
   fill_array_3(C, 1.0f, 1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 1.0f, 0.0f);
   fill_array_3(A, -1.0f, 1.0f, 0.0f);
   fill_array_3(B, 1.0f, 1.0f, 0.0f);
   fill_array_3(C, -1.0f, -1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 1.0f, 0.0f);
   //// rest
   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, 1.0f, 1.0f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 2.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 1.0f, 0.0f);
   //--
   fill_array_3(A, 1.0f, 1.0f, 0.0f);
   fill_array_3(B, -1.0f, 1.0f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 2.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 1.0f, 0.0f);
   //--
   fill_array_3(A, -1.0f, -1.0f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 2.0f);
   fill_array_3(C, -1.0f, 1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 1.0f, 0.0f);
   //--
   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 2.0f);
   fill_array_3(C, -1.0f, -1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 1.0f, 0.0f);
}

void render_scene(void) {
   float angle = timer.GetElapsedSeconds() * 3.14f / 10.0f;
   location[0] = -10.0f * cos(angle / 2.0f);
   location[1] = -10.0f * sin(angle / 2.0f);
   location[2] = 5.0f;
   light_0.position[0] = 10.0f * cos(-angle);
   light_0.position[1] = 10.0f * sin(-angle);
   light_0.position[2] = 3.0f;
   look_at(camera_frame, location, target, up_dir);
   camera_frame.GetCameraMatrix(camera_matrix);
   p_stack.LoadMatrix(view_frustum.GetProjectionMatrix());
   mv_stack.LoadMatrix(camera_matrix);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   //--
   glUseProgram(shader_color);
   mv_stack.PushMatrix();
   mv_stack.Translate(light_0.position[0], light_0.position[1], light_0.position[2]);
   mv_stack.Scale(0.25f, 0.25f, 0.25f);
   glUniformMatrix4fv(mvp_matrix_location_shader_color, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   draw_light();
   mv_stack.PopMatrix();
   //--
   glUseProgram(shader_light);
   glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, geometry_pipeline.GetNormalMatrix());
   glUniform3fv(intensity_ambient_component_location, 1, intensity_ambient_component);
   m3dTransformVector3(light_position_eye_space, light_0.position, camera_matrix);
   glUniform3fv(light_0_position_location, 1, light_position_eye_space);
   glUniform3fv(light_0_intensity_diffuse_location, 1, light_0.intensity_diffuse);
   glUniform3fv(light_0_intensity_specular_location, 1, light_0.intensity_specular);
   glUniform3fv(light_0_attenuation_location, 1, light_0.attenuation);
   glUniform1f(material_0_ka_location, material_0.ka);
   glUniform1f(material_0_kd_location, material_0.kd);
   glUniform1f(material_0_ks_location, material_0.ks);
   glUniform1f(material_0_alpha_location, material_0.alpha);
   //--
   mv_stack.PushMatrix();
   glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(mv_matrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
   glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);
   mv_stack.PopMatrix();
   //--
   glUseProgram(0);
   glutSwapBuffers();
   glutPostRedisplay();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("cw5");
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
