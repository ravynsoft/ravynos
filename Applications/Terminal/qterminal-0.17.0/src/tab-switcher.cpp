#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QStyledItemDelegate>

#include "tab-switcher.h"
#include "tabwidget.h"

// -----------------------------------------------------------------------------------------------------------

enum class AppRole {
    Display = Qt::DisplayRole,
    Index   = Qt::UserRole +1
};

// -----------------------------------------------------------------------------------------------------------

AppModel::AppModel(QObject* parent, TabWidget* tabs):
    QAbstractListModel(parent)
{
    for(QWidget* w: tabs->history()) {
        int index = w->property("tab_index").toInt();
        m_list.append({tabs->tabText(index), index});
    }
}

int AppModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_list.size();
}

QVariant AppModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_list.size())
        return QVariant();

    switch(static_cast<AppRole>(role)) {
    case AppRole::Display:
        return m_list[index.row()].name;
    case AppRole::Index:
        return m_list[index.row()].index;
    }

    return {};
}

// -----------------------------------------------------------------------------------------------------------

class AppItemDelegate: public QStyledItemDelegate
{
public:
    AppItemDelegate(int frameWidth = 0, QWidget* parent = nullptr) :
        QStyledItemDelegate(parent),
        mParent(parent),
        mFrameWidth(frameWidth) {}

protected:
    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override
    {
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();

        QString text = index.model()->data(index, static_cast<int>(AppRole::Display)).toString();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text = text;
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, option.widget);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.decorationSize = QSize(0, 0);
        opt.text = index.model()->data(index, static_cast<int>(AppRole::Display)).toString();;
        const QWidget* widget = option.widget;
        QStyle* style = widget ? widget->style() : QApplication::style();
        QSize contSize = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);

        return QSize(
            mParent ? qMin(mParent->width() - 2 * mFrameWidth, contSize.width()) : contSize.width(),
            contSize.height()
        );
    }
private:
    QWidget* mParent;
    int mFrameWidth;
};

// -----------------------------------------------------------------------------------------------------------

TabSwitcher::TabSwitcher(TabWidget* tabs):
    QListView(tabs),
    m_tabs(tabs)
{
    setWindowFlags(Qt::Widget | Qt::Popup | Qt::WindowStaysOnTopHint);
    setItemDelegate(new AppItemDelegate(frameWidth(), tabs));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    m_timer->setSingleShot(true);

    connect(m_timer, &QTimer::timeout, this, &TabSwitcher::timer);
}

TabSwitcher::~TabSwitcher()
{}

void TabSwitcher::showSwitcher()
{
    setModel(new AppModel(this, m_tabs));

    if (!model()->rowCount())
        return;

    QStyleOptionViewItem option;

    int w = 0;
    int h = 0;
    int maxApp = 15;

    for(int i = 0; i < model()->rowCount(); ++i){
        QSize siz = itemDelegate()->sizeHint(option, model()->index(i, 0));
        w = qMax(w, siz.width());
        h += siz.height();
        if (i > maxApp)
            break;
    }


    w += 2 * frameWidth();
    h += 2 * frameWidth();
    resize(w, h);

    QPoint pos = m_tabs->mapToGlobal(m_tabs->geometry().topLeft());
    move(pos.x()+m_tabs->geometry().width()/2 - w / 2, pos.y()+m_tabs->geometry().height()/2 - h / 2);

    show();
}

void TabSwitcher::selectItem(bool forward)
{
    if (!isVisible())
        showSwitcher();

    int current = currentIndex().row();
    current = (current < 0) ? 0 : current;

    m_timer->start();
    current += forward ? 1 : -1;

    if(current >= model()->rowCount())
        current = 0;

    if(current < 0)
        current = model()->rowCount()-1;

    setCurrentIndex(model()->index(current, 0));
}

void TabSwitcher::keyReleaseEvent(QKeyEvent *event)
{
    if (event->modifiers() == 0)
        close();

    QWidget::keyReleaseEvent(event);
}

void TabSwitcher::timer()
{
    if (QApplication::queryKeyboardModifiers() == Qt::NoModifier)
        close();
    else
        m_timer->start();
}

void TabSwitcher::closeEvent(QCloseEvent *)
{
    m_timer->stop();
    Q_EMIT activateTab(model()->data(model()->index(currentIndex().row(), 0), static_cast<int>(AppRole::Index)).value<int>());
}

// -----------------------------------------------------------------------------------------------------------

