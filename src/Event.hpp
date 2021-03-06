﻿#pragma once

//
// boost::signals2を利用した汎用的なイベント
//

#include <boost/signals2.hpp>
#include <boost/noncopyable.hpp>
#include <map>


namespace ngs {

using Connection = boost::signals2::connection;

template <typename... Args>
class Event
  : private boost::noncopyable
{
  // using dummy_mutex = boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>;
  // using SignalType = typename boost::signals2::signal_type<void(Args&...), dummy_mutex>::type;
  using SignalType = boost::signals2::signal<void(Args&...)>;

  std::map<std::string, SignalType> signals_;


public:
  Event()  = default;
  ~Event() = default;


  template <typename F>
  Connection connect(const std::string& msg, const F& callback) noexcept
  {
    return signals_[msg].connect_extended(callback);
  }  
  
  template <typename F>
  Connection connect(const std::string& msg, int prioriry, const F& callback) noexcept
  {
    return signals_[msg].connect_extended(prioriry, callback);
  }  
  
  template <typename... Args2>
  void signal(const std::string& msg, Args2&&... args) noexcept
  {
    signals_[msg](args...);
  }

  
private:
  
};

}
