/***
  This file is part of avahi.
 
  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.
 
  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <sys/time.h>
#if defined(QT5) || defined(QT4)
#include <QSocketNotifier>
#include <QObject>
#include <QTimer>
#else
#include <qsocketnotifier.h>
#include <qobject.h>
#include <qtimer.h>
#endif
#include <avahi-common/timeval.h>
#include "qt-watch.h"

class AvahiWatch : public QObject 
{
    Q_OBJECT
public:
    AvahiWatch(int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void* userdata);
    ~AvahiWatch() {}
    AvahiWatchEvent getEvents() const { return m_incallback ? m_lastEvent : (AvahiWatchEvent)0; }
    void setWatchedEvents(AvahiWatchEvent event);

private slots:
    void gotIn();
    void gotOut();

private:
    QSocketNotifier* m_in;
    QSocketNotifier* m_out;
    //FIXME: ERR and HUP?
    AvahiWatchCallback m_callback;
    AvahiWatchEvent m_lastEvent;
    int m_fd;
    void* m_userdata;
    bool m_incallback;
};

class AvahiTimeout : public QObject 
{
    Q_OBJECT
    
public:
    AvahiTimeout(const struct timeval* tv, AvahiTimeoutCallback callback, void* userdata);
    ~AvahiTimeout() {}
    void update(const struct timeval* tv);
    
private slots:
    void timeout();
    
private:
    QTimer m_timer;
    AvahiTimeoutCallback m_callback;
    void* m_userdata;
};



AvahiWatch::AvahiWatch(int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void* userdata) : 
    m_in(0), m_out(0),  m_callback(callback), m_fd(fd), m_userdata(userdata), m_incallback(false)
{
    setWatchedEvents(event);
}

void AvahiWatch::gotIn()
{
    m_lastEvent = AVAHI_WATCH_IN;
    m_incallback=true;
    m_callback(this,m_fd,m_lastEvent,m_userdata);
    m_incallback=false;
}

void AvahiWatch::gotOut()
{
    m_lastEvent = AVAHI_WATCH_IN;
    m_incallback=true;
    m_callback(this,m_fd,m_lastEvent,m_userdata);
    m_incallback=false;
}

void AvahiWatch::setWatchedEvents(AvahiWatchEvent event) 
{
    if (!(event & AVAHI_WATCH_IN)) { delete m_in; m_in=0; }
    if (!(event & AVAHI_WATCH_OUT)) { delete m_out; m_out=0; }
    if (event & AVAHI_WATCH_IN) { 
	m_in = new QSocketNotifier(m_fd,QSocketNotifier::Read, this);
	connect(m_in,SIGNAL(activated(int)),SLOT(gotIn()));
    }
    if (event & AVAHI_WATCH_OUT) { 
	m_out = new QSocketNotifier(m_fd,QSocketNotifier::Write, this);
	connect(m_out,SIGNAL(activated(int)),SLOT(gotOut()));
    }
}    

AvahiTimeout::AvahiTimeout(const struct timeval* tv, AvahiTimeoutCallback callback, void *userdata) : 
    m_callback(callback), m_userdata(userdata)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
#if defined(QT5) || defined(QT4)
    m_timer.setSingleShot(true);
#endif
    update(tv);
}

void AvahiTimeout::update(const struct timeval *tv)
{
    m_timer.stop();
    if (tv) {
    AvahiUsec u = avahi_age(tv)/1000;
#if defined(QT5) || defined(QT4)
    m_timer.start( (u>0) ? 0 : -u);
#else
    m_timer.start( (u>0) ? 0 : -u,true);
#endif
    }
}

void AvahiTimeout::timeout()
{
    m_callback(this,m_userdata);
}

static AvahiWatch* q_watch_new(const AvahiPoll *api, int fd, AvahiWatchEvent event, AvahiWatchCallback callback, 
    void *userdata) 
{
    return new AvahiWatch(fd, event, callback, userdata);
}

static void q_watch_update(AvahiWatch *w, AvahiWatchEvent events) 
{
    w->setWatchedEvents(events);
}

static AvahiWatchEvent q_watch_get_events(AvahiWatch *w) 
{
    return w->getEvents();
}
    
static void q_watch_free(AvahiWatch *w) 
{
    delete w;
}
    
static AvahiTimeout* q_timeout_new(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback callback, 
    void *userdata) 
{
    return new AvahiTimeout(tv, callback, userdata);
}

static void q_timeout_update(AvahiTimeout *t, const struct timeval *tv) 
{
    t->update(tv);
}

static void q_timeout_free(AvahiTimeout *t) 
{
    delete t;
}

const AvahiPoll* avahi_qt_poll_get(void) 
{
    static const AvahiPoll qt_poll = {
        NULL,
        q_watch_new,
        q_watch_update,
        q_watch_get_events,
        q_watch_free,
        q_timeout_new,
        q_timeout_update,
        q_timeout_free
    };

    return &qt_poll;
}

#if defined(QT5)
#include "qt-watch.moc5"
#elif defined(QT4)
#include "qt-watch.moc4"
#elif defined(QT3)
#include "qt-watch.moc3"
#endif
