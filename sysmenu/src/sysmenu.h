/*
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
