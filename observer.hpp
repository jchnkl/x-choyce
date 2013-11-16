#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <unordered_set>

template<typename T>
class observer {
  public:
    observer(void) {}
    virtual ~observer(void) {}
    virtual void notify(T *) = 0;
};

template<typename T>
class observable {
  public:
    observable(void) {}
    virtual ~observable(void) {}

    void attach(observer<T> * o)
    {
      m_observer.insert(o);
    }

    void detach(observer<T> * o)
    {
      m_observer.remove(o);
    }

    void notify(void)
    {
      for (auto o : m_observer) {
        o->notify(dynamic_cast<T *>(this));
      }
    }

  private:
    std::unordered_set<observer<T> *> m_observer;
};

#endif // OBSERVER_HPP
