# Nome: Milton Bezerra do Vale Filho
# Matrícula: 20250018898

# Descrição do Domínio Escolhido:

Este projeto implementa a arquitetura base para um jogo de RPG online focado em combate aéreo militar moderno. O sistema gerencia o inventário de aeronaves através de bases operacionais (`Hangar`), que armazenam caças de combate (`FighterJet`) equipados com sistemas eletrônicos integrados de radar e travamento de alvos (`Avionics`). Os jogadores atuam como pilotos táticos (`Pilot`), que possuem atributos de progressão de nível e horas de voo, podendo ser designados para diferentes aeronaves em missões.

```mermaid
classDiagram
    class Pilot {
        -string callsign_
        -int level_
        +Pilot(callsign, level)
        +~Pilot()
        +get_callsign() string
        +level_up() void
    }

    class Avionics {
        -int radar_
        +Avionics(radar)
        +~Avionics()
        +get_radar() int
    }

    class FighterJet {
        -string model_
        -unique_ptr~Avionics~ avionics_
        -Pilot* pilot_
        +FighterJet(model, radar)
        +~FighterJet()
        +assign_pilot(p) void
        +status() void
    }

    class Hangar {
        -string name_
        +Hangar(name)
        +~Hangar()
        +enter_base() void
    }

    %% Relações do sistema baseado no ciclo de vida (Composição e Agregação)
    FighterJet "1" *-- "1" Avionics : possui (Composição)
    FighterJet "1" --> "0..1" Pilot : usa (Agregação)
```