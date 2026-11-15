#include "jobmodel.h"

JobModel::JobModel(QObject *parent) : QAbstractListModel(parent) {}
// У линейной модели нет дочерних элементов: строки есть только у корня.
int JobModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_jobs.size());
}
// Роль определяет, какие сведения запросило представление.
QVariant JobModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= m_jobs.size()) return {};
    const auto &j = m_jobs[index.row()];
    switch (role) {
    // Основной текст элемента списка.
    case Qt::DisplayRole:
        return QString("№ %1   %2\n%3 стр. · поступило: %4 с%5").arg(j.id).arg(j.fileName).arg(j.pages).arg(j.arrivalTime).arg(j.reason.isEmpty() ? QString() : "\n" + j.reason);
    case Qt::EditRole: case FileNameRole: return j.fileName;
    // Подробности при наведении указателя на задание.
    case Qt::ToolTipRole:
        return QString("Задание № %1\nФайл: %2\nСтраниц: %3\nПоступило: %4 с%5").arg(j.id).arg(j.fileName).arg(j.pages).arg(j.arrivalTime).arg(j.completedTime < 0 ? QString() : QString("\nЗавершено: %1 с").arg(j.completedTime));
    case JobRole: return QVariant::fromValue(j);
    case IdRole: return j.id;
    case ArrivalRole: return j.arrivalTime;
    case PagesRole: return j.pages;
    default: return {};
    }
}
QVariant JobModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal && section == 0) return tr("Задания печати");
    return {};
}
// Задания можно выделять, но нельзя редактировать прямо в списке.
Qt::ItemFlags JobModel::flags(const QModelIndex &index) const {
    return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}
QHash<int, QByteArray> JobModel::roleNames() const {
    return {{JobRole,"job"},{IdRole,"jobId"},{FileNameRole,"fileName"},{ArrivalRole,"arrivalTime"},{PagesRole,"pages"}};
}
void JobModel::append(const PrintJob &job) {
    const int row = rowCount();
    // Смена данных и завершение операций.
    beginInsertRows({}, row, row);
    m_jobs.append(job);
    endInsertRows();
}
// Извлечение из начала списка обеспечивает порядок FIFO.
PrintJob JobModel::takeFirst() {
    // Проверка, что очередь не пуста.
    Q_ASSERT(!m_jobs.isEmpty());
    beginRemoveRows({}, 0, 0);
    const auto job = m_jobs.takeFirst();
    endRemoveRows();
    return job;
}
// Полный сброс данных содержимого.
void JobModel::clear() {
    if (m_jobs.isEmpty()) return;
    beginResetModel();
    m_jobs.clear();
    endResetModel();
}
