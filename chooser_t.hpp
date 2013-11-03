#ifndef _CHOOSER_T_HPP
#define _CHOOSER_T_HPP

class chooser_t {
  public:
    virtual void next(void) = 0;
    virtual void prev(void) = 0;
    virtual void show(void) = 0;
    virtual void hide(void) = 0;
    virtual void select(const unsigned int & window = 0) = 0;
};

#endif
