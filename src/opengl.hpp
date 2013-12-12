#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <fstream> // ifstream
#include <iostream> // cerr
#include <unordered_map>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h> // XRenderFindVisualFormat
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

struct GLXFBConfigPrintAdapter {
  GLXFBConfigPrintAdapter(Display * const dpy, const GLXFBConfig & config)
    : m_dpy(dpy), m_config(config)
  {}

  Display * const m_dpy;
  const GLXFBConfig & m_config;
};

namespace gl {

class api {
  public:
    api(void)
    {
      glXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)
        glXGetProcAddress((const GLubyte *)"glXBindTexImageEXT");
      glXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)
        glXGetProcAddress((const GLubyte *)"glXReleaseTexImageEXT");
      glXGetFBConfigs = (PFNGLXGETFBCONFIGSPROC)
        glXGetProcAddress((const GLubyte *)"glXGetFBConfigs");
      glCreateShader = (PFNGLCREATESHADERPROC)
        glXGetProcAddress((const GLubyte *)"glCreateShader");
      glDeleteShader = (PFNGLDELETESHADERPROC)
        glXGetProcAddress((const GLubyte *)"glDeleteShader");
      glShaderSource = (PFNGLSHADERSOURCEPROC)
        glXGetProcAddress((const GLubyte *)"glShaderSource");
      glCompileShader = (PFNGLCOMPILESHADERPROC)
        glXGetProcAddress((const GLubyte *)"glCompileShader");
      glCreateProgram = (PFNGLCREATEPROGRAMPROC)
        glXGetProcAddress((const GLubyte *)"glCreateProgram");
      glDeleteProgram = (PFNGLDELETEPROGRAMPROC)
        glXGetProcAddress((const GLubyte *)"glDeleteProgram");
      glAttachShader = (PFNGLATTACHSHADERPROC)
        glXGetProcAddress((const GLubyte *)"glAttachShader");
      glDetachShader = (PFNGLDETACHSHADERPROC)
        glXGetProcAddress((const GLubyte *)"glDetachShader");
      glLinkProgram = (PFNGLLINKPROGRAMPROC)
        glXGetProcAddress((const GLubyte *)"glLinkProgram");
      glUseProgram = (PFNGLUSEPROGRAMPROC)
        glXGetProcAddress((const GLubyte *)"glUseProgram");
      glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)
        glXGetProcAddress((const GLubyte *)"glGetProgramInfoLog");
      glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)
        glXGetProcAddress((const GLubyte *)"glGetShaderInfoLog");
      glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)
        glXGetProcAddress((const GLubyte *)"glGenerateMipmap");
      glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)
        glXGetProcAddress((const GLubyte *)"glGenerateMipmapEXT");
      glActiveTextureEXT = (PFNGLACTIVETEXTUREPROC)
        glXGetProcAddress((const GLubyte *)"glActiveTexture");
      glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)
        glXGetProcAddress((const GLubyte *)"glBlendFuncSeparate");

      glGenSamplers = (PFNGLGENSAMPLERSPROC)
        glXGetProcAddress((const GLubyte *)"glGenSamplers");
      glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)
        glXGetProcAddressARB((const GLubyte *)"glDeleteSamplers");
      glBindSampler = (PFNGLBINDSAMPLERPROC)
        glXGetProcAddressARB((const GLubyte *)"glBindSampler");

      glUniform1f = (PFNGLUNIFORM1FPROC)
        glXGetProcAddress((const GLubyte *)"glUniform1f");
      glUniform2f = (PFNGLUNIFORM2FPROC)
        glXGetProcAddress((const GLubyte *)"glUniform2f");
      glUniform3f = (PFNGLUNIFORM3FPROC)
        glXGetProcAddress((const GLubyte *)"glUniform3f");
      glUniform4f = (PFNGLUNIFORM4FPROC)
        glXGetProcAddress((const GLubyte *)"glUniform4f");

      glUniform1i = (PFNGLUNIFORM1IPROC)
        glXGetProcAddress((const GLubyte *)"glUniform1i");
      glUniform2i = (PFNGLUNIFORM2IPROC)
        glXGetProcAddress((const GLubyte *)"glUniform2i");
      glUniform3i = (PFNGLUNIFORM3IPROC)
        glXGetProcAddress((const GLubyte *)"glUniform3i");
      glUniform4i = (PFNGLUNIFORM4IPROC)
        glXGetProcAddress((const GLubyte *)"glUniform4i");

      glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)
        glXGetProcAddress((const GLubyte *)"glGetUniformLocation");
      glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)
        glXGetProcAddress((const GLubyte *)"glGetUniformBlockIndex");
      glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)
        glXGetProcAddress((const GLubyte *)"glUniformBlockBinding");

      glGetInteger64v = (PFNGLGETINTEGER64VPROC)
        glXGetProcAddress((const GLubyte *)"glGetInteger64v");

      glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)
        glXGetProcAddress((const GLubyte *)"glBlendFuncSeparateEXT");
      glBlendEquationSeparateEXT = (PFNGLBLENDEQUATIONSEPARATEEXTPROC)
        glXGetProcAddress((const GLubyte *)"glBlendEquationSeparateEXT");

      glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");
    }

    PFNGLXBINDTEXIMAGEEXTPROC         glXBindTexImageEXT         = 0;
    PFNGLXRELEASETEXIMAGEEXTPROC      glXReleaseTexImageEXT      = 0;
    PFNGLXGETFBCONFIGSPROC            glXGetFBConfigs            = 0;
    PFNGLCREATESHADERPROC             glCreateShader             = 0;
    PFNGLDELETESHADERPROC             glDeleteShader             = 0;
    PFNGLSHADERSOURCEPROC             glShaderSource             = 0;
    PFNGLCOMPILESHADERPROC            glCompileShader            = 0;
    PFNGLCREATEPROGRAMPROC            glCreateProgram            = 0;
    PFNGLDELETEPROGRAMPROC            glDeleteProgram            = 0;
    PFNGLATTACHSHADERPROC             glAttachShader             = 0;
    PFNGLDETACHSHADERPROC             glDetachShader             = 0;
    PFNGLLINKPROGRAMPROC              glLinkProgram              = 0;
    PFNGLUSEPROGRAMPROC               glUseProgram               = 0;
    PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog        = 0;
    PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog         = 0;
    PFNGLGENERATEMIPMAPPROC           glGenerateMipmap           = 0;
    PFNGLGENERATEMIPMAPEXTPROC        glGenerateMipmapEXT        = 0;
    PFNGLACTIVETEXTUREPROC            glActiveTextureEXT         = 0;
    PFNGLBLENDFUNCSEPARATEPROC        glBlendFuncSeparate        = 0;

    PFNGLGENSAMPLERSPROC              glGenSamplers              = 0;
    PFNGLDELETESAMPLERSPROC           glDeleteSamplers           = 0;
    PFNGLBINDSAMPLERPROC              glBindSampler              = 0;

    PFNGLUNIFORM1FPROC                glUniform1f                = 0;
    PFNGLUNIFORM2FPROC                glUniform2f                = 0;
    PFNGLUNIFORM3FPROC                glUniform3f                = 0;
    PFNGLUNIFORM4FPROC                glUniform4f                = 0;

    PFNGLUNIFORM1IPROC                glUniform1i                = 0;
    PFNGLUNIFORM2IPROC                glUniform2i                = 0;
    PFNGLUNIFORM3IPROC                glUniform3i                = 0;
    PFNGLUNIFORM4IPROC                glUniform4i                = 0;

    PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation       = 0;
    PFNGLGETUNIFORMBLOCKINDEXPROC     glGetUniformBlockIndex     = 0;
    PFNGLUNIFORMBLOCKBINDINGPROC      glUniformBlockBinding      = 0;

    PFNGLGETINTEGER64VPROC            glGetInteger64v            = 0;

    PFNGLBLENDFUNCSEPARATEEXTPROC     glBlendFuncSeparateEXT     = 0;
    PFNGLBLENDEQUATIONSEPARATEEXTPROC glBlendEquationSeparateEXT = 0;

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = 0;
}; // class api

