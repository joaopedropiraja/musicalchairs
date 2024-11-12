#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <chrono>
#include <random>

int getRandomInt(int min, int max) {
	srand((unsigned) time(NULL));

  return (rand() % (max - min + 1)) + min;
}

// Global variables for synchronization
constexpr int CADEIRAS_RETIRADAS_TURNO = 1;
constexpr int MUSICA_TEMPO_ESPERA_MIN = 2000;
constexpr int MUSICA_TEMPO_ESPERA_MAX = 10000;
constexpr int NUM_JOGADORES = 4;

int vencedor = -1;

std::counting_semaphore<NUM_JOGADORES> cadeira_sem(NUM_JOGADORES - 1); // Inicia com n-1 cadeiras, capacidade máxima n
std::condition_variable music_cv;
std::mutex music_mutex;
std::atomic<bool> musica_parada{ true };
std::atomic<bool> jogo_ativo{ true };

/*
 * Uso básico de um counting_semaphore em C++:
 * 
 * O `std::counting_semaphore` é um mecanismo de sincronização que permite controlar o acesso a um recurso compartilhado 
 * com um número máximo de acessos simultâneos. Neste projeto, ele é usado para gerenciar o número de cadeiras disponíveis.
 * Inicializamos o semáforo com `n - 1` para representar as cadeiras disponíveis no início do jogo. 
 * Cada jogador que tenta se sentar precisa fazer um `acquire()`, e o semáforo permite que até `n - 1` jogadores 
 * ocupem as cadeiras. Quando todos os assentos estão ocupados, jogadores adicionais ficam bloqueados até que 
 * o coordenador libere o semáforo com `release()`, sinalizando a eliminação dos jogadores.
 * O método `release()` também pode ser usado para liberar múltiplas permissões de uma só vez, por exemplo: `cadeira_sem.release(3);`,
 * o que permite destravar várias threads de uma só vez, como é feito na função `liberar_threads_eliminadas()`.
 *
 * Métodos da classe `std::counting_semaphore`:
 * 
 * 1. `acquire()`: Decrementa o contador do semáforo. Bloqueia a thread se o valor for zero.
 *    - Exemplo de uso: `cadeira_sem.acquire();` // Jogador tenta ocupar uma cadeira.
 * 
 * 2. `release(int n = 1)`: Incrementa o contador do semáforo em `n`. Pode liberar múltiplas permissões.
 *    - Exemplo de uso: `cadeira_sem.release(2);` // Libera 2 permissões simultaneamente.
 */

// Classes
class JogoDasCadeiras {
public:
    JogoDasCadeiras(int num_jogadores)
        : num_jogadores(num_jogadores) {
            this->jogadores_eliminados.reserve(CADEIRAS_RETIRADAS_TURNO);
            this->jogadores_com_cadeiras.reserve(num_jogadores - 1);
        }

    void iniciar_rodada() {
        // TODO: Inicia uma nova rodada, removendo uma cadeira e ressincronizando o semáforo
        this->cadeiras = this->num_jogadores - CADEIRAS_RETIRADAS_TURNO;
        this->jogadores_com_cadeiras.clear();
        this->jogadores_eliminados.clear();
        cadeira_sem.release(this->cadeiras);

        musica_parada = true;
        std::cout << "Iniciando rodada com " << this->num_jogadores << " jogadores e " << this->cadeiras << " cadeiras." << std::endl;
        std::cout << "A música está tocando... 🎵" << std::endl << std::endl;
    }

    void parar_musica() {
        // TODO: Simula o momento em que a música para e notifica os jogadores via variável de condição
        std::cout << "> A música parou! Os jogadores estão tentando se sentar..." << std::endl << std::endl;
        musica_parada = false;
        music_cv.notify_all();
    }

    void eliminar_jogador(int jogador_id) {
        // TODO: Elimina um jogador que não conseguiu uma cadeira
        this->jogadores_eliminados.push_back(jogador_id);
        this->num_jogadores--;
    }

    void exibir_estado() {
        std::cout << "-----------------------------------------------" << std::endl;

        int i = 1;
        for (int i = 0; i < this->jogadores_com_cadeiras.size(); ++i) {
            std::cout << "[Cadeira " << i + 1 << "]: Ocupada por P" << this->jogadores_com_cadeiras[i] << std::endl;
        }
        std::cout << std::endl;

        for (int &player : this->jogadores_eliminados) {
            std::cout << "Jogador P" << player << " não conseguiu uma cadeira e foi eliminado!" << std::endl;
        }

        std::cout << "-----------------------------------------------" << std::endl << std::endl;     
    }

