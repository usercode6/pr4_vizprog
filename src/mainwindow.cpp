#include "mainwindow.h"
#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>

namespace {
// Отображаем время модели в формате минуты: секунды.
QString timeText(int seconds) {
    return QString("%1:%2").arg(seconds / 60, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0'));
}
QSpinBox *spin(int min, int max, int value, QWidget *parent) {
    auto *s = new QSpinBox(parent); s->setRange(min, max); s->setValue(value);
    s->setMinimumWidth(75); return s;
}

// Каждый QListView получает собственную модель заданий.
QGroupBox *listBox(const QString &title, JobModel *model, QWidget *parent) {
    auto *box = new QGroupBox(title, parent);
    auto *layout = new QVBoxLayout(box);
    auto *view = new QListView(box);
    view->setModel(model);
    view->setAlternatingRowColors(true);
    view->setWordWrap(true);
    view->setSpacing(4);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(view);
    return box;
}
}

// Конструктор главного окна.
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("Моделирование очереди печати"));
    setWindowIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    resize(1150, 730); setMinimumSize(950, 600);
    auto *central = new QWidget(this); setCentralWidget(central);
    auto *root = new QVBoxLayout(central); root->setSpacing(16);
    root->setContentsMargins(16, 16, 16, 12);

    // Верхний блок: параметры генерации и управление запуском.
    auto *settings = new QGroupBox(tr("Настройки"), central);
    auto *settingsLayout = new QVBoxLayout(settings);
    auto *fields = new QHBoxLayout;

    m_seconds = spin(1, 60, 2, settings);
    m_capacity = spin(1, 1000, 5, settings);
    m_initial = spin(0, 5, 3, settings);
    m_probability = spin(0, 100, 30, settings); m_probability->setSuffix(" %");

    const QList<QPair<QString, QSpinBox*>> inputs = {
        {tr("Секунд на страницу:"), m_seconds}, {tr("Размер очереди:"), m_capacity},
        {tr("Начальных заданий:"), m_initial}, {tr("Вероятность поступления:"), m_probability}};
    for (const auto &field : inputs) {
        auto *label = new QLabel(field.first, settings); label->setBuddy(field.second);
        fields->addWidget(label); fields->addWidget(field.second);
    }

    settingsLayout->addLayout(fields);
    m_capacity->setToolTip(tr("Число ожидающих заданий. Печатающееся задание в лимит не входит."));
    m_probability->setToolTip(tr("Вероятность появления одного задания каждую секунду модели."));

    // Начальных заданий не может быть больше вместимости очереди.
    connect(m_capacity, qOverload<int>(&QSpinBox::valueChanged), m_initial, &QSpinBox::setMaximum);
    connect(m_capacity, qOverload<int>(&QSpinBox::valueChanged), this, [this] { refresh(); });
    auto *buttons = new QHBoxLayout;

    m_start = new QPushButton(tr("Старт"), settings); m_start->setObjectName("startButton");
    m_pause = new QPushButton(tr("Пауза"), settings);
    m_reset = new QPushButton(tr("Сброс"), settings);
    for (auto *b : {m_start, m_pause, m_reset}) { b->setMinimumSize(120, 36); buttons->addWidget(b); }
    buttons->addStretch(); m_clock = new QLabel(settings); buttons->addWidget(m_clock);
    settingsLayout->addSpacing(8); settingsLayout->addLayout(buttons);
    root->addWidget(settings);

    // Сведения о единственном задании, находящемся внутри принтера.
    auto *printing = new QGroupBox(tr("Сейчас печатается"), central);
    auto *printingLayout = new QVBoxLayout(printing);
    m_file = new QLabel(printing); m_file->setObjectName("fileName");
    m_details = new QLabel(printing);
    printingLayout->addWidget(m_file); printingLayout->addWidget(m_details);
    auto *progressRow = new QHBoxLayout;
    m_progress = new QProgressBar(printing); m_progress->setMinimumHeight(24);
    m_remaining = new QLabel(printing); m_remaining->setMinimumWidth(140);
    progressRow->addWidget(m_progress, 1); progressRow->addWidget(m_remaining);
    printingLayout->addLayout(progressRow); root->addWidget(printing);

    // Три независимых списка: ожидающие, выполненные и отклонённые задания.
    auto *lists = new QHBoxLayout;
    m_waitingBox = listBox({}, m_simulation.waiting(), central);
    m_completedBox = listBox({}, m_simulation.completed(), central);
    m_rejectedBox = listBox({}, m_simulation.rejected(), central);
    m_completedBox->setObjectName("completed"); m_rejectedBox->setObjectName("rejected");
    lists->addWidget(m_waitingBox, 1); lists->addWidget(m_completedBox, 1); lists->addWidget(m_rejectedBox, 1);
    root->addLayout(lists, 1);

    // Явные цвета обеспечивают читаемость при любой системной теме.
    setStyleSheet(
        "QWidget { color: #17212f; }"
        "QMainWindow { background: #f3f5f8; }"
        "QGroupBox { background: white; border: 1px solid #d5dbe3; border-radius: 6px; margin-top: 12px; padding: 16px 8px 8px; font-weight: bold; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 5px; }"
        "QListView { border: 0; background: white; alternate-background-color: #f1f5fa; font-weight: normal; }"
        "QListView::item { padding: 9px; border-bottom: 1px solid #e7ebf0; }"
        "QListView::item:selected { background: #d8eaff; color: #15263d; }"
        "QPushButton { padding: 5px 15px; }"
        "QPushButton#startButton:enabled { background: #c7e0ff; color: #10243b; border: 1px solid #619ce5; border-radius: 4px; }"
        "QPushButton:disabled, QSpinBox:disabled { color: #526173; background: #edf0f4; }"
        "QSpinBox { color: #17212f; background: white; }"
        "QToolTip { color: #17212f; background: #fff7dc; border: 1px solid #c5b879; }"
        "QLabel#fileName { font-size: 22px; font-weight: bold; }"
        "QProgressBar { border: 1px solid #ced5df; border-radius: 4px; text-align: center; background: #edf1f6; }"
        "QProgressBar::chunk { background: #087af5; }"
        "QGroupBox#completed::title { color: #287f42; }"
        "QGroupBox#rejected::title { color: #b6333c; }");

    // Раз в секунду продвигаем модель на один такт и обновляем отображение.
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, &m_simulation, &Simulation::step);
    connect(&m_simulation, &Simulation::changed, this, &MainWindow::refresh);
    connect(m_start, &QPushButton::clicked, this, &MainWindow::start);
    connect(m_pause, &QPushButton::clicked, this, &MainWindow::pause);
    connect(m_reset, &QPushButton::clicked, this, &MainWindow::reset);
    refresh();
}
// После паузы продолжаем существующий запуск, не генерируя очередь заново.
void MainWindow::start() {
    if (!m_simulation.initialized()) {
        m_simulation.initialize({m_seconds->value(), m_capacity->value(), m_initial->value(), m_probability->value()});
    }
    m_timer.start(); refresh();
}
// Остановка таймера сохраняет все задания и оставшееся время печати.
void MainWindow::pause() { m_timer.stop(); refresh(); }

