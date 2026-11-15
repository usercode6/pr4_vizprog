#include "simulation.h"
#include <QRandomGenerator>
#include <QStringList>
#include <algorithm>

Simulation::Simulation(QObject *parent) : QObject(parent) {}
// Возвращаем модель в исходное состояние, включая номера заданий и файлов.
void Simulation::reset() {
    m_waiting.clear(); m_completed.clear(); m_rejected.clear();
    m_current.reset(); m_elapsed = 0; m_nextId = 1; m_pageSeconds = 0;
    m_fileCounters.clear();
    m_initialized = false;
    emit changed();
}
// Проверяем границы параметров и создаём начальную очередь в момент времени 0.
void Simulation::initialize(const SimulationSettings &settings) {
    reset();
    m_settings = settings;
    m_settings.secondsPerPage = std::clamp(settings.secondsPerPage, 1, 60);
    m_settings.queueCapacity = std::clamp(settings.queueCapacity, 1, 1000);
    m_settings.initialJobs = std::clamp(settings.initialJobs, 0, m_settings.queueCapacity);
    m_settings.arrivalProbability = std::clamp(settings.arrivalProbability, 0, 100);
    m_initialized = true;
    for (int i = 0; i < m_settings.initialJobs; ++i) generateJob();
    startNext();
    emit changed();
}
// Генерация условного документа: содержимое реальных файлов не используется.
void Simulation::generateJob() {
    static const QStringList names = {"Отчёт.pdf", "Доклад.docx", "Таблица.xlsx", "Конспект.pdf", "План.docx", "Заявление.pdf"};
    auto *rng = QRandomGenerator::global();
    const QString baseName = names.at(rng->bounded(static_cast<int>(names.size())));
    // Номер увеличивается и для отклонённых заданий, поэтому имена не повторяются.
    const int number = ++m_fileCounters[baseName];
    QString numberedName = baseName;
    // Вставляем номер перед расширением.
    const int dot = numberedName.lastIndexOf('.');
    numberedName.insert(dot > 0 ? dot : numberedName.size(), QString(" №%1").arg(number));
    submitJob(numberedName, rng->bounded(1, 11));
}
// Лимит относится только к ожидающим заданиям; текущее печатается отдельно.
void Simulation::submitJob(QString name, int pages) {
    if (!m_initialized) return;
    PrintJob job;
    job.id = m_nextId++; job.fileName = std::move(name);
    job.arrivalTime = m_elapsed; job.pages = std::max(1, pages);
    if (m_waiting.rowCount() >= m_settings.queueCapacity) {
        job.reason = tr("Очередь заполнена"); m_rejected.append(job);
    } else m_waiting.append(job);
    emit changed();
}
// Освобождение принтера.
void Simulation::startNext() {
    if (!m_current && m_waiting.rowCount() > 0) {
        m_current = m_waiting.takeFirst(); m_pageSeconds = 0;
    }
}
// Учитываем и целые оставшиеся страницы, и время внутри текущей страницы.
int Simulation::remainingSeconds() const {
    return m_current ? (m_current->pages - m_current->printedPages) * m_settings.secondsPerPage - m_pageSeconds : 0;
}
void Simulation::step() {
    if (!m_initialized) return;
    ++m_elapsed;
    // Время печати получает только задание, печатавшееся в течение этого такта.
    if (m_current && ++m_pageSeconds >= m_settings.secondsPerPage) {
        m_pageSeconds = 0;
        if (++m_current->printedPages >= m_current->pages) {
            m_current->completedTime = m_elapsed;
            m_completed.append(*m_current); m_current.reset();
        }
    }
    startNext();
    if (QRandomGenerator::global()->bounded(100) < m_settings.arrivalProbability) generateJob();
    startNext();
    emit changed();
}
