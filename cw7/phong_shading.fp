#version 330

struct PointLight {
   // the light position in eye space
   vec3 position;
   vec3 intensity_diffuse;
   vec3 intensity_specular;
   // attenuation = 1 / (attenuation[0] + attenuation[1] * distance_to_light + attenuation[2] * pow(distance_to_light, 2))
   vec3 attenuation;
};

struct Material {
   float ka;    // the ratio of reflection of the ambient term present in all points in the scene rendered, [0, 1]
   float kd;    // the ratio of reflection of the diffuse term of incoming light, [0, 1]
   float ks;    // the ratio of reflection of the specular term of incoming light, [0, 1]
   float alpha; // shininess constant, which is larger for surfaces that are smoother and more mirror-like. When this constant is large the specular highlight is small
};

in vec3 vertex_position_eye_space;
in vec3 vertex_normal_eye_space;
in vec2 texture_coordinates;

out vec4 color;

uniform vec3 intensity_ambient_component;
uniform sampler2D texture_unit;
uniform PointLight light_0;
uniform Material material_0;

void main() {
   vec3 vector_to_light = light_0.position - vertex_position_eye_space;
   float distance_to_light = length(vector_to_light);
   vector_to_light = normalize(vector_to_light);
   //-- AMBIENT
   color = material_0.ka * vec4(intensity_ambient_component, 1);
   //-- DIFFUSE
   float attenuation = 1.0 / (light_0.attenuation[0] + light_0.attenuation[1] * distance_to_light + light_0.attenuation[2] * pow(distance_to_light, 2));
   float NdotL = dot(vector_to_light, vertex_normal_eye_space);
   // max() --> the diffuse component cannot be negative
   color += material_0.kd * texture2D(texture_unit, texture_coordinates) * max(NdotL, 0.0) * vec4(light_0.intensity_diffuse, 1) * attenuation;
   //-- SPECULAR (Blinn-Phong)
   // NdotL > 0.0 --> we see specular highlights only if we are looking at the front face
   if(NdotL > 0.0) {
      // camera at (0, 0, 0)
      vec3 H = normalize(vector_to_light - vertex_position_eye_space);
      color += material_0.ks * vec4(light_0.intensity_specular, 1) * pow(dot(H, vertex_normal_eye_space), material_0.alpha) * attenuation;
   }
}