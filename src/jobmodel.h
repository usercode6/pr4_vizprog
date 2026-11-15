#pragma once
#include <QAbstractListModel>
#include <QVector>
#include "printjob.h"

// Собственная линейная модель: хранит задания и предоставляет данные QListView. Для каждого списка используется отдельный экземпляр этого класса.
class JobModel final : public QAbstractListModel {
    Q_OBJECT
public:
    // Пользовательские роли: вся структура задания или отдельные поля.
    enum Role { JobRole = Qt::UserRole, IdRole, FileNameRole, ArrivalRole, PagesRole };
    explicit JobModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Изменения коллекции сопровождаются уведомлениями для представления.
    void append(const PrintJob &job);
    PrintJob takeFirst();
    void clear();
    const QVector<PrintJob> &jobs() const { return m_jobs; }
private:
    QVector<PrintJob> m_jobs;
};