    void verificar_fim_jogo() {
        if (this->jogadores_com_cadeiras.size() <= 1) {
            jogo_ativo = false;
            vencedor = this->jogadores_com_cadeiras[0];
            music_cv.notify_one();
        }
    }

    void ocupar_cadeira(int jogador_id) {
        this->cadeiras--;
        this->jogadores_com_cadeiras.push_back(jogador_id);
    }

    int tem_cadeira_disponivel() {
        return this->cadeiras > 0;
    }

    bool jogadores_nao_foram_eliminados() {
        return this->jogadores_eliminados.size() != CADEIRAS_RETIRADAS_TURNO;
    }    

private:
    std::vector<int> jogadores_com_cadeiras;
    std::vector<int> jogadores_eliminados;
    int num_jogadores;
    int cadeiras;
};

class Jogador {
public:
    Jogador(int id, JogoDasCadeiras& jogo)
        : id(id), jogo(jogo), eliminado(false) {}

    void tentar_ocupar_cadeira() {
        // TODO: Tenta ocupar uma cadeira utilizando o semáforo contador quando a música para (aguarda pela variável de condição)
        cadeira_sem.acquire();
    }

    void verificar_eliminacao() {
        // TODO: Verifica se foi eliminado após ser destravado do semáforo
        if (this->jogo.tem_cadeira_disponivel()) {
            this->jogo.ocupar_cadeira(this->id);
        } else {
            this->jogo.eliminar_jogador(this->id);
            this->eliminado = true;
        }
    }

    void joga() {
        while (!this->eliminado) {
            // TODO: Aguarda a música parar usando a variavel de condicao
            std::unique_lock<std::mutex> lock(music_mutex);
            do {
                music_cv.wait(lock);
            } while(musica_parada);

            if (!jogo_ativo) break;

            // TODO: Tenta ocupar uma cadeira
            this->tentar_ocupar_cadeira();
            
            // TODO: Verifica se foi eliminado
            this->verificar_eliminacao();
        }
    }

private:
    int id;
    JogoDasCadeiras& jogo;
    bool eliminado;
};

class Coordenador {
public:
    Coordenador(JogoDasCadeiras& jogo)
        : jogo(jogo) {}

    void iniciar_jogo() {
        // TODO: Começa o jogo, dorme por um período aleatório, e então para a música, sinalizando os jogadores 
        std::cout << "-----------------------------------------------" << std::endl;
        std::cout << "Bem-vindo ao Jogo das Cadeiras Concorrente!" << std::endl;
        std::cout << "-----------------------------------------------" << std::endl << std::endl;

        while (jogo_ativo) {
            this->jogo.iniciar_rodada();

            int totalTime = getRandomInt(MUSICA_TEMPO_ESPERA_MIN, MUSICA_TEMPO_ESPERA_MAX);
            std::this_thread::sleep_for(std::chrono::milliseconds(totalTime));

            this->jogo.parar_musica();
            while (this->jogo.tem_cadeira_disponivel());
            
            this->liberar_threads_eliminadas();
            while (this->jogo.jogadores_nao_foram_eliminados());

            this->jogo.verificar_fim_jogo();
            this->jogo.exibir_estado();
        }

        std::cout << "🏆 Vencedor: Jogador P" << vencedor << "! Parabéns! 🏆" << std::endl;
        std::cout << "-----------------------------------------------" << std::endl << std::endl;

        std::cout << "Obrigado por jogar o Jogo das Cadeiras Concorrente!" << std::endl;        
    }

    void liberar_threads_eliminadas() {
        // Libera múltiplas permissões no semáforo para destravar todas as threads que não conseguiram se sentar
        cadeira_sem.release(CADEIRAS_RETIRADAS_TURNO); // Libera o número de permissões igual ao número de jogadores que ficaram esperando
    }

private:
    JogoDasCadeiras& jogo;
};

// Main function
int main() {
    JogoDasCadeiras jogo(NUM_JOGADORES);
    Coordenador coordenador(jogo);
    std::vector<std::thread> jogadores;

    // Criação das threads dos jogadores
    std::vector<Jogador> jogadores_objs;
    for (int i = 1; i <= NUM_JOGADORES; ++i) {
        jogadores_objs.emplace_back(i, jogo);
    }

    for (int i = 0; i < NUM_JOGADORES; ++i) {
        jogadores.emplace_back(&Jogador::joga, &jogadores_objs[i]);
    }

    // Thread do coordenador
    std::thread coordenador_thread(&Coordenador::iniciar_jogo, &coordenador);

    // Esperar pelas threads dos jogadores
    for (auto& t : jogadores) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Esperar pela thread do coordenador
    if (coordenador_thread.joinable()) {
        coordenador_thread.join();
    }

    return 0;
}

