#pragma once
#include <QString>
#include <QMetaType>

// Данные одного задания. Общее время хранится в секундах модели.
struct PrintJob {
    int id = 0;                 // Уникальный номер задания в текущем запуске.
    QString fileName;           // Название с порядковым номером и расширением.
    int arrivalTime = 0;        // Момент поступления задания.
    int pages = 1;              // Общее число страниц.
    int printedPages = 0;       // Число полностью напечатанных страниц.
    int completedTime = -1;     // До завершения печати равен -1.
    QString reason;             // Причина отказа; для принятых заданий пуста.
};
// Регистрация типа позволяет передавать структуру через QVariant.
Q_DECLARE_METATYPE(PrintJob)
