#include <iostream>
#include <string>
#include <memory>

// 1. CLASSE PILOT (Independente)
class Pilot {
private:
    std::string callsign_;
    int level_;

public:
    Pilot(std::string callsign, int level) : callsign_(callsign), level_(level) {}
    
    ~Pilot() { 
        std::cout << "[SISTEMA] Dados do piloto " << callsign_ << " limpos da memoria da base.\n"; 
    }

    std::string get_callsign() const { return callsign_; }

    void level_up() {
        level_++;
        std::cout << "[RPG] " << callsign_ << " subiu para o nivel " << level_ << "!\n";
    }
};

// 2. CLASSE AVIONICS (Componente Interno do Caça)
class Avionics {
private:
    int radar_;

public:
    Avionics(int radar) : radar_(radar) {}
    
    ~Avionics() { 
        std::cout << "[SISTEMA] Sistema de radar integrado foi desligado e destruido.\n"; 
    }

    int get_radar() const { return radar_; }
};

// 3. CLASSE FIGHTERJET (Classe Principal)
class FighterJet {
private:
    std::string model_;
    std::unique_ptr<Avionics> avionics_; // COMPOSIÇÃO (O caça é dono da Avionics)
    Pilot* pilot_;                       // AGREGAÇÃO (O caça apenas usa o piloto)

public:
    FighterJet(std::string model, int radar)
        : model_(model), 
          avionics_(std::make_unique<Avionics>(radar)), 
          pilot_(nullptr) {}

    ~FighterJet() { 
        std::cout << "[COMBATE] Caca " << model_ << " foi abatido em missao e destruido.\n"; 
    }

    void assign_pilot(Pilot* p) { 
        pilot_ = p; 
    }

    void status() {
        std::cout << "[STATUS] Caca: " << model_ << " | Alcance do Radar: " << avionics_->get_radar() << "km";
        if (pilot_) {
            std::cout << " | Piloto no Cockpit: " << pilot_->get_callsign();
        } else {
            std::cout << " | Cockpit: Vazio";
        }
        std::cout << "\n";
    }
};

// 4. CLASSE HANGAR (Gerenciadora Simples)
class Hangar {
private:
    std::string name_;

public:
    Hangar(std::string name) : name_(name) {}
    ~Hangar() { std::cout << "[SISTEMA] Hangar " << name_ << " fechado.\n"; }

    void enter_base() {
        std::cout << "[BASE] Conectado a Base Aerea de " << name_ << ".\n";
    }
};

// Programa de teste
int main() {
    std::cout << "--- TESTE ---\n\n";

    Pilot* p1 = new Pilot("Rigel One", 1);

    {
        std::cout << ">> Inicio do bloco...\n";
        
        FighterJet jet("Jas-39 Gripen E", 100);
        jet.assign_pilot(p1);
        jet.status();
        
        std::cout << ">> Fim do bloco...\n";
    }

    std::cout << "\n>> Fora do bloco (Retorno a base)\n";
    
    std::cout << "Verificando integridade do jogador...\n";
    p1->level_up();

    delete p1;

    return 0;
}