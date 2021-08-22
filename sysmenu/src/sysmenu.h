/*
*/

#include <Plasma/Applet>
#include <QMessageBox>

/* A simple wrapper around QMessageBox that allows closing it
 * with the titlebar button */
class AXMessageBox: public QMessageBox
{
public:
    explicit AXMessageBox();
protected:
    void closeEvent(QCloseEvent *event) override;
};

class AiryxMenu: public Plasma::Applet
{
    Q_OBJECT

public:
    AiryxMenu(QObject *parent, const QVariantList &args);
    ~AiryxMenu();

    Q_INVOKABLE void aboutThisComputer();

signals:

private slots:
    void aboutFinished();

private:
    AXMessageBox *m_about;
};
