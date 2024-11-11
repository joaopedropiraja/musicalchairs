# Jogo das Cadeiras com Programação Concorrente

![Jogo das Cadeiras Concorrente](musical_chairs.gif)

## Descrição do Projeto

Neste trabalho prático, você irá implementar o clássico **Jogo das Cadeiras** com jogadores controlados pelo computador utilizando conceitos de programação concorrente em C++. O objetivo é praticar o uso de **semáforos contadores** para gerenciar o acesso às cadeiras limitadas, simulando a disputa entre os jogadores por recursos limitados.

A implementação envolverá quatro componentes principais:

1. **Jogadores Concorrentes**: Múltiplos jogadores tentarão ocupar as cadeiras quando a música parar.
2. **Cadeiras**: A cada rodada, há menos cadeiras do que jogadores.
3. **Controle de Sincronização**: Um **semáforo contador** será utilizado para controlar o número de jogadores que conseguem se sentar a cada rodada.
4. **Coordenador do Jogo**: Uma thread coordenadora que controla o início e a parada da música.

## Requisitos

- Utilizar um **semáforo contador** para permitir que até o número disponível de jogadores consiga ocupar as cadeiras simultaneamente. O semáforo deve ser inicializado com valor **n - 1** (onde n é o número de jogadores) e ter valor máximo possível de **n**.
- Utilizar uma **variável de condição** para que as threads dos jogadores aguardem até que a música pare para tentar ocupar uma cadeira.
- Implementar uma **thread coordenadora** que começa o jogo, dorme por um período aleatório e para a música, notificando os jogadores.
- Cada rodada deve remover uma cadeira, até que reste apenas um jogador vencedor.
- Implementar uma interface baseada em texto que simule o Jogo das Cadeiras de forma interativa.

## Pseudo Código da Solução

### Estrutura Geral do Código

```pseudo
Classe JogoDasCadeiras:
    - Atributos:
        - cadeiras: número de cadeiras disponíveis (inicialmente n - 1, onde n é o número de jogadores)
        - players: lista de jogadores ativos no jogo
        - semaphore: semáforo contador que controla o número de cadeiras ocupáveis
        - music_cv: variável de condição para sinalizar quando a música para
        - mutex: mutex para proteger o acesso à variável de condição e outros recursos compartilhados

    - Métodos:
        - iniciar_rodada(): inicia uma nova rodada do jogo, removendo uma cadeira e ressincronizando o semáforo
        - parar_musica(): simula o momento em que a música para e notifica os jogadores via variável de condição
        - eliminar_jogador(): elimina um jogador que não conseguiu uma cadeira
        - exibir_estado(): exibe o estado atual das cadeiras e dos jogadores

Classe Jogador:
    - Atributos:
        - id: identificador do jogador (P1, P2, ...)
        - jogo: referência para o objeto JogoDasCadeiras

    - Métodos:
        - tentar_ocupar_cadeira(): tenta ocupar uma cadeira utilizando o semáforo contador quando a música para (aguarda pela variável de condição)
        - verificar_eliminacao(): verifica se foi eliminado após ser destravado do semáforo

Classe Coordenador:
    - Atributos:
        - jogo: referência para o objeto JogoDasCadeiras

    - Métodos:
        - iniciar_jogo(): começa o jogo, dorme por um período aleatório, e então para a música, sinalizando os jogadores
        - liberar_threads_eliminadas(): após os jogadores se sentarem, faz `sem_post()` adicional para destravar threads que ficaram esperando no semáforo e sinaliza que elas foram eliminadas

Main:
    - Criar objeto JogoDasCadeiras com n jogadores
    - Criar thread Coordenador para controlar a música
    - Criar threads para cada jogador
    - Iniciar rodadas até que reste apenas um jogador
    - Exibir o vencedor
```

## Regras do Jogo

1. O jogo começa com **n jogadores** e **n - 1 cadeiras**.
2. Cada jogador é representado por um identificador único (P1, P2, P3, etc.).
3. Quando a música para, os jogadores tentam ocupar uma das cadeiras assim que a **variável de condição** os notifica.
4. Caso um jogador não consiga ocupar uma cadeira, ele é **eliminado**. O coordenador do jogo faz `sem_post()` adicional para liberar as threads que ficaram travadas no semáforo. Essas threads verificam uma **flag de eliminação** e encerram a execução.
5. A cada rodada, uma cadeira é removida, até que reste apenas **um jogador vencedor**.

