#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept> 
#include <fstream>   // Para gravação do relatório
 
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
            throw std::invalid_argument("Valor de dano invalido.");
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
            throw std::runtime_error("Municao insuficiente.");
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