class config {
  public:
    config(Display * const dpy)
      : m_dpy(dpy)
    {
      const int attrs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 16,
        None
      };

      int n_fb_configs = 0;
      GLXFBConfig * fb_configs =
        glXChooseFBConfig(m_dpy, DefaultScreen(m_dpy), attrs, &n_fb_configs);

      bool found = false;
      for (int i = 0; i < n_fb_configs; ++i) {
        m_visual_info = glXGetVisualFromFBConfig(m_dpy, fb_configs[i]);
        if (! m_visual_info) continue;

        XRenderPictFormat * pict_format =
          XRenderFindVisualFormat(m_dpy, m_visual_info->visual);
        if (! pict_format) continue;

        if (pict_format->direct.alphaMask > 0) {
          found = true;
          m_fb_config = fb_configs[i];
          break;
        }
      }

      if (fb_configs) delete fb_configs;
      if (! found) throw "Could not find a valid FBConfig!";

      m_colormap = XCreateColormap(m_dpy, DefaultRootWindow(m_dpy),
                                   m_visual_info->visual, AllocNone);
    }

    ~config(void)
    {
      if (m_visual_info) delete m_visual_info;
      if (m_colormap != None) XFreeColormap(m_dpy, m_colormap);
    }

    const gl::api & api(void) const { return m_api; }
    Display * const dpy(void) const { return m_dpy; }
    const GLXFBConfig & fb_config(void) const { return m_fb_config; }
    XVisualInfo * const visual_info(void) const { return m_visual_info; }
    const Colormap & colormap(void) const { return m_colormap; }

  private:
    gl::api m_api;
    Display * const m_dpy;

    GLXFBConfig m_fb_config;
    XVisualInfo * m_visual_info = NULL;
    Colormap m_colormap = None;
};

