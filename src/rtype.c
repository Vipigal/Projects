#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define NUM_INIMIGOS 13
#define NAVE_FRAMES 8
#define NUM_ESTRELAS 300

//===========================================ESTRUTURAS==========================================
typedef struct Ponto{
	float x,y;
}Ponto;

typedef struct Nave{
	float x, y;
	float speed;
	int dir_y,dir_x;
	ALLEGRO_COLOR cor;
	int points;
	ALLEGRO_BITMAP *spritesheet;
	ALLEGRO_BITMAP *sprite;
}Nave;

typedef struct Bloco{
	int x,y;
	int speed;
	int altura,comprimento;
	ALLEGRO_COLOR cor;

}Bloco;

typedef struct Tiro{
	Ponto centro;
	int speed, raioInicial;
	float raio;
	int status; //0=morto/1=vivo
	int estado; //0=pronto pra lancar/1=carregando/2=em trajetoria
	int potencia; //0=normal/1=carregado
	ALLEGRO_COLOR cor;
}Tiro;

typedef struct Hitbox{
	Ponto TL;
	Ponto BR;
	//caso retangulo:
	int altura,comprimento;
	//caso circulo:
	int raio;
	Ponto centro;

}Hitbox;

typedef struct Asteroide{
	Ponto centro;
	int tipo,raio,speed;
	int status;//1=vivo,0=morto
	ALLEGRO_COLOR cor;

}Asteroide;

typedef struct Estrela{
	Ponto centro;
	float speed;

}Estrela;

//===========================================CONSTANTES GLOBAIS==========================================

const float FPS = 120;

const int SCREEN_W = 960;
const int SCREEN_H = 540;

const int NAVE_W=96;//100
const int NAVE_H=80;//80

ALLEGRO_COLOR cor_cenario;
ALLEGRO_COLOR cor_tiro;

FILE *score;

ALLEGRO_BITMAP* nave_sprites[NAVE_FRAMES];

Estrela estrelas[NUM_ESTRELAS];

bool new_record=false;
int recorde=0;


//===========================================DECLARACOES DE FUNCOES==========================================

//COLISOES-------------------------------------------------------------------

int colisaoDoisRetangulos(Hitbox h1, Hitbox h2){
	if(h1.TL.x<=h2.BR.x&&h1.BR.x>=h2.TL.x&&h1.TL.y<=h2.BR.y&&h1.BR.y>=h2.TL.y)
		return 1;
	return 0;
}

//SPRITES-------------------------------------------------------------------

ALLEGRO_BITMAP* carregaNaveScaled(int w, int h, int frame){

	 ALLEGRO_BITMAP *resized_bmp, *loaded_bmp, *prev_target;

	  // 1. cria um bitmap temporario do tamanho da nave
	  resized_bmp = al_create_bitmap(w, h);
	  if (!resized_bmp) return NULL;

	  // 2. carrega o bitmap no seu tamanho original
	  if(frame>NAVE_FRAMES)
	  	return NULL;
	  loaded_bmp = nave_sprites[frame];
	  if (!loaded_bmp)
	  {
	     al_destroy_bitmap(resized_bmp);
	     return NULL;
	  }

	  // 3. salva o bitmap em display atual transforma o bitmap alvo no bitmap escalado
	  prev_target = al_get_target_bitmap();
	  al_set_target_bitmap(resized_bmp);

	  // 4. copia o bitmap original para o bitmap escalado
	  al_draw_scaled_bitmap(loaded_bmp,
	     0, 0,                                // source origin
	     al_get_bitmap_width(loaded_bmp),     // source width
	     al_get_bitmap_height(loaded_bmp),    // source height
	     0, 0,                                // target origin
	     w, h,                                // target dimensions
	     0                                    // flags
	  );

	  // 5. volta pro bitmap alvo anterior 
	  al_set_target_bitmap(prev_target);

	  return resized_bmp;      
}

//GERAIS-------------------------------------------------------------------

void iniciaGlobais(){
	cor_cenario = al_map_rgb(30,30,30);
	cor_tiro = al_map_rgb(255,50,0);
}

void atualizaCenario(){
	al_clear_to_color(cor_cenario);
}

float rand_float(float min, float max){
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}


