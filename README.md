# Arkanoid - Trabalho Final de Algoritmos

Este projeto é uma implementação do clássico jogo **Arkanoid**, desenvolvido integralmente em **C** utilizando a biblioteca gráfica **Raylib**. O objetivo do jogo é controlar uma plataforma para rebater uma bola e destruir todos os blocos de cada fase.

## 🎮 Como o jogo funciona
* **Objetivo:** Destruir todos os blocos da tela para avançar de nível.
* **Mecânicas:**
    * **Controle:** O jogador movimenta a plataforma horizontalmente para impedir que a bola caia.
    * **Progressão:** O jogo apresenta diferentes níveis de dificuldade, com layouts de blocos carregados via arquivos de texto externos (`fase_1.txt`, etc.).
    * **Power-ups:** Alguns blocos, ao serem destruídos, liberam itens especiais que alteram o comportamento da bola ou da plataforma.
    * **Persistência:** O jogo possui um sistema de `save.dat` para continuar o progresso e um `ranking.txt` que armazena as melhores pontuações dos jogadores.

## 🛠️ Tecnologias e Ferramentas
* **Linguagem:** C
* **Biblioteca Gráfica:** Raylib
* **Controle de Dados:** Manipulação de arquivos de texto e binários.
* **Ambiente de Desenvolvimento:** VS Code

## 👤 Autor
* **Yago Celeste** - Estudante de Ciência da Computação @ UFRGS

## 🚀 Como rodar
1. Clone este repositório:
   `git clone https://github.com/YagoCeleste/Arkanoid-UFRGS.git`
2. Certifique-se de ter a Raylib configurada em seu ambiente.
3. Compile utilizando o GCC:
   `gcc main.c logica.c -lraylib -o arkanoid`
4. Execute: `./arkanoid`