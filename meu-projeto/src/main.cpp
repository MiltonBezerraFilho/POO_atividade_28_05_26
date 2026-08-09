#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept> 
#include <fstream>   // Para gravação do relatório
#include <ranges>
#include <optional>
#include <variant>
#include <algorithm>
#include <numeric>
#include <map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <future>

// erro_dominio - excecao BASE do dominio, herda de std::runtime_error (Questao 2-A)
class erro_dominio : public std::runtime_error {
public:
    using std::runtime_error::runtime_error; // herda o construtor da classe mae
};

// municao_insuficiente - excecao ESPECIFICA: falta de municao ao disparar
class municao_insuficiente : public erro_dominio {
public:
    explicit municao_insuficiente(const std::string& msg)
        : erro_dominio{"municao insuficiente: " + msg} {}
};

// dano_invalido - excecao ESPECIFICA: valor de dano negativo
class dano_invalido : public erro_dominio {
public:
    explicit dano_invalido(const std::string& msg)
        : erro_dominio{"dano invalido: " + msg} {}
};

// Pilot - agregado à aeronave
class Pilot {
private:
    std::string callsign_;
    int hp_;
    int agility_;

public:
    Pilot(std::string callsign, int hp, int agility)
        : callsign_(callsign), hp_(hp), agility_(agility) {
        std::cout << "[PILOT] Objeto criado: " << callsign_ << "\n";
    }

    ~Pilot() {
        std::cout << "[PILOT] Objeto destruido: " << callsign_ << "\n";
    }

    std::string get_callsign() const { return callsign_; }
    int get_hp() const { return hp_; }
    int get_agility() const { return agility_; }

    void take_damage(int amount) {
        if (amount < 0) {
            throw dano_invalido("valor negativo (" + std::to_string(amount) + ")");
        }
        hp_ -= amount;
        if (hp_ < 0) hp_ = 0;
        std::cout << "[PILOT] Dano recebido: " << amount << " | HP atual: " << hp_ << "\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const Pilot& pilot) {
        os << "[PILOT] Callsign: " << pilot.callsign_ 
           << " | HP: " << pilot.hp_ 
           << " | Agilidade: " << pilot.agility_;
        return os;
    }
};

// Ammunition - composta na aeronave
class Ammunition {
private:
    int count_;

public:
    Ammunition(int count) : count_(count) {}
    ~Ammunition() {
        std::cout << "[AMMUNITION] Objeto destruido\n";
    }

    int get_count() const { return count_; }

    void consume(int quantity) {
        if (count_ < quantity) {
            throw municao_insuficiente("restam " + std::to_string(count_) +
                                        ", pedido " + std::to_string(quantity));
        }
        count_ -= quantity;
    }
};

// Weapon - composta na aeronave
class Weapon {
private:
    std::string name_;
    int damage_;

public:
    Weapon(std::string name, int damage) : name_(name), damage_(damage) {}
    ~Weapon() {
        std::cout << "[WEAPON] Objeto destruido: " << name_ << "\n";
    }

    std::string get_name() const { return name_; }
    int get_damage() const { return damage_; }
};

// registry<T> - container generico reutilizavel (Questao 1-A)
// Guarda uma lista de itens de qualquer tipo T (Pilot, string, etc).
template <typename T>
class registry {
private:
    std::vector<T> items_;

public:
    void add(T item) {
        items_.push_back(std::move(item));
    }

    const T& at(std::size_t i) const {
        return items_.at(i);
    }

    std::size_t size() const {
        return items_.size();
    }
};

// counted<Derived> - CRTP: adiciona contagem de instancias sem vtable (Questao 1-B)
// Cada classe que "herda" counted<SiMesma> ganha seu proprio contador estatico.
// buscar_piloto_por_callsign - optional em busca que pode falhar (Questao 2-B)
std::optional<Pilot> buscar_piloto_por_callsign(const registry<Pilot>& reg, const std::string& callsign) {
    for (std::size_t i = 0; i < reg.size(); ++i) {
        if (reg.at(i).get_callsign() == callsign) {
            return reg.at(i); // achou
        }
    }
    return std::nullopt; // nao achou
}

