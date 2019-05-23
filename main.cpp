#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>
#include <iomanip>

using namespace std;

bool button_is_pressed() {
    termios term;
    tcgetattr(0, &term);


    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}           // вернет true, если какая-то кнопка на клавиатуре нажата, иначе вернет false

char get_char_from_terminal() {
    struct termios oldt, newt;                  // состояния терминалов olddt - до, newt - после приема
    char ch;                                     // в нее запишется нажатая клавиша
    tcgetattr(STDIN_FILENO, &oldt);             //
    newt = oldt;                                // запомнили старый режим
    newt.c_lflag &= ~(ICANON |
                      ECHO);           // Во время приема необходим неканоничный ввод. Отключаем флаги "каноничный" и "эхо"
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);    // переключаемся в новый режим
    ch = getchar();                             // приняли клавишу
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);    // переключились в старый режим
    return ch;
}      // вернет нажатый символ

string status_game;


// интерактивное меню
class Menu {
    friend class Game;

public:
    Menu() {
        menu_pos = 1;
        menu_status = "Main";
    }

    void menu() {
        while (status_game == "New game") {
            if (menu_status == "Main") {
                start_menu();
            } else if (menu_status == "Help") {
                help_menu();
            }

            if (button_is_pressed()) {
                if (menu_status == "Help") {
                    menu_status = "Main";
                    char get_char = get_char_from_terminal();
                    continue;
                }
                switch (get_char_from_terminal()) {
                    case 'w':
                        if (menu_status == "Main") {
                            if (menu_pos > 1)
                                menu_pos--;
                        }
                        break;
                    case 's':
                        if (menu_status == "Main") {
                            if (menu_pos < 3)
                                menu_pos++;
                        }
                        break;
                    case '\n':
                        if (menu_status == "Main") {
                            if (menu_pos == 1) {
//                                cout << "";
                                return;
                            }
                            if (menu_pos == 2) {
                                menu_pos = 1;
                                menu_status = "Help";
                            }
                            if (menu_pos == 3) {
                                system("clear");
                                exit(0);
                            }
                        }
                        break;
                }
            }
            usleep(100000);     // задержка терминала
        }
    }

    void start_menu() {
        system("clear");
        switch (menu_pos) {
            case 1:
                cout << "################################################" << endl;
                cout << "#     Добро пожаловать в игру CZ v.1.0.0       #" << endl;
                cout << "################################################" << endl;
                cout << "# ->  1) Начать игру                           #" << endl;
                cout << "#     2) Помощь                                #" << endl;
                cout << "#     3) Выход                                 #" << endl;
                cout << "################################################" << endl;
                break;
            case 2:
                cout << "################################################" << endl;
                cout << "#     Добро пожаловать в игру CZ v.1.0.0       #" << endl;
                cout << "################################################" << endl;
                cout << "#     1) Начать игру                           #" << endl;
                cout << "# ->  2) Помощь                                #" << endl;
                cout << "#     3) Выход                                 #" << endl;
                cout << "################################################" << endl;
                break;
            case 3:
                cout << "################################################" << endl;
                cout << "#     Добро пожаловать в игру CZ v.1.0.0       #" << endl;
                cout << "################################################" << endl;
                cout << "#     1) Начать игру                           #" << endl;
                cout << "#     2) Помощь                                #" << endl;
                cout << "# ->  3) Выход                                 #" << endl;
                cout << "################################################" << endl;
                break;
        }
    }

    void help_menu() {
        system("clear");
        cout << "#################################################" << endl;
        cout << "#               Крестики-нолики                 #" << endl;
        cout << "#################################################" << endl;
        cout << "# Выбирайте клетку куда поставить свой крестик  #" << endl;
        cout << "#     цифрой от 1 до 9.                         #" << endl;
        cout << "# Ваша задача занять 3 клетки в одном линии.    #" << endl;
        cout << "# Удачи!                                        #" << endl;
        cout << "#################################################" << endl;
    }

