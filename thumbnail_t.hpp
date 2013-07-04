#ifndef _THUMBNAIL_T_HPP
#define _THUMBNAIL_T_HPP

class thumbnail_t {
  public:
    virtual void show(void) const = 0;
    virtual void hide(void) const = 0;
    virtual void update(void) const = 0;
    virtual void select(void) const = 0;
    virtual void highlight(bool want_highlight) const = 0;
};

#endif