//HITBOXES PROCEDIMENTOS:
void destroiHitbox(Hitbox *h){
	h->TL.x=0;
	h->TL.y=0;
	h->BR.x=0;
	h->BR.y=0;
}

void desenhaHitbox(Hitbox h){
	al_draw_rectangle(h.TL.x,h.TL.y,h.BR.x,h.BR.y,al_map_rgb(255,255,255),1);
}


//PROCEDIMENTOS NAVE-------------------------------------------------------------------
void iniciaNave(Nave *nave){
	int sX=0,sY=0;
	int sW=32,sH=32;
	nave->speed=2.3;
	nave->x=10+NAVE_W;
	nave->y=SCREEN_H/2;
	nave->dir_y=0;
	nave->dir_x=0;
	nave->cor=al_map_rgb(0,255,0);
	nave->points=0;

	//corta a spritesheet nos bitmaps das naves
	nave->spritesheet=al_load_bitmap("spritesheet_nave.png");
	for(int i=0;i<NAVE_FRAMES;i++){
		nave_sprites[i]=al_create_sub_bitmap(nave->spritesheet,sX,sY,sW,sH);
		sX+=sW;
	}
}

void criaHitboxNave(Nave nave, Hitbox *Hnave){
	Hnave->comprimento=NAVE_W;
	Hnave->altura=NAVE_H;
	Hnave->TL.x=nave.x;
	Hnave->TL.y=nave.y+17;//+17 para compensar o tamanho do bitmap
	Hnave->BR.x=nave.x+NAVE_W;
	Hnave->BR.y=nave.y+NAVE_H-17;
}

void desenhaNave(Nave nave, ALLEGRO_TIMER *timer_nave){
	
	int frame=al_get_timer_count(timer_nave) % 5;

	switch(frame){
		case 0:
			nave.sprite=carregaNaveScaled(NAVE_W,NAVE_H,0);
			al_draw_bitmap(nave.sprite,nave.x,nave.y,0);
			break;
		case 1:
			nave.sprite=carregaNaveScaled(NAVE_W,NAVE_H,1);
			al_draw_bitmap(nave.sprite,nave.x,nave.y,0);
			break;
		case 2:
			nave.sprite=carregaNaveScaled(NAVE_W,NAVE_H,2);
			al_draw_bitmap(nave.sprite,nave.x,nave.y,0);
			break;
		case 3:
			nave.sprite=carregaNaveScaled(NAVE_W,NAVE_H,3);
			al_draw_bitmap(nave.sprite,nave.x,nave.y,0);
			break;
		case 4:
			nave.sprite=carregaNaveScaled(NAVE_W,NAVE_H,4);
			al_draw_bitmap(nave.sprite,nave.x,nave.y,0);
			break;
	}
}

//atualiza a nave e checa constantemente se ela nao ultrapassou os limites
void atualizaNave(Nave *nave, Hitbox *Hnave){

	//se a nave estiver encostada em um canto ela nao se move!!
	//EIXO X:

	if(nave->x>0&&nave->x+NAVE_W<SCREEN_W){//(nave->x-NAVE_W>0&&nave->x<SCREEN_W)
		nave->x+=nave->dir_x*nave->speed;
		Hnave->BR.x+=nave->dir_x*nave->speed;
		Hnave->TL.x+=nave->dir_x*nave->speed;
	}
	else if(nave->x<=0){//caso tenha ultrapassado para tras, teleportar 1px pra frente
		nave->x++;
		Hnave->BR.x++;
		Hnave->TL.x++;
	}
	else if(nave->x+NAVE_W>=SCREEN_W){//caso tenha ultrapassado para frente, teleportar 1px pra tras
		nave->x--;
		Hnave->BR.x--;
		Hnave->TL.x--;
	}

	//EIXO Y:

	if(nave->y+17>0&&nave->y+NAVE_H-17<SCREEN_H){//17=ajuste de hitbox do bitmap
		nave->y+=nave->dir_y*nave->speed;
		Hnave->BR.y+=nave->dir_y*nave->speed;
		Hnave->TL.y+=nave->dir_y*nave->speed;
	}
	else if(nave->y+17<=0){//caso tenha ultrapassado para cima, teleportar 1px pra baixo
		nave->y++;
		Hnave->BR.y++;
		Hnave->TL.y++;
	}
	else if(nave->y+NAVE_H-17>=SCREEN_H){//caso tenha ultrapassado para baixo, teleportar 1px pra cima
		nave->y--;
		Hnave->BR.y--;
		Hnave->TL.y--;
	}
	
}

