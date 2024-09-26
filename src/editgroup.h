#ifndef EDITGROUP_H
#define EDITGROUP_H

#include <QWidget>
#include <QSqlRecord>

namespace Ui {
class EditGroup;
}

class EditGroup : public QWidget
{
    Q_OBJECT

public:
    explicit EditGroup(QWidget *parent = nullptr);
    ~EditGroup();

signals:
    void switchToDatabaseScreenRequest();
    void dataForAddingIsReady(QSqlRecord r);
    void dataForUpdatingIsReady(QSqlRecord r, int row);

private slots:
    void setUpdateData(bool m, int row, QSqlRecord record, QString name);
    void on_apply_button_clicked();
    void on_cancel_button_clicked();

private:
    Ui::EditGroup *ui;
    bool mode;
    int editedGroupRow;

    void clearAllLineEdits();
    QSqlRecord getRecordFromLines();
};

#endif // EDITGROUP_H
