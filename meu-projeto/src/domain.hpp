#pragma once
// domain.hpp - logica de dominio do RPG de Aviacao (Questoes 1 a 4)
// Separado de main.cpp para permitir testes automatizados (Questao 5)

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
#include <nlohmann/json.hpp> // Para serializacao (Questao 4-A)

using json = nlohmann::json;

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
    int get_speed() const { return speed_; }
    int get_evasiveness() const { return evasiveness_; }
    const Weapon& get_weapon() const { return *weapon_; }

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

// ============================================================
// QUESTAO 4 - Serializacao (JSON) e SOLID (DIP)
// ============================================================

// aircraft_snapshot - representacao PLANA (nao-polimorfica) de uma aeronave,
// usada apenas para serializar/desserializar o estado (Questao 4-A/B).
// O campo "type_" guarda o tipo concreto (FighterJet/Interceptor) para
// permitir recriar o objeto certo na desserializacao.
struct aircraft_snapshot {
    std::string type_;
    std::string model_;
    int speed_;
    int evasiveness_;
    int ammo_;
    std::string weapon_name_;
    int weapon_dmg_;

    // Necessario para o teste de round-trip (estado_missao == estado_missao)
    bool operator==(const aircraft_snapshot& outro) const {
        return type_ == outro.type_ && model_ == outro.model_ &&
               speed_ == outro.speed_ && evasiveness_ == outro.evasiveness_ &&
               ammo_ == outro.ammo_ && weapon_name_ == outro.weapon_name_ &&
               weapon_dmg_ == outro.weapon_dmg_;
    }
};

// to_json/from_json nao-intrusivos para aircraft_snapshot (Questao 4-A/B)
void to_json(json& j, const aircraft_snapshot& a) {
    j = json{{"type", a.type_},
             {"model", a.model_},
             {"speed", a.speed_},
             {"evasiveness", a.evasiveness_},
             {"ammo", a.ammo_},
             {"weapon_name", a.weapon_name_},
             {"weapon_dmg", a.weapon_dmg_}};
}

void from_json(const json& j, aircraft_snapshot& a) {
    j.at("type").get_to(a.type_);
    j.at("model").get_to(a.model_);
    j.at("speed").get_to(a.speed_);
    j.at("evasiveness").get_to(a.evasiveness_);
    j.at("ammo").get_to(a.ammo_);
    j.at("weapon_name").get_to(a.weapon_name_);
    j.at("weapon_dmg").get_to(a.weapon_dmg_);
}

// estado_missao - estado completo do sistema (piloto + frota), o que de fato
// e persistido/restaurado. E o "estado" mencionado no roteiro de testes.
struct estado_missao {
    std::string pilot_callsign_;
    int pilot_hp_;
    int pilot_agility_;
    std::vector<aircraft_snapshot> aeronaves_;

    bool operator==(const estado_missao& outro) const {
        return pilot_callsign_ == outro.pilot_callsign_ &&
               pilot_hp_ == outro.pilot_hp_ &&
               pilot_agility_ == outro.pilot_agility_ &&
               aeronaves_ == outro.aeronaves_;
    }
};

// to_json/from_json de estado_missao, com campo "version" (Questao 4-B)
void to_json(json& j, const estado_missao& e) {
    j = json{{"version", 1},
             {"pilot", {{"callsign", e.pilot_callsign_},
                        {"hp", e.pilot_hp_},
                        {"agility", e.pilot_agility_}}},
             {"aeronaves", e.aeronaves_}};
}

void from_json(const json& j, estado_missao& e) {
    // trata por versao: hoje so existe a versao 1, mas o "if" abaixo e o
    // ponto de extensao (OCP) para migrar formatos antigos no futuro.
    int versao = j.at("version").get<int>();
    if (versao != 1) {
        throw erro_dominio("versao de estado nao suportada: " + std::to_string(versao));
    }
    j.at("pilot").at("callsign").get_to(e.pilot_callsign_);
    j.at("pilot").at("hp").get_to(e.pilot_hp_);
    j.at("pilot").at("agility").get_to(e.pilot_agility_);
    j.at("aeronaves").get_to(e.aeronaves_);
}