//PROCEDIMENTOS BLOCO-------------------------------------------------------------------
void iniciaBloco(Bloco *bloco){
	bloco->speed=2;
	bloco->altura=SCREEN_H/(rand()%4+2);
	bloco->comprimento=SCREEN_W*(rand()%2+1);
	bloco->cor=al_map_rgb(162,145,105);
	bloco->x=SCREEN_W+rand()%(SCREEN_W);
	bloco->y=SCREEN_H-bloco->altura;
}

void criaHitboxBloco(Bloco *bloco, Hitbox *Hbloco){
	Hbloco->comprimento=bloco->comprimento;
	Hbloco->altura=bloco->altura;
	Hbloco->TL.x=bloco->x;
	Hbloco->TL.y=bloco->y;
	Hbloco->BR.x=bloco->x+bloco->comprimento;
	Hbloco->BR.y=bloco->y+bloco->altura;
}

void desenhaBloco(Bloco bloco){
	al_draw_filled_rectangle(bloco.x,bloco.y,bloco.x+bloco.comprimento,bloco.y+bloco.altura,bloco.cor);

}

void atualizaBloco(Bloco *bloco,Hitbox *Hbloco){
	bloco->x-=bloco->speed;
	Hbloco->TL.x-=bloco->speed;
	Hbloco->BR.x-=bloco->speed;
	if(bloco->x+bloco->comprimento<0){
		iniciaBloco(bloco);
		criaHitboxBloco(bloco,Hbloco);
	}
}

//PROCEDIMENTOS ASTEROIDES-------------------------------------------------------------------
void iniciaAsteroide(Asteroide *ast){
	ast->centro.x=SCREEN_W+rand()%30;
	ast->centro.y=20+rand()%480;
	ast->raio=10+rand()%40;
	ast->cor=al_map_rgb(50+rand()/110,50+rand()/110,50+rand()/110);
	ast->speed=1+rand()%4;
	ast->status=0;//comeca morto

}

void criaHitboxAsteroide(Asteroide *ast, Hitbox *Hast){
	Hast->TL.x=ast->centro.x-ast->raio;
	Hast->TL.y=ast->centro.y-ast->raio;
	Hast->BR.x=ast->centro.x+ast->raio;
	Hast->BR.y=ast->centro.y+ast->raio;
}

void desenhaAsteroide(Asteroide ast){
	al_draw_filled_circle(ast.centro.x,ast.centro.y,ast.raio,ast.cor);
}


void atualizaAsteroide(Asteroide *ast, Hitbox *Hast){
	ast->centro.x-=ast->speed;
	Hast->TL.x-=ast->speed;
	Hast->BR.x-=ast->speed;
	if(Hast->BR.x<0){//se sair da tela na esquerda reiniciar ele
		iniciaAsteroide(ast);
		criaHitboxAsteroide(ast,Hast);
	}
}

//PROCEDIMENTOS TIRO-----------------------------------------------------------------------------
void iniciaTiro(Nave nave, Tiro *tiro, Hitbox *Htiro){
	tiro->raioInicial=3;
	tiro->centro.x=nave.x+NAVE_W+tiro->raioInicial;//distancia para nao aparecer colado na nave
	tiro->centro.y=nave.y+(NAVE_H/2);
	tiro->raio=tiro->raioInicial;
	tiro->cor=cor_tiro;
	tiro->speed=5;
	tiro->status=1;
	tiro->potencia=0;
	
}

//cria hitbox de um tiro circular
void criaHitboxTiro(Tiro *tiro, Hitbox *Htiro){

	Htiro->altura=2*tiro->raio;
	Htiro->comprimento=2*tiro->raio;
	Htiro->TL.x=tiro->centro.x-tiro->raio;
	Htiro->TL.y=tiro->centro.y-tiro->raio;
	Htiro->BR.x=tiro->centro.x+tiro->raio;
	Htiro->BR.y=tiro->centro.y+tiro->raio;
}

