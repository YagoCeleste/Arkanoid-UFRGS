#include <stdio.h>
#include <stdlib.h>
#include "arkanoid.h"

// Função que abre o arquivo de texto específico da fase atual e monta o mapa
void CarregaFaseTXT(Tijolo tijolos[], int *qtd_tijolos, int fase) {
    char nome_arquivo[20];
    
    // O sprintf formata a string trocando o %d pelo número da fase (ex: "fase_2.txt")
    sprintf(nome_arquivo, "fase_%d.txt", fase); 
    
    FILE *arq = fopen(nome_arquivo, "r");
    
    // Verificação de segurança: se o arquivo não existir, cria um bloco padrão para o jogo não fechar sozinho
    if (arq == NULL) {
        tijolos[0] = (Tijolo){100, 100, 50, 20, 1, 1, 1, 0};
        *qtd_tijolos = 1;
        return;
    }

    int linha = 0, col = 0, indice = 0;
    char caractere;
    float larg_tijolo = LARGURA / 25.0f; // Divide a tela em 25 colunas
    float alt_tijolo = 20.0f;
    
    // Lê o arquivo caractere por caractere até chegar ao fim (EOF)
    while ((caractere = fgetc(arq)) != EOF) {
        // Se encontrar uma quebra de linha, zera a coluna e desce uma linha no mapa
        if (caractere == '\n') { linha++; col = 0; continue; }
        
        // Ignora os espaços em branco
        if (caractere != ' ' && caractere != '\r') {
            // Calcula as coordenadas reais em pixels baseadas na matriz do texto
            tijolos[indice].x = col * larg_tijolo;
            tijolos[indice].y = 50 + (linha * alt_tijolo); 
            tijolos[indice].larg = larg_tijolo - 2; // O -2 serve para deixar uma bordinha entre os tijolos
            tijolos[indice].altura = alt_tijolo - 2;
            tijolos[indice].ativo = 1; 
            
            // X representa os blocos indestrutíveis
            if (caractere == 'X') {
                tijolos[indice].tipo = 99; 
                tijolos[indice].colisoes_necessarias = 99; 
                tijolos[indice].tem_powerup = 0; 
            } else {
                // Subtrair '0' converte o caractere numérico para seu valor inteiro real
                tijolos[indice].tipo = caractere - '0'; 
                tijolos[indice].colisoes_necessarias = tijolos[indice].tipo;
                
                // Probabilidade de 20% de sortear um power-up (tipos de 1 a 3) para este bloco
                tijolos[indice].tem_powerup = (GetRandomValue(1, 100) <= 20) ? GetRandomValue(1, 3) : 0;
            }
            indice++; // Avança a posição no vetor de tijolos
        }
        col++; 
    }
    // Modifica o valor na main usando o ponteiro passado por parâmetro
    *qtd_tijolos = indice; 
    fclose(arq); 
}

// Função responsável por instanciar a bolinha do poder caindo na tela
void SpawnaPowerUp(PowerUp pups[], float x, float y, int tipo) {
    // Procura no vetor o primeiro power-up que estiver inativo para reaproveitar a posição
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (pups[i].ativo == 0) {
            pups[i].x = x; pups[i].y = y; pups[i].raio = 8.0f;
            pups[i].vel = 3.0f; pups[i].tipo = tipo; pups[i].ativo = 1;
            break; 
        }
    }
}