    void game_over() {
        system("clear");
        cout << "################################################" << endl;
        cout << "#               Вы проиграли!                  #" << endl;
        cout << "################################################" << endl;
        cout << "#          Удачи в следующий раз!              #" << endl;
        cout << "#               Хорошего дня!                  #" << endl;
        cout << "################################################" << endl;
        usleep(2000000);
        system("clear");
    }

    void game_win() {
        system("clear");
        cout << "################################################" << endl;
        cout << "#                 Вы победили!                 #" << endl;
        cout << "################################################" << endl;
        cout << "#                 Поздравляем!                 #" << endl;
        cout << "#                Хорошего дня!                 #" << endl;
        cout << "################################################" << endl;
        usleep(2000000);
        system("clear");
    }

private:
    int menu_pos;
    string menu_status;
};

//  сама игра и её логика
class Game {
    friend Menu;
public:
    Game() {
        status_game = "New game";
        srand(time(nullptr));
        menu.menu();
        place.resize(3);
        vector<char> vec(3, ' ');
        for (int i = 0; i < place.size(); i++) {
            place[i] = vec;
        }
        turn = 0;
    }

    void play() {
        while (status_game != "Game over") {
            print();            // рисуем
            control();          // ловим нажатия
            print();            // рисуем
            logic();            // обрабатываем
            print();            // рисуем
            usleep(150000);     // задержка терминала
        }
        usleep(2000000);
        if (perhaps_win() == 'X') {
            menu.game_win();
        } else  {
            menu.game_over();
        }
    }

private:
    char perhaps_win() {
        if (place[0][0] == 'X' && place[1][0] == 'X' && place[2][0] == 'X' ||
            place[0][0] == 'O' && place[1][0] == 'O' && place[2][0] == 'O')
            return place[0][0];

        if (place[0][1] == 'X' && place[1][1] == 'X' && place[2][1] == 'X' ||
            place[0][1] == 'O' && place[1][1] == 'O' && place[2][1] == 'O')
            return place[0][1];

        if (place[0][2] == 'X' && place[1][2] == 'X' && place[2][2] == 'X' ||
            place[0][2] == 'O' && place[1][2] == 'O' && place[2][2] == 'O')
            return place[0][2];


        if (place[0][0] == 'X' && place[0][1] == 'X' && place[0][2] == 'X' ||
            place[0][0] == 'O' && place[0][1] == 'O' && place[0][2] == 'O')
            return place[0][0];

        if (place[1][0] == 'X' && place[1][1] == 'X' && place[1][2] == 'X' ||
            place[1][0] == 'O' && place[1][1] == 'O' && place[1][2] == 'O')
            return place[1][0];

        if (place[2][0] == 'X' && place[2][1] == 'X' && place[2][2] == 'X' ||
            place[2][0] == 'O' && place[2][1] == 'O' && place[2][2] == 'O')
            return place[2][0];

//        диагонали
        if (place[0][0] == 'X' && place[1][1] == 'X' && place[2][2] == 'X' ||
            place[0][0] == 'O' && place[1][1] == 'O' && place[2][2] == 'O')
            return place[0][0];

        if (place[0][2] == 'X' && place[1][1] == 'X' && place[2][0] == 'X' ||
            place[0][2] == 'O' && place[1][1] == 'O' && place[2][0] == 'O')
            return place[0][2];

        return ' ';

    } 

