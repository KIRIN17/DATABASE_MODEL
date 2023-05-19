#include "database.h"
#include "condition_parser.h"
#include "test_functions.h"

#include <set>
#include <fstream>

/*
 * 1. Хранит данные (и сами данные и структуру
 * 2. Занимается взаимодействие с пользователем
 * 2.1 Приветсвие
 * 2.2. Логин
 * 2.3. Пригглашение ко входу
 * 2.4. Операции
 * 2.5. ....
 * 3. Вызовы команд
 */
int main() {
    TestAll();
    system("chcp 65001");

    map<pair<string, string>, Database> ACCOUNTS;

    string login;
    string password;

    cout << "Введите логин : ";
    cin >> login;
    cout << "Введите пароль : ";
    cin >> password;

    ACCOUNTS[{login, password}] = Database();

    //Database db;
    cout << "==============================================================================\n"
            "Введите HELP для получения большей информации об этой модели и её возможностях;\n"
            "\n"
            "Введите CHANGE для смены аккаунта;\n"
            "==============================================================================\n";

    for (string line; getline(cin, line);) {

        istringstream is(line);

        string command;
        is >> command;
        try {
            if (command == "HELP" || command == "help" || command == "Help") {
                cout << "Add date event — добавить в базу данных пару (date, event);\n"
                        "\n"
                        "Print — вывести всё содержимое базы данных;\n"
                        "\n"
                        "Find condition — вывести все записи, содержащиеся в базе данных, которые удовлетворяют условию condition;\n"
                        "\n"
                        "Del condition — удалить из базы все записи, которые удовлетворяют условию condition;\n"
                        "\n"
                        "Last date — вывести запись с последним событием, случившимся не позже данной даты.\n"
                        "\n"
                        "Условия в командах Find и Del накладывают определённые ограничения на даты и события, например:\n"
                        "\n"
                        "Find date < 2017-11-06 — найти все события, которые случились раньше 6 ноября 2017 года;\n"
                        "\n"
                        "Del event != \"holiday\" — удалить из базы все события, кроме «holiday»;\n"
                        "\n"
                        "Find date >= 2017-01-01 AND date < 2017-07-01 AND event == \"sport event\" — найти всё события «sport event», случившиеся в первой половине 2017 года;\n"
                        "\n"
                        "Del date < 2017-01-01 AND (event == \"holiday\" OR event == \"sport event\") — удалить из базы все события «holiday» и «sport event», случившиеся до 2017 года.\n"
                        "\n"
                        "В командах обоих типов условия могут быть пустыми: под такое условие попадают все события.\n";
            } else if (command == "CHANGE") {
                cout << "Введите логин : ";
                cin >> login;
                cout << "Введите пароль : ";
                cin >> password;
            } else if (command == "Add") {
                const auto date = ParseDate(is);
                const auto event = ParseEvent(is);
                ACCOUNTS[{login, password}].Add(date, event);
            } else if (command == "Print") {
                ACCOUNTS[{login, password}].Print(cout);
            } else if (command == "Del") {

                auto condition = ParseCondition(is);
                auto predicate = [condition](const Date &date, const string &event) {
                    return condition->Evaluate(date, event);
                };

                int count = ACCOUNTS[{login, password}].RemoveIf(predicate);
                cout << "Removed " << count << " entries" << endl;
            } else if (command == "Find") {
                auto condition = ParseCondition(is);
                auto predicate = [condition](const Date &date, const string &event) {
                    return condition->Evaluate(date, event);
                };

                const auto entries = ACCOUNTS[{login, password}].FindIf(predicate);
                for (const auto &entry: entries) {
                    cout << entry << endl;
                }
                cout << "Found " << entries.size() << " entries" << endl;
            } else if (command == "Last") {
                try {
                    cout << ACCOUNTS[{login, password}].Last(ParseDate(is)) << endl;
                } catch (invalid_argument &) {
                    cout << "No entries" << endl;
                }
            } else if (command.empty()) {
                continue;
            } else {
                //возможно сделать исключение, когда будет убрано из main
                //throw logic_error("Unknown command: " + command);

                cout << "Unknown command:" + command << endl;
            }
        } catch (logic_error e) {
            cout << e.what() << "\n";
        }
        catch(runtime_error r){
            cout << r.what() << "\n";
        }
    }

    return 0;
}
