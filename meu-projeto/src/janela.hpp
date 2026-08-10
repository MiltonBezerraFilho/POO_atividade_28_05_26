#pragma once
// janela.hpp - Questao 6 (Qt): GUI minima sobre a logica ja existente.
// A janela e uma CAMADA FINA: nenhuma regra de negocio aqui (Questao 6-B).
// Ela so monta widgets e chama funcoes/classes ja definidas em domain.hpp
// (criar_fighter_jet, criar_interceptor, calcular_poder_total, monta_estado,
// missao_app), respeitando o DIP da Questao 4.

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

#include "domain.hpp"

class Janela : public QWidget {
public:
    explicit Janela(QWidget* parent = nullptr)
        : QWidget(parent),
          pilot_("Wormwood", 100, 15),
          repo_("estado_gui.json"),
          app_(repo_) {
        setWindowTitle("RPG de Aviacao - Painel de Missao (Qt)");
        resize(480, 420);
        montar_layout();
    }

private:
    // --- estado da janela: nada disso e "regra de negocio", so guarda o
    // que a logica de dominio ja existente precisa para operar ---
    Pilot pilot_;
    std::vector<std::unique_ptr<Aircraft>> frota_;
    json_repository repo_;   // implementacao de PRODUCAO do repository (Questao 4-D)
    missao_app app_;         // classe de alto nivel, injetada com repo_ (Questao 4-C)

    QListWidget* lista_ = nullptr;
    QLabel* label_poder_ = nullptr;

    void montar_layout() {
        lista_ = new QListWidget(this);
        label_poder_ = new QLabel("Poder de fogo total: -", this);

        auto* btn_add_jet = new QPushButton("+ Fighter Jet", this);
        auto* btn_add_interceptor = new QPushButton("+ Interceptor", this);
        auto* btn_remover = new QPushButton("Remover selecionada", this);
        auto* btn_calcular = new QPushButton("Calcular poder de fogo", this);
        auto* btn_salvar = new QPushButton("Salvar", this);
        auto* btn_carregar = new QPushButton("Carregar", this);

        // Conexao direta a metodos membro (Qt6, sem precisar de Q_OBJECT/moc)
        connect(btn_add_jet, &QPushButton::clicked, this, &Janela::adicionar_fighter_jet);
        connect(btn_add_interceptor, &QPushButton::clicked, this, &Janela::adicionar_interceptor);
        connect(btn_remover, &QPushButton::clicked, this, &Janela::remover_selecionada);
        connect(btn_calcular, &QPushButton::clicked, this, &Janela::calcular_poder);
        connect(btn_salvar, &QPushButton::clicked, this, &Janela::salvar_estado);
        connect(btn_carregar, &QPushButton::clicked, this, &Janela::carregar_estado);

        auto* linha_add = new QHBoxLayout();
        linha_add->addWidget(btn_add_jet);
        linha_add->addWidget(btn_add_interceptor);
        linha_add->addWidget(btn_remover);

        auto* linha_acoes = new QHBoxLayout();
        linha_acoes->addWidget(btn_calcular);
        linha_acoes->addWidget(btn_salvar);
        linha_acoes->addWidget(btn_carregar);

        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Frota atual:", this));
        layout->addWidget(lista_);
        layout->addLayout(linha_add);
        layout->addWidget(label_poder_);
        layout->addLayout(linha_acoes);
    }

    // atualizar_lista - so espelha frota_ no QListWidget, sem calcular nada
    void atualizar_lista() {
        lista_->clear();
        for (const auto& aeronave : frota_) {
            lista_->addItem(QString::fromStdString(aeronave->get_model()));
        }
    }

    // --- slots: cada um so PEDE um dado (nome via dialogo) e delega para
    // a logica de dominio (criar_fighter_jet/criar_interceptor/etc) ---

    void adicionar_fighter_jet() {
        bool ok = false;
        QString nome = QInputDialog::getText(this, "Novo Fighter Jet", "Modelo:",
                                              QLineEdit::Normal, "", &ok);
        if (!ok || nome.isEmpty()) return;
        frota_.push_back(criar_fighter_jet(nome.toStdString(), &pilot_)); // logica de dominio
        atualizar_lista();
    }

    void adicionar_interceptor() {
        bool ok = false;
        QString nome = QInputDialog::getText(this, "Novo Interceptor", "Modelo:",
                                              QLineEdit::Normal, "", &ok);
        if (!ok || nome.isEmpty()) return;
        frota_.push_back(criar_interceptor(nome.toStdString(), &pilot_)); // logica de dominio
        atualizar_lista();
    }

    void remover_selecionada() {
        int linha = lista_->currentRow();
        if (linha < 0) {
            QMessageBox::information(this, "Nada selecionado", "Selecione uma aeronave na lista.");
            return;
        }
        frota_.erase(frota_.begin() + linha);
        atualizar_lista();
    }

    void calcular_poder() {
        double total = calcular_poder_total(frota_); // logica de dominio (Questao 3/6)
        label_poder_->setText(QString("Poder de fogo total: %1").arg(total));
    }

    void salvar_estado() {
        estado_missao estado = monta_estado(frota_, pilot_); // logica de dominio (Questao 4)
        try {
            app_.salvar(estado); // via missao_app -> respeita o DIP (Questao 4-C / 6-B)
            QMessageBox::information(this, "Salvo", "Estado salvo em estado_gui.json");
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Erro ao salvar", QString::fromStdString(e.what()));
        }
    }

    void carregar_estado() {
        try {
            estado_missao estado = app_.carregar(); // via missao_app (Questao 4)
            frota_.clear();
            for (const auto& snap : estado.aeronaves_) {
                if (snap.type_ == "FighterJet") {
                    frota_.push_back(criar_fighter_jet(snap.model_, &pilot_));
                } else {
                    frota_.push_back(criar_interceptor(snap.model_, &pilot_));
                }
            }
            atualizar_lista();
            QMessageBox::information(this, "Carregado", "Estado carregado de estado_gui.json");
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Erro ao carregar", QString::fromStdString(e.what()));
        }
    }
};
