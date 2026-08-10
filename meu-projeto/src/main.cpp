#include "domain.hpp"

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

        std::cout << "\n--- QUESTAO 4: Serializacao JSON + DIP (json_repository / memory_repository) ---\n";

        // Monta o estado atual (piloto + frota) a partir dos objetos polimorficos
        estado_missao estado_atual = monta_estado(frota, *player_pilot);

        // (4-C)(4-D) missao_app depende so da ABSTRACAO estado_repository.
        // Aqui trocamos a implementacao (arquivo <-> memoria) sem mudar missao_app.
        json_repository repo_arquivo("estado_missao.json");
        missao_app app_producao(repo_arquivo);
        app_producao.salvar(estado_atual);
        estado_missao estado_recarregado = app_producao.carregar();
        std::cout << "[QUESTAO 4] Round-trip via arquivo OK? "
                  << (estado_atual == estado_recarregado ? "sim" : "nao") << "\n";

        // memory_repository: exercita a MESMA logica de missao_app sem tocar o disco
        memory_repository repo_memoria;
        missao_app app_teste(repo_memoria);
        app_teste.salvar(estado_atual);
        estado_missao estado_em_memoria = app_teste.carregar();
        std::cout << "[QUESTAO 4] Round-trip via memoria (sem I/O) OK? "
                  << (estado_atual == estado_em_memoria ? "sim" : "nao") << "\n";

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