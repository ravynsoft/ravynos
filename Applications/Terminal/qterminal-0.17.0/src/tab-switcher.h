#pragma once

#include <QListWidget>
#include <QAbstractListModel>

class TabWidget;

// -----------------------------------------------------------------------------------------------------------

class AppModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AppModel(QObject* parent, TabWidget* tabs);

protected:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    struct AppInfo {
        QString name;
        int index;
    };

    QList<AppInfo> m_list;
};

// -----------------------------------------------------------------------------------------------------------

class TabSwitcher: public QListView
{
    Q_OBJECT

public:
    TabSwitcher(TabWidget* tabs);
    ~TabSwitcher() override;
    void selectItem(bool forward = true);

signals:
    void activateTab(int index) const;

protected:
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *) override;

private:
    void showSwitcher();
    void timer();

private:
    QTimer *m_timer;
    TabWidget* m_tabs;
};

// -----------------------------------------------------------------------------------------------------------

