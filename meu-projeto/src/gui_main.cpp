// gui_main.cpp - ponto de entrada da GUI (Questao 6).
// So cria a aplicacao Qt e mostra a janela; nenhuma logica aqui.
#include <QApplication>
#include "janela.hpp"

int main(int argc, char** argv) {
    QApplication qt{argc, argv};

    Janela janela;
    janela.show();

    return qt.exec();
}
