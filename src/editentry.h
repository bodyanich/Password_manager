#ifndef EDITENTRY_H
#define EDITENTRY_H

#include <QWidget>
#include <QSqlRecord>

namespace Ui {
class EditEntry;
}

class EditEntry : public QWidget
{
    Q_OBJECT

public:
    explicit EditEntry(QWidget *parent = nullptr);
    ~EditEntry();

private slots:
    void setAddData(bool m, int id, QString name);
    void setUpdateData(bool m, int row, QSqlRecord record, QString name);
    void on_apply_button_clicked();
    void on_cancel_button_clicked();

    void on_lineEdit_password_textChanged(const QString &arg1);
    void calculateEtropy(QString pwd);

    void on_pass_button_clicked();

    void on_gen_button_clicked();

signals:
    void switchToDatabaseScreenRequest();
    void dataForAddingIsReady(QSqlRecord r);
    void dataForUpdatingIsReady(QSqlRecord r, int row);
    void toPasswordGenerator();

private:
    Ui::EditEntry *ui;
    bool mode;
    int group_id;
    QString group_name;
    int editedEntryRow;
    bool pass_mode = true;
    QPixmap pass_mode_view;
    QPixmap pass_mode_hide;
    void ChangeButtonPassMode();
    void ChangeButtonPassGen();

    void clearAllLineEdits();
    QSqlRecord getRecordFromLines();

};

#endif // EDITENTRY_H