class context {
  public:
    context(const config & config)
      : m_config(config), m_api(config.api()), m_dpy(config.dpy())
    {
      // > https://www.opengl.org/registry/specs/ARB/glx_create_context.txt

      const int context_attribs[] =
      {
        GLX_RENDER_TYPE,               GLX_RGBA_TYPE,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
          // forward compatibility: must not support functionality marked
          //                        as <deprecated> by that version of the API
        // GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
          // profile mask is ignored for < 3.2
        // GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        None
      };

      m_context = m_api.glXCreateContextAttribsARB(m_dpy, config.fb_config(),
                                                   0, True, context_attribs);
    }

    ~context(void)
    {
      glXDestroyContext(m_dpy, m_context);
    }

    context(const context & ctx) = delete;

    context & operator=(const context &) = delete;

    const Drawable & drawable(void)
    {
      return m_drawable;
    }

    void drawable(const Drawable & drawable)
    {
      m_drawable = drawable;
    }

    const GLuint & program(const std::string & name)
    {
      return m_programs.at(name);
    }

    const GLuint & texture(unsigned int id)
    {
      return m_textures.at(id);
    }

    const GLXPixmap & pixmap(const std::string & id)
    {
      return m_pixmaps.at(id);
    }

    context & active_texture(unsigned int id)
    {
      glActiveTexture(GL_TEXTURE0 + id);
      glEnable(GL_TEXTURE_2D);
      try {
        glBindTexture(GL_TEXTURE_2D, m_textures.at(id));
      } catch (...) {}
      return *this;
    }

    template<typename ... FS>
    context & texture(unsigned int id, FS ... fs)
    {
      glActiveTexture(GL_TEXTURE0 + id);
      glEnable(GL_TEXTURE_2D);
      try {
        glBindTexture(GL_TEXTURE_2D, m_textures.at(id));
        unfold(m_textures.at(id), fs ...);
      } catch (...) {}
      glBindTexture(GL_TEXTURE_2D, 0);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      return *this;
    }

    template<typename ... FS>
    context & pixmap(const std::string & id, FS ... fs)
    {
      try {
        unfold(m_pixmaps.at(id), fs ...);
      } catch (...) {}
      return *this;
    }

    template<typename ... FS>
    context & run(FS ...  fs)
    {
      if (m_drawable != None) {
        glXMakeCurrent(m_dpy, m_drawable, m_context);
        unfold(*this, fs ...);
        glXMakeCurrent(m_dpy, None, NULL);
      }
      return *this;
    }

