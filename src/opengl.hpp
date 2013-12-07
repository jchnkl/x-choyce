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

namespace x {

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
    config(const api & api, Display * const dpy)
      : m_api(api), m_dpy(dpy)
    {
      const int attrs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        // GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
        // GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, True,
        // GLX_RED_SIZE, 8,
        // GLX_GREEN_SIZE, 8,
        // GLX_BLUE_SIZE, 8,
        // GLX_ALPHA_SIZE, 8,
        // GLX_DEPTH_SIZE, 16,
        None
      };

      int n_fb_configs = 0;
      GLXFBConfig * fb_configs =
        glXChooseFBConfig(m_dpy, DefaultScreen(m_dpy), attrs, &n_fb_configs);

      bool found = false;
      for (int i = 0; i < n_fb_configs; ++i) {
        XVisualInfo * m_vi = glXGetVisualFromFBConfig(m_dpy, fb_configs[i]);
        if (! m_vi) continue;

        XRenderPictFormat * pict_format = XRenderFindVisualFormat(m_dpy, m_vi->visual);
        if (! pict_format) continue;

        if (pict_format->direct.alphaMask > 0) {
          found = true;
          m_fb_config = fb_configs[i];
          break;
        }
      }

      if (fb_configs) delete fb_configs;
      if (! found) throw "Could not find a valid FBConfig!";
    }

    ~config(void)
    {
      if (m_visual_info) delete m_visual_info;
    }

    const api & api(void) const { return m_api; }
    Display * const dpy(void) const { return m_dpy; }
    const GLXFBConfig & fb_config(void) const { return m_fb_config; }
    XVisualInfo * const visual_info(void) const { return m_visual_info; }

  private:
    const class api & m_api;
    Display * const m_dpy;

    GLXFBConfig m_fb_config;
    XVisualInfo * m_visual_info = NULL;
};

class context {
  public:
    context(const config & config)
      : m_config(config), m_api(config.api()), m_dpy(config.dpy())
    {
      m_context = glXCreateNewContext(
          m_dpy, config.fb_config(), GLX_RGBA_TYPE, 0, True);

      if (Success !=
          glXGetFBConfigAttrib(m_dpy, m_config.fb_config(),
                               GLX_BIND_TO_TEXTURE_TARGETS_EXT, &m_target))
      {
        m_target = GLX_TEXTURE_2D_EXT;
      }
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
      return m_programs.at(name).first;
    }

    const GLuint & texture(unsigned int id)
    {
      return m_textures.at(id);
    }

    const GLXPixmap & pixmap(unsigned int id)
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
    context & pixmap(unsigned int id, FS ... fs)
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
    context & load(const std::string & p_name,
                   const std::string & v_file,
                   const std::string & f_file)
    {
      std::ifstream file(v_file);
      std::string v_source((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
      file.close();

      file.open(f_file);
      std::string f_source((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
      file.close();

      shader_t v = m_api.glCreateShader(GL_VERTEX_SHADER);
      shader_t f = m_api.glCreateShader(GL_FRAGMENT_SHADER);

      {
        const GLchar * v_c_str[] = { v_source.c_str() };
        const GLint v_c_len[] = { (GLint)v_source.length() };
        const GLchar * f_c_str[] = { f_source.c_str() };
        const GLint f_c_len[] = { (GLint)f_source.length() };
        m_api.glShaderSource(v, 1, v_c_str, v_c_len);
        m_api.glShaderSource(f, 1, f_c_str, f_c_len);
      }

      m_api.glCompileShader(v);
      m_api.glCompileShader(f);

      GLsizei log_length = 0, max_len = 1024;
      GLchar log_buffer[max_len];


      m_api.glGetShaderInfoLog(v, max_len, &log_length, log_buffer);
      if (log_length > 0) {
        std::cerr << "Shader compilation for " << p_name << " failed:"
                  << std::endl << log_buffer << std::endl;
      }

      m_api.glGetShaderInfoLog(f, max_len, &log_length, log_buffer);
      if (log_length > 0) {
        std::cerr << "Shader compilation for " << p_name << " failed:"
                  << std::endl << log_buffer << std::endl;
      }

      program_t p = m_api.glCreateProgram();

      m_api.glAttachShader(p, v);
      m_api.glAttachShader(p, f);
      m_api.glLinkProgram(p);

      m_api.glGetProgramInfoLog(p, max_len, &log_length, log_buffer);
      if (log_length > 0) {
        std::cerr << "Program creation for " << p_name << " failed:" << std::endl
                  << log_buffer << std::endl;
      }

      m_programs[p_name] = { p, 0 };

      return *this;
    }

    // load texture
    // depth: one of GLX_TEXTURE_FORMAT_RGB_EXT,
    //               GLX_TEXTURE_FORMAT_RGBA_EXT
    //            or GLX_TEXTURE_FORMAT_NONE_EXT
    context & load(unsigned int id, const Pixmap & pixmap, int depth)
    {
      try {
        m_api.glXReleaseTexImageEXT(m_dpy, m_pixmaps.at(id), GLX_FRONT_EXT);
        glXDestroyGLXPixmap(m_dpy, m_pixmaps.at(id));
        m_pixmaps.erase(id);
        glDeleteTextures(1, &m_textures.at(id));
        m_textures.erase(id);
      } catch (...) {}

      if (pixmap == None) return *this;

      const int attrs[] = {
        GLX_TEXTURE_TARGET_EXT, m_target,
        GLX_MIPMAP_TEXTURE_EXT, GL_TRUE,
        GLX_TEXTURE_FORMAT_EXT, depth,
        None
      };

      glGenTextures(1, &m_textures[id]);

      m_pixmaps[id] = glXCreatePixmap(m_dpy, m_config.fb_config(), pixmap, attrs);

      texture(id, [&, this](const GLuint &)
      {
        m_api.glXBindTexImageEXT(m_dpy, m_pixmaps[id], GLX_FRONT_EXT, NULL);
      });

      return *this;
    }

  private:
    typedef GLuint shader_t;
    typedef GLuint program_t;

    const config & m_config;
    const api & m_api;
    Display * m_dpy;
    Drawable m_drawable;

    GLint m_target;

    GLXContext m_context = None;
    std::unordered_map<unsigned int, GLuint> m_textures;
    std::unordered_map<unsigned int, GLXPixmap> m_pixmaps;

    std::unordered_map<std::string, std::pair<program_t, shader_t>> m_programs;

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

}; // class context

}; // namespace gl

}; // namespace x

std::ostream &
operator<<(std::ostream &, const GLXFBConfigPrintAdapter &);

#endif // OPENGL_HPP
