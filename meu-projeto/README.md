# Nome: Milton Bezerra do Vale Filho
# Matrícula: 20250018898

# Descrição do Domínio Escolhido:

Este projeto implementa a arquitetura base para um sistema de combates em turnos no estilo RPG. O sistema utiliza a classe FighterJet como a definição de atributos e classe do personagem, enquanto o Pilot funciona como o personagem base com dados de integridade física. O modelo de armamento (Weapon) define o multiplicador de dano das habilidades executadas, e o gerenciamento de recursos/stamina é controlado pelo módulo interno de munição (Ammunition).

```mermaid
classDiagram
    class Pilot {
        -string callsign_
        -int hp_
        -int agility_
        +Pilot(string callsign, int hp, int agility)
        +~Pilot()
        +get_callsign() string const
        +get_hp() int const
        +get_agility() int const
        +take_damage(int amount) void
    }

    class Ammunition {
        -int count_
        +Ammunition(int count)
        +~Ammunition()
        +get_count() int const
        +consume(int quantity) bool
    }

    class Weapon {
        -string name_
        -int damage_
        +Weapon(string name, int damage)
        +~Weapon()
        +get_name() string const
        +calculate_strike(int pilot_agility) int const
    }

    class FighterJet {
        -string model_
        -int defense_
        -unique_ptr~Ammunition~ ammo_
        -unique_ptr~Weapon~ weapon_
        -Pilot* pilot_
        +FighterJet(string model, int defense, int initial_ammo, string weapon_name, int weapon_dmg)
        +~FighterJet()
        +assign_pilot(Pilot* p) void
        +fire_at_target() void
    }

    %% Relações do sistema baseado no ciclo de vida (Composição e Agregação)
    FighterJet "1" *-- "1" Ammunition : possui (Composição)
    FighterJet "1" *-- "1" Weapon : possui (Composição)
    FighterJet "1" --> "0..1" Pilot : usa (Agregação)
```