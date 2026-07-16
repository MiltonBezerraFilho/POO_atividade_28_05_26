#include <iostream>
#include <string>
#include <memory>

// 1. CLASSE PILOT (Agregação - Personagem Base)
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

    // Lógica real: Modifica a vida com base no dano recebido
    void take_damage(int amount) {
        hp_ -= amount;
        if (hp_ < 0) hp_ = 0;
        std::cout << "[PILOT] Dano recebido: " << amount << " | HP atual: " << hp_ << "\n";
    }
};

// 2. CLASSE AMMUNITION (Composição - A "Mana/Stamina" do RPG)
class Ammunition {
private:
    int count_;

public:
    Ammunition(int count) : count_(count) {}
    ~Ammunition() {
        std::cout << "[AMMUNITION] Objeto destruido\n";
    }

    int get_count() const { return count_; }

    // Lógica real: Consome o recurso se houver quantidade suficiente
    bool consume(int quantity) {
        if (count_ >= quantity) {
            count_ -= quantity;
            return true;
        }
        return false;
    }
};

// 3. CLASSE WEAPON (Composição - Armamento/Habilidade)
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

    // Lógica real: Calcula o dano final baseado na agilidade do piloto
    int calculate_strike(int pilot_agility) const {
        return damage_ + (pilot_agility * 2);
    }
};

// 4. CLASSE FIGHTERJET (Composição Dupla e Controladora - A Classe do RPG)
class FighterJet {
private:
    std::string model_;
    int defense_;
    std::unique_ptr<Ammunition> ammo_;
    std::unique_ptr<Weapon> weapon_;
    Pilot* pilot_; // Ponteiro bruto observador para agregação

public:
    FighterJet(std::string model, int defense, int initial_ammo, std::string weapon_name, int weapon_dmg)
        : model_(model),
          defense_(defense),
          ammo_(std::make_unique<Ammunition>(initial_ammo)),
          weapon_(std::make_unique<Weapon>(weapon_name, weapon_dmg)),
          pilot_(nullptr) {
        std::cout << "[FIGHTERJET] Objeto criado: " << model_ << "\n";
    }

    ~FighterJet() {
        std::cout << "[FIGHTERJET] Objeto destruido: " << model_ << "\n";
    }

    void assign_pilot(Pilot* p) {
        pilot_ = p;
    }

    // Lógica real: Orquestra o ataque consumindo munição e calculando o dano
    void fire_at_target() {
        std::cout << "[FIGHTERJET] Metodo fire_at_target() executado\n";
        if (!pilot_) {
            std::cout << "[ERRO] Ponteiro pilot_ nulo\n";
            return;
        }

        if (ammo_->consume(1)) {
            int total_damage = weapon_->calculate_strike(pilot_->get_agility());
            std::cout << "[ATAQUE] Piloto: " << pilot_->get_callsign() << " | Modelo: " << model_ 
                      << " | Arma: " << weapon_->get_name() << "\n"
                      << " > Dano calculado: " << total_damage << " \n"
                      << " > Municao restante: " << ammo_->get_count() << "\n";
        } else {
            std::cout << "[FALHA] Municao insuficiente para o disparo\n";
        }
    }
};

// ROTEIRO DE TESTES MANUAIS
int main() {
    std::cout << "--- INICIO DA EXECUCAO ---\n\n";

    // Instanciando o Piloto fora do escopo (Agregação)
    Pilot* player_pilot = new Pilot("Wormwood", 100, 15);

    // Bloco de escopo fechado para provar os tempos de vida (Testes 5 e 6 do edital)
    {
        std::cout << "\n[ENTRANDO NO BLOCO DE ESCOPO]\n";
        
        FighterJet my_jet("MiG-21 Fishbed", 12, 2, "R-60 Missile", 40);
        my_jet.assign_pilot(player_pilot);
        
        my_jet.fire_at_target();
        my_jet.fire_at_target();

        std::cout << "[SAINDO DO BLOCO DE ESCOPO]\n";
    } // O caça morre aqui. Arma e Munição somem. O piloto sobrevive.

    std::cout << "\n[FORA DO BLOCO DE ESCOPO]\n";
    std::cout << "Verificando integridade do ponteiro pilot_: " << player_pilot->get_callsign() << "\n";
    
    // Testando método lógico no objeto sobrevivente para validar a agregação
    player_pilot->take_damage(25);

    delete player_pilot;

    std::cout << "\n--- FIM DA EXECUCAO ---\n";
    return 0;
}