// Сначала останавливаем таймер, затем очищаем состояние модели.
void MainWindow::reset() { m_timer.stop(); m_simulation.reset(); refresh(); }

// Виджеты читают состояние контроллера; алгоритм печати здесь не выполняется.
void MainWindow::refresh() {
    const bool active = m_timer.isActive(), initialized = m_simulation.initialized();
    m_start->setEnabled(!active); m_start->setText(initialized ? tr("Продолжить") : tr("Старт"));
    m_pause->setEnabled(active); m_reset->setEnabled(initialized);

    // Параметры можно менять только до запуска или после сброса.
    for (auto *s : {m_seconds, m_capacity, m_initial, m_probability}) s->setEnabled(!initialized);
    m_clock->setText(tr("Время модели: %1").arg(timeText(m_simulation.elapsed())));
    m_waitingBox->setTitle(tr("Ожидают – %1 / %2").arg(m_simulation.waiting()->rowCount()).arg(initialized ? m_simulation.settings().queueCapacity : m_capacity->value()));
    m_completedBox->setTitle(tr("Выполнены – %1").arg(m_simulation.completed()->rowCount()));
    m_rejectedBox->setTitle(tr("Отклонены – %1").arg(m_simulation.rejected()->rowCount()));
    if (m_simulation.current()) {
        const auto &job = *m_simulation.current();
        m_file->setText(job.fileName);
        m_details->setText(tr("Задание № %1 · Напечатано %2 из %3 страниц").arg(job.id).arg(job.printedPages).arg(job.pages));

        // Шкала показывает затраченное время, включая неполную страницу.
        const int total = job.pages * m_simulation.settings().secondsPerPage;
        m_progress->setRange(0, total); m_progress->setValue(total - m_simulation.remainingSeconds());
        m_remaining->setText(tr("Осталось: %1 с").arg(m_simulation.remainingSeconds()));
    } else {
        m_file->setText(tr("Принтер свободен"));
        m_details->setText(initialized ? tr("Ожидание нового задания") : tr("Укажите настройки и нажмите «Старт»"));
        m_progress->setRange(0, 100); m_progress->setValue(0); m_remaining->setText(tr("Осталось: —"));
    }
    statusBar()->showMessage(active ? tr("Моделирование запущено") : initialized ? tr("Моделирование приостановлено") : tr("Готово к запуску"));
}
