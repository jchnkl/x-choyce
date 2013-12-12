#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <algorithm> // find
#include <deque>

#include "config_t.hpp"
#include "observer.hpp"

namespace generic {

class config : public config_t
             , public observer<config_t>
{
  public:

    template<typename ... CS>
    config(config_t * c, CS ... cs)
    {
      attach(c, cs ...);
    }

    ~config(void)
    {
      for (auto & c : m_configs) {
        c->detach(this);
      }
    }

    template<typename ... CS>
    void
    attach(config_t * c, CS ... cs)
    {
      unfold_attach(c, cs ...);
    }

    template<typename ... CS>
    void
    detach(config_t * c, CS ... cs)
    {
      unfold_detach(c, cs ...);
    }

    const option &
    operator[](const std::string & name)
    {
      for (auto & c : m_configs) {
        try {
          return (*c)[name];
        } catch (...) {}
      }

      throw std::invalid_argument("No config value for \"" + name + "\"");
    }

    void
    notify(config_t * c)
    {
      observable::notify();
    }

  private:
    std::deque<config_t *> m_configs;

    template<typename ... CS>
    void
    unfold_attach(config_t * c, CS ... cs)
    {
      unfold_attach(c);
      unfold_attach(cs ...);
    }

    void
    unfold_attach(config_t * c)
    {
      c->attach(this);
      m_configs.push_back(c);
    }

    template<typename ... CS>
    void
    unfold_detach(config_t * c, CS ... cs)
    {
      unfold_detach(c);
      unfold_detach(cs ...);
    }

    void
    unfold_detach(config_t * c)
    {
      c->detach(this);
      auto result = std::find(m_configs.begin(), m_configs.end(), c);
      if (result != m_configs.end()) {
        m_configs.erase(result);
      }
    }

}; // class config

}; // namespace generic

#endif // CONFIG_HPP