//carrega o tamanho do tiro
void carregaTiro(Nave nave, Tiro *tiro, Hitbox *Htiro){

	tiro->centro.x=nave.x+NAVE_W+tiro->raio+2;//distancia para nao aparecer colado na nave
	tiro->centro.y=nave.y+(NAVE_W/2)-5;

	if(tiro->raio<20){
		tiro->raio+=0.5;//aumenta o tiro em uma vel 0.5
	}

	if(tiro->raio==20){
		tiro->potencia=1;
		tiro->cor=al_map_rgb(255,200,0);
	}
	else{
		tiro->potencia=0;
		tiro->cor=cor_tiro;
	}

	al_draw_filled_circle(tiro->centro.x,tiro->centro.y,tiro->raio,tiro->cor);

}

//movimenta o tiro
void atualizaTiro(Tiro *tiro, Hitbox *Htiro){
	criaHitboxTiro(tiro,Htiro);

	tiro->centro.x+=tiro->speed;
	Htiro->TL.x+=tiro->speed;
	Htiro->BR.x+=tiro->speed;
	if(tiro->centro.x>SCREEN_W){
		tiro->estado=0;
		tiro->status=0;
		tiro->raio=tiro->raioInicial;
		destroiHitbox(Htiro);
	}
}

//desenha o tiro em movimento
void soltaTiro(Tiro *tiro){
	al_draw_filled_circle(tiro->centro.x,tiro->centro.y,tiro->raio,tiro->cor);
	
}

void logicaTiro(Nave nave, Tiro *tiro, Hitbox *Htiro, Hitbox Hbloco){
	//logica do tiro, se o estado == 1, o tiro esta sendo carregado. Se estado == 2, esta em trajetoria.
	if(tiro->estado==1){
		if(tiro->status==0)iniciaTiro(nave,tiro,Htiro);
		carregaTiro(nave,tiro,Htiro);
	}

	else if (tiro->estado==2){
		atualizaTiro(tiro,Htiro);
		soltaTiro(tiro);
		if(colisaoDoisRetangulos(*Htiro,Hbloco)==1){
			tiro->estado=0;
			tiro->status=0;
			destroiHitbox(Htiro);
			tiro->raio=tiro->raioInicial;
		}				
	}
}


//PROCEDIMENTOS ESTRELAS:
void iniciaEstrelas(){
	int x=1;
	for(int i=0;i<NUM_ESTRELAS;i++){
		estrelas[i].centro.y=rand_float(1,SCREEN_H);
		estrelas[i].centro.x=x;
		estrelas[i].speed=rand_float(0.1,1);
		x+=3;
	}
}

void desenhaEstrela(){
	for(int i=0;i<NUM_ESTRELAS;i++){
		float l = estrelas[i].speed * 0.8;
		al_draw_pixel(estrelas[i].centro.x,estrelas[i].centro.y,al_map_rgb_f(l,l,l));
	}
}

void atualizaEstrela(){
	for(int i=0;i<NUM_ESTRELAS;i++){
		estrelas[i].centro.x-=estrelas[i].speed;
		if(estrelas[i].centro.x<=0){
			estrelas[i].centro.x=SCREEN_W;
			estrelas[i].speed=rand_float(0.1,1);
		}
	}
}




//===========================================COMECO DA MAIN==========================================

