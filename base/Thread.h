#ifndef THREAD_H
#define THREAD_H

#include "Atomic.h"
#include "CountDownLatch.h"
#include "Types.h"

#include "noncopyable.h"
#include <pthread.h>

namespace base
{

class Thread : base::noncopyable
{
 public:
  typedef void(*ThreadFunc) ();

  explicit Thread(const ThreadFunc&, const string& name = string());

  ~Thread();

  void start();
  int join(); // return pthread_join()

  bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();

  bool       started_;
  bool       joined_;
  pthread_t  pthreadId_;
  pid_t      tid_;
  ThreadFunc func_;
  string     name_;
  CountDownLatch latch_;

  static AtomicInt32 numCreated_;
};

}
#endif
