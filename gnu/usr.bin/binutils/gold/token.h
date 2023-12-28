// token.h -- lock tokens for gold   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#ifndef GOLD_TOKEN_H
#define GOLD_TOKEN_H

namespace gold
{

class Condvar;
class Task;

// A list of Tasks, managed through the next_locked_ field in the
// class Task.  We define this class here because we need it in
// Task_token.

class Task_list
{
 public:
  Task_list()
    : head_(NULL), tail_(NULL)
  { }

  ~Task_list()
  { gold_assert(this->head_ == NULL && this->tail_ == NULL); }

  // Return whether the list is empty.
  bool
  empty() const
  { return this->head_ == NULL; }

  // Add T to the head of the list.
  void
  push_front(Task* t);

  // Add T to the end of the list.
  void
  push_back(Task* t);

  // Remove the first Task on the list and return it.  Return NULL if
  // the list is empty.
  Task*
  pop_front();

 private:
  // The start of the list.  NULL if the list is empty.
  Task* head_;
  // The end of the list.  NULL if the list is empty.
  Task* tail_;
};

// We support two basic types of locks, which are both implemented
// using the single class Task_token.

// A write lock may be held by a single Task at a time.  This is used
// to control access to a single shared resource such as an Object.

// A blocker is used to indicate that a Task A must be run after some
// set of Tasks B.  For each of the Tasks B, we increment the blocker
// when the Task is created, and decrement it when the Task is
// completed.  When the count goes to 0, the task A is ready to run.

// There are no shared read locks.  We always read and write objects
// in predictable patterns.  The purpose of the locks is to permit
// some flexibility for the threading system, for cases where the
// execution order does not matter.

// These tokens are only manipulated when the workqueue lock is held
// or when they are first created.  They do not require any locking
// themselves.

class Task_token
{
 public:
  Task_token(bool is_blocker)
    : is_blocker_(is_blocker), blockers_(0), writer_(NULL), waiting_()
  { }

  ~Task_token()
  {
    gold_assert(this->blockers_ == 0);
    gold_assert(this->writer_ == NULL);
  }

  // Return whether this is a blocker.
  bool
  is_blocker() const
  { return this->is_blocker_; }

  // A write lock token uses these methods.

  // Is the token writable?
  bool
  is_writable() const
  {
    gold_assert(!this->is_blocker_);
    return this->writer_ == NULL;
  }

  // Add the task as the token's writer (there may only be one
  // writer).
  void
  add_writer(const Task* t)
  {
    gold_assert(!this->is_blocker_ && this->writer_ == NULL);
    this->writer_ = t;
  }

  // Remove the task as the token's writer.
  void
  remove_writer(const Task* t)
  {
    gold_assert(!this->is_blocker_ && this->writer_ == t);
    this->writer_ = NULL;
  }

  // A blocker token uses these methods.

  // Add a blocker to the token.
  void
  add_blocker()
  {
    gold_assert(this->is_blocker_);
    ++this->blockers_;
    this->writer_ = NULL;
  }

  // Add some number of blockers to the token.
  void
  add_blockers(int c)
  {
    gold_assert(this->is_blocker_);
    this->blockers_ += c;
    this->writer_ = NULL;
  }

  // Remove a blocker from the token.  Returns true if block count
  // drops to zero.
  bool
  remove_blocker()
  {
    gold_assert(this->is_blocker_ && this->blockers_ > 0);
    --this->blockers_;
    this->writer_ = NULL;
    return this->blockers_ == 0;
  }

  // Is the token currently blocked?
  bool
  is_blocked() const
  {
    gold_assert(this->is_blocker_);
    return this->blockers_ > 0;
  }

  // Both blocker and write lock tokens use these methods.

  // Add T to the list of tasks waiting for this token to be released.
  void
  add_waiting(Task* t)
  { this->waiting_.push_back(t); }

  // Add T to the front of the list of tasks waiting for this token to
  // be released.
  void
  add_waiting_front(Task* t)
  { this->waiting_.push_front(t); }

  // Remove the first Task waiting for this token to be released, and
  // return it.  Return NULL if no Tasks are waiting.
  Task*
  remove_first_waiting()
  { return this->waiting_.pop_front(); }

 private:
  // It makes no sense to copy these.
  Task_token(const Task_token&);
  Task_token& operator=(const Task_token&);

  // Whether this is a blocker token.
  bool is_blocker_;
  // The number of blockers.
  int blockers_;
  // The single writer.
  const Task* writer_;
  // The list of Tasks waiting for this token to be released.
  Task_list waiting_;
};

// In order to support tokens more reliably, we provide objects which
// handle them using RAII.

// RAII class to get a write lock on a token.  This requires
// specifying the task which is doing the lock.

class Task_write_token
{
 public:
  Task_write_token(Task_token* token, const Task* task)
    : token_(token), task_(task)
  { this->token_->add_writer(this->task_); }

  ~Task_write_token()
  { this->token_->remove_writer(this->task_); }

 private:
  Task_write_token(const Task_write_token&);
  Task_write_token& operator=(const Task_write_token&);

  Task_token* token_;
  const Task* task_;
};

// RAII class for a blocker.

class Task_block_token
{
 public:
  // The blocker count must be incremented when the task is created.
  // This object is created when the task is run, so we don't do
  // anything in the constructor.
  Task_block_token(Task_token* token)
    : token_(token)
  { gold_assert(this->token_->is_blocked()); }

  ~Task_block_token()
  { this->token_->remove_blocker(); }

 private:
  Task_block_token(const Task_block_token&);
  Task_block_token& operator=(const Task_block_token&);

  Task_token* token_;
};

// An object which implements an RAII lock for any object which
// supports lock and unlock methods.

template<typename Obj>
class Task_lock_obj
{
 public:
  Task_lock_obj(const Task* task, Obj* obj)
    : task_(task), obj_(obj)
  { this->obj_->lock(task); }

  ~Task_lock_obj()
  { this->obj_->unlock(this->task_); }

 private:
  Task_lock_obj(const Task_lock_obj&);
  Task_lock_obj& operator=(const Task_lock_obj&);

  const Task* task_;
  Obj* obj_;
};

// A class which holds the set of Task_tokens which must be locked for
// a Task.  No Task requires more than four Task_tokens, so we set
// that as a limit.

class Task_locker
{
 public:
  static const int max_task_count = 4;

  Task_locker()
    : count_(0)
  { }

  ~Task_locker()
  { }

  // Clear the locker.
  void
  clear()
  { this->count_ = 0; }

  // Add a token to the locker.
  void
  add(Task* t, Task_token* token)
  {
    gold_assert(this->count_ < max_task_count);
    this->tokens_[this->count_] = token;
    ++this->count_;
    // A blocker will have been incremented when the task is created.
    // A writer we need to lock now.
    if (!token->is_blocker())
      token->add_writer(t);
  }

  // Iterate over the tokens.

  typedef Task_token** iterator;

  iterator
  begin()
  { return &this->tokens_[0]; }

  iterator
  end()
  { return &this->tokens_[this->count_]; }

 private:
  Task_locker(const Task_locker&);
  Task_locker& operator=(const Task_locker&);

  // The number of tokens.
  int count_;
  // The tokens.
  Task_token* tokens_[max_task_count];
};

} // End namespace gold.

#endif // !defined(GOLD_TOKEN_H)