    // load shader
    template<GLenum ST>
    context & load(const std::string & name, const std::string & shader)
    {
      m_shader[name] = m_api.glCreateShader(ST);

      const GLchar * str[] = { shader.c_str() };
      const GLint len[] = { (GLint)shader.length() };
      m_api.glShaderSource(m_shader[name], 1, str, len);

      m_api.glCompileShader(m_shader[name]);

      GLsizei log_length = 0, max_len = 1024;
      GLchar log_buffer[max_len];
      m_api.glGetShaderInfoLog(m_shader[name], max_len, &log_length, log_buffer);

      if (log_length > 0) {
        std::cerr << "Shader compilation for " << name << " failed:"
                  << std::endl << log_buffer << std::endl;
      }

      return *this;
    }

    // name: name of program
    // shaders: const std::string & shader_name_1, shader_name_2, etc.
    template<typename ... SHADERS>
    context & compile(const std::string & name, SHADERS ... shaders)
    {
      m_programs[name] = m_api.glCreateProgram();

      attach(m_programs[name], shaders ...);

      m_api.glLinkProgram(m_programs[name]);

      GLsizei log_length = 0, max_len = 1024;
      GLchar log_buffer[max_len];
      m_api.glGetProgramInfoLog(m_programs[name], max_len, &log_length, log_buffer);

      if (log_length > 0) {
        std::cerr << "Program creation for " << name << " failed:" << std::endl
                  << log_buffer << std::endl;
      }

      return *this;
    }

    // load texture
    // depth: one of GLX_TEXTURE_FORMAT_RGB_EXT,
    //               GLX_TEXTURE_FORMAT_RGBA_EXT
    //            or GLX_TEXTURE_FORMAT_NONE_EXT
    context & load(const std::string & id, const Pixmap & pixmap, int depth)
    {
      try {
        m_api.glXReleaseTexImageEXT(m_dpy, m_pixmaps.at(id), GLX_FRONT_EXT);
        glXDestroyGLXPixmap(m_dpy, m_pixmaps.at(id));
        m_pixmaps.erase(id);
      } catch (...) {}

      if (pixmap == None) return *this;

      const int attrs[] = {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_MIPMAP_TEXTURE_EXT, GL_TRUE,
        GLX_TEXTURE_FORMAT_EXT, depth,
        None
      };

      m_pixmaps[id] = glXCreatePixmap(m_dpy, m_config.fb_config(), pixmap, attrs);

      return *this;
    }

    context & bind(unsigned int texture_id, const std::string & pixmap_id)
    {
      try {
        glDeleteTextures(1, &m_textures.at(texture_id));
        m_textures.erase(texture_id);
      } catch (...) {}

      glGenTextures(1, &m_textures[texture_id]);

      texture(texture_id, [&, this](const GLuint &)
      {
        m_api.glXBindTexImageEXT(m_dpy, m_pixmaps[pixmap_id], GLX_FRONT_EXT, NULL);
      });

      return *this;
    }

  private:
    const config & m_config;
    const api & m_api;
    Display * m_dpy;
    Drawable m_drawable;

    GLXContext m_context = None;

    std::unordered_map<unsigned int, GLuint> m_textures;
    std::unordered_map<std::string, GLuint> m_shader;
    std::unordered_map<std::string, GLuint> m_programs;
    std::unordered_map<std::string, GLXPixmap> m_pixmaps;

    template<typename A, typename F, typename ... FS>
    void unfold(A&& a, F f, FS ... fs)
    {
      f(a);
      unfold(std::forward<A>(a), fs ...);
    }

    template<typename A, typename F>
    void unfold(A&& a, F f)
    {
      f(std::forward<A>(a));
    }

    template<typename P, typename S, typename ... SS>
    void attach(P&& p, S s, SS ... ss)
    {
      m_api.glAttachShader(p, m_shader[s]);
      attach(p, ss ...);
    }

    template<typename P, typename S>
    void attach(P&& p, S s)
    {
      m_api.glAttachShader(p, m_shader[s]);
    }

}; // class context

// helper for reading shaders from a file
// use with load:
// load("my_shader", read("/path/to/my_shader_src.txt"));
std::string
read(const std::string & filename);

}; // namespace gl

std::ostream &
operator<<(std::ostream &, const GLXFBConfigPrintAdapter &);

#endif // OPENGL_HPP
