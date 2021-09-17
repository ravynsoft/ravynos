/*  SPDX-License-Identifier: LGPL-2.0-or-later

    SPDX-FileCopyrightText: 2014 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_UPDATE_DISABLER
#define KATE_UPDATE_DISABLER

#include <QPointer>
#include <QWidget>

class KateUpdateDisabler
{
public:
    /**
     * Disable updates for given widget.
     * Will auto-enable them on destruction, like a mutex locker releases its lock.
     * @param widget widget to disable updates for
     */
    explicit KateUpdateDisabler(QWidget *widget)
        : m_widget((widget && widget->updatesEnabled()) ? widget : nullptr)
    {
        if (m_widget) {
            m_widget->setUpdatesEnabled(false);
        }
    }

    /**
     * Enable updates again on destruction.
     */
    ~KateUpdateDisabler()
    {
        if (m_widget) {
            m_widget->setUpdatesEnabled(true);
        }
    }

private:
    /**
     * No copying please
     */
    Q_DISABLE_COPY(KateUpdateDisabler)

    /**
     * pointer to widget, if not null, enable/disable widgets
     */
    QPointer<QWidget> m_widget;
};

#endif
