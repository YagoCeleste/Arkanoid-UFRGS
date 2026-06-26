#ifndef ARKANOID_H
#define ARKANOID_H

#include <raylib.h>

// Definindo constantes para não usar "números mágicos" soltos no código
#define LARGURA 800
#define ALTURA 600
#define MAX_TIJOLOS 375 // Tamanho máximo da matriz do mapa (15x25) exigido no trabalho
#define MAX_BOLAS 3     // Limite para o power-up de bolas extras
#define MAX_POWERUPS 30 // Quantidade máxima de power-ups caindo na tela

// Enumerador para controlar as telas do jogo de forma estruturada (máquina de estados)
typedef enum { MENU, JOGO, PAUSE, GAMEOVER, VITORIA } EstadoJogo;

// Struct para organizar as informações da nossa raquete
typedef struct { 
    float x, y;          // Posição na tela
    float larg, altura;  // Dimensões da barra
    float vel;           // Velocidade de movimento
} Plataforma;

// Struct da bola (dx e dy controlam a direção e a velocidade do movimento)
typedef struct { 
    float x, y, dx, dy; 
    int ativa; // -1: não existe, 0: colada na raquete, 1: em movimento
    float raio; 
} Bola;

// Struct para armazenar o progresso do jogador (útil para salvar no arquivo binário)
typedef struct { 
    int vidas; 
    int pontos; 
    char nome[20]; 
} Jogador;

// Struct dos tijolos que compõem a fase
typedef struct { 
    float x, y, larg, altura; 
    int tipo;                 // O tipo define a cor e quantas batidas o bloco aguenta. 99 = indestrutível.
    int ativo;                // 1 = visível, 0 = destruído
    int colisoes_necessarias; // Pontos de vida do bloco atual
    int tem_powerup;          // Armazena se este bloco vai dropar algum poder
} Tijolo;

// Struct dos poderes que caem dos blocos destruídos
typedef struct { 
    float x, y, raio, vel; 
    int tipo;  // 1: expandir barra, 2: bola de fogo, 3: bolas extras
    int ativo; 
} PowerUp;

// Protótipos das funções para modularizar o código e avisar o compilador que elas existem
void CarregaFaseTXT(Tijolo tijolos[], int *qtd_tijolos, int fase);
void MoveBolas(Bola bolas[], Plataforma *p, Jogador *jog, EstadoJogo *estado, Tijolo tijolos[], int qtd_tijolos, PowerUp pups[], int *fogo_timer);
void MovePowerUps(PowerUp pups[], Plataforma *p, Bola bolas[], int *fogo_timer);
void SalvaJogo(Jogador *jog, Tijolo tijolos[], int quantidade);
int CarregaJogo(Jogador *jog, Tijolo tijolos[], int *quantidade);

#endif