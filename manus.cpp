//desenvolvido por Jhonn Vyctor Lima Alves
#include <allegro.h>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <vector>
#include <ctime>
#include <algorithm>
#include <iostream>
#define LARGURA 640
#define ALTURA 480
SAMPLE * somFundo;

#define MAPA_LARGURA 20
#define MAPA_ALTURA 20
//bem aqui, a gente define o layout do mapa,ele deve obdecer a escala das variaveis "MAPA_LARGURA" e "MAPA_ALTURA"

const char mapa[MAPA_ALTURA][MAPA_LARGURA + 1] =
{
    "####################",
    "#        #         #",
    "# ###### # ####### #",
    "# #      #       # #",
    "# # #### ##### # # #",
    "# # #          # # #",
    "# # # ########## # #",
    "# # #            # #",
    "# # ####### ###### #",
    "# #       #        #",
    "# ####### # ###### #",
    "#         #      # #",
    "######### ###### # #",
    "#       #        # #",
    "# ##### ########## #",
    "#     #            #",
    "##### ########### #",
    "#                  #",
    "#                  #",
    "####################"
};
//estados que o perseguidor pode escolher
#define PERSEGUIR 0
#define ESPREITAR 1
#define ATACAR 2
#define PROCURAR 3
#define PATRULHAR 4
struct Jogador
{
    float x;
    float y;
    float angulo;
    float persona_x;
    float persona_y;
};

Jogador jogador = { 5.0f, 5.0f, 0.0f, 5.0f, 5.0f };
struct Inimigo
{
	float x;
	float y;
	float velocidade;
	int comportamento;
    float tempoComportamento;
    float ultimaPosicaoX;
    float ultimaPosicaoY;
    int destinoPatrulhaX;
    int destinoPatrulhaY;
};
struct No{
	int x;
	int y;
};
Inimigo inimigo =
{
    17.0f,
    17.0f,
    0.04f,
    PATRULHAR,
    0.0f,
    7.0f,
    7.0f,
    17,
    17
};
bool parede(float x, float y)
{
    int mapaX = (int)x;
    int mapaY = (int)y;

    if (mapaX < 0 || mapaX >= MAPA_LARGURA ||
        mapaY < 0 || mapaY >= MAPA_ALTURA)
    {
        return true;
    }

    return mapa[mapaY][mapaX] == '#';
}
void efeito_vhs(BITMAP *tela)
{
    // Linhas horizontais
    for (int y = 0; y < ALTURA; y += 3)
    {
        line(
            tela,
            0,
            y,
            LARGURA,
            y,
            makecol(20, 20, 20)
        );
    }

    // Ruído
    for (int i = 0; i < 1200; i++)
    {
        int x = rand() % LARGURA;
        int y = rand() % ALTURA;

        int brilho = rand() % 80;

        putpixel(
            tela,
            x,
            y,
            makecol(brilho, brilho, brilho)
        );
    }

    // Faixa de interferência
    if (rand() % 100 < 8)
    {
        int y = rand() % ALTURA;
        int altura = 2 + rand() % 8;

        rectfill(
            tela,
            0,
            y,
            LARGURA,
            y + altura,
            makecol(30, 30, 30)
        );
    }
}
void desenhar_comandos(BITMAP*tela){
	textout_right(
		tela,
		font,
		"M PARA ABRIR O MINIMAPA",
		LARGURA -250,
		ALTURA - 25,
		makecol(255,255,255));
}
void desenhar_vhs(BITMAP * tela){
	textout_right(
		tela,
		font,
		"VHS",
		LARGURA -15,
		ALTURA - 25,
		makecol(255,255,255));
};

