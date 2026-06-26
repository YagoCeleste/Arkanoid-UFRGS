#include <stdio.h>
#include <raylib.h>
#include "arkanoid.h"

int main(void) {
    // Inicialização da janela principal através da biblioteca Raylib
    InitWindow(LARGURA, ALTURA, "Arkanoid - Trabalho Final");
    SetTargetFPS(60); // Travar o FPS é fundamental para a física da bola não quebrar

    // Variáveis de controle de fluxo do jogo
    EstadoJogo estado_atual = MENU; 
    int selecionada_menu = 0;       
    bool sair = false;              
    int tempoMensagemSave = 0; // Controla os frames que a mensagem "Jogo Salvo" fica na tela      
    
    // Variável que armazena a fase atual. O trabalho exige no mínimo 3 fases.
    int fase_atual = 1;             

    // Inicializando as structs com os valores iniciais
    Plataforma plataforma = {LARGURA/2 - 50, ALTURA - 40, 100, 15, 7.0f};
    Jogador jogador = {3, 0, ""}; // O jogador começa com exatamente 3 VIDAS e 0 pontos
    
    // Preparando o vetor de bolas
    Bola bolas[MAX_BOLAS];
    for (int i = 0; i < MAX_BOLAS; i++) bolas[i] = (Bola){0, 0, 5.0f, -5.0f, -1, 6.0f}; 
    
    // Preparando o vetor de powerups
    PowerUp powerups[MAX_POWERUPS];
    for (int i=0; i<MAX_POWERUPS; i++) powerups[i].ativo = 0;
    
    int fogo_timer = 0; 
    Tijolo tijolos[MAX_TIJOLOS]; 
    int qtd_tijolos = 0; 

    // Game loop principal. Mantém o jogo rodando enquanto a janela não for fechada.
    while (!WindowShouldClose() && !sair) {
        
        // ==========================================
        // UPDATE: Lógica matemática e captura de teclas
        // ==========================================
        switch (estado_atual) {
            case MENU:
                // Lógica de navegação do menu principal
                if (IsKeyPressed(KEY_UP)) selecionada_menu--;
                if (IsKeyPressed(KEY_DOWN)) selecionada_menu++;
                
                // Transforma o menu num loop circular
                if (selecionada_menu < 0) selecionada_menu = 3;
                if (selecionada_menu > 3) selecionada_menu = 0;

                if (IsKeyPressed(KEY_ENTER)) {
                    if (selecionada_menu == 0) { 
                        // NOVO JOGO: Reseta todas as variáveis importantes para os padrões iniciais
                        jogador.vidas = 3; // Reseta as 3 vidas
                        jogador.pontos = 0; 
                        plataforma.larg = 100; 
                        bolas[0].ativa = 0; // Prende a bola na raquete
                        bolas[1].ativa = -1; // Desativa clones
                        bolas[2].ativa = -1; 
                        fogo_timer = 0; 
                        fase_atual = 1; // Força o jogo a começar na fase 1
                        
                        for(int i=0; i<MAX_POWERUPS; i++) powerups[i].ativo = 0; 
                        
                        // Chama a função que lê o arquivo fase_1.txt
                        CarregaFaseTXT(tijolos, &qtd_tijolos, fase_atual); 
                        estado_atual = JOGO; 
                    } 
                    else if (selecionada_menu == 1) { 
                        // CARREGAR JOGO: Puxa o save do binário
                        if (CarregaJogo(&jogador, tijolos, &qtd_tijolos)) {
                            bolas[0].ativa = 0; 
                            bolas[1].ativa = -1; 
                            bolas[2].ativa = -1;
                            estado_atual = JOGO;
                        }
                    }
                    else if (selecionada_menu == 3) { 
                        sair = true; // Quebra o while e encerra
                    }
                }
                break;

            case JOGO:
                if (IsKeyPressed(KEY_P)) estado_atual = PAUSE; 
                
                // Tecla S salva o progresso usando função da Prática 12
                if (IsKeyPressed(KEY_S)) { 
                    SalvaJogo(&jogador, tijolos, qtd_tijolos);
                    tempoMensagemSave = 120; // Ativa a mensagem na tela por 120 frames
                }
                
                // Desconta o timer do save e do poder de fogo a cada frame
                if (tempoMensagemSave > 0) tempoMensagemSave--;
                if (fogo_timer > 0) fogo_timer--;

                // Controle híbrido da plataforma (feature extra: Teclado + Mouse)
                if (IsKeyDown(KEY_LEFT)) plataforma.x -= plataforma.vel;
                if (IsKeyDown(KEY_RIGHT)) plataforma.x += plataforma.vel;
                
                Vector2 deltaMouse = GetMouseDelta();
                if (deltaMouse.x != 0) plataforma.x = GetMouseX() - (plataforma.larg / 2);
                
                // Colisão da plataforma com as bordas da janela
                if (plataforma.x < 0) plataforma.x = 0;
                if (plataforma.x > LARGURA - plataforma.larg) plataforma.x = LARGURA - plataforma.larg;

                // Se a bola principal estiver no estado inicial (presa na raquete)
                if (bolas[0].ativa == 0) {
                    bolas[0].x = plataforma.x + (plataforma.larg / 2);
                    bolas[0].y = plataforma.y - bolas[0].raio - 2;
                    
                    // Lança a bola ao detectar o clique ou espaço
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE)) bolas[0].ativa = 1; 
                } else {
                    // Atualiza a física das bolas e dos powerups
                    MoveBolas(bolas, &plataforma, &jogador, &estado_atual, tijolos, qtd_tijolos, powerups, &fogo_timer);
                    MovePowerUps(powerups, &plataforma, bolas, &fogo_timer);
                }

                // Verifica a condição de vitória da fase atual (contando blocos restantes)
                int blocos_vivos = 0;
                for (int i = 0; i < qtd_tijolos; i++) {
                    // Blocos do tipo 99 (indestrutíveis) não contam para passar de fase
                    if (tijolos[i].ativo == 1 && tijolos[i].tipo != 99) blocos_vivos++; 
                }
                
                // PROGRESSÃO DAS 3 FASES
                // Se todos os blocos normais foram destruídos:
                if (blocos_vivos == 0) {
                    fase_atual++; // Aumenta o contador de fase
                    
                    // O jogo exige passar pelas 3 fases. Se concluir a terceira, o jogador ganha!
                    if (fase_atual > 3) {
                        estado_atual = VITORIA; 
                    } else {
                        // Se for para a fase 2 ou 3, limpa a tela e carrega o próximo arquivo txt
                        bolas[0].ativa = 0; 
                        bolas[1].ativa = -1; 
                        bolas[2].ativa = -1;
                        for(int i=0; i<MAX_POWERUPS; i++) powerups[i].ativo = 0;
                        
                        CarregaFaseTXT(tijolos, &qtd_tijolos, fase_atual);
                    }
                }
                break;

            case PAUSE:
                // Retoma com P ou volta ao menu principal com ESC
                if (IsKeyPressed(KEY_P)) estado_atual = JOGO;
                if (IsKeyPressed(KEY_ESCAPE)) estado_atual = MENU;
                break;
                
            case GAMEOVER:
            case VITORIA:
                // Aguarda o Enter para retornar ao menu após o final do jogo
                if (IsKeyPressed(KEY_ENTER)) estado_atual = MENU;
                break;
        }

        // ==========================================
        // DRAW: Renderização visual de pixels na tela
        // ==========================================
        BeginDrawing();
        ClearBackground(RAYWHITE); // Limpa o frame anterior com uma cor de fundo

        switch (estado_atual) {
            case MENU:
                DrawText("ARKANOID DO DESESPERO", 40, 80, 50, DARKBLUE);
                // Uso de if ternário para destacar a opção selecionada com cor e colchetes
                DrawText(selecionada_menu == 0 ? "[ NOVO JOGO ]" : "NOVO JOGO", 300, 250, 30, selecionada_menu == 0 ? RED : DARKGRAY);
                DrawText(selecionada_menu == 1 ? "[ CARREGAR JOGO ]" : "CARREGAR JOGO", 280, 300, 30, selecionada_menu == 1 ? RED : DARKGRAY);
                DrawText(selecionada_menu == 2 ? "[ RANKING ]" : "RANKING", 320, 350, 30, selecionada_menu == 2 ? RED : DARKGRAY);
                DrawText(selecionada_menu == 3 ? "[ SAIR ]" : "SAIR", 350, 400, 30, selecionada_menu == 3 ? RED : DARKGRAY);
                break;

            case JOGO:
            case PAUSE:
                // Desenha a plataforma (raquete) do jogador
                DrawRectangle(plataforma.x, plataforma.y, plataforma.larg, plataforma.altura, DARKBLUE);
                
                // Desenha as bolas ativas no jogo (laranja se tiver o buff de fogo)
                Color corBola = (fogo_timer > 0) ? ORANGE : RED;
                for(int i=0; i<MAX_BOLAS; i++) {
                    if (bolas[i].ativa >= 0) DrawCircle(bolas[i].x, bolas[i].y, bolas[i].raio, corBola);
                }
                
                // Desenha as pílulas de poder caindo
                for(int i=0; i<MAX_POWERUPS; i++) {
                    if(powerups[i].ativo == 1) {
                        Color corPup = BLACK;
                        if (powerups[i].tipo == 1) corPup = BLUE;    
                        if (powerups[i].tipo == 2) corPup = RED;     
                        if (powerups[i].tipo == 3) corPup = GREEN;   
                        DrawCircle(powerups[i].x, powerups[i].y, powerups[i].raio, corPup);
                    }
                }
                
                // Loop que varre a matriz e desenha tijolo por tijolo
                for (int i = 0; i < qtd_tijolos; i++) {
                    if (tijolos[i].ativo == 1) {
                        Color corTijolo;
                        // Define a cor baseada na resistência estrutural do bloco
                        if (tijolos[i].tipo == 99) corTijolo = BLACK; 
                        else if (tijolos[i].tipo == 1) corTijolo = GREEN;
                        else if (tijolos[i].tipo == 2) corTijolo = ORANGE;
                        else corTijolo = RED;

                        DrawRectangle(tijolos[i].x, tijolos[i].y, tijolos[i].larg, tijolos[i].altura, corTijolo);
                        // Desenha um contorno branco para separar visualmente os blocos
                        DrawRectangleLines(tijolos[i].x, tijolos[i].y, tijolos[i].larg, tijolos[i].altura, WHITE);
                    }
                }

                // HUD principal mostrando ao jogador a FASE, as VIDAS e os PONTOS
                DrawText(TextFormat("FASE: %d   VIDAS: %d   SCORE: %d", fase_atual, jogador.vidas, jogador.pontos), 10, 10, 20, BLACK);
                
                // Renderização visual da mensagem exigida na Prática 12
                if (tempoMensagemSave > 0) DrawText("JOGO SALVO", LARGURA - 150, 10, 20, RED);

                // Efeito visual escurecendo a tela durante a pausa
                if (estado_atual == PAUSE) {
                    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.5f)); 
                    DrawText("JOGO EM PAUSA", LARGURA/2 - 140, ALTURA/2 - 20, 40, WHITE);
                }
                break;

            case GAMEOVER:
                DrawText("FOSTE DE BASE", LARGURA/2 - 150, ALTURA/2 - 50, 40, RED);
                DrawText("Aperta ENTER pra voltar", LARGURA/2 - 130, ALTURA/2 + 20, 20, DARKGRAY);
                break;
                
            case VITORIA:
                DrawText("PASSOU NA DISCIPLINA!", LARGURA/2 - 220, ALTURA/2 - 50, 40, GREEN);
                DrawText("Aperta ENTER pro menu", LARGURA/2 - 130, ALTURA/2 + 20, 20, DARKGRAY);
                break;
        }

        EndDrawing(); 
    }
    CloseWindow(); // Libera os recursos alocados pela raylib
    return 0;
}