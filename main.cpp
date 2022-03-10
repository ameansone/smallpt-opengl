const char *vsc  = R"(#version 330 core
layout (location = 0) in vec2 pos;
void main() { gl_Position = vec4(pos, 0, 1); })";
const char *fosc = R"(#version 330 core
out vec4 oc; uniform sampler2D tex;
void main() { oc = texture(tex, gl_FragCoord.xy/vec2(1024, 768)); })";
const char *fsc  = R"(#version 330 core
out vec4 oc;uniform int samps;uniform sampler2D tex;float pi=3.141592653589793;
vec2 xy = gl_FragCoord.xy, wh = vec2(1024, 768); float rc=0;
struct ray { vec3 o, d; }; struct sphere { float r; vec3 p, c; int t; };
sphere s[] = sphere[](//Scene: radius, position, color/emission, material
  sphere(1e5, vec3( 1e5+1,40.8,81.6), vec3(.75,.25,.25),1), //Left
  sphere(1e5, vec3(-1e5+99,40.8,81.6),vec3(.25,.25,.75),1), //Rght
  sphere(1e5, vec3(50,40.8, 1e5),     vec3(.75,.75,.75),1), //Back
  sphere(1e5, vec3(50,40.8,-1e5+170), vec3(0, 0, 0),    1), //Frnt
  sphere(1e5, vec3(50, 1e5, 81.6),    vec3(.75,.75,.75),1), //Botm
  sphere(1e5, vec3(50,-1e5+81.6,81.6),vec3(.75,.75,.75),1), //Top
  sphere(16.5,vec3(27,16.5,47),       vec3(1,1,1)*.999, 2), //Mirr
  sphere(16.5,vec3(73,16.5,78),       vec3(1,1,1)*.999, 3), //Glas
  sphere(600, vec3(50,681.6-.27,81.6),vec3(12, 12, 12), 0));//Lite
vec3 gamma(vec3 c,float r) { return vec3(pow(c.r,r), pow(c.g,r), pow(c.b,r)); }
float rand(float a, float b) {return fract(cos(a*12.9898+b*4.1414)*43758.5453);}
float rand() { return rand(samps, rand(rc++, rand(xy.x, xy.y))); }
float is(in sphere s, in ray r) { vec3 op = s.p-r.o; float t, eps = 1e-4;
  float b=dot(r.d,op), det=b*b-dot(op, op)+s.r*s.r; if (det < 0) return 0;
  det = sqrt(det); return eps<(t=b-det) ? t : (eps<(t=b+det) ? t : 0); }
bool intersect(in ray r, out float t, out int si) { float d, inf = t = 1e20;
  for (int i=0; i<s.length(); ++i) if (0<(d=is(s[i],r)) && d<t) { t=d; si=i; }
  return t < inf; }
vec3 radiance(in ray r) { int si; float t; vec3 rc = vec3(1, 1, 1);
  for(int i = 0; true; ++i) { if (!intersect(r, t, si)) { rc *= 0; break; }
    if (s[si].t == 0) { rc *= s[si].c; break; }
    float P = clamp(max(s[si].c.r, max(s[si].c.g, s[si].c.b)), 0, .75);
    if (5 < i) { if (rand() < P) rc /= P; else { rc *= 0; break; } }
    vec3 p = r.o+t*r.d, n = normalize(p-s[si].p); bool into = (dot(r.d,n)<0);
    n *= (into?1:-1); r.o = p + .1 * n;
    if (s[si].t == 1) {rc*=s[si].c; float r1=2*pi*rand(),r2=rand(),r2s=sqrt(r2);
      vec3 u=normalize(cross(n, abs(n.x)<0.1 ? vec3(1,0,0) : vec3(0,1,0)));
      vec3 v=cross(u,n); r.d=(cos(r1)*r2s*u+sin(r1)*r2s*v+sqrt(1-r2)*n);
    } else if (s[si].t == 2) { r.d = reflect(r.d, n);
    } else { float nc=1.0, nt=1.5, ddn=dot(r.d, n), nnt = into ? nc/nt : nt/nc;
      float cos2t = 1-nnt*nnt*(1-ddn*ddn);
      if (cos2t < 0) { r.d = reflect(r.d, n); continue; }
      vec3 td = normalize(r.d * nnt - n * (ddn*nnt+sqrt(cos2t)));
      float a = nt-nc, b = nt+nc, c = 1+(into?ddn:dot(td, n));
      float R0=(a*a)/(b*b), Re=R0+(1-R0)*c*c*c*c*c, Tr=1-Re;
      float P =0.25+0.5*Re, RP=(i<3)?Re*2:Re/P,     TP=(i<3)?Tr*2:Tr/(1-P);
      if (i<3?rand()<0.5:rand()<P) { r.d=reflect(r.d,n); rc *= RP; }
      else { r.d = td; rc *= TP; r.o -= .2 * n; }
    }
  } return rc;}