void desenhar_3d(BITMAP *tela, BITMAP *texturaParede, BITMAP *texturaChao)
{
    const float FOV = 3.14159f / 3.0f; // 60 graus

    for (int x = 0; x < LARGURA; x++)
    {
        // Ângulo do raio
        float cameraX = (float)x / LARGURA;

        float raioAngulo =
            jogador.angulo - FOV / 2.0f + cameraX * FOV;

        float raioX = cos(raioAngulo);
        float raioY = sin(raioAngulo);

        float distancia = 0.0f;

        // Avança o raio pelo mapa
        while (distancia < 20.0f)
        {
            distancia += 0.02f;

            float raioPosX = jogador.x + raioX * distancia;
            float raioPosY = jogador.y + raioY * distancia;

            if (parede(raioPosX, raioPosY))
                break;
        }

        // Corrige efeito "fish eye"
        float diferenca =
            raioAngulo - jogador.angulo;

        distancia *= cos(diferenca);

        if (distancia < 0.01f)
            distancia = 0.01f;

        // Altura da parede
        int alturaParede =
            (int)(ALTURA / distancia);

        int topo = ALTURA / 2 - alturaParede / 2;
        int baixo = ALTURA / 2 + alturaParede / 2;

        if (topo < 0)
            topo = 0;

        if (baixo >= ALTURA)
            baixo = ALTURA - 1;

        // Céu
        line(
            tela,
            x,
            0,
            x,
            topo,
            makecol(0, 50, 0)
        );

        // Parede
        float pontoX = jogador.x + raioX * distancia;
float pontoY = jogador.y + raioY * distancia;

// Descobre qual lado da parede foi atingido
float fracX = pontoX - floor(pontoX);
float fracY = pontoY - floor(pontoY);

float distanciaX = std::min(fracX, 1.0f - fracX);
float distanciaY = std::min(fracY, 1.0f - fracY);

float coordenadaTextura;

if (distanciaX < distanciaY)
{
    coordenadaTextura = pontoY - floor(pontoY);
}
else
{
    coordenadaTextura = pontoX - floor(pontoX);
}

int texX = (int)(coordenadaTextura * texturaParede->w);

if (texX < 0)
    texX = 0;

if (texX >= texturaParede->w)
    texX = texturaParede->w - 1;

// Desenha uma coluna da textura esticada verticalmente
stretch_blit(
    texturaParede,
    tela,
    texX,
    0,
    1,
    texturaParede->h,
    x,
    topo,
    1,
    baixo - topo + 1
);

        // Chão
       for (int y = baixo + 1; y < ALTURA; y++)
{
    int distanciaLinha = y - ALTURA / 2;

    if (distanciaLinha <= 0)
        continue;

    float distanciaChao =
        (float)ALTURA / (2.0f * distanciaLinha);

    float chaoX = jogador.x + raioX * distanciaChao;
    float chaoY = jogador.y + raioY * distanciaChao;

    int texX = (int)floor(chaoX * texturaChao->w);
    int texY = (int)floor(chaoY * texturaChao->h);

    texX %= texturaChao->w;
    texY %= texturaChao->h;

    if (texX < 0)
        texX += texturaChao->w;

    if (texY < 0)
        texY += texturaChao->h;

    int cor = getpixel(texturaChao, texX, texY);

    putpixel(tela, x, y, cor);
}
    }
}
bool inimigo_visivel()
{
    float dx = inimigo.x - jogador.x;
    float dy = inimigo.y - jogador.y;

    float distancia = sqrt(dx * dx + dy * dy);

    float passo = 0.02f;

    for (float d = 0.0f; d < distancia; d += passo)
    {
        float x = jogador.x + (dx / distancia) * d;
        float y = jogador.y + (dy / distancia) * d;

        if (parede(x, y))
        {
            return false;
        }
    }

    return true;
}
void desenhar_inimigo(BITMAP *tela, BITMAP *sprite)
{
    float dx = inimigo.x - jogador.x;
    float dy = inimigo.y - jogador.y;

    // Distância até o inimigo
    float distancia = sqrt(dx * dx + dy * dy);
	if (!inimigo_visivel()){
		return;
	}
    // Ângulo até o inimigo
    float anguloInimigo = atan2(dy, dx);

    // Diferença entre a direção do jogador e a direção do inimigo
    float diferenca = anguloInimigo - jogador.angulo;

    // Mantém o ângulo entre -PI e PI
    while (diferenca > 3.14159f)
        diferenca -= 2.0f * 3.14159f;

    while (diferenca < -3.14159f)
        diferenca += 2.0f * 3.14159f;

    const float FOV = 3.14159f / 3.0f;

    // Se estiver fora da visão, não desenha
    if (fabs(diferenca) > FOV / 2.0f)
        return;

    // Posição X na tela
    float cameraX = (diferenca + FOV / 2.0f) / FOV;

    int telaX = (int)(cameraX * LARGURA);

    // Tamanho do inimigo baseado na distância
    int tamanho = (int)(ALTURA / distancia);

    if (tamanho < 5)
        tamanho = 5;

    if (tamanho > ALTURA)
        tamanho = ALTURA;

    int centroY = ALTURA / 2;

    int esquerda = telaX - tamanho / 2;
    int direita = telaX + tamanho / 2;

    int topo = centroY - tamanho / 2;
    int baixo = centroY + tamanho / 2;

    // Limites da tela
    if (esquerda < 0)
        esquerda = 0;

    if (direita >= LARGURA)
        direita = LARGURA - 1;

    if (topo < 0)
        topo = 0;

    if (baixo >= ALTURA)
        baixo = ALTURA - 1;

    // Desenha o inimigo
    stretch_blit(
    sprite,
    tela,
    0,
    0,
    sprite->w,
    sprite->h,
    esquerda,
    topo,
    tamanho,
    tamanho
);
}
bool encontrar_caminho(
    int inicioX,
    int inicioY,
    int destinoX,
    int destinoY,
    std::vector<No>& caminho)
{
    bool visitado[MAPA_ALTURA][MAPA_LARGURA] = {};

    No anterior[MAPA_ALTURA][MAPA_LARGURA];

    std::queue<No> fila;

    fila.push({inicioX, inicioY});
    visitado[inicioY][inicioX] = true;

    int direcoesX[4] = { 1, -1, 0, 0 };
    int direcoesY[4] = { 0, 0, 1, -1 };

    bool encontrou = false;

    while (!fila.empty())
    {
        No atual = fila.front();
        fila.pop();

        if (atual.x == destinoX &&
            atual.y == destinoY)
        {
            encontrou = true;
            break;
        }

        for (int i = 0; i < 4; i++)
        {
            int novoX = atual.x + direcoesX[i];
            int novoY = atual.y + direcoesY[i];

            if (novoX < 0 ||
                novoX >= MAPA_LARGURA ||
                novoY < 0 ||
                novoY >= MAPA_ALTURA)
                continue;

            if (mapa[novoY][novoX] == '#')
                continue;

            if (visitado[novoY][novoX])
                continue;

            visitado[novoY][novoX] = true;

            anterior[novoY][novoX] = atual;

            fila.push({novoX, novoY});
        }
    }

    if (!encontrou)
        return false;

    No atual = { destinoX, destinoY };

    while (!(atual.x == inicioX &&
             atual.y == inicioY))
    {
        caminho.push_back(atual);

        atual = anterior[atual.y][atual.x];
    }

    caminho.push_back({ inicioX, inicioY });

    std::reverse(caminho.begin(), caminho.end());

    return true;
}
bool jogador_perto()
{
    float dx = jogador.x - inimigo.x;
    float dy = jogador.y - inimigo.y;

    float distancia = sqrt(dx * dx + dy * dy);

    // Distância na qual o inimigo percebe o jogador
    return distancia < 6.0f;
}void decidir_comportamento()
{	//essa função foi um cu  de fazer, usei i.a, forum, uma porra de coisa pra no final virar essa gambiarra
    float dx = jogador.x - inimigo.x;
    float dy = jogador.y - inimigo.y;

    float distancia = sqrt(dx * dx + dy * dy);

    // =====================================
    // ATAQUE
    // =====================================

    if (distancia < 2.0f)
    {
        inimigo.comportamento = ATACAR;
        return;
    }

    // =====================================
    // SEMPRE PERSEGUIR
    // =====================================

    inimigo.ultimaPosicaoX = jogador.x;
    inimigo.ultimaPosicaoY = jogador.y;

    inimigo.comportamento = PERSEGUIR;
}
void atualizar_velocidade_inimigo()
{
    if (inimigo.comportamento == ATACAR)
    {
        inimigo.velocidade = 0.07f;
    
    }
    else if (inimigo.comportamento == PATRULHAR){
    	inimigo.velocidade = 0.03f;
	}
    else if (inimigo.comportamento == PERSEGUIR)
    {
        inimigo.velocidade = 0.02f;
    }
    else if (inimigo.comportamento == PROCURAR)
    {
        inimigo.velocidade = 0.04f;
    }
    else if (inimigo.comportamento == ESPREITAR)
    {
        inimigo.velocidade = 0.02f;
    }
}
void escolher_destino_patrulha()
{
    int x;
    int y;

    do
    {
        x =  rand() % MAPA_LARGURA;
        y= rand() %  MAPA_ALTURA;
    }
    while (mapa[y][x] == '#');

    inimigo.destinoPatrulhaX = x;
    inimigo.destinoPatrulhaY = y;
}