template <typename Derived>
class counted {
private:
    static inline int count_ = 0;

public:
    counted() { ++count_; }
    ~counted() { --count_; }

    static int alive() { return count_; }
};

// Classe base abstrata
class Aircraft {
protected:
    std::string model_;
    int speed_;         
    int evasiveness_;   
    std::unique_ptr<Ammunition> ammo_;
    std::unique_ptr<Weapon> weapon_;
    Pilot* pilot_;

public:
    Aircraft(std::string model, int speed, int evasiveness, int initial_ammo, std::string weapon_name, int weapon_dmg)
        : model_(model),
          speed_(speed),
          evasiveness_(evasiveness),
          ammo_(std::make_unique<Ammunition>(initial_ammo)),
          weapon_(std::make_unique<Weapon>(weapon_name, weapon_dmg)),
          pilot_(nullptr) {}

    // Regra dos 5: unique_ptr nao pode ser copiado, entao proibimos copia
    // e habilitamos movimentacao explicitamente (necessario p/ std::vector<FighterJet>)
    Aircraft(const Aircraft&) = delete;
    Aircraft& operator=(const Aircraft&) = delete;
    Aircraft(Aircraft&&) = default;
    Aircraft& operator=(Aircraft&&) = default;

    virtual ~Aircraft() {
        std::cout << "[AIRCRAFT] Destruido: " << model_ << "\n";
    }

    void assign_pilot(Pilot* p) { pilot_ = p; }

    std::string get_model() const { return model_; }
    int get_ammo_count() const { return ammo_->get_count(); }

    void fire_weapon(int rounds) {
        std::cout << "[AIRCRAFT] Disparando " << rounds << " tiro(s)...\n";
        ammo_->consume(rounds); 
        std::cout << "[AIRCRAFT] Disparo efetuado. Municao restante: " << ammo_->get_count() << "\n";
    }

    virtual float calculate_firepower() const = 0;

    virtual void display_status() const {
        std::cout << "[STATUS] Modelo: " << model_ << " | Vel: " << speed_ << " | Eva: " << evasiveness_;
        if (pilot_) {
            std::cout << " | Piloto: " << pilot_->get_callsign();
        }
        std::cout << "\n";
    }
};

// FighterJet - caça padrão
class FighterJet : public Aircraft, public counted<FighterJet> {
public:
    FighterJet(std::string model, int speed, int evasiveness, int initial_ammo, std::string weapon_name, int weapon_dmg)
        : Aircraft(model, speed, evasiveness, initial_ammo, weapon_name, weapon_dmg) {
        std::cout << "[FIGHTERJET] Criado: " << model_ << "\n";
    }

    FighterJet(FighterJet&&) = default;
    FighterJet& operator=(FighterJet&&) = default;

    ~FighterJet() override {
        std::cout << "[FIGHTERJET] Destruido\n";
    }

    float calculate_firepower() const override {
        if (!pilot_) return 0.0f;
        return weapon_->get_damage() + (pilot_->get_agility() * 1.5f) + (evasiveness_ * 1.2f);
    }
};

// Interceptor - veloz e pesado
class Interceptor : public Aircraft, public counted<Interceptor> {
public:
    Interceptor(std::string model, int speed, int evasiveness, int initial_ammo, std::string weapon_name, int weapon_dmg)
        : Aircraft(model, speed, evasiveness, initial_ammo, weapon_name, weapon_dmg) {
        std::cout << "[INTERCEPTOR] Criado: " << model_ << "\n";
    }

    Interceptor(Interceptor&&) = default;
    Interceptor& operator=(Interceptor&&) = default;

    ~Interceptor() override {
        std::cout << "[INTERCEPTOR] Destruido\n";
    }

    float calculate_firepower() const override {
        if (!pilot_) return 0.0f;
        return weapon_->get_damage() + pilot_->get_agility() + (speed_ * 2.5f);
    }

    void display_status() const override {
        Aircraft::display_status(); 
        std::cout << " > Tipo: Interceptor\n";
    }
};

#include <concepts>