    void control() {
        turn++;
        int pos;
        while (true) {
            char ch = get_char_from_terminal();
            pos = static_cast<int>(ch) - 48;
            bool flag = true;
            for (int i = 0; i < turns.size(); i++) {
                if (turns[i] == pos) {
                    flag = false;
                }
            }
            if (flag)
                break;
        }
        turns.push_back(pos);
        switch (pos) {
            case 1:
                if (place[0][0] == ' ') {        // проверка, чтобы не затереть нолик
                    place[0][0] = 'X';
                }
                break;
            case 2:
                if (place[0][1] == ' ') {        // проверка, чтобы не затереть нолик
                    place[0][1] = 'X';
                }
                break;
            case 3:
                if (place[0][2] == ' ') {        // проверка, чтобы не затереть нолик
                    place[0][2] = 'X';
                }
                break;
            case 4:
                if (place[1][0] == ' ') {        // проверка, чтобы не затереть нолик
                    place[1][0] = 'X';
                }
                break;
            case 5:
                if (place[1][1] == ' ') {        // проверка, чтобы не затереть нолик
                    place[1][1] = 'X';
                }
                break;
            case 6:
                if (place[1][2] == ' ') {        // проверка, чтобы не затереть нолик
                    place[1][2] = 'X';
                }
                break;
            case 7:
                if (place[2][0] == ' ') {        // проверка, чтобы не затереть нолик
                    place[2][0] = 'X';
                }
                break;
            case 8:
                if (place[2][1] == ' ') {        // проверка, чтобы не затереть нолик
                    place[2][1] = 'X';
                }
                break;
            case 9:
                if (place[2][2] == ' ') {        // проверка, чтобы не затереть нолик
                    place[2][2] = 'X';
                }
                break;
        }

    }

    void print() {
        system("clear");
        cout << turn << " ход" << endl;
        cout << place[0][0] << "|" << place[0][1] << "|" << place[0][2] << endl;
        cout << "-----" << endl;
        cout << place[1][0] << "|" << place[1][1] << "|" << place[1][2] << endl;
        cout << "-----" << endl;
        cout << place[2][0] << "|" << place[2][1] << "|" << place[2][2] << endl;
    }