void mover_inimigo()
{
    int inicioX = (int)inimigo.x;
    int inicioY = (int)inimigo.y;

    int destinoX;
    int destinoY;

    // =====================================
    // ESCOLHER DESTINO
    // =====================================
	if(inimigo.comportamento == PATRULHAR)
{
    float dx =
        (inimigo.destinoPatrulhaX + 0.5f) - inimigo.x;

    float dy =
        (inimigo.destinoPatrulhaY + 0.5f) - inimigo.y;

    float distancia =
        sqrt(dx * dx + dy * dy);

    if (distancia < 0.2f)
    {
        escolher_destino_patrulha();
    }
}
    if (inimigo.comportamento == PERSEGUIR ||
        inimigo.comportamento == ATACAR)
    {
        destinoX = (int)jogador.x;
        destinoY = (int)jogador.y;
    }
    else if (inimigo.comportamento == PROCURAR)
    {
        destinoX = (int)inimigo.ultimaPosicaoX;
        destinoY = (int)inimigo.ultimaPosicaoY;
    }
    else if (inimigo.comportamento == PATRULHAR){
    	destinoX = inimigo.destinoPatrulhaX;
    	destinoY = inimigo.destinoPatrulhaY;
	}
    else
    {	destinoX = inimigo.destinoPatrulhaX;
    	destinoY = inimigo.destinoPatrulhaY;
        // ESPREITAR
        // Por enquanto fica parado
        return;
    }
	//após uma lida , o algoritmo de achar caminhos foi um baita desafio
    // =====================================
    // PATHFINDER
    // =====================================

    std::vector<No> caminho;

    if (!encontrar_caminho(
        inicioX,
        inicioY,
        destinoX,
        destinoY,
        caminho))
    {
        return;
    }

    // =====================================
    // SE EXISTE UM PRÓXIMO PONTO
    // =====================================

    if (caminho.size() > 1)
    {
        No proximo = caminho[1];

        float alvoX = proximo.x + 0.5f;
        float alvoY = proximo.y + 0.5f;

        float dx = alvoX - inimigo.x;
        float dy = alvoY - inimigo.y;

        float distancia = sqrt(dx * dx + dy * dy);

        if (distancia > 0.05f)
        {
            inimigo.x +=
                (dx / distancia) * inimigo.velocidade;

            inimigo.y +=
                (dy / distancia) * inimigo.velocidade;
        }
    }
}
bool inimigo_encostou(){
	float dx= jogador.x - inimigo.x;
	float dy= jogador.y - inimigo.y;
	float distancia = sqrt(dx*dx + dy* dy);
	if (distancia < 0.9f){
		return true;
	}
	return false;
}
void desenhar_aviso(BITMAP *tela)
{
    float dx = jogador.x - inimigo.x;
    float dy = jogador.y - inimigo.y;

    float distancia = sqrt(dx * dx + dy * dy);

    if (distancia < 2.0f)
    {
    		
        textout_centre(
            tela,
            font,
            "CUIDADO!",
            LARGURA / 2,
            30,
            makecol(255, 0, 0)
        );
    }
    else if (distancia < 8.0f)
    {
        textout_centre(
            tela,
            font,
            "ALGO ESTA PERTO...",
            LARGURA / 2,
            30,
            makecol(255, 255, 0)
        );
    
    }
     else if (distancia >= 15.0f)
    {
        textout_centre(
            tela,
            font,
            "SEGURO...",
            LARGURA / 2,
            30,
            makecol(0, 255, 0)
        );
    
    }
}
void desenhar_minimapa(BITMAP *tela)
{
    int tamanho = 8;

    int inicioX = 10;
    int inicioY = 10;

    // Fundo do minimapa
    rectfill(
        tela,
        inicioX - 2,
        inicioY - 2,
        inicioX + MAPA_LARGURA * tamanho + 2,
        inicioY + MAPA_ALTURA * tamanho + 2,
        makecol(0, 0, 0)
    );

    // Desenha o mapa
    for (int y = 0; y < MAPA_ALTURA; y++)
    {
        for (int x = 0; x < MAPA_LARGURA; x++)
        {
            if (mapa[y][x] == '#')
            {
                rectfill(
                    tela,
                    inicioX + x * tamanho,
                    inicioY + y * tamanho,
                    inicioX + x * tamanho + tamanho - 1,
                    inicioY + y * tamanho + tamanho - 1,
                    makecol(255, 255, 255)
                );
            }
        }
    }

    // Jogador
    int jogadorX =
        inicioX + (int)(jogador.x * tamanho);

    int jogadorY =
        inicioY + (int)(jogador.y * tamanho);

    circlefill(
        tela,
        jogadorX,
        jogadorY,
        3,
        makecol(0, 255, 0)
    );

    // Inimigo
    int inimigoX =
        inicioX + (int)(inimigo.x * tamanho);

    int inimigoY =
        inicioY + (int)(inimigo.y * tamanho);

    circlefill(
        tela,
        inimigoX,
        inimigoY,
        3,
        makecol(255, 0, 0)
    );
}
void texto_maquina(const char* texto, int velocidade)
{
    for (int i = 0; texto[i] != '\0'; i++)
    {
        std::cout << texto[i] << std::flush;

        clock_t inicio = clock();

        while ((clock() - inicio) * 1000 / CLOCKS_PER_SEC < velocidade)
        {
            // espera
        }
    }

    std::cout << std::endl;
}
int main()

