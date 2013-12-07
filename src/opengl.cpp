#include "opengl.hpp"

#include <iomanip> // setw()

std::ostream &
operator<<(std::ostream & os, const GLXFBConfigPrintAdapter & a)
{
  int value = 0;
  char buffer[30];

  // XID of the given GLXFBConfig.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_FBCONFIG_ID, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_FBCONFIG_ID:");
    os << buffer << value << std::endl;
  }

  // Number of bits per color buffer.
  // If the frame buffer configuration supports RGBA contexts, then
  // GLX_BUFFER_SIZE is the sum of
  // GLX_RED_SIZE,
  // GLX_GREEN_SIZE,
  // GLX_BLUE_SIZE, and
  // GLX_ALPHA_SIZE.
  // If the frame buffer configuration supports only color index contexts,
  // GLX_BUFFER_SIZE is the size of the
  // color indexes.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_BUFFER_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_BUFFER_SIZE:");
    os << buffer << value << std::endl;
  }

  // Frame buffer level of the configuration.
  // Level zero is the default frame buffer.
  // Positive levels correspond to frame buffers that overlay the default buffer,
  // and negative levels correspond to frame buffers that underlie the default
  // buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_LEVEL, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_LEVEL:");
    os << buffer << value << std::endl;
  }

  // True if color buffers exist in front/back pairs that can be swapped,
  // False otherwise.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_DOUBLEBUFFER, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_DOUBLEBUFFER:");
    os << buffer << value << std::endl;
  }

  // True if color buffers exist in left/right pairs,
  // False otherwise.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_STEREO, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_STEREO:");
    os << buffer << value << std::endl;
  }

  // Number of auxiliary color buffers that are available.
  // Zero indicates that no auxiliary color buffers exist.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_AUX_BUFFERS, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_AUX_BUFFERS:");
    os << buffer << value << std::endl;
  }

  // Number of bits of red stored in each color buffer.
  // Undefined if RGBA contexts are not supported by the frame buffer configuration.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_RED_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_RED_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of green stored in each color buffer.
  // Undefined if RGBA contexts are not supported by the frame buffer configuration.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_GREEN_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_GREEN_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of blue stored in each color buffer.
  // Undefined if RGBA contexts are not supported by the frame buffer configuration.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_BLUE_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_BLUE_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of alpha stored in each color buffer.
  // Undefined if RGBA contexts are not supported by the frame buffer configuration.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_ALPHA_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_ALPHA_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits in the depth buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_DEPTH_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_DEPTH_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits in the stencil buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_STENCIL_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_STENCIL_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of red stored in the accumulation buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_ACCUM_RED_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_ACCUM_RED_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of green stored in the accumulation buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_ACCUM_GREEN_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_ACCUM_GREEN_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of blue stored in the accumulation buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_ACCUM_BLUE_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_ACCUM_BLUE_SIZE:");
    os << buffer << value << std::endl;
  }

  // Number of bits of alpha stored in the accumulation buffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_ACCUM_ALPHA_SIZE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_ACCUM_ALPHA_SIZE:");
    os << buffer << value << std::endl;
  }

  // Mask indicating what type of GLX contexts can be made
  // current to the frame buffer configuration. Valid bits are
  // GLX_RGBA_BIT and
  // GLX_COLOR_INDEX_BIT.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_RENDER_TYPE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_RENDER_TYPE:");
    os << buffer << value << std::endl;
  }

  // Mask indicating what drawable types the frame buffer configuration
  // supports. Valid bits are GLX_WINDOW_BIT,
  // GLX_PIXMAP_BIT, and GLX_PBUFFER_BIT.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_DRAWABLE_TYPE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_DRAWABLE_TYPE:");
    os << buffer << value << std::endl;
  }

  // True if drawables created with the
  // frame buffer configuration can be rendered to by X.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_X_RENDERABLE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_X_RENDERABLE:");
    os << buffer << value << std::endl;
  }

  // XID of the corresponding visual, or zero
  // if there is no associated visual (i.e., if
  // GLX_X_RENDERABLE is False or
  // GLX_DRAWABLE_TYPE does not have the
  // GLX_WINDOW_BIT bit set).

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_VISUAL_ID, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_VISUAL_ID:");
    os << buffer << value << std::endl;
  }

  // Visual type of associated visual. The returned value will be one of:
  // GLX_TRUE_COLOR, GLX_DIRECT_COLOR, GLX_PSEUDO_COLOR,
  // GLX_STATIC_COLOR, GLX_GRAY_SCALE, GLX_STATIC_GRAY,
  // or GLX_NONE, if there is no associated visual (i.e., if
  // GLX_X_RENDERABLE is False or
  // GLX_DRAWABLE_TYPE does not have the
  // GLX_WINDOW_BIT bit set).

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_X_VISUAL_TYPE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_X_VISUAL_TYPE:");
    os << buffer << value << std::endl;
  }

  // One of GLX_NONE,
  // GLX_SLOW_CONFIG, or
  // GLX_NON_CONFORMANT_CONFIG, indicating
  // that the frame buffer configuration has no caveats,
  // some aspect of the frame buffer configuration runs slower
  // than other frame buffer configurations, or some aspect of the
  // frame buffer configuration is nonconformant, respectively.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_CONFIG_CAVEAT, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_CONFIG_CAVEAT:");
    os << buffer << value << std::endl;
  }

  // One of GLX_NONE,
  // GLX_TRANSPARENT_RGB,
  // GLX_TRANSPARENT_INDEX, indicating that
  // the frame buffer configuration is opaque, is transparent for particular
  // values of red, green, and blue, or is transparent for
  // particular index values, respectively.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_TRANSPARENT_TYPE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_TRANSPARENT_TYPE:");
    os << buffer << value << std::endl;
  }

  // Integer value between 0 and the maximum
  // frame buffer value for indices, indicating the transparent
  // index value for the frame buffer configuration.
  // Undefined if GLX_TRANSPARENT_TYPE
  // is not GLX_TRANSPARENT_INDEX.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_TRANSPARENT_INDEX_VALUE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_TRANSPARENT_INDEX_VALUE:");
    os << buffer << value << std::endl;
  }

  // Integer value between 0 and the maximum
  // frame buffer value for red, indicating the transparent
  // red value for the frame buffer configuration.
  // Undefined if GLX_TRANSPARENT_TYPE
  // is not GLX_TRANSPARENT_RGB.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_TRANSPARENT_RED_VALUE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_TRANSPARENT_RED_VALUE:");
    os << buffer << value << std::endl;
  }

  // Integer value between 0 and the maximum
  // frame buffer value for green, indicating the transparent
  // green value for the frame buffer configuration.
  // Undefined if GLX_TRANSPARENT_TYPE
  // is not GLX_TRANSPARENT_RGB.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_TRANSPARENT_GREEN_VALUE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_TRANSPARENT_GREEN_VALUE:");
    os << buffer << value << std::endl;
  }

  // Integer value between 0 and the maximum
  // frame buffer value for blue, indicating the transparent
  // blue value for the frame buffer configuration.
  // Undefined if GLX_TRANSPARENT_TYPE
  // is not GLX_TRANSPARENT_RGB.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_TRANSPARENT_BLUE_VALUE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_TRANSPARENT_BLUE_VALUE:");
    os << buffer << value << std::endl;
  }

  // Integer value between 0 and the maximum
  // frame buffer value for alpha, indicating the transparent
  // blue value for the frame buffer configuration.
  // Undefined if GLX_TRANSPARENT_TYPE
  // is not GLX_TRANSPARENT_RGB.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_TRANSPARENT_ALPHA_VALUE, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_TRANSPARENT_ALPHA_VALUE:");
    os << buffer << value << std::endl;
  }

  // The maximum width that can be specified to
  // glXCreatePbuffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_MAX_PBUFFER_WIDTH, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_MAX_PBUFFER_WIDTH:");
    os << buffer << value << std::endl;
  }

  // The maximum height that can be specified to
  // glXCreatePbuffer.

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_MAX_PBUFFER_HEIGHT, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_MAX_PBUFFER_HEIGHT:");
    os << buffer << value << std::endl;
  }

  // The maximum number of pixels (width times height) for a
  // pixel buffer. Note that this value may be less than
  // GLX_MAX_PBUFFER_WIDTH times
  // GLX_MAX_PBUFFER_HEIGHT. Also, this
  // value is static and assumes that no other pixel buffers
  // or X resources are contending for the frame buffer memory.
  // As a result, it may not be possible to allocate a pixel buffer of
  // the size given by GLX_MAX_PBUFFER_PIXELS

  if (Success == glXGetFBConfigAttrib(a.m_dpy, a.m_config, GLX_MAX_PBUFFER_PIXELS, &value)) {
    std::sprintf(buffer, "%-30s", "GLX_MAX_PBUFFER_PIXELS:");
    os << buffer << value << std::endl;
  }

  return os;
}
