#ifndef TURBO_APPUTILS_H
#define TURBO_APPUTILS_H

#include <tvision/tv.h>

#include <unordered_map>
#include <forward_list>
#include <string_view>
#include <string>
#include <tvision/compat/borland/dir.h>

template<typename T>
struct list_head
{
    T *self;
    list_head *next;
    list_head *prev;

    list_head()
    {
       self = 0;
       next = prev = this;
    }

    ~list_head()
    {
        remove();
    }

    list_head(T *self_)
    {
        self = self_;
        next = prev = 0;
    }

    void insert_after(list_head *other)
    {
        remove();
        prev = other;
        next = other->next;
        other->next = this;
        if (next)
            next->prev = this;
    }

    void remove() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = next;
        next = prev = 0;
    }

    size_t size() const {
        size_t i = 0;
        const list_head *head = next;
        while (head != this) {
            ++i;
            head = head->next;
        }
        return i;
    }

    bool empty() const {
        return this == next;
    }

    bool detached() const {
        return !next && !prev;
    }

    template<class Func>
    void forEach(Func &&callback) {
        list_head *head = next;
        while (head != this) {
            callback(head->self);
            head = head->next;
        }
    }

};

template<typename T>
class list_head_iterator
{

    list_head<T> *list;
    size_t listSize;
    intptr_t it;
    list_head<T> *itItem;

public:

    list_head_iterator(list_head<T> *list_) :
        list(list_),
        listSize(list_->size()),
        it(-1),
        itItem(list)
    {
    }

    size_t size() const
    {
        return listSize;
    }

    list_head<T>* at(intptr_t i)
    {
        if (i > it) {
            if (i - it <= intptr_t(listSize) - i)
                return seekF(i);
            else {
                it = listSize;
                itItem = list;
                return seekB(i);
            }
        } else if (it > i) {
            if (it - i >= i) {
                it = -1;
                itItem = list;
                return seekF(i);
            } else
                return seekB(i);
        }
        return itItem;
    }

private:

    list_head<T>* seekF(intptr_t i)
    {
        while (it < i) {
            itItem = itItem->next;
            ++it;
        }
        return itItem;
    }

    list_head<T>* seekB(intptr_t i)
    {
        while (it > i) {
            itItem = itItem->prev;
            --it;
        }
        return itItem;
    }

};

struct active_counter {
    // Counter for enumerating editors opened in the same file.
    // 'count' is only reset when the number of editors reaches zero.
    uint count {0};
    uint active {0};

    uint operator++()
    {
        ++count;
        ++active;
        return count;
    }

    void operator--()
    {
        --active;
        if (active < 1)
            count = active;
    }
};

struct CwdGuard
{
    char *lastCwd;
    CwdGuard(const char *newCwd)
    {
        if (newCwd && newCwd[0])
        {
            lastCwd = ::getcwd(nullptr, 0);
            int r = ::chdir(newCwd); (void) r;
        }
        else
            lastCwd = nullptr;
    }
    ~CwdGuard()
    {
        if (lastCwd)
        {
            int r = chdir(lastCwd); (void) r;
            ::free(lastCwd);
        }
    }
};

class FileCounter
{
    std::unordered_map<std::string_view, active_counter> map;
    std::forward_list<std::string> strings;
public:
    active_counter &operator[](std::string_view) noexcept;
};


template <class T>
class Preset
{
    T &globalValue;
    T localValue;
    bool initialized {false};

public:

    Preset(T &aGlobalValue) noexcept :
        globalValue(aGlobalValue)
    {
    }

    T get()
    {
        if (!initialized)
        {
            localValue = globalValue;
            initialized = true;
        }
        return localValue;
    }

    void set(T aLocalValue)
    {
        localValue = aLocalValue;
        globalValue = localValue;
        initialized = true;
    }
};

#endif // TURBO_APP_UTIL_H