//aqui faremos o menu, optei por ser no terminal mesmo, daria mais charme
{	int opcao;
	system("color 4");
	texto_maquina("_______________ola, tem alguem ai?________________", 100);
	texto_maquina("_______________me sinto tao sozinha...__________", 100);
	texto_maquina("_______________voce nao me entende_______________", 100);

	std::cout<<"pressione enter para continuar"<<std::endl;

	std::cin.get();
	std::cout <<std::endl;
    std::cout << "==============================" << std::endl ;
    std::cout << "          FREE_ME             " << std::endl ;
    std::cout << "==============================V.1.5" << std::endl;
    std::cout<<"DESENVOLVIDO POR JHONN VYCTOR LIMA ALVES,AKA MrEupH0RI4 AKA SucodeFruta" <<std::endl;
    std::cout << std::endl;
    std::cout << "1 - JOGAR" << std::endl;
    std::cout << "2 - SAIR" <<std:: endl;
    std::cout << std::endl;
    std::cout << "Escolha: ";

    std::cin >> opcao;

    if (opcao == 2)
    {
        return 0;
    }

    if (opcao != 1)
    {
        std::cout << "Opcao invalida!" << std::endl;
        return 0;
    }

    allegro_init();
    install_keyboard();
	install_mouse();
    set_color_depth(32);
    install_sound(
		DIGI_AUTODETECT,
		MIDI_AUTODETECT,
		NULL);

    set_gfx_mode(
        GFX_AUTODETECT_WINDOWED,
        LARGURA,
        ALTURA,
        0,
        0
    );
	set_window_title("free_me");
	BITMAP *tela = create_bitmap(LARGURA, ALTURA);

	BITMAP *spriteInimigo =
    load_bitmap("MS_CHAOS_Allegro4.bmp", NULL);

	BITMAP *texturaPedra =
    load_bitmap("pedra_64x64.bmp", NULL);
//TÁ ISSO FOI DE FUDER
BITMAP *texturaChao =
    load_bitmap("chao_pedra.bmp", NULL);

if (texturaPedra == NULL)
{
    allegro_message("ERRO: textura da parede nao foi carregada!");
    destroy_bitmap(tela);
    return 1;
}

if (texturaChao == NULL)
{
    allegro_message("ERRO: textura do chao nao foi carregada!");
    destroy_bitmap(texturaPedra);
    destroy_bitmap(tela);
    return 1;
}
	somFundo = load_sample("amaze.wav");
	if (somFundo == NULL){
		allegro_message("ERRO NO AUDIO");
		destroy_bitmap(tela);
		return 1;}
	play_sample(
	somFundo,
	180,
	128,
	1000,
	TRUE);
	if (spriteInimigo == NULL)
{
    allegro_message("ERRO: sprite nao foi carregado!");
    return 1;
}

    bool rodando = true;
	int centroX = LARGURA /2;
	int centroY = ALTURA /2;
	position_mouse(centroX, centroY);
	int fafas = 0;
    while (rodando)
    	
    {	decidir_comportamento();
   		atualizar_velocidade_inimigo();
		mover_inimigo();
    	if (inimigo_encostou())
    	
    	//consequencias quando a senhorita caos te pegar
	{
	    allegro_message("VOCE FOI PEGO!");
	    system("start https://www.bing.com/images/search?q=creep%20eye&qs=HS&form=QBIRMH&sp=1&lq=0&pq=cree&sc=10-4&cvid=919004A770FC4517A229F92CAF5232D0&first=1");
	    rodando = false;
	}
        // =========================
        // SAIR
        // =========================
		int diferencaMouse = mouse_x - centroX;
		float sensibilidade = 0.003f;
		jogador.angulo +=
		    diferencaMouse * sensibilidade;
		position_mouse(
			centroX,
			centroY
			);
        if (key[KEY_ESC])
            rodando = false;

		
        // =========================
        // MOVIMENTAÇÃO
        // =========================

        float velocidade = 0.05f;

        float frenteX = cos(jogador.angulo);
        float frenteY = sin(jogador.angulo);

        float novoX = jogador.x;
        float novoY = jogador.y;
	
        if (key[KEY_W])
        {
            novoX += frenteX * velocidade;
            novoY += frenteY * velocidade;
        }

        if (key[KEY_S])
        {
            novoX -= frenteX * velocidade;
            novoY -= frenteY * velocidade;
        }

        if (key[KEY_A])
        {
            novoX += frenteY * velocidade;
            novoY -= frenteX * velocidade;
        }

        if (key[KEY_D])
        {
            novoX -= frenteY * velocidade;
            novoY += frenteX * velocidade;
        }
        // Colisão
        if (!parede(novoX, jogador.y))
            jogador.x = novoX;

        if (!parede(jogador.x, novoY))
            jogador.y = novoY;


        // =========================
        // ROTAÇÃO
        // =========================

        if (key[KEY_LEFT])
            jogador.angulo -= 0.04f;

        if (key[KEY_RIGHT])
            jogador.angulo += 0.04f;


        // =========================
        // DESENHAR
        // =========================

        clear_to_color(
            tela,
            makecol(0, 0, 0)
        );
		int tempo = 10;
        desenhar_3d(tela,texturaPedra,texturaChao);
        desenhar_inimigo(tela, spriteInimigo);
        desenhar_aviso(tela);
        
    	desenhar_vhs(tela);
		efeito_vhs(tela);
		fafas ++;
		if(fafas <=50){
			desenhar_comandos(tela);
		}
		
    
        if(key[KEY_M] && tempo >=1){
        	desenhar_minimapa(tela);
        	tempo -=1;

		}


        // Mira
        line(
            tela,
            LARGURA / 2 - 5,
            ALTURA / 2,
            LARGURA / 2 + 5,
            ALTURA / 2,
            makecol(255, 255, 255)
        );

        line(
            tela,
            LARGURA / 2,
            ALTURA / 2 - 5,
            LARGURA / 2,
            ALTURA / 2 + 5,
            makecol(255, 255, 255)
        );


        // Mostrar na tela
        blit(
            tela,
            screen,
            0,
            0,
            0,
            0,
            LARGURA,
            ALTURA
        );

        rest(10);
    }
	stop_sample(somFundo);
	destroy_sample(somFundo);
	destroy_bitmap(texturaPedra);

	destroy_bitmap(texturaPedra);
	destroy_bitmap(spriteInimigo);
    destroy_bitmap(tela);


    return 0;
}

END_OF_MAIN()