int main(int argc, char **argv){

	srand(time(NULL));
	
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_DISPLAY *display = NULL;

   
	//----------------------- rotinas de inicializacao ---------------------------------------
    
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
   
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//ANTI ALIASING - APENAS USAR COM FORMAS PRIMITIVAS E CUIDADO COM IMAGENS TAMANHO MENORES
	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);



	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	al_set_window_title(display, "R-TYPE");

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	
	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("r-type.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}

	
	//----------------------FILA DE EVENTOS-----------------------------------------------------

	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	//=======================FIM DA FILA DE EVENTOS==============================================


	//----------------------INICIALIZACAO JOGO---------------------------------------------------------
	init: //label para goto do fim do jogo
	
	iniciaGlobais();
	int estadoEspaco=0;


	//INICIALIZACAO NAVE:
	Nave nave; //cria uma struct nave
	Hitbox Hnave; //cria uma hitbox da nave

	ALLEGRO_TIMER *timer_nave; //cria o timer da nave para ciclar os sprites
	timer_nave = al_create_timer(0.01);
    if(!timer_nave) {
		fprintf(stderr, "failed to create timer_nave!\n");
		return -1;
	}

	iniciaNave(&nave);
	criaHitboxNave(nave,&Hnave);

	//INICIALIZACAO BLOCO
	Bloco bloco; //cria um struct bloco
	Hitbox Hbloco; //cria uma hitbox do bloco
	iniciaBloco(&bloco);
	criaHitboxBloco(&bloco,&Hbloco);

	//INICIALIZACAO TIRO
	Tiro tiro; //cria um struct tiro
	Hitbox Htiro; //cria uma hitbox do tiro
	destroiHitbox(&Htiro);

	tiro.estado=0;//o tiro comeca no estado inicial (inativo)
	tiro.status=0;//o tiro comeca no status inicial (nao carregado)

	//INICIALIZACAO ASTEROIDES
	Asteroide asteroides[NUM_INIMIGOS];
	Hitbox asteroide_hitboxes[NUM_INIMIGOS];
	for(int i=0;i<NUM_INIMIGOS;i++){
		iniciaAsteroide(&asteroides[i]);
		criaHitboxAsteroide(&asteroides[i],&asteroide_hitboxes[i]);
	}

	//INICIALIZACAO ESTRELAS
	
	iniciaEstrelas();

	//inicia o timer
	al_start_timer(timer);
	al_start_timer(timer_nave);
	
	//===========================================LOOP PRINCIPAL==========================================
	bool playing = true;
	bool game_over = false;
	while(playing) {

		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//revive todos os asteroides
		for(int i=0;i<NUM_INIMIGOS;i++){
			if(asteroides[i].status==0)asteroides[i].status=1;		
		}

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			atualizaCenario();

			atualizaNave(&nave,&Hnave);
			atualizaBloco(&bloco,&Hbloco);

			atualizaEstrela();
			desenhaEstrela();

			//colisao entre bloco e nave
			if(colisaoDoisRetangulos(Hnave, Hbloco)){
				playing=false;
				game_over=true;
			}

			logicaTiro(nave, &tiro, &Htiro, Hbloco);

			//asteroides funcionalidades
			for(int i=0;i<NUM_INIMIGOS;i++){
				atualizaAsteroide(&asteroides[i],&asteroide_hitboxes[i]);

				//colisao entre os asteroides e o bloco
				if(colisaoDoisRetangulos(asteroide_hitboxes[i],Hbloco)){
					asteroides[i].status=0;
					iniciaAsteroide(&asteroides[i]);
					criaHitboxAsteroide(&asteroides[i],&asteroide_hitboxes[i]);
				}

				//colisao entre dois asteroides diferentes
				for(int j=i+1;j<NUM_INIMIGOS;j++){
					if(colisaoDoisRetangulos(asteroide_hitboxes[i],asteroide_hitboxes[j])){
						asteroides[i].status=0;
						iniciaAsteroide(&asteroides[i]);
						criaHitboxAsteroide(&asteroides[i],&asteroide_hitboxes[i]);
					}
				}

				//colisao entre a nave e o asteroide
				if(colisaoDoisRetangulos(Hnave,asteroide_hitboxes[i])){
					playing=false;
					game_over=true;
				}


				//colisao entre o asteroide e o tiro
				if(colisaoDoisRetangulos(Htiro,asteroide_hitboxes[i])==1){
					asteroides[i].status=0;
					iniciaAsteroide(&asteroides[i]);
					criaHitboxAsteroide(&asteroides[i],&asteroide_hitboxes[i]);
					destroiHitbox(&Htiro);
					
					nave.points+=asteroides[i].raio;
					if(tiro.potencia==0){
						tiro.status=0;
						tiro.estado=0;
						destroiHitbox(&Htiro);
						tiro.raio=tiro.raioInicial;
					}
				}

				//se o asteroide estiver vivo, desenha-lo
				if(asteroides[i].status!=0){
					desenhaAsteroide(asteroides[i]);
				}
			}

			//desenha nave e bloco
			desenhaBloco(bloco);
			desenhaNave(nave,timer_nave);

			
			if(tiro.estado==0)
				al_draw_text(size_32, al_map_rgb(0,255,255), SCREEN_W/2, SCREEN_H-60, ALLEGRO_ALIGN_CENTER, "READY");
			else 
				al_draw_text(size_32, al_map_rgb(0,255,255), SCREEN_W/2,SCREEN_H-60, ALLEGRO_ALIGN_CENTER, "NOT READY");


			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();

			
			if(al_get_timer_count(timer)%(int)FPS == 0)
				printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));

		}


		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = false;
		}
	
		//===========================================MOVIMENTACAO DA NAVE==========================================
		
		//KEY DOWN DA TECLA
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch(ev.keyboard.keycode){
				case ALLEGRO_KEY_W:
					nave.dir_y--;
				break;
				case ALLEGRO_KEY_S:
					nave.dir_y++;
				break;
				case ALLEGRO_KEY_A:
					nave.dir_x--;
				break;
				case ALLEGRO_KEY_D:
					nave.dir_x++;
				break;
			}
			
			if(ev.keyboard.keycode==ALLEGRO_KEY_SPACE&&tiro.estado==0){
				tiro.estado=1;
				//adicionar contador para checar se o espaco foi apertado, assim so o down do teclado pode mudar o estado do tiro
				estadoEspaco=1;	
			}
		}
		//KEY UP DA TECLA
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			switch(ev.keyboard.keycode){
				case ALLEGRO_KEY_W:
					nave.dir_y++;
				break;
				case ALLEGRO_KEY_S:
					nave.dir_y--;
				break;
				case ALLEGRO_KEY_A:
					nave.dir_x++;
				break;
				case ALLEGRO_KEY_D:
					nave.dir_x--;
				break;
			}

			if(ev.keyboard.keycode==ALLEGRO_KEY_SPACE&&estadoEspaco==1){
				tiro.estado=2;
				estadoEspaco=0;
			}
		}


		//===========================================RECORDE E PONTUACAO==========================================
		if(playing==false){
			score=fopen("recorde.txt","r");
			fscanf(score,"%d",&recorde);
			fclose(score);

			if(nave.points>recorde){
		    	score=fopen("recorde.txt","w");
		    	fprintf(score, "%d", nave.points);
		    	fclose(score);
		 		recorde=nave.points;
		 		new_record=true;
		 	}


		 	al_clear_to_color(al_map_rgb(0,0,0));
	
		 	al_draw_text(size_32, al_map_rgb(255,0,0), SCREEN_W/2, 60, ALLEGRO_ALIGN_CENTER, "GAME OVER");
		 	al_draw_textf(size_32, al_map_rgb(0,155,255), SCREEN_W/2, SCREEN_H-80, ALLEGRO_ALIGN_CENTER, "recorde; %d",recorde);
		 	al_draw_textf(size_32, al_map_rgb(0,155,255), SCREEN_W/2, SCREEN_H-180, ALLEGRO_ALIGN_CENTER, "pontuacao; %d",nave.points);//pontuacao_str);
		 	if(new_record){
		 		al_draw_text(size_32, al_map_rgb(0,255,0), SCREEN_W-700, SCREEN_H-300, 0, "NOVO RECORDE!");
		 		new_record=false;
		 	}
		 	al_flip_display();
		 	al_rest(2.0);
	   }

	} //fim do while playing

	while(game_over){
		ALLEGRO_EVENT ev;

		al_clear_to_color(al_map_rgb(0,0,0));

		al_draw_text(size_32, al_map_rgb(0,255,0), SCREEN_W/2, 60, ALLEGRO_ALIGN_CENTER, "QUER JOGAR NOVAMENTE?");
		al_draw_text(size_32, al_map_rgb(0,155,255), SCREEN_W/2, SCREEN_H-80, ALLEGRO_ALIGN_CENTER, "APERTE ENTER PARA SIM");
		al_draw_text(size_32, al_map_rgb(0,155,255), SCREEN_W/2, SCREEN_H-180, ALLEGRO_ALIGN_CENTER, "APERTE ESC PARA NAO");
		al_flip_display();
		
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
		 	switch(ev.keyboard.keycode){
		 		case ALLEGRO_KEY_ENTER:
		 			playing=true;
		 			goto init;
		 		break;
		 		case ALLEGRO_KEY_ESCAPE:
		 			game_over=false;
		 		break;
		 	}
		}

		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
		}

	}//fim do while game over

	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
	al_destroy_timer(timer);
	al_destroy_timer(timer_nave);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
   
	return 0;
}