// Lógica principal de física: atualiza posições e testa as colisões de todas as bolas ativas
void MoveBolas(Bola bolas[], Plataforma *p, Jogador *jog, EstadoJogo *estado, Tijolo tijolos[], int qtd_tijolos, PowerUp pups[], int *fogo_timer) {
    int bolas_vivas = 0;

    for (int b = 0; b < MAX_BOLAS; b++) {
        // Só processa as bolas que estão em jogo
        if (bolas[b].ativa == 1) {
            bolas_vivas++;
            bolas[b].x += bolas[b].dx;
            bolas[b].y += bolas[b].dy;

            // Tratamento de colisão com as paredes laterais (inverte o eixo x)
            if (bolas[b].x - bolas[b].raio <= 0 || bolas[b].x + bolas[b].raio >= LARGURA) bolas[b].dx *= -1;
            // Tratamento de colisão com o teto (inverte o eixo y)
            if (bolas[b].y - bolas[b].raio <= 0) bolas[b].dy *= -1;

            // Structs da raylib necessárias para a função de colisão nativa
            Vector2 centroBola = {bolas[b].x, bolas[b].y};
            Rectangle rectPlat = {p->x, p->y, p->larg, p->altura};

            // Teste de colisão entre o círculo da bola e o retângulo da plataforma
            if (CheckCollisionCircleRec(centroBola, bolas[b].raio, rectPlat)) {
                bolas[b].dy *= -1; 
                // Ajuste para evitar que a bola fique presa dentro da barra
                bolas[b].y = p->y - bolas[b].raio; 
            }

            // Loop para verificar colisão contra cada tijolo do mapa
            for (int i = 0; i < qtd_tijolos; i++) {
                if (tijolos[i].ativo == 1) {
                    Rectangle rectTijolo = {tijolos[i].x, tijolos[i].y, tijolos[i].larg, tijolos[i].altura};
                    
                    if (CheckCollisionCircleRec(centroBola, bolas[b].raio, rectTijolo)) {
                        
                        // Se não estiver com o power-up de fogo, a bola ressalta. 
                        // Se estiver com fogo, ela passa varando os blocos normais e só ressalta nos indestrutíveis (99)
                        if (*fogo_timer <= 0 || tijolos[i].tipo == 99) {
                            // Lógica simples para tentar inverter o eixo certo dependendo de onde bateu
                            if (bolas[b].x < rectTijolo.x || bolas[b].x > rectTijolo.x + rectTijolo.width) bolas[b].dx *= -1;
                            else bolas[b].dy *= -1;
                        }

                        // Aplica dano ao tijolo caso ele não seja indestrutível
                        if (tijolos[i].tipo != 99) { 
                            if (*fogo_timer > 0) tijolos[i].colisoes_necessarias = 0; // Hitkill com bola de fogo
                            else tijolos[i].colisoes_necessarias--; // Tira apenas 1 vida do bloco
                            
                            // Se os pontos de vida do bloco zerarem, ele é destruído
                            if (tijolos[i].colisoes_necessarias <= 0) {
                                tijolos[i].ativo = 0; 
                                jog->pontos += (tijolos[i].tipo * 10); // Atualiza a pontuação baseada no tipo
                                
                                // Se esse bloco tinha um power-up guardado, faz ele cair na tela
                                if (tijolos[i].tem_powerup > 0) SpawnaPowerUp(pups, tijolos[i].x + tijolos[i].larg/2, tijolos[i].y, tijolos[i].tem_powerup);
                            }
                        }
                        break; // Sai do loop para a bola não destruir dois blocos no mesmo frame
                    }
                }
            }
            // Se a bola passou da parte de baixo da tela, ela é desativada
            if (bolas[b].y + bolas[b].raio >= ALTURA) bolas[b].ativa = -1; 
        }
    }

    // Se nenhuma bola estiver mais na tela, o jogador perde uma vida
    if (bolas_vivas == 0) {
        jog->vidas--; 
        
        // Se zerar as vidas, muda o estado para a tela de fim de jogo
        if (jog->vidas <= 0) *estado = GAMEOVER;
        else {
            // Se ainda tem vidas, reposiciona a bola principal na raquete e limpa as bolas extras
            bolas[0].ativa = 0; bolas[0].dx = 5.0f; bolas[0].dy = -5.0f;
            bolas[1].ativa = -1; bolas[2].ativa = -1; 
        }
    }
}

// Atualiza a queda dos power-ups e verifica se o jogador conseguiu pegar com a barra
void MovePowerUps(PowerUp pups[], Plataforma *p, Bola bolas[], int *fogo_timer) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (pups[i].ativo == 1) {
            pups[i].y += pups[i].vel; // Faz a bolinha cair constantemente
            
            Vector2 centro = {pups[i].x, pups[i].y};
            Rectangle rectPlat = {p->x, p->y, p->larg, p->altura};
            
            // Verifica a colisão entre o poder e a plataforma
            if (CheckCollisionCircleRec(centro, pups[i].raio, rectPlat)) {
                pups[i].ativo = 0; // Desativa a bolinha da tela pois foi coletada
                
                // Aplica o efeito correspondente ao tipo do poder
                if (pups[i].tipo == 1) p->larg = 200; // Dobra o tamanho da barra
                else if (pups[i].tipo == 2) *fogo_timer = 600; // Dá 10 segundos (600 frames) de bola de fogo
                else if (pups[i].tipo == 3) {
                    // Spawna as duas bolas extras a partir da posição da bola original
                    if (bolas[0].ativa == 1) { 
                        bolas[1].ativa = 1; bolas[1].x = bolas[0].x; bolas[1].y = bolas[0].y;
                        bolas[1].dx = -bolas[0].dx; bolas[1].dy = bolas[0].dy;
                        bolas[2].ativa = 1; bolas[2].x = bolas[0].x; bolas[2].y = bolas[0].y;
                        bolas[2].dx = bolas[0].dx; bolas[2].dy = -bolas[0].dy;
                    }
                }
            }
            // Se a bolinha do poder caiu no buraco sem ser pega, desativa
            if (pups[i].y > ALTURA) pups[i].ativo = 0; 
        }
    }
}

// Implementação da Prática 12: Salvar estruturas em arquivo binário
void SalvaJogo(Jogador *jog, Tijolo tijolos[], int quantidade) {
    FILE *arq = fopen("save.dat", "wb"); // 'wb' abre o arquivo em modo de escrita binária
    if (arq != NULL) {
        fwrite(jog, sizeof(Jogador), 1, arq);
        fwrite(&quantidade, sizeof(int), 1, arq); 
        fwrite(tijolos, sizeof(Tijolo), quantidade, arq);
        fclose(arq);
    }
}

// Implementação da Prática 12: Carregar as estruturas do arquivo binário
int CarregaJogo(Jogador *jog, Tijolo tijolos[], int *quantidade) {
    FILE *arq = fopen("save.dat", "rb"); // 'rb' modo de leitura binária
    if (arq != NULL) {
        fread(jog, sizeof(Jogador), 1, arq);
        fread(quantidade, sizeof(int), 1, arq);
        fread(tijolos, sizeof(Tijolo), *quantidade, arq);
        fclose(arq);
        return 1; // Retorna verdadeiro (1) indicando sucesso
    }
    return 0; // Retorna 0 se o save não existir
}