void main() {ray cam = ray(vec3(50,52,295.6),normalize(vec3(0,-0.042612,-1)));
  vec3 cx=vec3(wh.x/wh.y,0,0)/2,cy=normalize(cross(cx,cam.d))/2,tc=vec3(0,0,0);
  for(float y=.5;y<2;++y) for(float x=.5;x<2;++x) {vec2 r=vec2(rand(),rand())*2;
      r.x=r.x<1?sqrt(r.x)-1:1-sqrt(2-r.x); r.y=r.y<1?sqrt(r.y)-1:1-sqrt(2-r.y);
      vec2 f=(xy+(vec2(x,y)+r)/2)/wh-0.5; vec3 d = f.x*cx + f.y*cy + cam.d;
      tc += radiance(ray(cam.o+d*140, normalize(d))) / 4;
  } vec3 pc = gamma(texture(tex, xy/wh).rgb, 2.2);
  oc = vec4(gamma((1.0/samps) * tc + ((samps-1.0)/samps) * pc, 1/2.2), 1);
})";
#include <glad/glad.h>
#include <GLFW/glfw3.h>
int main() {
  GLuint hd[8],samps=1,w=1024,h=768;GLfloat v[]={1,1,1,-1,-1,1,-1,-1,1,-1,-1,1};
  glfwInit();
  GLFWwindow* wd = glfwCreateWindow(w, h, "smallpt", 0, 0);
  glfwMakeContextCurrent(wd);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glViewport(0, 0, w, h);
  glGenBuffers     (1, hd+0); glBindBuffer     (GL_ARRAY_BUFFER, hd[0]);
  glGenTextures    (1, hd+1); glBindTexture    (GL_TEXTURE_2D,   hd[1]);
  glGenFramebuffers(1, hd+2); glBindFramebuffer(GL_FRAMEBUFFER,  hd[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
  hd[2], 0);
  hd[3] = glCreateShader(GL_VERTEX_SHADER);  glShaderSource(hd[3],1,&vsc, NULL);
  hd[4] = glCreateShader(GL_FRAGMENT_SHADER);glShaderSource(hd[4],1,&fsc, NULL);
  hd[5] = glCreateShader(GL_FRAGMENT_SHADER);glShaderSource(hd[5],1,&fosc,NULL);
  hd[6] = glCreateProgram();
  glAttachShader(hd[6],hd[3]);glAttachShader(hd[6],hd[4]); glLinkProgram(hd[6]);
  hd[7] = glCreateProgram();
  glAttachShader(hd[7],hd[3]);glAttachShader(hd[7],hd[5]); glLinkProgram(hd[7]);
  while (!glfwWindowShouldClose(wd)) {
      glBindFramebuffer(GL_FRAMEBUFFER, hd[2]);
      glUseProgram(hd[6]);
      glUniform1i(glGetUniformLocation(hd[6], "samps"), samps++);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glUseProgram(hd[7]);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glfwSwapBuffers(wd);
      glfwPollEvents();
  }
  glfwTerminate();
}