## Sincronização e Concorrência

- O **semáforo contador** é utilizado para garantir que apenas o número disponível de jogadores consiga ocupar as cadeiras simultaneamente. Ele é inicializado com **n - 1** e tem valor máximo de **n**.
- Uma **variável de condição** é utilizada para sincronizar as threads dos jogadores, fazendo com que elas aguardem até que a música pare.
- Uma **thread coordenadora** é responsável por iniciar o jogo, dormir por um período aleatório e então parar a música, notificando os jogadores para que tentem ocupar as cadeiras. Após isso, a thread coordenadora realiza `sem_post()` adicional para destravar as threads que ficaram esperando no semáforo e sinaliza que elas foram eliminadas.
- Cada jogador atua como uma **thread** concorrente, que tenta ocupar uma cadeira quando a música para. Se destravado após o `sem_post()` adicional, o jogador verifica a **flag de eliminação** e encerra sua execução.

## Exemplo de Interface Gráfica em Texto

Aqui está um exemplo de como a interface gráfica baseada em texto para o **Jogo das Cadeiras** poderia se comportar. O objetivo é proporcionar uma experiência interativa e visual que facilite o acompanhamento do jogo rodada a rodada.

### Interface: Execução do Jogo

```plaintext
-----------------------------------------------
Bem-vindo ao Jogo das Cadeiras Concorrente!
-----------------------------------------------

Iniciando rodada com 4 jogadores e 3 cadeiras.
A música está tocando... 🎵

> A música parou! Os jogadores estão tentando se sentar...

-----------------------------------------------
[Cadeira 1]: Ocupada por P1
[Cadeira 2]: Ocupada por P3
[Cadeira 3]: Ocupada por P4

Jogador P2 não conseguiu uma cadeira e foi eliminado!
-----------------------------------------------

Próxima rodada com 3 jogadores e 2 cadeiras.
A música está tocando... 🎵

> A música parou! Os jogadores estão tentando se sentar...

-----------------------------------------------
[Cadeira 1]: Ocupada por P3
[Cadeira 2]: Ocupada por P1

Jogador P4 não conseguiu uma cadeira e foi eliminado!
-----------------------------------------------

Próxima rodada com 2 jogadores e 1 cadeira.
A música está tocando... 🎵

> A música parou! Os jogadores estão tentando se sentar...

-----------------------------------------------
[Cadeira 1]: Ocupada por P3

Jogador P1 não conseguiu uma cadeira e foi eliminado!
-----------------------------------------------

🏆 Vencedor: Jogador P3! Parabéns! 🏆
-----------------------------------------------

Obrigado por jogar o Jogo das Cadeiras Concorrente!
```

### Descrição dos Elementos da Interface

1. **Rodadas**: Cada rodada é claramente demarcada por uma linha, indicando o número de jogadores e cadeiras restantes.
2. **Música Tocando/Parando**: Indica quando a música está tocando e quando os jogadores devem tentar se sentar. Isso é feito de forma sequencial e visual para que o aluno acompanhe o momento certo em que a sincronização acontece.
3. **Cadeiras e Jogadores**:
   - Cada cadeira é representada como `[Cadeira X]` e mostra se está "Ocupada por Px" ou vazia.
   - Os jogadores eliminados são anunciados de forma clara após cada rodada.
4. **Vencedor**: Quando resta apenas um jogador, ele é anunciado como o vencedor do jogo.

---

## Instruções de Uso

1. Clone este repositório e compile o código usando um compilador C++ compatível (ex.: `g++`).
2. Execute o programa. O jogo iniciará com os jogadores especificados.
3. A cada rodada, a interface exibirá o estado atual dos jogadores e cadeiras.
4. Observe o progresso até que restem apenas um jogador vencedor.

Divirta-se programando e aprendendo sobre sincronização concorrente com este clássico Jogo das Cadeiras! ⛰️
