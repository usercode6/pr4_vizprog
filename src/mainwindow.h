#pragma once
#include <QMainWindow>
#include <QTimer>
#include "simulation.h"
class QSpinBox;
class QPushButton;
class QLabel;
class QProgressBar;
class QGroupBox;

// Представление: создаёт интерфейс и связывает действия пользователя с моделью.
class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    void start();
    void pause();
    void reset();
    void refresh(); // Обновляет элементы окна по текущему состоянию моделирования.
    Simulation m_simulation;
    QTimer m_timer; // Источник тактов; на паузе время модели не меняется.
    QSpinBox *m_seconds, *m_capacity, *m_initial, *m_probability;
    QPushButton *m_start, *m_pause, *m_reset;
    QLabel *m_clock, *m_file, *m_details, *m_remaining;
    QProgressBar *m_progress;
    QGroupBox *m_waitingBox, *m_completedBox, *m_rejectedBox;
};
