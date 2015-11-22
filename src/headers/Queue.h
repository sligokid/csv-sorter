/**
 * Thread-safe queue template, which can block if too big and notify threads when desired size is reached.
 * @class Queue
 * @see STL queue container
 */

/**
 * Declaration and implementation of Queue class.
 * @file Queue.h
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <deque>

#include "std.h"


template <typename T>
class Queue
{
public:
   Queue(size_t set_maxsize = DEFAULT_MAXSIZE, size_t set_fillsize = DEFAULT_FILLSIZE);
   bool put(const T& item, bool block = true);
   bool get(T& item, bool block = true);
   T get(bool block = true);
   void wait_fill(void);
   void fake_fill(void);
   string stats(void);
   void close(void);
   bool closed(void) {return _closed;};
   bool empty(void) {return _size == 0;};
   size_t size(void) {return _size;};
   ~Queue();

   size_t maxsize;                      ///< block if trying to put() more than this->maxsize
   size_t fillsize;                     ///< broadcast setsize_reached when this->queue.size() >= this->_fillsize


private:
   static const size_t DEFAULT_MAXSIZE = 5000;
   static const size_t DEFAULT_FILLSIZE = 1000;

   // Internal state
   deque<T> queue;
   size_t _size;
   bool _closed;

   // Synchronisation
   pthread_mutex_t mutex;
   pthread_cond_t fillsize_or_close;
   pthread_cond_t put_or_closed;
   pthread_cond_t got_or_closed;

   // Some stats
   size_t maxfill;                     ///< How big the queue ever got
   size_t avgfill;                     ///< How big the queue usually was
   long long numput;                   ///< Count put()
   long long numget;                   ///< Count get()
   long long totalfill;                ///< avgfill = totalfill / (numget+numput)
};

template <typename T>
Queue<T>::Queue(size_t set_maxsize, size_t set_fillsize) :
   maxsize(set_maxsize), fillsize(set_fillsize), _size(0),_closed(false), maxfill(0), avgfill(0), numput(0), numget(0), totalfill(0)
{
   if (set_maxsize <= 0)
      this->maxsize = DEFAULT_MAXSIZE;
/*    pthread_mutexattr_t mutattr; */
/*    pthread_mutexattr_settype(&mutattr, PTHREAD_MUTEX_RECURSIVE_NP); */
/*    pthread_mutex_init(&this->mutex, &mutattr); */
   pthread_mutex_init(&this->mutex, NULL);
   pthread_cond_init(&this->put_or_closed, NULL);
   pthread_cond_init(&this->got_or_closed, NULL);
   pthread_cond_init(&this->fillsize_or_close, NULL);
}

/**
 * Add an item onto the queue
 * @todo get rid of parameter 'block' since probably nobody uses it
 */
template <typename T>
bool Queue<T>::put(const T& item, bool block) 
{
   pthread_mutex_lock(&this->mutex);
   if (_closed) 
   {
      pthread_mutex_unlock(&this->mutex);
      return true;
   }
   if (this->_size >= this->maxsize) 
   {
      if (block) 
      {
         pthread_cond_wait(&this->got_or_closed, &this->mutex);
         if (_closed) 
         {
            pthread_mutex_unlock(&this->mutex);
            return true;
         }
      } 
      else 
      {
         pthread_mutex_unlock(&this->mutex);
         return true;
      }
   }
   queue.push_back(item);
   this->_size += 1;
   //cout << "filled or closed.." << this->fillsize << "this->_size"   << this->_size << endl;
   if (this->_size > this->maxfill)
      maxfill = this->_size;
   if (this->_size >= this->fillsize)
      pthread_cond_broadcast(&this->fillsize_or_close);
   ++numput;
   totalfill += this->_size;
   avgfill = totalfill / (numput + numget);
   pthread_cond_broadcast(&this->put_or_closed);
   pthread_mutex_unlock(&this->mutex);
   return false;
}

/**
 * Get the next object in the queue, removing it from queue.
 * 
 * @param block if true, wait untill some data is available
 * @param item is the item to add
 * @return false and set 'item' to the object on success, true if no item can be got
 */
template <typename T>
bool Queue<T>::get(T& item, bool block) 
{
   pthread_mutex_lock(&mutex);
   if (queue.empty()) 
   {
      if (block) 
      {
         if (_closed)  
         {
	    pthread_mutex_unlock(&mutex);
            return true;
         }
         pthread_cond_wait(&put_or_closed, &mutex);
         if (queue.empty()) 
         {
            pthread_mutex_unlock(&mutex);
            return true;
         }
      }
      else 
      {
         pthread_mutex_unlock(&mutex);
         return true;
      }
   }
   this->_size -= 1;
   item = queue.front();
   queue.pop_front();
   ++numget;
   totalfill += this->_size;
   avgfill = totalfill / (numput + numget);
   pthread_cond_broadcast(&got_or_closed);
   pthread_mutex_unlock(&mutex);
   return false;
}

/**
 * Convenience function instanciating an object and calling get(item, block).
 * @warning returning local variable, make sure the queue contains data usable by value
 */
template <typename T>
T Queue<T>::get(bool block)
{
   T item;
   get(item, block);
   return item;
}

/**
 * Block the current thread untill queue has received enough items (from another thread).
 */
template <typename T>
void Queue<T>::wait_fill()
{
   pthread_mutex_lock(&this->mutex);
   pthread_cond_wait(&this->fillsize_or_close, &this->mutex);
   pthread_mutex_unlock(&this->mutex);
}

/**
 * Pretend the queue has filled up, to wake up threads that are waiting.
 */
template <typename T>
void Queue<T>::fake_fill()
{
   pthread_mutex_lock(&this->mutex);
   pthread_cond_broadcast(&this->fillsize_or_close);
   pthread_mutex_unlock(&this->mutex);
}

/**
 * Close the queue, so that nothing more can be added.
 * The reports use that to detect the end of the day and start persisting.
 */
template <typename T>
void Queue<T>::close()
{
   //cout <<"wueue prolb" <<endl;
   pthread_mutex_lock(&this->mutex);
   _closed = true;
   pthread_cond_broadcast(&this->put_or_closed);
   pthread_cond_broadcast(&this->got_or_closed);
   pthread_cond_broadcast(&this->fillsize_or_close);
   pthread_mutex_unlock(&this->mutex);
}

/**
 * destructor
 * @warning make sure all threads are finished accessing the Queue.
 */
template <typename T>
Queue<T>::~Queue()
{
   pthread_cond_destroy(&this->put_or_closed);
   pthread_cond_destroy(&this->got_or_closed);
   pthread_cond_destroy(&this->fillsize_or_close);
   pthread_mutex_destroy(&this->mutex);
}

/**
 * Return a string with some statistics.
 */
template <typename T>
string Queue<T>::stats()
{
   pthread_mutex_lock(&this->mutex);
   format stat("curfill:%d\tmaxfill:%d\tavgfill:%d\tnumput:%d\tnumget:%d");
   stat % this->_size % this->maxfill % this->avgfill % this->numput % this->numget;
   pthread_mutex_unlock(&this->mutex);
   return stat.str();
}

#endif // QUEUE_H
