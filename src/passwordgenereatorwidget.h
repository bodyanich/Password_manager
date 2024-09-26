#ifndef PASSWORDGENEREATORWIDGET_H
#define PASSWORDGENEREATORWIDGET_H

#include <QWidget>
#include "passwordgeneratorcore.h"

namespace Ui {
class PasswordGenereatorWidget;
}

class PasswordGenereatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordGenereatorWidget(QWidget *parent = nullptr);
    ~PasswordGenereatorWidget();

signals:
    void closed();

public slots:
    void regeneratePassword();
    void copyPassword();


private slots:
    void on_lineEdit_textChanged(const QString &arg1);
    void passwordLengthChanged(int length);
    void updateGenerator();

private:
    Ui::PasswordGenereatorWidget *ui;
    PasswordGenerator::CharClasses charClasses();
    PasswordGenerator::GeneratorFlags generatorFlags();
    const QScopedPointer<PasswordGenerator> m_passwordGenerator;
    QClipboard* pclipboard;
};

#endif // PASSWORDGENEREATORWIDGET_H
