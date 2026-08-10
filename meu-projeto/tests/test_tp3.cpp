// test_tp3.cpp - testes automatizados com Catch2 v3 (Questao 5)
// Cobre: (1) template/concept da Q1, (2) excecao+optional da Q2,
// (3) serializacao round-trip da Q4, (4) DIP com memory_repository.

#include <catch2/catch_test_macros.hpp>
#include "../src/domain.hpp"

// ------------------------------------------------------------
// (1) Questao 1: template registry<T> + concept possui_poder_de_fogo
// ------------------------------------------------------------
TEST_CASE("registry<T> guarda e recupera itens de qualquer tipo") {
    registry<std::string> registro_bases;
    registro_bases.add("Base Aerea Aurora");
    registro_bases.add("Base Aerea Talon");

    REQUIRE(registro_bases.size() == 2);
    REQUIRE(registro_bases.at(0) == "Base Aerea Aurora");
    REQUIRE(registro_bases.at(1) == "Base Aerea Talon");
}

TEST_CASE("soma_poder_de_fogo restrito pelo concept possui_poder_de_fogo") {
    Pilot piloto("Ghost", 90, 18);

    std::vector<FighterJet> esquadrao;
    esquadrao.emplace_back("F-16 Fighting Falcon", 70, 60, 3, "AIM-9", 30);
    esquadrao.back().assign_pilot(&piloto);

    // FighterJet satisfaz o concept (tem calculate_firepower()), entao compila e soma > 0
    REQUIRE(soma_poder_de_fogo(esquadrao) > 0.0f);

    // Nota: soma_poder_de_fogo(std::vector<Pilot>{}) NAO compilaria aqui de proposito -
    // e exatamente o comportamento esperado do concept (Questao 1-D).
}

// ------------------------------------------------------------
// (2) Questao 2: excecao propria (captura pela base) + optional
// ------------------------------------------------------------
TEST_CASE("excecao do dominio: municao_insuficiente e capturada pela base erro_dominio") {
    FighterJet caca("F-22 Raptor", 75, 80, 2, "AIM-120", 45);

    REQUIRE_THROWS_AS(caca.fire_weapon(999), erro_dominio);
}

TEST_CASE("busca com optional: achou e nao achou") {
    registry<Pilot> registro_pilotos;
    registro_pilotos.add(Pilot("Ghost", 90, 18));
    registro_pilotos.add(Pilot("Falcon", 100, 12));

    auto achado = buscar_piloto_por_callsign(registro_pilotos, "Ghost");
    REQUIRE(achado.has_value());
    REQUIRE(achado->get_callsign() == "Ghost");

    auto nao_achado = buscar_piloto_por_callsign(registro_pilotos, "Maverick");
    REQUIRE_FALSE(nao_achado.has_value());
}

// ------------------------------------------------------------
// (3) Questao 4: serializacao JSON round-trip (via memory_repository,
//     que ja e a implementacao de teste - sem tocar disco)
// ------------------------------------------------------------
TEST_CASE("serializacao round-trip: estado_missao == estado apos save/load") {
    estado_missao original;
    original.pilot_callsign_ = "Wormwood";
    original.pilot_hp_ = 100;
    original.pilot_agility_ = 15;
    original.aeronaves_.push_back(
        aircraft_snapshot{"FighterJet", "F-22 Raptor", 75, 80, 4, "AIM-120", 45});

    memory_repository repo;
    repo.save(original);
    estado_missao recuperado = repo.load();

    REQUIRE(recuperado == original); // sem tocar o disco
}

// ------------------------------------------------------------
// (4) Questao 4: DIP - missao_app exercitado com memory_repository,
//     provando que a logica de alto nivel independe da implementacao concreta
// ------------------------------------------------------------
TEST_CASE("DIP: missao_app funciona com memory_repository sem efeito colateral") {
    memory_repository repo;
    missao_app app(repo); // injecao de dependencia

    estado_missao estado;
    estado.pilot_callsign_ = "Ace";
    estado.pilot_hp_ = 80;
    estado.pilot_agility_ = 20;
    estado.aeronaves_.push_back(
        aircraft_snapshot{"Interceptor", "MiG-31 Foxhound", 110, 15, 6, "R-37", 60});

    app.salvar(estado);
    estado_missao carregado = app.carregar();

    REQUIRE(carregado == estado);
    REQUIRE(carregado.aeronaves_.size() == 1);
    REQUIRE(carregado.aeronaves_.front().type_ == "Interceptor");
}

TEST_CASE("memory_repository lanca erro_dominio ao carregar sem ter salvo antes") {
    memory_repository repo_vazio;
    REQUIRE_THROWS_AS(repo_vazio.load(), erro_dominio);
}
