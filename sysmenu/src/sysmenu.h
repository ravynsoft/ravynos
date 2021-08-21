/*
*/

#include <Plasma/Applet>
#include <QMenu>

class AiryxMenu: public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QMenu *menu READ menu WRITE setMenu NOTIFY menuChanged)

public:
    AiryxMenu(QObject *parent, const QVariantList &args);
    ~AiryxMenu();
    QMenu *menu() const;
    void setMenu(QMenu *menu);

signals:
    void menuChanged();

private slots:

private:
    QMenu *m_menu;
};
