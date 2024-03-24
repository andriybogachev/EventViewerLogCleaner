# Clear Event Logs (English)

Clear Event Logs is a Windows program that allows you to clear all event logs on your system. It features a user-friendly graphical interface that lets you monitor the progress of the clearing process and view lists of cleared logs.

## Features

- Clear all event logs on the system
- Display progress of log clearing with a progress bar and percentage
- Display the count of cleared logs
- Display the elapsed time since the start of the clearing process
- Ability to cancel the clearing operation
- Check and request administrative rights for running the program

## System Requirements

- Windows 7 or later
- Administrative rights for successful event log clearing

## Usage

1. Download or clone the repository to your computer.
2. Open the solution file in Microsoft Visual Studio.
3. Build the project.
4. Run the program. If you don't have administrative rights, the program will prompt you to grant them automatically.
5. Click the "Start" button to begin the event log clearing process.
6. Monitor the progress of the clearing in the graphical user interface.
7. After the clearing process is complete, you will see a message indicating successful clearing.
8. Optionally, you can cancel the clearing operation by clicking the "Cancel" button.

## Project Structure

The project consists of the following files:

- `main.cpp`: The main program file, containing code for creating the window and running the main message loop.
- `LogManager.hpp` and `LogManager.cpp`: Files containing functions related to event log operations.
- `UiUtils.hpp` and `UiUtils.cpp`: Files with helper functions for working with the user interface and handling window messages.
- `resource.h`: Resource file for the program icon.

## Versions

There are two versions of the program available:

1. **ClearEventViewerMultiThreaded**: This version uses multithreading to clear event logs in parallel, resulting in faster execution times, especially on systems with multiple CPU cores.

2. **ClearEventViewerSingleThreaded**: This version clears event logs sequentially on a single thread, which may be slower than the multithreaded version, but can be more suitable for systems with limited resources.

## Author

This program was created by Andrii Bohachev.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

# Clear Event Logs (Українська)

Clear Event Logs - це програма для Windows, яка дозволяє очищувати всі журнали подій на вашій системі. Вона має зручний графічний інтерфейс, який дозволяє відслідковувати прогрес очищення та переглядати списки очищених журналів.

## Функції

- Очищення всіх журналів подій на системі
- Відображення прогресу очищення журналів у вигляді прогрес-бару та відсотків
- Відображення кількості очищених журналів
- Відображення часу, що минув з початку очищення
- Можливість скасувати операцію очищення
- Перевірка та запит прав адміністратора для виконання програми

## Системні вимоги

- Windows 7 або новіша версія
- Права адміністратора для успішного очищення журналів подій

## Використання

1. Завантажте або склонуйте репозиторій на свій комп'ютер.
2. Відкрийте файл рішення в Microsoft Visual Studio.
3. Збережіть проєкт.
4. Запустіть програму. Якщо ви не маєте прав адміністратора, програма запросить їх автоматично.
5. Натисніть кнопку "Start", щоб розпочати процес очищення журналів подій.
6. Спостерігайте за прогресом очищення в графічному інтерфейсі.
7. Після завершення процесу очищення ви побачите повідомлення про успішне очищення.
8. За бажанням ви можете скасувати операцію очищення, натиснувши кнопку "Cancel".

## Структура проєкту

Проєкт складається з наступних файлів:

- `main.cpp`: Головний файл програми, містить код для створення вікна та запуску головного циклу повідомлень.
- `LogManager.hpp` та `LogManager.cpp`: Файли, які містять функції, пов'язані з операціями над журналами подій.
- `UiUtils.hpp` та `UiUtils.cpp`: Файли з допоміжними функціями для роботи з інтерфейсом користувача та обробки повідомлень вікна.
- `resource.h`: Файл ресурсів для іконки програми.

## Версії

Існує дві версії програми:

1. **ClearEventViewerMultiThreaded**: Ця версія використовує багатопоточність для паралельного очищення журналів подій, що призводить до швидшого виконання, особливо на системах з кількома ядрами процесора.

2. **ClearEventViewerSingleThreaded**: Ця версія очищує журнали подій послідовно в одному потоці, що може бути повільнішим за багатопоточну версію, але може бути більш підходящим для систем з обмеженими ресурсами.

## Автор

Ця програма була створена Андрієм Богачевим.

## Ліцензія

Цей проєкт розповсюджується під ліцензією MIT. Детальніше про умови ліцензії можна дізнатися у файлі [LICENSE](LICENSE).