    bool check_two_one_line() {
        char ch = 'O';

        vector<pair<pair<int, int>, pair<int, int>>> pos;
        pos.push_back(make_pair(make_pair(0, 0), make_pair(0, 1)));
        pos.push_back(make_pair(make_pair(0, 1), make_pair(0, 2)));
        pos.push_back(make_pair(make_pair(1, 0), make_pair(1, 1)));
        pos.push_back(make_pair(make_pair(1, 1), make_pair(1, 2)));
        pos.push_back(make_pair(make_pair(2, 0), make_pair(2, 1)));
        pos.push_back(make_pair(make_pair(2, 1), make_pair(2, 2)));

        pos.push_back(make_pair(make_pair(0, 0), make_pair(1, 0)));
        pos.push_back(make_pair(make_pair(1, 0), make_pair(2, 0)));
        pos.push_back(make_pair(make_pair(0, 1), make_pair(1, 1)));
        pos.push_back(make_pair(make_pair(1, 1), make_pair(2, 1)));
        pos.push_back(make_pair(make_pair(0, 2), make_pair(1, 2)));
        pos.push_back(make_pair(make_pair(1, 2), make_pair(2, 2)));

        pos.push_back(make_pair(make_pair(0, 0), make_pair(0, 2)));
        pos.push_back(make_pair(make_pair(1, 0), make_pair(1, 2)));
        pos.push_back(make_pair(make_pair(2, 0), make_pair(2, 2)));

        pos.push_back(make_pair(make_pair(0, 0), make_pair(2, 0)));
        pos.push_back(make_pair(make_pair(0, 1), make_pair(2, 1)));
        pos.push_back(make_pair(make_pair(0, 2), make_pair(2, 2)));

        pos.push_back(make_pair(make_pair(0, 2), make_pair(1, 1)));
        pos.push_back(make_pair(make_pair(1, 1), make_pair(2, 0)));

        pos.push_back(make_pair(make_pair(0, 0), make_pair(1, 1)));
        pos.push_back(make_pair(make_pair(1, 1), make_pair(2, 2)));

        pos.push_back(make_pair(make_pair(0, 0), make_pair(2, 2)));
        pos.push_back(make_pair(make_pair(0, 2), make_pair(2, 0)));

        bool flag = false;
        for (int i = 0; i < pos.size(); i++) {
            pair<int, int> pos_f = pos[i].first;
            pair<int, int> pos_s = pos[i].second;
            if (place[pos_f.first][pos_f.second] == ch && place[pos_s.first][pos_s.second] == ch) {
//                cout << pos_f.first << " " << pos_f.second << endl;
//                cout << pos_s.first << " " << pos_s.second << endl;
//                usleep(3000000);

                if (pos_f.second == pos_s.second + 2) {
                    if (place[pos_f.first][0] == ch && place[pos_f.first][2] == ch && place[pos_f.first][1] == ' ') {
                        place[pos_f.first][1] = 'O';
                        flag = true;
                    }
                }

                if (pos_f.first == pos_s.first + 2) {
                    if (place[0][pos_f.second] == ch && place[2][pos_f.second] == ch && place[1][pos_f.second] == ' ') {
                        place[1][pos_f.second] = 'O';
                        flag = true;
                    }
                }

                if (pos_f.first == pos_s.first) {   // две рядом на горизонтали
                    if (pos_f.second == 0 && place[pos_f.first][2] == ' ') {
                        place[pos_f.first][2] = 'O';
                        flag = true;
                    }
                    if (pos_f.second == 1 && place[pos_f.first][0] == ' ') {
                        place[pos_f.first][0] = 'O';
                        flag = true;
                    }
                }

                if (pos_f.second == pos_s.second) {     // две рядом на вертикали
                    if (pos_f.first == 0 && place[2][pos_f.second] == ' ') {
                        place[2][pos_f.second] = 'O';
                        flag = true;
                    }
                    if (pos_f.first == 1 && place[0][pos_f.second] == ' ') {
                        place[0][pos_f.second] = 'O';
                        flag = true;
                    }
                }

                if (place[0][0] == ch && place[1][1] == ch && place[2][2] == ' ') {
                    place[2][2] = 'O';
                    flag = true;
                }
                if (place[2][2] == ch && place[1][1] == ch && place[0][0] == ' ') {
                    place[0][0] = 'O';
                    flag = true;
                }

                if (place[0][2] == ch && place[1][1] == ch && place[2][0] == ' ') {
                    place[2][0] = 'O';
                    flag = true;
                }
                if (place[2][0] == ch && place[1][1] == ch && place[0][2] == ' ') {
                    place[0][2] = 'O';
                    flag = true;
                }

                if (place[0][0] == ch && place[2][2] == ch && place[1][1] == ' ') {
                    place[1][1] = 'O';
                    flag = true;
                }

                if (place[0][2] == ch && place[2][0] == ch && place[1][1] == ' ') {
                    place[1][1] = 'O';
                    flag = true;
                }


            }
        }
        if (!flag) {
            ch = 'X';
            for (int i = 0; i < pos.size(); i++) {
                pair<int, int> pos_f = pos[i].first;
                pair<int, int> pos_s = pos[i].second;
                if (place[pos_f.first][pos_f.second] == ch && place[pos_s.first][pos_s.second] == ch) {
//                cout << pos_f.first << " " << pos_f.second << endl;
//                cout << pos_s.first << " " << pos_s.second << endl;
//                cout << "------------------------" << endl;
//                usleep(3000000);

                    if (pos_f.second == pos_s.second + 2) {
                        if (place[pos_f.first][0] == ch && place[pos_f.first][2] == ch && place[pos_f.first][1] == ' ') {
                            place[pos_f.first][1] = 'O';
                            flag = true;
                        }
                    }

                    if (pos_f.first == pos_s.first + 2) {
                        if (place[0][pos_f.second] == ch && place[2][pos_f.second] == ch && place[1][pos_f.second] == ' ') {
                            place[1][pos_f.second] = 'O';
                            flag = true;
                        }
                    }

                    if (pos_f.first == pos_s.first) {   // две рядом на горизонтали
                        if (pos_f.second == 0 && place[pos_f.first][2] == ' ') {
                            place[pos_f.first][2] = 'O';
                            flag = true;
                        }
                        if (pos_f.second == 1 && place[pos_f.first][0] == ' ') {
                            place[pos_f.first][0] = 'O';
                            flag = true;
                        }
                    }

                    if (pos_f.second == pos_s.second) {     // две рядом на вертикали
                        if (pos_f.first == 0 && place[2][pos_f.second] == ' ') {
                            place[2][pos_f.second] = 'O';
                            flag = true;
                        }
                        if (pos_f.first == 1 && place[0][pos_f.second] == ' ') {
                            place[0][pos_f.second] = 'O';
                            flag = true;
                        }
                    }

                    if (place[0][0] == ch && place[1][1] == ch && place[2][2] == ' ') {
                        place[2][2] = 'O';
                        flag = true;
                    }
                    if (place[2][2] == ch && place[1][1] == ch && place[0][0] == ' ') {
                        place[0][0] = 'O';
                        flag = true;
                    }

                    if (place[0][2] == ch && place[1][1] == ch && place[2][0] == ' ') {
                        place[2][0] = 'O';
                        flag = true;
                    }
                    if (place[2][0] == ch && place[1][1] == ch && place[0][2] == ' ') {
                        place[0][2] = 'O';
                        flag = true;
                    }

                    if (place[0][0] == ch && place[2][2] == ch && place[1][1] == ' ') {
                        place[1][1] = 'O';
                        flag = true;
                    }

                    if (place[0][2] == ch && place[2][0] == ch && place[1][1] == ' ') {
                        place[1][1] = 'O';
                        flag = true;
                    }


                }
            }

        }
        return flag;
    }

