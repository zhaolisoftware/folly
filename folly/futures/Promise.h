/*
 * Copyright 2014 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/futures/Deprecated.h>
#include <folly/futures/Try.h>
#include <functional>

namespace folly {

// forward declaration
template <class T> class Future;

template <class T>
class Promise {
public:
  Promise();
  ~Promise();

  // not copyable
  Promise(Promise const&) = delete;
  Promise& operator=(Promise const&) = delete;

  // movable
  Promise(Promise<T>&&);
  Promise& operator=(Promise<T>&&);

  /** Return a Future tied to the shared core state. This can be called only
    once, thereafter Future already retrieved exception will be raised. */
  Future<T> getFuture();

  /** Fulfil the Promise with an exception_wrapper */
  void setException(exception_wrapper ew);

  /** Fulfil the Promise with an exception_ptr, e.g.
    try {
      ...
    } catch (...) {
      p.setException(std::current_exception());
    }
    */
  void setException(std::exception_ptr const&) DEPRECATED;

  /** Fulfil the Promise with an exception type E, which can be passed to
    std::make_exception_ptr(). Useful for originating exceptions. If you
    caught an exception the exception_wrapper form is more appropriate.
    */
  template <class E>
  typename std::enable_if<std::is_base_of<std::exception, E>::value>::type
  setException(E const&);

  /// Set an interrupt handler to handle interrupts. See the documentation for
  /// Future::raise(). Your handler can do whatever it wants, but if you
  /// bother to set one then you probably will want to fulfil the promise with
  /// an exception (or special value) indicating how the interrupt was
  /// handled.
  void setInterruptHandler(std::function<void(exception_wrapper const&)>);

  /** Fulfil this Promise (only for Promise<void>) */
  void setValue();

  /** Set the value (use perfect forwarding for both move and copy) */
  template <class M>
  void setValue(M&& value);

  void fulfilTry(Try<T> t);

  /** Fulfil this Promise with the result of a function that takes no
    arguments and returns something implicitly convertible to T.
    Captures exceptions. e.g.

    p.fulfil([] { do something that may throw; return a T; });
  */
  template <class F>
  void fulfil(F&& func);

private:
  typedef typename Future<T>::corePtr corePtr;

  // Whether the Future has been retrieved (a one-time operation).
  bool retrieved_;

  // shared core state object
  corePtr core_;

  void throwIfFulfilled();
  void throwIfRetrieved();
  void detach();
};

}

#include <folly/futures/Future.h>
#include <folly/futures/Promise-inl.h>
