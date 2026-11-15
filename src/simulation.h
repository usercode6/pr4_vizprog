#pragma once
#include <QObject>
#include <QHash>
#include <optional>
#include "jobmodel.h"

// Параметры одного запуска. Настройки фиксируются до сброса.
struct SimulationSettings {
    int secondsPerPage = 2;       // Время печати одной страницы.
    int queueCapacity = 5;        // Лимит ожидающих заданий, без текущего.
    int initialJobs = 3;          // Число заданий в момент запуска.
    int arrivalProbability = 30;  // Вероятность нового задания за такт, в процентах.
};

// Контроллер моделирования. Один вызов step() соответствует одной секунде.
class Simulation final : public QObject {
    Q_OBJECT
public:
    explicit Simulation(QObject *parent = nullptr);
    JobModel *waiting() { return &m_waiting; }
    JobModel *completed() { return &m_completed; }
    JobModel *rejected() { return &m_rejected; }
    const std::optional<PrintJob> &current() const { return m_current; }
    const SimulationSettings &settings() const { return m_settings; }
    int elapsed() const { return m_elapsed; }
    int remainingSeconds() const;
    bool initialized() const { return m_initialized; }
    void reset();
    void initialize(const SimulationSettings &settings);
    void step();
    void submitJob(QString name, int pages);
signals:
    void changed(); // Представление обновляет подписи и индикатор по этому сигналу.
private:
    void generateJob();
    void startNext();
    SimulationSettings m_settings;
    JobModel m_waiting, m_completed, m_rejected;
    std::optional<PrintJob> m_current; // Пустое значение означает свободный принтер.
    QHash<QString, int> m_fileCounters; // Отдельный счётчик для каждого названия.
    int m_elapsed = 0;
    int m_nextId = 1;
    int m_pageSeconds = 0; // Уже затраченное время на текущую страницу.
    bool m_initialized = false;
};