    void put_zero() {
        turn++;
        if (!check_two_one_line()) {
            srand(time(nullptr));
            int pos;
            while (true) {      // с помощью рандома выбираем позицию для нолика
                pos = rand() % 9 + 1;
                bool flag = true;
                for (int i = 0; i < turns.size(); i++) {
                    if (turns[i] == pos) {
                        flag = false;
                    }
                }
                if (flag)
                    break;
            }
            turns.push_back(pos);
            switch (pos) {
                case 1:
                    if (place[0][0] == ' ') {        // проверка, чтобы не затереть крестик
                        place[0][0] = 'O';
                        turns.push_back(1);
                    }
                    break;
                case 2:
                    if (place[0][1] == ' ') {        // проверка, чтобы не затереть крестик
                        place[0][1] = 'O';
                        turns.push_back(2);
                    }
                    break;
                case 3:
                    if (place[0][2] == ' ') {        // проверка, чтобы не затереть крестик
                        place[0][2] = 'O';
                        turns.push_back(3);
                    }
                    break;
                case 4:
                    if (place[1][0] == ' ') {        // проверка, чтобы не затереть крестик
                        place[1][0] = 'O';
                        turns.push_back(4);
                    }
                    break;
                case 5:
                    if (place[1][1] == ' ') {        // проверка, чтобы не затереть крестик
                        place[1][1] = 'O';
                        turns.push_back(5);
                    }
                    break;
                case 6:
                    if (place[1][2] == ' ') {        // проверка, чтобы не затереть крестик
                        place[1][2] = 'O';
                        turns.push_back(6);
                    }
                    break;
                case 7:
                    if (place[2][0] == ' ') {        // проверка, чтобы не затереть крестик
                        place[2][0] = 'O';
                        turns.push_back(7);
                    }
                    break;
                case 8:
                    if (place[2][1] == ' ') {        // проверка, чтобы не затереть крестик
                        place[2][1] = 'O';
                        turns.push_back(8);
                    }
                    break;
                case 9:
                    if (place[2][2] == ' ') {        // проверка, чтобы не затереть крестик
                        place[2][2] = 'O';
                        turns.push_back(9);
                    }
                    break;
            }
        }
    }

    void logic() {
        if (perhaps_win() == 'X' or perhaps_win() == 'O' or turn == 10) {
            status_game = "Game over";
            return;
        }
        put_zero();     // Ход компьютера
        if (perhaps_win() == 'X' or perhaps_win() == 'O' or turn == 10) {
            status_game = "Game over";
            return;
        }
    }

    int turn;
    Menu menu;
    vector<vector<char>> place;
    vector<int> turns;
};

int main() {
    Game game;
    game.play();
    return 0;
}