// possui_poder_de_fogo - concept que restringe tipos com calculate_firepower() (Questao 1-C)
template <typename T>
concept possui_poder_de_fogo = requires (const T& t) {
    { t.calculate_firepower() } -> std::convertible_to<float>;
};

// soma_poder_de_fogo - usa o concept na assinatura via template <possui_poder_de_fogo T> (Questao 1-D)
template <possui_poder_de_fogo T>
float soma_poder_de_fogo(const std::vector<T>& v) {
    float total = 0.0f;
    for (const auto& item : v) {
        total += item.calculate_firepower();
    }
    return total;
}

// TESTE MANUAL (Questao 1-D): descomentar as 2 linhas abaixo prova que o concept barra
// tipos invalidos com erro CLARO de compilacao, citando "possui_poder_de_fogo" e
// "t.calculate_firepower() is invalid" - nao um erro obscuro de template.
// std::vector<Pilot> pilotos_teste;
// soma_poder_de_fogo(pilotos_teste); // ERRO: Pilot nao tem calculate_firepower()

// resultado_busca - variant: sucesso (ponteiro p/ Aircraft) OU erro (mensagem) (Questao 2-C)
using resultado_busca = std::variant<Aircraft*, std::string>;

resultado_busca buscar_aeronave_por_modelo(const std::vector<std::unique_ptr<Aircraft>>& frota,
                                            const std::string& modelo) {
    for (const auto& aeronave : frota) {
        if (aeronave->get_model() == modelo) {
            return aeronave.get(); // sucesso: ponteiro nao-dono (nao deleta)
        }
    }
    return std::string("aeronave nao encontrada: " + modelo); // erro: mensagem
}

// Salva relatório em arquivo
void salvar_relatorio(const std::string& filename, const std::vector<std::unique_ptr<Aircraft>>& frota, const Pilot& pilot) {
    std::ofstream arquivo(filename);

    if (!arquivo.is_open()) {
        throw std::runtime_error("Erro ao criar arquivo de relatorio.");
    }

    arquivo << "----------------------------------------\n";
    arquivo << "        Relatorio de Missao\n";
    arquivo << "----------------------------------------\n\n";

    arquivo << "Dados do piloto:\n";
    arquivo << pilot << "\n\n";

    arquivo << "Frota:\n";
    for (const auto& aeronave : frota) {
        arquivo << "- Modelo: " << aeronave->get_model() << "\n"
                << "  Poder de fogo: " << aeronave->calculate_firepower() << "\n"
                << "  Municao restante: " << aeronave->get_ammo_count() << "\n\n";
    }

    arquivo << "----------------------------------------\n";
    arquivo.close();
    std::cout << "[RELATORIO] Arquivo salvo: " << filename << "\n";
}