// monta_estado - converte a frota polimorfica (Aircraft) + piloto no
// estado_missao plano, gravando o tipo concreto de cada aeronave
estado_missao monta_estado(const std::vector<std::unique_ptr<Aircraft>>& frota, const Pilot& pilot) {
    estado_missao estado;
    estado.pilot_callsign_ = pilot.get_callsign();
    estado.pilot_hp_ = pilot.get_hp();
    estado.pilot_agility_ = pilot.get_agility();

    for (const auto& aeronave : frota) {
        aircraft_snapshot snap;
        snap.type_ = dynamic_cast<FighterJet*>(aeronave.get()) ? "FighterJet" : "Interceptor";
        snap.model_ = aeronave->get_model();
        snap.speed_ = aeronave->get_speed();
        snap.evasiveness_ = aeronave->get_evasiveness();
        snap.ammo_ = aeronave->get_ammo_count();
        snap.weapon_name_ = aeronave->get_weapon().get_name();
        snap.weapon_dmg_ = aeronave->get_weapon().get_damage();
        estado.aeronaves_.push_back(std::move(snap));
    }
    return estado;
}

// estado_repository - abstracao (DIP) para persistencia do estado_missao.
// Classes de alto nivel (missao_app) dependem SOMENTE desta interface,
// nunca de uma implementacao concreta (Questao 4-C).
class estado_repository {
public:
    virtual void save(const estado_missao& estado) = 0;
    virtual estado_missao load() = 0;
    virtual ~estado_repository() = default;
};

// json_repository - implementacao de PRODUCAO: grava/le em arquivo .json (Questao 4-D)
class json_repository : public estado_repository {
private:
    std::string filename_;

public:
    explicit json_repository(std::string filename) : filename_(std::move(filename)) {}

    void save(const estado_missao& estado) override {
        std::ofstream arquivo(filename_);
        if (!arquivo.is_open()) {
            throw std::runtime_error("nao foi possivel abrir arquivo para salvar: " + filename_);
        }
        json doc = estado;
        arquivo << doc.dump(2);
        std::cout << "[JSON_REPOSITORY] Estado salvo em: " << filename_ << "\n";
    }

    estado_missao load() override {
        std::ifstream arquivo(filename_);
        if (!arquivo.is_open()) {
            throw std::runtime_error("nao foi possivel abrir arquivo para carregar: " + filename_);
        }
        json doc;
        arquivo >> doc;
        std::cout << "[JSON_REPOSITORY] Estado carregado de: " << filename_ << "\n";
        return doc.get<estado_missao>();
    }
};

// memory_repository - implementacao de TESTE: guarda o estado em memoria,
// sem tocar disco nem rede (Questao 4-D), ideal para testes automatizados.
class memory_repository : public estado_repository {
private:
    std::optional<estado_missao> ultimo_estado_;

public:
    void save(const estado_missao& estado) override {
        ultimo_estado_ = estado;
        std::cout << "[MEMORY_REPOSITORY] Estado salvo em memoria (sem I/O)\n";
    }

    estado_missao load() override {
        if (!ultimo_estado_.has_value()) {
            throw erro_dominio("memory_repository: nenhum estado salvo ainda");
        }
        std::cout << "[MEMORY_REPOSITORY] Estado carregado da memoria (sem I/O)\n";
        return *ultimo_estado_;
    }
};

// missao_app - classe de ALTO NIVEL: depende da ABSTRACAO estado_repository,
// recebida por injecao de dependencia no construtor (Questao 4-C).
// Nao sabe (nem precisa saber) se o repositorio e arquivo ou memoria.
class missao_app {
private:
    estado_repository& repo_;

public:
    explicit missao_app(estado_repository& repo) : repo_(repo) {}

    void salvar(const estado_missao& estado) { repo_.save(estado); }
    estado_missao carregar() { return repo_.load(); }
};

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

// ============================================================
// QUESTAO 6 - funcoes de dominio auxiliares, reutilizadas pela GUI (Qt)
// Ficam aqui (nao na janela) porque sao LOGICA DE DOMINIO, nao GUI.
// Marcadas "inline" pois o header pode ser incluido por mais de um
// arquivo dentro do mesmo executavel (gui_main.cpp e janela.hpp).
// ============================================================

inline std::unique_ptr<Aircraft> criar_fighter_jet(const std::string& model, Pilot* pilot) {
    auto aeronave = std::make_unique<FighterJet>(model, 70, 60, 4, "AIM-9", 30);
    aeronave->assign_pilot(pilot);
    return aeronave;
}

inline std::unique_ptr<Aircraft> criar_interceptor(const std::string& model, Pilot* pilot) {
    auto aeronave = std::make_unique<Interceptor>(model, 110, 15, 6, "R-37", 60);
    aeronave->assign_pilot(pilot);
    return aeronave;
}

// calcular_poder_total - mesma logica de reducao da Questao 3-B (accumulate),
// extraida em funcao para a GUI poder chamar sem reimplementar a regra.
inline double calcular_poder_total(const std::vector<std::unique_ptr<Aircraft>>& frota) {
    return std::accumulate(frota.begin(), frota.end(), 0.0,
        [](double acc, const std::unique_ptr<Aircraft>& a) { return acc + a->calculate_firepower(); });
}