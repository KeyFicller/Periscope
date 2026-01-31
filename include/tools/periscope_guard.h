#pragma once

#include <functional>

namespace periscope {
// ------------------------ Main template -----------------------

// guard is RAII guard to execute function on exit
class guard
{
  public:
    guard(const std::function<void()>& _on_exit)
      : m_on_exit(_on_exit)
    {
    }
    ~guard() { m_on_exit(); }

  private:
    std::function<void()> m_on_exit;
};
}