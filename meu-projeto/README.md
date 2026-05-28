# Nome: Milton Bezerra do Vale Filho
# Matrícula: 20250018898

# Descrição do Domínio Escolhido:

classDiagram
    class CombatLobby {
        -string lobby_id_
        -vector~unique_ptr~Aircraft~~ active_fighters_
        +register_player(unique_ptr~Aircraft~ jet) void
        +broadcast_status() void
    }
    
    class Aircraft {
        -string pilot_callsign_
        -int hull_integrity_
        -int current_speed_
        -int missile_count_
        -unique_ptr~CockpitHUD~ hud_
        -shared_ptr~Weapon~ equipped_weapon_
        +fly_maneuver(int target_speed) void
        +fire_active_weapon(Aircraft& target) void
        +take_damage(int amount) void
    }

    class CockpitHUD {
        -string display_color_
        +render_telemetry(int speed, int armor, int ammo) void
        +show_lock_warning() void
    }

    class Weapon {
        -string caliber_name_
        -int base_damage_
        +is_reload_ready() bool
    }

    CombatLobby "1" *-- "0..*" Aircraft : composição (sessão retém a posse física das aeronaves ativas)
    Aircraft "1" *-- "1" CockpitHUD : composição (o visor HUD é parte física intrínseca do caça)
    Aircraft "1" o-- "0..1" Weapon : agregação (arma de uso livre que existe no hangar geral do jogo)
