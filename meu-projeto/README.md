# Nome: Milton Bezerra do Vale Filho
# Matrícula: 20250018898

# Descrição do Domínio Escolhido:

Este projeto implementa um sistema de simulação de combate aéreo RPG baseado na classe abstrata Aircraft, que define o cálculo de poder de fogo. Munição e armas são gerenciadas por composição com std::unique_ptr, garantindo liberação automática. O piloto é associado por agregação. Inclui tratamento de exceções para falhas como falta de munição, sobrecarga de operadores para relatórios e persistência em arquivo via std::ofstream.


```mermaid
classDiagram
    class Pilot {
        -string callsign_
        -int hp_
        -int agility_
        +take_damage(int amount)
        +operator<<(ostream& os, const Pilot& pilot)
    }

    class Aircraft {
        <<abstract>>
        #unique_ptr~Ammunition~ ammo_
        #unique_ptr~Weapon~ weapon_
        #Pilot* pilot_
        +fire_weapon(int rounds)
        +calculate_firepower() float*
        +display_status() void
    }

    class FighterJet {
        +calculate_firepower() float
    }

    class Interceptor {
        +calculate_firepower() float
        +display_status() void
    }

    Aircraft <|-- FighterJet : herda
    Aircraft <|-- Interceptor : herda
    Aircraft "1" *-- "1" Ammunition : composta
    Aircraft "1" *-- "1" Weapon : composta
    Aircraft "0..*" o-- "0..1" Pilot : agrega
```