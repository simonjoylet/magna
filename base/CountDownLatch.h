#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

#include "noncopyable.h"

namespace base
{

class CountDownLatch : base::noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}
#endif  // COUNTDOWNLATCH_H