// Testes Q1 a Q4
int main() {
    std::cout << "--- INICIO DA EXECUCAO (Q1 a Q4) ---\n\n";

    Pilot* player_pilot = new Pilot("Wormwood", 100, 15);

    std::cout << "\n--- QUESTAO 1(A): Template registry<T> ---\n";

    // Uso 1: registry guardando Pilot
    registry<Pilot> registro_pilotos;
    registro_pilotos.add(Pilot("Ghost", 90, 18));
    registro_pilotos.add(Pilot("Falcon", 100, 12));

    std::cout << "Total de pilotos no registro: " << registro_pilotos.size() << "\n";
    std::cout << "Piloto na posicao 0: " << registro_pilotos.at(0) << "\n";

    // Uso 2: registry guardando std::string (nomes de bases aereas)
    registry<std::string> registro_bases;
    registro_bases.add("Base Aerea Aurora");
    registro_bases.add("Base Aerea Talon");

    std::cout << "Total de bases no registro: " << registro_bases.size() << "\n";
    std::cout << "Base na posicao 1: " << registro_bases.at(1) << "\n";

    {
        std::cout << "\n[ENTRANDO NO ESCOPO]\n";
        std::vector<std::unique_ptr<Aircraft>> frota;
        frota.push_back(std::make_unique<FighterJet>("F-22 Raptor", 75, 80, 4, "AIM-120", 45));
        frota.push_back(std::make_unique<Interceptor>("MiG-31 Foxhound", 110, 15, 6, "R-37", 60));

        for (const auto& aeronave : frota) {
            aeronave->assign_pilot(player_pilot);
        }

        std::cout << "\n--- QUESTAO 1(B): CRTP - contagem sem vtable ---\n";
        std::cout << "FighterJet vivos: " << FighterJet::alive() << "\n";
        std::cout << "Interceptor vivos: " << Interceptor::alive() << "\n";

        std::cout << "\n--- QUESTAO 1(D): Concept aplicado (possui_poder_de_fogo) ---\n";
        std::vector<FighterJet> esquadrao_teste;
        esquadrao_teste.emplace_back("F-16 Fighting Falcon", 70, 60, 3, "AIM-9", 30);
        esquadrao_teste.back().assign_pilot(player_pilot);
        std::cout << "Soma de poder de fogo (FighterJet): "
                  << soma_poder_de_fogo(esquadrao_teste) << "\n";

        std::cout << "\n--- QUESTAO 1(E): Pipeline de ranges (filter + transform) ---\n";
        namespace rv = std::ranges::views;

        auto modelos_com_municao = frota
            | rv::filter([](const auto& aeronave) { return aeronave->get_ammo_count() > 0; })
            | rv::transform([](const auto& aeronave) { return aeronave->get_model(); });

        std::cout << "Aeronaves com municao disponivel:\n";
        for (const auto& nome : modelos_com_municao) {
            std::cout << " - " << nome << "\n";
        }

        std::cout << "\n--- QUESTAO 2(D): excecao (base) + optional (2 casos) + variant ---\n";

        // (D-1) try/catch capturando a excecao ESPECIFICA pela classe BASE
        try {
            std::cout << "[TESTE] Tentando disparar mais tiros do que a municao restante...\n";
            frota.front()->fire_weapon(999); // deve lancar municao_insuficiente
        }
        catch (const erro_dominio& e) { // captura pela BASE, nao pela especifica
            std::cerr << "[ERRO_DOMINIO] " << e.what() << "\n";
        }

        // (D-2) optional nos dois casos: achou e nao achou
        auto piloto_achado = buscar_piloto_por_callsign(registro_pilotos, "Ghost");
        if (piloto_achado.has_value()) {
            std::cout << "[OPTIONAL] Piloto encontrado: " << piloto_achado->get_callsign() << "\n";
        }

        auto piloto_nao_achado = buscar_piloto_por_callsign(registro_pilotos, "Maverick");
        if (!piloto_nao_achado.has_value()) {
            std::cout << "[OPTIONAL] Piloto 'Maverick' nao encontrado (nullopt), como esperado\n";
        }

        // (D-3) variant tratado com std::visit
        resultado_busca busca_ok = buscar_aeronave_por_modelo(frota, "F-22 Raptor");
        resultado_busca busca_falha = buscar_aeronave_por_modelo(frota, "Su-57");

        auto trata_resultado = [](const resultado_busca& r) {
            std::visit([](const auto& valor) {
                using T = std::decay_t<decltype(valor)>;
                if constexpr (std::is_same_v<T, Aircraft*>) {
                    std::cout << "[VARIANT] Encontrado: " << valor->get_model() << "\n";
                } else {
                    std::cout << "[VARIANT] Erro: " << valor << "\n";
                }
            }, r);
        };
        trata_resultado(busca_ok);
        trata_resultado(busca_falha);

        std::cout << "\n--- QUESTAO 3(A): Containers STL (map + unordered_set) ---\n";

        // std::map: indice ORDENADO por chave (nome do modelo -> ponteiro da aeronave)
        std::map<std::string, Aircraft*> indice_por_modelo;
        for (const auto& aeronave : frota) {
            indice_por_modelo[aeronave->get_model()] = aeronave.get();
        }
        std::cout << "Indice por modelo (ordem alfabetica automatica):\n";
        for (const auto& [modelo, ptr] : indice_por_modelo) {
            std::cout << " - " << modelo << "\n";
        }

        // std::unordered_set: garante armas UNICAS, busca O(1)
        std::unordered_set<std::string> armas_unicas;
        for (const auto& aeronave : frota) {
            armas_unicas.insert(aeronave->get_model()); // reutilizando modelo como exemplo de unicidade
        }
        std::cout << "Total de modelos unicos na frota: " << armas_unicas.size() << "\n";

        std::cout << "\n--- QUESTAO 3(B): Algoritmos STL + lambda com captura ---\n";

        // Vetor de ponteiros nao-donos, construido com std::transform (algoritmo 1)
        std::vector<Aircraft*> ponteiros_frota;
        std::transform(frota.begin(), frota.end(), std::back_inserter(ponteiros_frota),
                        [](const auto& aeronave) { return aeronave.get(); });

        // std::sort com comparador (algoritmo 2): ordena por poder de fogo, do maior pro menor
        std::sort(ponteiros_frota.begin(), ponteiros_frota.end(),
                  [](Aircraft* a, Aircraft* b) { return a->calculate_firepower() > b->calculate_firepower(); });

        std::cout << "Frota ordenada por poder de fogo (desc):\n";
        for (Aircraft* a : ponteiros_frota) {
            std::cout << " - " << a->get_model() << " (" << a->calculate_firepower() << ")\n";
        }

        // std::count_if com LAMBDA COM CAPTURA (algoritmo 3)
        int municao_minima = 3;
        auto qtd_com_municao_alta = std::count_if(
            ponteiros_frota.begin(), ponteiros_frota.end(),
            [municao_minima](Aircraft* a) { return a->get_ammo_count() >= municao_minima; }); // captura por valor
        std::cout << "Aeronaves com municao >= " << municao_minima << ": " << qtd_com_municao_alta << "\n";

        // std::accumulate (algoritmo 4): soma total do poder de fogo
        double soma_poder_total = std::accumulate(
            ponteiros_frota.begin(), ponteiros_frota.end(), 0.0,
            [](double acc, Aircraft* a) { return acc + a->calculate_firepower(); });
        std::cout << "Soma total de poder de fogo: " << soma_poder_total << "\n";

        std::cout << "\n--- QUESTAO 3(C)(D): Paralelizacao com std::async + mutex ---\n";

        double soma_paralela = 0.0;
        std::mutex mtx_soma;
        std::vector<std::future<void>> tarefas;

        for (Aircraft* aeronave : ponteiros_frota) {
            // cada tarefa e independente: so le os dados da PROPRIA aeronave
            tarefas.push_back(std::async(std::launch::async, [aeronave, &soma_paralela, &mtx_soma] {
                double parcial = aeronave->calculate_firepower(); // calculo independente
                std::lock_guard<std::mutex> lock(mtx_soma);       // regiao critica protegida
                soma_paralela += parcial;                          // escrita no estado compartilhado
            }));
        }

        for (auto& tarefa : tarefas) {
            tarefa.get(); // aguarda (equivalente a join) e propaga excecoes, se houver
        }

        std::cout << "Soma de poder de fogo (calculada em paralelo): " << soma_paralela << "\n";
        std::cout << "Soma serial (Q3-B) para comparacao: " << soma_poder_total << "\n";

        // Simulando combate
        try {
            std::cout << "[SIMULACAO] Disparando armas do F-22...\n";
            frota.front()->fire_weapon(2); 
            player_pilot->take_damage(20);
        } 
        catch (const std::exception& e) {
            std::cerr << "Excecao capturada: " << e.what() << "\n";
        }

        try {
            salvar_relatorio("log_combate.txt", frota, *player_pilot);
        }
        catch (const std::runtime_error& e) {
            std::cerr << e.what() << "\n";
        }

        std::cout << "\n[SAINDO DO ESCOPO]\n";
    } 

    std::cout << "\n[FORA DO ESCOPO]\n";
    std::cout << "FighterJet vivos apos escopo: " << FighterJet::alive() << "\n";
    std::cout << "Interceptor vivos apos escopo: " << Interceptor::alive() << "\n";
    delete player_pilot;

    std::cout << "\n--- FIM DA EXECUCAO ---\n";
    return 0;
}