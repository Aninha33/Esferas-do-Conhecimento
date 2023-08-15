// Os arquivos de cabeçalho
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
//exibição ao estilo de janela do SO
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
// Inclui o cabeçalho do add-on para uso de imagens
#include <allegro5/allegro_image.h>
// Para utilizarmos a função fprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Atributos da tela
#define FPS 60.0
#define LARGURA_TELA 1243
#define ALTURA_TELA 700

ALLEGRO_DISPLAY *janela;
ALLEGRO_AUDIO_STREAM *musica ;
int audio_on=0;
float volume = 1;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *folha_sprite = NULL;
ALLEGRO_BITMAP *folha_sprite2 = NULL;
ALLEGRO_BITMAP *folha_sprite3 = NULL;
ALLEGRO_BITMAP *folha_sprite4 = NULL;
ALLEGRO_BITMAP *fundo = NULL;
ALLEGRO_BITMAP *fundofim = NULL;
ALLEGRO_COLOR cor_tempo;
ALLEGRO_FONT *fonte = NULL;
ALLEGRO_EVENT evento;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
//veis para contar o tempo
void SalvarArquivo();

int segundos=0;
int minutos=3;
int milisegundos=60;

//responsaveis pelo ranking
char str[20];
bool concluido = false;

//diferenciar o SO para limpar a tela
#ifdef __unix__
#include <unistd.h>
#include <stdio_ext.h>

void limparBuffer() {
  __fpurge(stdin);
}

#elif defined(_WIN32) || defined(WIN32)
#define OS_Windows
#include <windows.h>

void limparBuffer(){
  fflush(stdin);
}
#endif

typedef struct{
  int altura_sprite, largura_sprite;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  int colunas_folha, coluna_atual;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  int linha_atual, linhas_folha;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  int regiao_x_folha, regiao_y_folha;
  //quantos frames devem se passar para atualizar para o proximo sprite
  int frames_sprite, cont_frames;
  //posicao X Y da janela em que sera mostrado o sprite
  int pos_x_sprite, pos_y_sprite;
  //velocidade X Y que o sprite ira se mover pela janela
  int vel_x_sprite, vel_y_sprite;
  //colunas e linhas na sprites final e inicial
  int sprite_final, sprite_inicial;
}Personagem;

typedef struct{
  char nome[20];
  int segundos;
  int minutos;
}Ranking;

Ranking listaRanking[3];



//funcao para mensagem de erro
void mensagem_erro(char *text){
  if (janela)
  al_show_native_message_box(janela,"ERRO",
  "Ocorreu o seguinte erro e o programa sera finalizado:",
  text,NULL,ALLEGRO_MESSAGEBOX_ERROR);
  else
  al_show_native_message_box(NULL,"ERRO",
  "Ocorreu o seguinte erro e o programa sera finalizado:",
  text,NULL,ALLEGRO_MESSAGEBOX_ERROR);
}//mensagem_erro

void SalvarArquivo(){
  FILE *arqRanking;
  arqRanking = fopen("ranking.txt", "r+b");
  int gravarMinutos = 2-minutos;
  int gravarSegundos = 59-segundos;

  fread(&listaRanking, sizeof(Ranking), 3, arqRanking);

  if((gravarMinutos <= listaRanking[2].minutos) && (gravarSegundos < listaRanking[2].segundos)){
    strcpy(listaRanking[2].nome, str);

    listaRanking[2].minutos = gravarMinutos;
    listaRanking[2].segundos = gravarSegundos;
  }//if
  if((gravarMinutos <= listaRanking[1].minutos) && (gravarSegundos < listaRanking[1].segundos)){
    listaRanking[2].minutos = listaRanking[1].minutos;
    listaRanking[2].segundos = listaRanking[1].segundos;
    strcpy(listaRanking[2].nome, listaRanking[1].nome);

    listaRanking[1].minutos = gravarMinutos;
    listaRanking[1].segundos = gravarSegundos;
    strcpy(listaRanking[1].nome, str);
  }//if
  if((gravarMinutos <= listaRanking[0].minutos) && (gravarSegundos < listaRanking[0].segundos)){
    listaRanking[1].minutos = listaRanking[0].minutos;
    listaRanking[1].segundos = listaRanking[0].segundos;
    strcpy(listaRanking[1].nome, listaRanking[0].nome);

    listaRanking[0].minutos = gravarMinutos;
    listaRanking[0].segundos = gravarSegundos;
    strcpy(listaRanking[0].nome, str);
  }//if

  fclose(arqRanking);
  arqRanking = fopen("ranking.txt", "r+b");
  fwrite(&listaRanking, sizeof(Ranking), 3, arqRanking);
  fclose(arqRanking);

}//funcao Salvar arquivo


void nomeRanking(ALLEGRO_EVENT evento){
  if (evento.type == ALLEGRO_EVENT_KEY_CHAR){
    if (strlen(str) <= 19){
      char temp[] = {evento.keyboard.unichar, '\0'};
      if (evento.keyboard.unichar == ' '){
        strcat(str, temp);
        limparBuffer();
      }//if
      if (evento.keyboard.unichar >= '0' && evento.keyboard.unichar <= '9'){
        strcat(str, temp);
        limparBuffer();
      }//else if
      if (evento.keyboard.unichar >= 'A' && evento.keyboard.unichar <= 'Z'){
        strcat(str, temp);
        limparBuffer();
      }//else if
      if (evento.keyboard.unichar >= 'a' && evento.keyboard.unichar <= 'z'){
        strcat(str, temp);
        limparBuffer();
      }//else if
    }//if
    if (evento.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && strlen(str) != 0){
      str[strlen(str) - 1] = '\0';
      limparBuffer();
    }//if
    if(evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
      // FAZER A IMPLEMENTAÇÃO PARA SALVAR OS VALORES INFORMADOS NO ARQUIVO
      SalvarArquivo();
      concluido = 1;
    }//if
  }//if
}//nomeRanking

int inicializaBiblioteca(){
  // Inicializa a Allegro
  al_init();

  // Inicialização da biblioteca Allegro
  if (!al_init()) {
    mensagem_erro( "Falha ao inicializar a Allegro.\n");
    return 0;
  }//if al_init

  // Inicializa o add-on para utilização de imagens
  al_init_image_addon();

  if (!al_init_image_addon()) {
    mensagem_erro( "Falha ao inicializar o add-on de imagens da Allegro.\n");
    return 0;
  }//if al_init_image_addon

  // Inicialização do add-on para uso de fontes
  al_init_font_addon();

  strcpy(str, "");

  if (!al_install_keyboard()){
    mensagem_erro("Falha ao inicializar o teclado");
    return 0;
  }//if

  // Inicialização do add-on para uso de fontes True Type
  if (!al_init_ttf_addon()) {
    mensagem_erro("Falha ao inicializar add-on allegro_ttf.\n");
    return 0;
  }//if al_init_ttf_addon

  //addon que da suporte as extensoes de audio
  if(!al_init_acodec_addon()){
    mensagem_erro("Falha ao inicializar o codec de audio");
    return 0;
  }//if al_init_acodec_addon
  //addon de audio
  if(!al_install_audio()){
    mensagem_erro("Falha ao inicializar o audio");
    return 0;
  }//if al_install_audio

  //cria o mixer (e torna ele o mixer padrao), e adciona 5 samples de audio nele
  if (!al_reserve_samples(1)){
    mensagem_erro("Falha ao reservar amostrar de audio");
    return 0;
  }//if al_reserve_samples

  janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
  if (!janela){//Se não criar a janela, destroi os ponteiros e retorna 0
    mensagem_erro( "Falha ao criar janela.\n");
    al_destroy_display(janela);
    return 0;
  }//if janela
  //criando temporizador
  timer = al_create_timer(0.9 / 60);
  if(!timer) {
    mensagem_erro("Falha ao criar temporizador");
    return 0;
  }//if

  // Configura o título da janela
  al_set_window_title(janela, "Jogo");

  //eventos
  fila_eventos = al_create_event_queue();
  if(!fila_eventos) {
    mensagem_erro("Falha ao criar fila de eventos");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    return 0;
  }//if

  // Carregando o arquivo de fonte
  fonte = al_load_font("fonts/army.ttf", 48, 0);
  if (!fonte){
    al_destroy_display(janela);
    mensagem_erro( "Falha ao carregar fonte.\n");
    return 0;
  }//if

  //carrega a folha de sprites na variavel
  folha_sprite = al_load_bitmap("images/goku.png");
  if (!folha_sprite){
    mensagem_erro("Falha ao carregar sprites");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    return 0;
  }//if

  //usa a cor rosa como transparencia
  al_convert_mask_to_alpha(folha_sprite,al_map_rgb(255,0,255));

  //carrega a folha de sprites na variavel
  folha_sprite2 = al_load_bitmap("images/freeza.png");
  if (!folha_sprite2){
    mensagem_erro("Falha ao carregar sprites");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    return 0;
  }//if

  //usa a cor rosa como transparencia
  al_convert_mask_to_alpha(folha_sprite2,al_map_rgb(255,0,255));

  //carrega a folha de sprites na variavel
  folha_sprite3 = al_load_bitmap("images/semente1.png");
  if (!folha_sprite3){
    mensagem_erro("Falha ao carregar sprites");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    return 0;
  }//if

  //usa a cor rosa como transparencia
  al_convert_mask_to_alpha(folha_sprite3,al_map_rgb(255,0,255));

  //carrega a folha de sprites na variavel
  folha_sprite4 = al_load_bitmap("images/esferinhasarrumadas.png");
  if (!folha_sprite4){
    mensagem_erro("Falha ao carregar sprites");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    return 0;
  }//if

  //usa a cor rosa como transparencia
  al_convert_mask_to_alpha(folha_sprite4,al_map_rgb(255,0,255));

  //carrega o fundo

  /* code */
  fundo = al_load_bitmap("images/mapa.png");
  if (!fundo){
    mensagem_erro("Falha ao carregar fundo");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    al_destroy_bitmap(folha_sprite);
    al_destroy_bitmap(folha_sprite2);
    al_destroy_bitmap(folha_sprite3);
    return 0;
  }//if

  fundofim = al_load_bitmap("images/telinhaRank.png");
  if (!fundofim){
    mensagem_erro("Falha ao carregar fundo");
    al_destroy_timer(timer);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    al_destroy_bitmap(folha_sprite);
    al_destroy_bitmap(folha_sprite2);
    al_destroy_bitmap(folha_sprite3);
    return 0;
  }//if


  al_register_event_source(fila_eventos, al_get_display_event_source(janela));
  al_register_event_source(fila_eventos, al_get_keyboard_event_source());
  al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
  //al_start_timer(timer);
  al_start_timer(timer);


  return 0;
}//inicializaBiblioteca



//função destruir ponteiros
void destroiMenu(ALLEGRO_BITMAP *btn_sair,ALLEGRO_BITMAP *btn_iniciar,ALLEGRO_BITMAP *btn_tutorial,ALLEGRO_BITMAP *btn_creditos,
  ALLEGRO_BITMAP *btn_configuracao,ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem, ALLEGRO_EVENT_QUEUE *fila_eventos){

  al_destroy_bitmap(btn_sair);
  al_destroy_bitmap(btn_iniciar);
  al_destroy_bitmap(btn_tutorial);
  al_destroy_bitmap(btn_creditos);
  al_destroy_bitmap(btn_configuracao);
  al_destroy_bitmap(imagem);
  al_destroy_font(fonte);
  al_destroy_event_queue(fila_eventos);

}//destroiMenu /-/ Destroi a tela de Menu

void destroiCreditos(ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_voltar){

  al_destroy_bitmap(btn_voltar);
  al_destroy_event_queue(fila_eventos);
  al_destroy_font(fonte);
  al_destroy_bitmap(imagem);

}//destroiCreditos /-/ Destroi a tela de Créditos

void destroiTutorial(ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_voltar){

  al_destroy_bitmap(btn_voltar);
  al_destroy_event_queue(fila_eventos);
  al_destroy_font(fonte);
  al_destroy_bitmap(imagem);

}//destroiTutorial /-/ Destroi a tela de Tutorial

void destroiConfiguracoes(ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_voltar, ALLEGRO_BITMAP *btn_som){

  al_destroy_bitmap(btn_voltar);
  al_destroy_bitmap(btn_som);
  al_destroy_event_queue(fila_eventos);
  al_destroy_font(fonte);
  al_destroy_bitmap(imagem);

}//destroiConfiguracoes /-/ Destroi a tela de Configurações

void destroiJogar(ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_medio, ALLEGRO_BITMAP *btn_facil, ALLEGRO_BITMAP *btn_dificil,
ALLEGRO_BITMAP *btn_voltar){

  al_destroy_bitmap(btn_facil);
  al_destroy_bitmap(btn_medio);
  al_destroy_bitmap(btn_dificil);
  al_destroy_bitmap(btn_voltar);
  al_destroy_event_queue(fila_eventos);
  al_destroy_font(fonte);
  al_destroy_bitmap(imagem);
}//destroiJogar /-/ Destroi a tela de seleção da dificuldade

  //******************************************************** FUNÇÃO TELA MENU **********************************************************;
  //função de gerar menu
int geraMenu(ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_iniciar, ALLEGRO_BITMAP *btn_tutorial, ALLEGRO_BITMAP *btn_configuracao,
ALLEGRO_BITMAP *btn_creditos, ALLEGRO_BITMAP *btn_sair, int sair, ALLEGRO_AUDIO_STREAM *musica){
    // Atualização da janela
    janela = al_get_current_display();

    // Configura o título da janela
    al_set_window_title(janela, "Menu");

    //colocando o fundinho do menu
    imagem = al_load_bitmap("images/menu.png");//carrega a imagem do menu na variavel imagem

    fila_eventos = al_create_event_queue();//cria a fila de eventos


    if (!imagem){//se não carregar a imagem na tela destroi os ponteiros e retorna 0
      mensagem_erro( "Falha ao carregar o arquivo de imagem.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if imagens

    // Torna apto o uso de mouse na aplicação
    if (!al_install_mouse()){//se não inicializar o mouse destroi os ponteiros e retorna 0
      mensagem_erro( "Falha ao inicializar o mouse.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if mouse

    // Atribui o cursor padrão do sistema para ser usado
    if (!al_set_system_mouse_cursor(janela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)){// se não atribuir ponteiro do mouse destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao atribuir ponteiro do mouse.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if mouse cursor
    // Dizemos que vamos tratar os eventos vindos do mouse
    al_register_event_source(fila_eventos, al_get_mouse_event_source());

    // Alocamos o botão Iniciar na tela
    btn_iniciar = al_create_bitmap(300, 200);
    if (!btn_iniciar) {//se não criar o botão iniciar destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao criar bitmap.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if botao inicial

    //Tutorial
    btn_tutorial = al_create_bitmap(300, 300);
    if (!btn_tutorial) {//se não criar o botão tutorial destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao criar bitmap.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if botao de tutorial

    //configurações
    btn_configuracao = al_create_bitmap(300, 400);
    if (!btn_configuracao) {//se não criar o botão configurações destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao criar bitmap.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if botao de configurações

    //creditos
    btn_creditos = al_create_bitmap(300, 500);
    if (!btn_creditos) {//se não criar botão creditos destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao criar bitmap.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if botao creditos

    //Botão para fechar a aplicação
    btn_sair = al_create_bitmap(300, 600);
    if (!btn_sair){//se não criar botão sair destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao criar botão de saída.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if botao sair

    //eventos
    if (!fila_eventos){//se não criar a fila de eventos destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao criar fila de eventos.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if eventos

    // Carregando o arquivo de fonte
    fonte = al_load_font("fonts/army.ttf", 48, 0);//carrega a fonte
    if (!fonte){//se não carregar fonte destroi ponteiros e retorna 0
      mensagem_erro( "Falha ao carregar fonte.\n");
      destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
      al_destroy_audio_stream(musica);
      al_destroy_display(janela);
      return 0;
    }//if fonte

    // Dizemos que vamos tratar os eventos vindos do mouse
    al_register_event_source(fila_eventos, al_get_mouse_event_source());
    // Flag indicando se o mouse está sobre o retângulo central
    int na_btn_iniciar = 0;
    int na_btn_tutorial = 0;
    int na_btn_creditos = 0;
    int na_btn_configuracao = 0;
    int na_btn_ranking = 0;
    int na_btn_sair = 0;

    //al_register_event_source(fila_eventos, al_get_display_event_source(janela));//registra os eventos na janela

    //EVENTOS SOBRE BOTÃO
  while (!sair) {
    // Verificamos se há eventos na fila
    al_draw_bitmap(imagem,0,0,0);
    while (!al_is_event_queue_empty(fila_eventos)) {//enquanto a fila de eventos não estiver vazia
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);//espera pelo evento

      if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //caixa de texto para confirmar sair do jogo
        //exibe o titulo do jogo
        char tcaixa[50] = "Menu";
        //exibe se deseja sair do jogo
        char titulo[100] = "Atenção";
        //exibe mensagem de aviso
        char texto[200] = "Deseja mesmo sair?";
        //mostra a caixa de texto
        int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        // printf("%i",r);
        if(r==true){
          destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
          al_destroy_audio_stream(musica);
          al_destroy_display(janela);
          exit( 0);
        }//if
      }//else do X na tela

      // Se o evento foi de movimentação do mouse
      if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){

        // Verificamos se ele está sobre a região do botão Iniciar
        //INICIAR - BOTÃO
        if (evento.mouse.x >= 510  && //tamanho da largura da esquerda
          evento.mouse.x <= 740 && //tamanho da largura da direita
          evento.mouse.y >= 150  && //tamanho da altura de cima
          evento.mouse.y <= 200){ //tamanho da altura  de baixo

          na_btn_iniciar = 1;
          }//if
        else{
          na_btn_iniciar = 0;
        }//else

        // Verificamos se ele está sobre a região do botão Tutorial
        //TUTORIAL - BOTÃO
        if (evento.mouse.x >= 480  && //tamanho da largura da esquerda
          evento.mouse.x <= 780  && //tamanho da largura da direita
          evento.mouse.y >= 250  && //tamanho da altura de cima
          evento.mouse.y <= 300 ){ //tamanho da altura  de baixo

          na_btn_tutorial = 1;
          }//if
        else{
          na_btn_tutorial = 0;
        }//else

        //Verificamos se ele está sobre a região do botão Configurações
        //CONFIGURAÇÕES - BOTÃO
        if (evento.mouse.x >= 380  && //tamanho da largura da esquerda
          evento.mouse.x <= 860  && //tamanho da largura da direita
          evento.mouse.y >= 350 && //tamanho da altura de cima
          evento.mouse.y <= 400 ){ //tamanho da altura  de baixo

            na_btn_configuracao = 1;
          }//if
        else{
          na_btn_configuracao = 0;
        }//else

        //Verificamos se ele está sobre a região do botão Créditos
        //CRÉDITOS - BOTÃO
        if (evento.mouse.x >= 620  && //tamanho da largura da esquerda
          evento.mouse.x <= 920  && //tamanho da largura da direita
          evento.mouse.y >= 450  && //tamanho da altura de cima
          evento.mouse.y <= 500 ){ //tamanho da altura  de baixo

          na_btn_creditos = 1;
        }//if
        else{
          na_btn_creditos = 0;
        }//else

        //Verificamos se ele está sobre a região do botão Ranking
        //RANKING - BOTÃO
        if (evento.mouse.x >= 300  && //tamanho da largura da esquerda
          evento.mouse.x <= 590  && //tamanho da largura da direita
          evento.mouse.y >= 450  && //tamanho da altura de cima
          evento.mouse.y <= 500 ){ //tamanho da altura  de baixo

          na_btn_ranking = 1;
        }//if
        else{
          na_btn_ranking = 0;
        }//else

        //SAIR - BOTÃO SAIR
        if (evento.mouse.x >= 540  && //tamanho da largura da esquerda
          evento.mouse.x <= 690 && //tamanho da largura da direita
          evento.mouse.y >= 550  && //tamanho da altura de cima
          evento.mouse.y <= 600 ){ //tamanho da altura  de baixo

          na_btn_sair = 1;
        }//if
        else{
          na_btn_sair = 0;
        }//else
      }//if
      // Ou se o evento foi um clique do mouse

      else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){

        //BOTÃO INICIAR
        if (evento.mouse.x >= 510  && //tamanho da largura da esquerda
          evento.mouse.x <= 740 && //tamanho da largura da direita
          evento.mouse.y >= 150  && //tamanho da altura de cima
          evento.mouse.y <= 200){ //tamanho da altura  de baixo

          destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);//destroi ponteiros
          return 1;
        }//if

        //BOTÃO TUTORIAL
        if (evento.mouse.x >= 480  && //tamanho da largura da esquerda
          evento.mouse.x <= 780  && //tamanho da largura da direita
          evento.mouse.y >= 250  && //tamanho da altura de cima
          evento.mouse.y <= 300 ){ //tamanho da altura  de baixo

          destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);//destroi ponteiros
          return 2;
        }//if

        //BOTÃO CONFIGURAÇÕES
        if (evento.mouse.x >= 380  && //tamanho da largura da esquerda
          evento.mouse.x <= 860  && //tamanho da largura da direita
          evento.mouse.y >= 350 && //tamanho da altura de cima
          evento.mouse.y <= 400 ){ //tamanho da altura  de baixo
          destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);//destroi ponteiros
          return 3;
        }//if

        //BOTÃO CRÉDITOS
        if (evento.mouse.x >= 620  && //tamanho da largura da esquerda
          evento.mouse.x <= 920  && //tamanho da largura da direita
          evento.mouse.y >= 450  && //tamanho da altura de cima
          evento.mouse.y <= 500 ){ //tamanho da altura  de baixo
          destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);//destroi ponteiros
          return 4;
        }//if

        //BOTÃO RANKING
        if (evento.mouse.x >= 300  && //tamanho da largura da esquerda
          evento.mouse.x <= 590  && //tamanho da largura da direita
          evento.mouse.y >= 450  && //tamanho da altura de cima
          evento.mouse.y <= 500 ){ //tamanho da altura  de baixo

          }//if

        //BOTÃO SAIR
        if (evento.mouse.x >= 540  && //tamanho da largura da esquerda
          evento.mouse.x <= 690 && //tamanho da largura da direita
          evento.mouse.y >= 550  && //tamanho da altura de cima
          evento.mouse.y <= 600 ){//tamanho da altura de baixo
          //confirmar mensagem de saida
          char tcaixa[50] = "Menu";
          //exibe se deseja sair do jogo
          char titulo[100] = "Atenção";
          //exibe mensagem de aviso
          char texto[200] = "Deseja mesmo sair?";
          //mostra a caixa de texto
          int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
          //al_get_current_display() - retorna a janela ativa;
          // printf("%i",r);
          if(r==true){
            destroiMenu(btn_sair, btn_iniciar, btn_tutorial, btn_creditos, btn_configuracao, fonte, imagem, fila_eventos);
            al_destroy_audio_stream(musica);
            al_destroy_display(janela);
            exit( 0);
          }//if
        }//if
      }//else if botao sair
    }//while


    // Colorimos o bitmap correspondente ao retângulo central,
    // com a cor condicionada ao conteúdo da flag na_btn_iniciar
    // Texto centralizado e movimentações dos botões
    if (!na_btn_iniciar){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 621, 150, ALLEGRO_ALIGN_CENTRE, "Iniciar");
    }//if botão iniciar
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 621, 150, ALLEGRO_ALIGN_CENTRE, "Iniciar");
    }//else

    if (!na_btn_tutorial){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 621, 250, ALLEGRO_ALIGN_CENTRE, "Tutorial");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 621, 250, ALLEGRO_ALIGN_CENTRE, "Tutorial");
    }//else

    if (!na_btn_creditos){
      al_draw_text(fonte,al_map_rgb(0,0,0), 771, 450, ALLEGRO_ALIGN_CENTRE, "Creditos");
    }//else
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 771, 450, ALLEGRO_ALIGN_CENTRE, "Creditos");
    }//else

    if (!na_btn_configuracao){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 621, 350, ALLEGRO_ALIGN_CENTRE, "Configurações");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 621, 350, ALLEGRO_ALIGN_CENTRE, "Configurações");
    }//else

    if(!na_btn_ranking){
      //cor do botao ranking sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 451, 450, ALLEGRO_ALIGN_CENTRE, "Ranking");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 451, 450, ALLEGRO_ALIGN_CENTRE, "Ranking");
    }//else

    if (!na_btn_sair){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 621, 550, ALLEGRO_ALIGN_CENTRE, "Sair");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 621, 550, ALLEGRO_ALIGN_CENTRE, "Sair");
    }//else

    // Desenhamos os retângulos na tela
    al_set_target_bitmap(al_get_backbuffer(janela));
    al_draw_text(fonte,al_map_rgb(102,205,170),LARGURA_TELA / 2,
    (ALTURA_TELA / 2.5 - al_get_bitmap_height(btn_iniciar)) / 2, ALLEGRO_ALIGN_CENTRE, "Menu Inicial"); //menu

    // Atualiza a tela
    al_flip_display();
  } //while sair
  return 0;
}// função gerarMenu

//******************************************************** FUNÇÃO TELA JOGAR **********************************************************;
//Função de gerar a tela inicial de Jogar



int geraJogar(ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_medio, ALLEGRO_BITMAP *btn_facil, ALLEGRO_BITMAP *btn_dificil,
ALLEGRO_BITMAP *btn_voltar, int sair,int voltar, ALLEGRO_AUDIO_STREAM *musica){
  // Atualização da janela
  janela = al_get_current_display();
  if (!janela){
    mensagem_erro( "Falha ao criar janela.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if janela
  // Configura o título da janela
  al_set_window_title(janela, "Menu");
  //colocando o fundinho do menu
  imagem = al_load_bitmap("images/menu.png");
  fila_eventos = al_create_event_queue();

  if (!imagem){
    mensagem_erro( "Falha ao carregar o arquivo de imagem.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if imagens

  // Torna apto o uso de mouse na aplicação
  if (!al_install_mouse()){
    mensagem_erro( "Falha ao inicializar o mouse.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse

  // Atribui o cursor padrão do sistema para ser usado
  if (!al_set_system_mouse_cursor(janela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)){
    mensagem_erro( "Falha ao atribuir ponteiro do mouse.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse cursor

  // Alocamos o botão central da tela
  btn_medio = al_create_bitmap(300, 200);
  if (!btn_medio) {
    mensagem_erro( "Falha ao criar bitmap.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao medio

  //Tutorial
  btn_facil = al_create_bitmap(300, 300);
  if (!btn_facil) {
    mensagem_erro( "Falha ao criar bitmap.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao facil

  //configurações
  btn_dificil = al_create_bitmap(300, 400);
  if (!btn_dificil) {
    mensagem_erro( "Falha ao criar bitmap.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao dificil

  //Botão para voltar a aplicação
  btn_voltar = al_create_bitmap(300, 600);
  if (!btn_voltar){
    mensagem_erro( "Falha ao criar botão Voltar.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  //eventos
  if (!fila_eventos){
    mensagem_erro( "Falha ao criar fila de eventos.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if eventos

  // Carregando o arquivo de fonte
  fonte = al_load_font("fonts/army.ttf", 48, 0);
  if (!fonte){
    mensagem_erro( "Falha ao carregar fonte.\n");
    destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if fonte

  // Dizemos que vamos tratar os eventos vindos do mouse
  al_register_event_source(fila_eventos, al_get_mouse_event_source());
  // Flag indicando Dificuldade
  int na_btn_facil = 0;
  int na_btn_medio = 0;
  int na_btn_dificil = 0;
  int na_btn_voltar = 0;

  al_register_event_source(fila_eventos, al_get_display_event_source(janela));

  //EVENTOS SOBRE BOTÃO
  while (!voltar) {
    // Verificamos se há eventos na fila
    al_draw_bitmap(imagem,0,0,0);
    while (!al_is_event_queue_empty(fila_eventos)) {
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);

      if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //caixa de texto para confirmar sair do jogo
        //exibe o titulo do jogo
        char tcaixa[50] = "Menu - Seleção de dificuldade";
        //exibe se deseja sair do jogo
        char titulo[100] = "Atenção";
        //exibe mensagem de aviso
        char texto[200] = "Deseja mesmo sair?";
        //mostra a caixa de texto
        int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        // printf("%i",r);
        if(r==true){
          destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
          al_destroy_audio_stream(musica);
          al_destroy_display(janela);
          return 0;
        }//if
      }//if do X na tela
      // Se o evento foi de movimentação do mouse
      if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
        // Verificamos se ele está sobre a região do retângulo central
        if (evento.mouse.x >= 90  && //tamanho da largura da esquerda
        evento.mouse.x <= 240 && //tamanho da largura da direita
        evento.mouse.y >= 200  && //tamanho da altura de cima
        evento.mouse.y <= 270){ //tamanho da altura  de baixo

          na_btn_facil = 1;
        }//if
        else{
          na_btn_facil = 0;
        }//else
        // Verificamos se ele está sobre a região do retângulo central
        if (evento.mouse.x >= 550  && //tamanho da largura da esquerda
        evento.mouse.x <= 730  && //tamanho da largura da direita
        evento.mouse.y >= 200  && //tamanho da altura de cima
        evento.mouse.y <= 270 ){ //tamanho da altura  de baixo

          na_btn_medio = 1;
        }//if
        else{
          na_btn_medio = 0;
        }//else

        if (evento.mouse.x >= 950  && //tamanho da largura da esquerda
        evento.mouse.x <= 1130  && //tamanho da largura da direita
        evento.mouse.y >= 200  && //tamanho da altura de cima
        evento.mouse.y <= 270 ){ //tamanho da altura  de baixo
          na_btn_dificil = 1;
        }//if
        else{
          na_btn_dificil = 0;
        }//else

        if (evento.mouse.x >= 500  && //tamanho da largura da esquerda
        evento.mouse.x <= 740  && //tamanho da largura da direita
        evento.mouse.y >= 500 && //tamanho da altura de cima
        evento.mouse.y <= 570 ){ //tamanho da altura  de baixo
          na_btn_voltar = 1;
        }//if
        else{
          na_btn_voltar = 0;
        }//else
      }//if movimentação do mouse

      // Ou se o evento foi um clique do mouse
      else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){

        //BOTÃO - SAIR
        if (evento.mouse.x >= 500  && //tamanho da largura da esquerda
        evento.mouse.x <= 740  && //tamanho da largura da direita
        evento.mouse.y >= 500 && //tamanho da altura de cima
        evento.mouse.y <= 570 ){//tamanho da altura para baixo
          destroiJogar(fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar);
          return 5;
        }//if
        // Verificamos se ele está sobre a região do retângulo central
        if (evento.mouse.x >= 90  && //tamanho da largura da esquerda
        evento.mouse.x <= 240 && //tamanho da largura da direita
        evento.mouse.y >= 200  && //tamanho da altura de cima
        evento.mouse.y <= 270){ //tamanho da altura  de baixo

          return 6;
        }//if
        // Verificamos se ele está sobre a região do retângulo central
        if (evento.mouse.x >= 550  && //tamanho da largura da esquerda
        evento.mouse.x <= 730  && //tamanho da largura da direita
        evento.mouse.y >= 200  && //tamanho da altura de cima
        evento.mouse.y <= 270 ){ //tamanho da altura  de baixo

          return 7;
        }//if

        if (evento.mouse.x >= 950  && //tamanho da largura da esquerda
        evento.mouse.x <= 1130  && //tamanho da largura da direita
        evento.mouse.y >= 200  && //tamanho da altura de cima
        evento.mouse.y <= 270 ){ //tamanho da altura  de baixo
          return 8;
        }//if
      }//else if botao sair
    }//while

    // Colorimos o bitmap correspondente ao retângulo central,
    // com a cor condicionada ao conteúdo da flag na_btn_iniciar
    // Texto centralizado
    if (!na_btn_facil){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0),  221, 200 , ALLEGRO_ALIGN_RIGHT, "Fácil");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 221, 200 , ALLEGRO_ALIGN_RIGHT , "Fácil");
    }//else

    if (!na_btn_medio){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 721, 200 , ALLEGRO_ALIGN_RIGHT, "Médio");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 721, 200 , ALLEGRO_ALIGN_RIGHT, "Médio");
    }//else

    if (!na_btn_dificil){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 1170, 200 , ALLEGRO_ALIGN_RIGHT, "Difícil");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 1170, 200 , ALLEGRO_ALIGN_RIGHT, "Difícil");
    }//else

    if (!na_btn_voltar){
      // Texto centralizado
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 621, 500, ALLEGRO_ALIGN_CENTRE, "Voltar");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204),621, 500, ALLEGRO_ALIGN_CENTRE, "Voltar");
    }//else


    // Desenhamos os retângulos na tela
    al_set_target_bitmap(al_get_backbuffer(janela));
    al_draw_text(fonte, al_map_rgb(102,205,170),LARGURA_TELA / 2,
    (ALTURA_TELA / 1.5 - al_get_bitmap_height(btn_facil)) / 2, ALLEGRO_ALIGN_CENTRE, "Nível de Dificuldade"); //dificuldade

    // Atualiza a tela
    al_flip_display();
  } //while sair
  return 0;
}// função gerarJOgar

//******************************************************** FUNÇÃO TELA TUTORIAL **********************************************************;
//Função de gerar a tela inicial de Tutorial
int geraTutorial(ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_voltar, int sair,int voltar, ALLEGRO_AUDIO_STREAM *musica, ALLEGRO_BITMAP *btn_prox, ALLEGRO_BITMAP *btn_menu){
  // Atualização da janela
    janela = al_get_current_display();
  if (!janela){
    mensagem_erro( "Falha ao criar janela.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if janela

  // Configura o título da janela
  al_set_window_title(janela, "Tutorial");
  //colocando o fundinho do menu
  imagem = al_load_bitmap("images/tutorial.png");
  fila_eventos = al_create_event_queue();

  if (!imagem){
    mensagem_erro( "Falha ao carregar o arquivo de imagem.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if imagens

  // Torna apto o uso de mouse na aplicação
  if (!al_install_mouse()){
    mensagem_erro( "Falha ao inicializar o mouse.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse

  // Atribui o cursor padrão do sistema para ser usado
  if (!al_set_system_mouse_cursor(janela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)){
    mensagem_erro( "Falha ao atribuir ponteiro do mouse.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse cursor

  //Botão para voltar a aplicação
  btn_voltar = al_create_bitmap(300, 600);
  if (!btn_voltar){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  //Botão para passar para a próxima tela
  btn_prox = al_create_bitmap(300, 600);
  if (!btn_prox){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  btn_menu = al_create_bitmap(300, 600);
  if (!btn_menu){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  //eventos
  if (!fila_eventos){
    mensagem_erro( "Falha ao criar fila de eventos.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if eventos

  // Carregando o arquivo de fonte
  fonte = al_load_font("fonts/army.ttf", 48, 0);
  if (!fonte){
    mensagem_erro( "Falha ao carregar fonte.\n");
    destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if fonte

  // Dizemos que vamos tratar os eventos vindos do mouse
  al_register_event_source(fila_eventos, al_get_mouse_event_source());
  // Flag indicando Dificuldade
  int na_btn_voltar = 0;
  int na_btn_prox=0;
  int cont=1;

  al_register_event_source(fila_eventos, al_get_display_event_source(janela));

  //EVENTOS SOBRE BOTÃO
  while (!voltar) {
    // Verificamos se há eventos na fila
    al_draw_bitmap(imagem,0,0,0);
    while (!al_is_event_queue_empty(fila_eventos)) {
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);

      if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //caixa de texto para confirmar sair do jogo
        //exibe o titulo do jogo
        char tcaixa[50] = "Tutorial";
        //exibe se deseja sair do jogo
        char titulo[100] = "Atenção";
        //exibe mensagem de aviso
        char texto[200] = "Deseja mesmo sair?";
        //mostra a caixa de texto
        int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        // printf("%i",r);
        if(r==true){
          destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
          al_destroy_audio_stream(musica);
          al_destroy_display(janela);
          return 0;
        }//if
      }//if do X na tela

      // Se o evento foi de movimentação do mouse
      if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
        //BOTÃO - VOLTAR
        if (evento.mouse.x >= 280  && //tamanho da largura da esquerda
        evento.mouse.x <= 500  && //tamanho da largura da direita
        evento.mouse.y >= 600 && //tamanho da altura de cima
        evento.mouse.y <= 650 ){ //tamanho da altura  de baixo
          na_btn_voltar = 1;
        }//if
        else{
          na_btn_voltar = 0;
        }//else

      //BOTÃO - PRÓXIMO
      if (evento.mouse.x >= 720  && //tamanho da largura da esquerda
      evento.mouse.x <= 1020  && //tamanho da largura da direita
      evento.mouse.y >= 600 && //tamanho da altura de cima
      evento.mouse.y <= 650 ){ //tamanho da altura  de baixo
        na_btn_prox = 1;
      }//if
      else{
        na_btn_prox = 0;
      }//else
    }//if

    // Ou se o evento foi de clique do mouse
      else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){

        //BOTÃO - VOLTAR
        if (evento.mouse.x >= 280  && //tamanho da largura da esquerda
        evento.mouse.x <= 500  && //tamanho da largura da direita
        evento.mouse.y >= 600 && //tamanho da altura de cima
        evento.mouse.y <= 650 ){//tamanho da altura para baixo
          if(cont == 2){
            imagem = al_load_bitmap("images/tutorial.png");
            cont--;
          }
          else{
            destroiTutorial(fonte, imagem, fila_eventos, btn_voltar);
            return 5;
          }
        }//if

        //BOTÃO - PRÓXIMO
        if (evento.mouse.x >= 720  && //tamanho da largura da esquerda
        evento.mouse.x <= 1020  && //tamanho da largura da direita
        evento.mouse.y >= 600 && //tamanho da altura de cima
        evento.mouse.y <= 650 ){ //tamanho da altura  de baixo
          imagem = al_load_bitmap("images/TutorialParte2.png");
          if(cont==2){
            return 5;
          }
          // al_destroy_bitmap(btn_prox);
          cont++;
          al_flip_display();
        }//if

      }//else if
    }//while

    if (!na_btn_voltar){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 540, 600, 550, "Voltar");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204),540, 600, 550, "Voltar");
    }//else

    if(!na_btn_prox){
      //cor do botão proximo sem nada
      if(cont==1){
      al_draw_text(fonte, al_map_rgb(0,0,0), 1000, 600, 550, "Próximo");
      }
      else
      {
        al_draw_text(fonte, al_map_rgb(0,0,0), 1000, 600, 550, "Menu");
      }
    }
    else{
      if(cont==1){
        al_draw_text(fonte, al_map_rgb(72,209,204), 1000, 600, 550, "Próximo");
      }
      else{
        al_draw_text(fonte, al_map_rgb(72,209,204), 1000, 600, 550, "Menu");
      }
    }

    // Desenhamos os retângulos na tela
    al_set_target_bitmap(al_get_backbuffer(janela));
    al_draw_text(fonte,al_map_rgb(102,205,170),LARGURA_TELA / 2,
    (ALTURA_TELA / 2.5 - al_get_bitmap_height(btn_voltar)) / 2, ALLEGRO_ALIGN_CENTRE, "Tutorial"); //dificuldade

    // Atualiza a tela
    al_flip_display();
  } //while sair
  return 0;
}// função gerarTutorial

//******************************************************** FUNÇÃO TELA CREDITOS **********************************************************;
//Função de gerar a tela inicial de Creditos
int geraCredito(ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_voltar, int sair,int voltar, ALLEGRO_AUDIO_STREAM *musica){
  // Atualização da janela
    janela = al_get_current_display();
  if (!janela){
    mensagem_erro( "Falha ao criar janela.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if janela

  // Configura o título da janela
  al_set_window_title(janela, "Créditos");
  //colocando o fundinho do menu
  imagem = al_load_bitmap("images/creditos.png");
  fila_eventos = al_create_event_queue();

  if (!imagem){
    mensagem_erro( "Falha ao carregar o arquivo de imagem.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if imagens

  // Torna apto o uso de mouse na aplicação
  if (!al_install_mouse()){
    mensagem_erro( "Falha ao inicializar o mouse.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse

  // Atribui o cursor padrão do sistema para ser usado
  if (!al_set_system_mouse_cursor(janela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)){
    mensagem_erro( "Falha ao atribuir ponteiro do mouse.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse cursor

  //Botão para voltar a aplicação
  btn_voltar = al_create_bitmap(300, 600);
  if (!btn_voltar){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  //eventos
  if (!fila_eventos){
    mensagem_erro( "Falha ao criar fila de eventos.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if eventos

  // Carregando o arquivo de fonte
  fonte = al_load_font("fonts/army.ttf", 48, 0);
  if (!fonte){
    mensagem_erro( "Falha ao carregar fonte.\n");
    destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if fonte

  // Dizemos que vamos tratar os eventos vindos do mouse
  al_register_event_source(fila_eventos, al_get_mouse_event_source());
  // Flag indicando Dificuldade
  int na_btn_voltar = 0;


  //EVENTOS SOBRE BOTÃO
  while (!voltar) {
    // Verificamos se há eventos na fila
    al_draw_bitmap(imagem,0,0,0);
    while (!al_is_event_queue_empty(fila_eventos)) {
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);

      if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //caixa de texto para confirmar sair do jogo
        //exibe o titulo do jogo
        char tcaixa[50] = "Jogo";
        //exibe se deseja sair do jogo
        char titulo[100] = "Atenção";
        //exibe mensagem de aviso
        char texto[200] = "Deseja mesmo sair?";
        //mostra a caixa de texto
        int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        // printf("%i",r);
        if(r==true){
          destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
          al_destroy_audio_stream(musica);
          al_destroy_display(janela);
          return 0;
        }//if
      }//if do X na tela

      // Se o evento foi de movimentação do mouse
      if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
        //voltar
        if (evento.mouse.x >= 510  && //tamanho da largura da esquerda
        evento.mouse.x <= 750  && //tamanho da largura da direita
        evento.mouse.y >= 600 && //tamanho da altura de cima
        evento.mouse.y <= 650 ){ //tamanho da altura  de baixo
          na_btn_voltar = 1;
        }//if
        else{
          na_btn_voltar = 0;
        }//else
      }//if

      else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
        if (evento.mouse.x >= 510  && //tamanho da largura da esquerda
        evento.mouse.x <= 750  && //tamanho da largura da direita
        evento.mouse.y >= 600 && //tamanho da altura de cima
        evento.mouse.y <= 650 ){//tamanho da altura para baixo
          destroiCreditos(fonte, imagem, fila_eventos, btn_voltar);
          return 5;
        }//if
      }//else if
    }//while

    if (!na_btn_voltar){
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 621, 600, ALLEGRO_ALIGN_CENTRE, "Voltar");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204),621, 600, ALLEGRO_ALIGN_CENTRE, "Voltar");
    }//else

    // Atualiza a tela
    al_flip_display();
  } //while sair
  return 0;
}// função geraCredito

//******************************************************** FUNÇÃO TELA CONFIGURAÇÃO **********************************************************;
//Função de gerar a tela inicial de Configuração
int geraConfiguracao(ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *imagem,
ALLEGRO_EVENT_QUEUE *fila_eventos, ALLEGRO_BITMAP *btn_voltar, ALLEGRO_BITMAP *btn_som , int sair,int voltar, ALLEGRO_AUDIO_STREAM *musica, float volume){
  // Atualização da janela
    janela = al_get_current_display();
  if (!janela){
    mensagem_erro( "Falha ao criar janela.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if janela

  // Configura o título da janela
  al_set_window_title(janela, "Configuração");
  //colocando o fundinho do menu
  imagem = al_load_bitmap("images/menu.png");
  fila_eventos = al_create_event_queue();

  if (!imagem){
    mensagem_erro( "Falha ao carregar o arquivo de imagem.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if imagens

  // Torna apto o uso de mouse na aplicação
  if (!al_install_mouse()){
    mensagem_erro( "Falha ao inicializar o mouse.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse

  // Atribui o cursor padrão do sistema para ser usado
  if (!al_set_system_mouse_cursor(janela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)){
    mensagem_erro( "Falha ao atribuir ponteiro do mouse.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if mouse cursor

  //Botão para voltar a aplicação
  btn_voltar = al_create_bitmap(300, 600);
  if (!btn_voltar){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  //Botão para desabilitar ou habilitar som
  btn_som = al_create_bitmap(300, 600);
  if (!btn_som){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar
  ALLEGRO_BITMAP* audio_display;
  ALLEGRO_BITMAP* audio_up;
  ALLEGRO_BITMAP* audio_down;
  ALLEGRO_BITMAP* btn_mais;
  ALLEGRO_BITMAP* btn_menos;

  audio_display = al_create_bitmap(300, 600);
  if (!audio_display){
    mensagem_erro( "Falha ao criar botão de saída.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  btn_mais = al_load_bitmap("images/botao_up.png");
  if (!btn_mais){
    mensagem_erro( "Falha ao criar botão +.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  btn_menos = al_load_bitmap("images/botao_down.png");
  if (!btn_menos){
    mensagem_erro( "Falha ao criar botão -.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  audio_up = al_create_bitmap(300, 300);
  if (!audio_up){
    mensagem_erro( "Falha ao criar botão audio_on.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  audio_down = al_create_bitmap(300, 300);
  if (!audio_down){
    mensagem_erro( "Falha ao criar botão audio_down.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if botao voltar

  //eventos
  if (!fila_eventos){
    mensagem_erro( "Falha ao criar fila de eventos.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if eventos

  // Carregando o arquivo de fonte
  fonte = al_load_font("fonts/army.ttf", 48, 0);
  if (!fonte){
    mensagem_erro( "Falha ao carregar fonte.\n");
    destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    return 0;
  }//if fonte

  // Dizemos que vamos tratar os eventos vindos do mouse
  al_register_event_source(fila_eventos,al_get_mouse_event_source());
  // Flag indicando Dificuldade
  int na_btn_voltar = 0, na_btn_som = 0, na_btn_mais=0, na_btn_menos=0;

  al_register_event_source(fila_eventos, al_get_display_event_source(janela));
  //EVENTOS SOBRE BOTÃO
  while (!voltar) {
    // Verificamos se há eventos na fila
    al_draw_bitmap(imagem,0,0,0);
    while (!al_is_event_queue_empty(fila_eventos)) {
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);

      if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //caixa de texto para confirmar sair do jogo
        //exibe o titulo do jogo
        char tcaixa[50] = "Configuração";
        //exibe se deseja sair do jogo
        char titulo[100] = "Atenção";
        //exibe mensagem de aviso
        char texto[200] = "Deseja mesmo sair?";
        //mostra a caixa de texto
        int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        // printf("%i",r);
        if(r==true){
          destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
          al_destroy_audio_stream(musica);
          al_destroy_display(janela);
          return 0;
        }//if
      }//if do X na tela

      // Se o evento foi de movimentação do mouse
      if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
        //voltar
        if (evento.mouse.x >= 500  && //tamanho da largura da esquerda
        evento.mouse.x <= 750  && //tamanho da largura da direita
        evento.mouse.y >= 500 && //tamanho da altura de cima
        evento.mouse.y <= 550 ){ //tamanho da altura  de baixo
          na_btn_voltar = 1;
        }//if
        else{
          na_btn_voltar = 0;
        }//else

        //Som
        if (evento.mouse.x >= 100  && //tamanho da largura da esquerda
        evento.mouse.x <= 230  && //tamanho da largura da direita
        evento.mouse.y >= 60 && //tamanho da altura de cima
        evento.mouse.y <= 120 ){ //tamanho da altura  de baixo
          na_btn_som = 1;
        }//if
        else{
          na_btn_som = 0;
        }//else

        if (evento.mouse.x >= 370  && //tamanho da largura da esquerda
        evento.mouse.x <= 470  && //tamanho da largura da direita
        evento.mouse.y >= 20 && //tamanho da altura de cima
        evento.mouse.y <= 120 ){ //tamanho da altura  de baixo
          na_btn_menos = 1;
        }//if
        else{
          na_btn_menos = 0;
        }//else

        if (evento.mouse.x >= 900  && //tamanho da largura da esquerda
        evento.mouse.x <= 1130  && //tamanho da largura da direita
        evento.mouse.y >= 20 && //tamanho da altura de cima
        evento.mouse.y <= 120 ){ //tamanho da altura  de baixo
          na_btn_mais = 1;
        }//if
        else{
          na_btn_mais = 0;
        }//else

      }//if movimentação mouse

      else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
        //evento ao clicar no botão voltar
        if (evento.mouse.x >= 500  && //tamanho da largura da esquerda
        evento.mouse.x <= 750  && //tamanho da largura da direita
        evento.mouse.y >= 500 && //tamanho da altura de cima
        evento.mouse.y <= 550 ){
          destroiConfiguracoes(fonte, imagem, fila_eventos, btn_voltar, btn_som);
          return 5;
        }//if botão voltar

        //evento ao clicar no botão som
        if (evento.mouse.x >= 100  && //tamanho da largura da esquerda
        evento.mouse.x <= 230  && //tamanho da largura da direita
        evento.mouse.y >= 60 && //tamanho da altura de cima
        evento.mouse.y <= 120 ){ //tamanho da altura  de baixo
        if(audio_on == 0)
        {
          audio_on= 1;
          al_set_audio_stream_playing(musica, 0);
        }//if audio_on
        else
        {
          audio_on= 0;
          al_rewind_audio_stream(musica);
          al_set_audio_stream_playing(musica, 1);
        }//else audio_on
      }//if botão som

      if (evento.mouse.x >= 370  && //tamanho da largura da esquerda
      evento.mouse.x <= 470  && //tamanho da largura da direita
      evento.mouse.y >= 20 && //tamanho da altura de cima
      evento.mouse.y <= 120 ){ //tamanho da altura  de baixo
        if(volume>0.2){
          volume= volume - 0.2;
        }
        al_set_audio_stream_gain(musica, volume);
      }//if

      if (evento.mouse.x >= 900  && //tamanho da largura da esquerda
      evento.mouse.x <= 1130  && //tamanho da largura da direita
      evento.mouse.y >= 20 && //tamanho da altura de cima
      evento.mouse.y <= 120 ){ //tamanho da altura  de baixo
        if(volume<0.8){
          volume = volume + 0.2;
        }
        al_set_audio_stream_gain(musica, volume);
      }//if

      }//else if clique no botão
    }//while
    if(audio_on == 0)
    {
      al_draw_text(fonte,al_map_rgb(142, 35, 107), 250, 50, 0, "On");
    }
    else
    {
      al_draw_text(fonte,al_map_rgb(255,0,0), 250, 50, 0, "Off");
    }
      al_draw_bitmap(btn_mais, 730, -250, 0);
      al_draw_scaled_bitmap(btn_menos, 0, 0, 248, 253, 370, 20, 100, 100, 0);

      if(!na_btn_menos){
        al_draw_text(fonte, al_map_rgb(0, 0, 0), 450, 50, 0, "-");
      }
      else{
        al_draw_text(fonte, al_map_rgb(72, 209, 204), 450, 50, 0, "-");
      }

      if(!na_btn_mais){
        al_draw_text(fonte, al_map_rgb(0, 0, 0), 1050, 50, 0, "-");
      }
      else{
        al_draw_text(fonte, al_map_rgb(72, 209, 204), 1050, 50, 0, "-");
      }

    if (!na_btn_voltar){
      // Texto centralizado
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0),LARGURA_TELA/2, 500, ALLEGRO_ALIGN_CENTRE,  "Voltar");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204),LARGURA_TELA/2, 500, ALLEGRO_ALIGN_CENTRE, "Voltar");
    }//else

    if (!na_btn_som){
      // Texto centralizado
      //cor do botao iniciar sem nada
      al_draw_text(fonte,al_map_rgb(0,0,0), 100, 50, 0, "Som");
    }//if
    else{
      //cor do botao ao passar mouse encima
      al_draw_text(fonte,al_map_rgb(72,209,204), 100, 50, 0, "Som");
    }//else

    // Desenhamos os retângulos na tela
    al_set_target_bitmap(al_get_backbuffer(janela));
    al_draw_text(fonte,al_map_rgb(102,205,170),LARGURA_TELA / 2,
    (ALTURA_TELA / 2.5 - al_get_bitmap_height(btn_som)) / 2, ALLEGRO_ALIGN_CENTRE, "Tutorial"); //dificuldade

    // Atualiza a tela
    al_flip_display();
  } //while sair
  return 0;
}// função geraConfiguracao
int verificaColisao(Personagem goku, Personagem inimigo, Personagem inimigo2,Personagem inimigo3, Personagem semente1,Personagem semente2,Personagem semente3,int captacolisao_goku, int captacolisao_semente1, int captacolisao_semente2, int captacolisao_semente3){
  if((((((((goku.pos_x_sprite+goku.largura_sprite/2)>=(inimigo.pos_x_sprite - inimigo.largura_sprite/2))&&//goku morre se encostar em um dos inimigos
  ((goku.pos_x_sprite-goku.largura_sprite/2)<=(inimigo.pos_x_sprite + inimigo.largura_sprite/2)))&&
  ((goku.pos_y_sprite+goku.altura_sprite/2)>=(inimigo.pos_y_sprite -20)))&&
  ((goku.pos_y_sprite-goku.altura_sprite/2)<=(inimigo.pos_y_sprite +20)))&&
  captacolisao_semente1 <=0)||
  ((((((goku.pos_x_sprite+goku.largura_sprite/2)>=(inimigo2.pos_x_sprite - inimigo2.largura_sprite/2))&&
  ((goku.pos_x_sprite-goku.largura_sprite/2)<=(inimigo2.pos_x_sprite + inimigo2.largura_sprite/2)))&&
  ((goku.pos_y_sprite+goku.altura_sprite/2)>=(inimigo2.pos_y_sprite -20)))&&
  ((goku.pos_y_sprite-goku.altura_sprite/2)<=(inimigo2.pos_y_sprite + 20)))&&
  captacolisao_semente2 <=0)||
  ((((((goku.pos_x_sprite+goku.largura_sprite/2)>=(inimigo3.pos_x_sprite - inimigo3.largura_sprite/2))&&
  ((goku.pos_x_sprite-goku.largura_sprite/2)<=(inimigo3.pos_x_sprite + inimigo3.largura_sprite/2)))&&
  ((goku.pos_y_sprite+goku.altura_sprite/2)>=(inimigo3.pos_y_sprite -20)))&&
  ((goku.pos_y_sprite-goku.altura_sprite/2)<=(inimigo3.pos_y_sprite + 20))))&&
  captacolisao_semente3 <=0) &&
  (captacolisao_goku <=0 )){
    return 1;
  }//if

  if ((((((goku.pos_x_sprite+goku.largura_sprite/2)>=(semente1.pos_x_sprite -20 ))&&//se encostar na semente1 ela desaparece
  ((goku.pos_x_sprite-goku.largura_sprite/2)<=(semente1.pos_x_sprite )))&&
  ((goku.pos_y_sprite+goku.altura_sprite/2)>=(semente1.pos_y_sprite -20)))&&
  ((goku.pos_y_sprite-goku.altura_sprite/2)<=(semente1.pos_y_sprite )))) {
    return 2;
  }//if

  if (captacolisao_semente2 >=1 &&
    (((((goku.pos_x_sprite+goku.largura_sprite/2)>=(inimigo2.pos_x_sprite - inimigo2.largura_sprite/2))&&
    ((goku.pos_x_sprite-goku.largura_sprite/2)<=(inimigo2.pos_x_sprite + inimigo2.largura_sprite/2)))&&
    ((goku.pos_y_sprite+goku.altura_sprite/2)>=(inimigo2.pos_y_sprite -20)))&&
    ((goku.pos_y_sprite-goku.altura_sprite/2)<=(inimigo2.pos_y_sprite + 20)))) {
      return 3;
    }//if
    if (captacolisao_semente1 >= 1 &&
      (((((goku.pos_x_sprite+goku.largura_sprite/2)>=(inimigo.pos_x_sprite - inimigo.largura_sprite/2))&&
      ((goku.pos_x_sprite-goku.largura_sprite/2)<=(inimigo.pos_x_sprite + inimigo.largura_sprite/2)))&&
      ((goku.pos_y_sprite+goku.altura_sprite/2)>=(inimigo.pos_y_sprite -20)))&&
      ((goku.pos_y_sprite-goku.altura_sprite/2)<=(inimigo.pos_y_sprite + 20)))) {
        return 4;
      }//if
      if (captacolisao_semente3 >= 1 &&
        (((((goku.pos_x_sprite+goku.largura_sprite/2)>=(inimigo3.pos_x_sprite - inimigo3.largura_sprite/2))&&
        ((goku.pos_x_sprite-goku.largura_sprite/2)<=(inimigo3.pos_x_sprite + inimigo3.largura_sprite/2)))&&
        ((goku.pos_y_sprite+goku.altura_sprite/2)>=(inimigo3.pos_y_sprite -20)))&&
        ((goku.pos_y_sprite-goku.altura_sprite/2)<=(inimigo3.pos_y_sprite + 20)))) {
          return 5;
        }//if
        if ((((((goku.pos_x_sprite+goku.largura_sprite/2)>=(semente2.pos_x_sprite -20 ))&&//se encostar na semente2 ela desaparece
        ((goku.pos_x_sprite-goku.largura_sprite/2)<=(semente2.pos_x_sprite )))&&
        ((goku.pos_y_sprite+goku.altura_sprite/2)>=(semente2.pos_y_sprite -20)))&&
        ((goku.pos_y_sprite-goku.altura_sprite/2)<=(semente2.pos_y_sprite )))) {
          return 6;
        }//if
        if ((((((goku.pos_x_sprite+goku.largura_sprite/2)>=(semente3.pos_x_sprite -20 ))&&//se encostar na semente3 ela desaparece
        ((goku.pos_x_sprite-goku.largura_sprite/2)<=(semente3.pos_x_sprite )))&&
        ((goku.pos_y_sprite+goku.altura_sprite/2)>=(semente3.pos_y_sprite -20)))&&
        ((goku.pos_y_sprite-goku.altura_sprite/2)<=(semente3.pos_y_sprite )))) {
          return 7;
        }//if
        else return 0;
      }//funcao verificaColisao
int Atualizarjogo1(Personagem goku, Personagem inimigo, Personagem inimigo2,Personagem inimigo3,Personagem semente1,Personagem semente2,Personagem semente3,int i,Personagem vidas){
  int desenha = 1;
  //int sair = 0;
  int captacolisao_goku = 0;
  int captacolisao_inimigo1 = 0;
  int captacolisao_inimigo2 = 0;
  int captacolisao_inimigo3 = 0;
  int captacolisao_semente1 = 0;
  int captacolisao_semente2 = 0;
  int captacolisao_semente3 = 0;
  int atualiza_x_1 = 2;
  int atualiza_x_2 = 2;
  int atualiza_x_3 = 2;
  bool pressed_key[ALLEGRO_KEY_MAX]  ;
  pressed_key[ALLEGRO_KEY_UP] = false;
  pressed_key[ALLEGRO_KEY_DOWN] = false;
  pressed_key[ALLEGRO_KEY_LEFT] = false;
  pressed_key[ALLEGRO_KEY_RIGHT] = false;
  bool done = false;
  int colisao;
  int cont = 0;
  //largura e altura de cada sprite dentro da folha
  goku.altura_sprite=68; goku.largura_sprite=34;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  goku.colunas_folha=10, goku.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  goku.linha_atual=3, goku.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  goku.regiao_x_folha=0, goku.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  goku.frames_sprite=0, goku.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  goku.pos_x_sprite=300, goku.pos_y_sprite=260;
  //velocidade X Y que o sprite ira se mover pela janela
  goku.vel_x_sprite=13, goku.vel_y_sprite=13;
  // inicializarGoku(goku);

  //largura e altura de cada sprite dentro da folha
  inimigo.altura_sprite=80; inimigo.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo.colunas_folha=3, inimigo.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo.linha_atual=1, inimigo.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo.regiao_x_folha=0, inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo.frames_sprite=10, inimigo.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo.pos_x_sprite=400, inimigo.pos_y_sprite=50;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo.vel_x_sprite=13, inimigo.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  inimigo2.altura_sprite=80; inimigo2.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo2.colunas_folha=3, inimigo2.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo2.linha_atual=1, inimigo2.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo2.regiao_x_folha=0, inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo2.frames_sprite=10, inimigo2.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo2.pos_x_sprite=400, inimigo2.pos_y_sprite=550;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo2.vel_x_sprite=13, inimigo2.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  inimigo3.altura_sprite=80; inimigo3.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo3.colunas_folha=3, inimigo3.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo3.linha_atual=1, inimigo3.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo3.regiao_x_folha=0, inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo3.frames_sprite=10, inimigo3.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo3.pos_x_sprite=400, inimigo3.pos_y_sprite=210;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo3.vel_x_sprite=13, inimigo3.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente1.altura_sprite=30; semente1.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente1.colunas_folha=10, semente1.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente1.linha_atual=3, semente1.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente1.regiao_x_folha=0, semente1.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  semente1.frames_sprite=0, semente1.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente1.pos_x_sprite= 360, semente1.pos_y_sprite=160;
  //velocidade X Y que o sprite ira se mover pela janela
  semente1.vel_x_sprite=13, semente1.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente2.altura_sprite=30; semente2.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente2.colunas_folha=10, semente2.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente2.linha_atual=3, semente2.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente2.regiao_x_folha=0, semente2.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  semente2.frames_sprite=0, semente2.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente2.pos_x_sprite= 360, semente2.pos_y_sprite=570;
  //velocidade X Y que o sprite ira se mover pela janela
  semente2.vel_x_sprite=13, semente2.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente3.altura_sprite=30; semente3.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente3.colunas_folha=10, semente3.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente3.linha_atual=3, semente3.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente3.regiao_x_folha=0, semente3.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  semente3.frames_sprite=0, semente3.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente3.pos_x_sprite= 290, semente3.pos_y_sprite=70;
  //velocidade X Y que o sprite ira se mover pela janela
  semente3.vel_x_sprite=13, semente3.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  vidas.altura_sprite=110; vidas.largura_sprite=420;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  vidas.colunas_folha=4, vidas.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  vidas.linha_atual=1, vidas.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  vidas.regiao_x_folha = 0;
  vidas.regiao_y_folha = 0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  vidas.frames_sprite=0, vidas.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  vidas.pos_x_sprite= 920, vidas.pos_y_sprite=40;
  //velocidade X Y que o sprite ira se mover pela janela
  vidas.vel_x_sprite=13, vidas.vel_y_sprite=13;





  ALLEGRO_KEYBOARD_STATE keys;

  while(!done){
    al_wait_for_event(fila_eventos,&evento);
    if(evento.type == ALLEGRO_EVENT_KEY_DOWN) {
      al_get_keyboard_state(&keys);
      char tcaixa[50] = "Pausado",titulo[100] = "Jogo Pausado", texto[200] = "Deseja fechar o jogo?";
      int r;
      //se apertar nas teclas
      switch(evento.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        if (goku.pos_y_sprite > 20 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_UP]=true;
          goku.linha_atual=3;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
        }//if
        break;

        case ALLEGRO_KEY_DOWN:
        if (goku.pos_y_sprite + goku.altura_sprite < ALTURA_TELA -20 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_DOWN]=true;
          goku.linha_atual=1;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
        }//if
        break;

        case ALLEGRO_KEY_LEFT:
        if (goku.pos_x_sprite > 270 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_LEFT]=true;
          goku.linha_atual = 2;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
          goku.vel_x_sprite = -1;
        }//if
        break;

        case ALLEGRO_KEY_RIGHT:
        if (goku.pos_x_sprite + goku.altura_sprite < 902 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_RIGHT]=true;
          goku.linha_atual = 2;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
          goku.vel_x_sprite = 1;
        }//if
        break;

        case ALLEGRO_KEY_ESCAPE:
        //mostra a caixa de texto
        r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        printf("%i",r);
        if(r==true){
          done=true;
        }//if
        break;
      }//switch
    }//if evento movimentação apertando teclado

    else if(evento.type == ALLEGRO_EVENT_KEY_UP) {
      al_get_keyboard_state(&keys);
      //se soltar a tecla
      switch(evento.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        pressed_key[ALLEGRO_KEY_UP]=false;
        break;

        case ALLEGRO_KEY_DOWN:
        pressed_key[ALLEGRO_KEY_DOWN]=false;
        break;

        case ALLEGRO_KEY_LEFT:
        pressed_key[ALLEGRO_KEY_LEFT]=false;
        break;

        case ALLEGRO_KEY_RIGHT:
        pressed_key[ALLEGRO_KEY_RIGHT]=false;
        break;
      }//switch

      desenha=1;
    }//if se solta a tecla

    if (evento.type == ALLEGRO_EVENT_TIMER) {
      if (atualiza_x_1 > 0 &&(inimigo.pos_x_sprite >= 790 || inimigo.pos_x_sprite < 270)){
        atualiza_x_1*= -1;
        inimigo.vel_x_sprite = 1;
        inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
      }//if
      else if(atualiza_x_1 < 0 &&(inimigo.pos_x_sprite >= 790 || inimigo.pos_x_sprite < 270)){
        atualiza_x_1*= -1;
        inimigo.vel_x_sprite = -1;
        inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
      }//else if
      if (atualiza_x_2 > 0 &&(inimigo2.pos_x_sprite >= 750 || inimigo2.pos_x_sprite < 350)){
        atualiza_x_2*= -1;
        inimigo2.vel_x_sprite = 1;
        inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
      }//if
      else if(atualiza_x_2 < 0 &&(inimigo2.pos_x_sprite >= 750 || inimigo2.pos_x_sprite < 350)){
        atualiza_x_2*= -1;
        inimigo2.vel_x_sprite = -1;
        inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
      }//else if
      if (atualiza_x_3 > 0 &&(inimigo3.pos_x_sprite >= 750 || inimigo3.pos_x_sprite < 350)){
        atualiza_x_3*= -1;
        inimigo3.vel_x_sprite = 1;
        inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
      }//if
      else if(atualiza_x_3 < 0 &&(inimigo3.pos_x_sprite >= 750 || inimigo3.pos_x_sprite < 350)){
        atualiza_x_3*= -1;
        inimigo3.vel_x_sprite = -1;
        inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
      }//else if
      //fantasma se move para direita
      //if(inimigo.pos_y_sprite + inimigo.altura_sprite > ALTURA_TELA -20 || inimigo.pos_y_sprite < 20/*&& !((inimigo.pos_y_sprite >= 201 && inimigo.pos_y_sprite <= 249) && (inimigo.pos_x_sprite >= 300 && inimigo.pos_x_sprite <= 400) || (inimigo.pos_x_sprite >= 150 && inimigo.pos_x_sprite <= 250))*/)
      inimigo.pos_x_sprite -= atualiza_x_1;
      inimigo2.pos_x_sprite -= atualiza_x_2;
      inimigo3.pos_x_sprite -= atualiza_x_3;
      cont++;
      if (cont == 15) {
        inimigo.coluna_atual = (inimigo.coluna_atual+1)% inimigo.colunas_folha;
        inimigo.regiao_x_folha = inimigo.coluna_atual * inimigo.largura_sprite;
        inimigo2.coluna_atual = (inimigo2.coluna_atual+1)% inimigo2.colunas_folha;
        inimigo2.regiao_x_folha = inimigo2.coluna_atual * inimigo2.largura_sprite;
        inimigo3.coluna_atual = (inimigo3.coluna_atual+1)% inimigo3.colunas_folha;
        inimigo3.regiao_x_folha = inimigo3.coluna_atual * inimigo3.largura_sprite;
        cont = 0;
      }//if
      //pressiona para cima (GOKU)
      if (pressed_key[ALLEGRO_KEY_UP]==true && goku.pos_y_sprite > 40 &&
        ((goku.pos_y_sprite != 118 || goku.pos_x_sprite < 580 || goku.pos_x_sprite > 745) &&//bloco 2
        (goku.pos_y_sprite != 118 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 540)) &&//bloco 1
        ((goku.pos_y_sprite != 642 || goku.pos_x_sprite < 595 || goku.pos_x_sprite > 744) &&//bloco 4
        (goku.pos_y_sprite != 642 || goku.pos_x_sprite < 398 || goku.pos_x_sprite > 549)) &&//bloco 3
        ((goku.pos_y_sprite != 260 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 5 e 7
        (goku.pos_y_sprite != 260 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//blocos 6 e 8
        ((goku.pos_y_sprite != 650 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 9 e 10
        (goku.pos_y_sprite != 630 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//bloco 11 e 12
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 13
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 15
        (goku.pos_y_sprite != 540 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 16
        (goku.pos_y_sprite != 540 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 18
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 14
        (goku.pos_y_sprite != 520 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 17
        (goku.pos_y_sprite != 410 || goku.pos_x_sprite < 290 || goku.pos_x_sprite > 360)  &&//bloco 19
        (goku.pos_y_sprite != 410 || goku.pos_x_sprite < 779 || goku.pos_x_sprite > 890)  &&//bloco 20
        (goku.pos_y_sprite != 430 || goku.pos_x_sprite < 410 || goku.pos_x_sprite > 735)  &&//bloco 21
        (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 420 || goku.pos_x_sprite > 549)  &&//bloco 21
        (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 599 || goku.pos_x_sprite > 735)){//bloco 21
          goku.pos_y_sprite -= 2;
          goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
          goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
        }//if

        //pressiona para baixo
        else if(pressed_key[ALLEGRO_KEY_DOWN]==true && goku.pos_y_sprite < 680 &&
          ((goku.pos_y_sprite != 50 || goku.pos_x_sprite < 580 || goku.pos_x_sprite > 745)  &&//bloco 2
          (goku.pos_y_sprite != 50 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 540))  &&//bloco 1
          ((goku.pos_y_sprite != 566 || goku.pos_x_sprite < 595 || goku.pos_x_sprite > 746) &&//bloco 4
          (goku.pos_y_sprite != 566 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 551)) &&//bloco 3
          ((goku.pos_y_sprite != 76 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361)  &&//blocos 5 e 7
          (goku.pos_y_sprite != 80 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830))  &&//blocos 6 e 8
          ((goku.pos_y_sprite != 444 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 9 e 10
          (goku.pos_y_sprite != 444 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//bloco 11 e 12
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 13
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 15
          (goku.pos_y_sprite != 480 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 16
          (goku.pos_y_sprite != 480 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 18
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 14
          (goku.pos_y_sprite != 460 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 17
          (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 290 || goku.pos_x_sprite > 360)  &&//bloco 19
          (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 779 || goku.pos_x_sprite > 890)  &&//bloco 20
          (goku.pos_y_sprite != 360 || goku.pos_x_sprite < 410 || goku.pos_x_sprite > 730)  &&//bloco 21
          (goku.pos_y_sprite != 220 || goku.pos_x_sprite < 420 || goku.pos_x_sprite > 549)  &&//bloco 21
          (goku.pos_y_sprite != 220 || goku.pos_x_sprite < 599 || goku.pos_x_sprite > 750)){//bloco 21
            goku.pos_y_sprite += 2;
            goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
            goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
          }//if

          //pressiona para esquerda
          if (pressed_key[ALLEGRO_KEY_LEFT]==true && goku.pos_x_sprite > 290 && !
            ((((goku.pos_y_sprite >= 51 && goku.pos_y_sprite <= 117)&&//posicao em y dos blocos 1 e 2
            ((goku.pos_x_sprite >= 589 && goku.pos_x_sprite <=745) ||//bloco 2
            (goku.pos_x_sprite >= 380 && goku.pos_x_sprite <=550))) ||//bloco 1
            ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 569) &&//posicao em y dos blocos 3 e 4
            ((goku.pos_x_sprite >= 594 && goku.pos_x_sprite <=745) ||//bloco 4
            (goku.pos_x_sprite >= 380 && goku.pos_x_sprite <= 550))) ||//bloco 3
            ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 77) &&//posicao em y dos blocos 5 e 7
            (goku.pos_x_sprite >= 315 && goku.pos_x_sprite <=366))||//bloco 5 e 7
            ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 81) &&//posicao em y dos blocos 6 e 8
            (goku.pos_x_sprite >= 775 && goku.pos_x_sprite <= 830))||//bloco 6 e 8
            ((goku.pos_y_sprite <= 649 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 9 e 10
            (goku.pos_x_sprite >= 315 && goku.pos_x_sprite <=366))||//bloco 9 e 10
            ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 11 e 12
            (goku.pos_x_sprite >= 785 && goku.pos_x_sprite <= 830))||//blocos 11 e 12
            ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 13
            (goku.pos_x_sprite >= 330 && goku.pos_x_sprite <= 441))||//bloco 13
            ((goku.pos_y_sprite <= 539 && goku.pos_y_sprite >= 480) &&//posicao em y do bloco 16
            (goku.pos_x_sprite >= 330 && goku.pos_x_sprite <=441))||//bloco 16
            ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 14
            (goku.pos_x_sprite >= 550 && goku.pos_x_sprite <= 651))||//bloco 14
            ((goku.pos_y_sprite <= 519 && goku.pos_y_sprite >= 461) &&//posicao em y do bloco 17
            (goku.pos_x_sprite >= 550 && goku.pos_x_sprite <= 651))||//bloco 17
            ((goku.pos_y_sprite <= 409 && goku.pos_y_sprite >= 289) &&//posicao em y do bloco 19
            (goku.pos_x_sprite >= 250 && goku.pos_x_sprite <= 363))||//bloco 19
            ((goku.pos_y_sprite <= 429 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
            ((goku.pos_x_sprite >= 690 && goku.pos_x_sprite <= 750) ||
            (goku.pos_x_sprite >= 420 && goku.pos_x_sprite <= 475)))||//bloco 21
            ((goku.pos_y_sprite <= 289 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
            (goku.pos_x_sprite >= 440 && goku.pos_x_sprite <= 550))))){//bloco 21
              goku.pos_x_sprite -= 2;
              goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
              goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
            }//if

            //pressiona para direita
            else if(pressed_key[ALLEGRO_KEY_RIGHT]==true && goku.pos_x_sprite < 850 && !
              ((((goku.pos_y_sprite >= 51 && goku.pos_y_sprite <= 117)&&//posicao em y dos blocos 1 e 2
              ((goku.pos_x_sprite >= 570 && goku.pos_x_sprite <= 737) ||//bloco 2
              (goku.pos_x_sprite >= 378 && goku.pos_x_sprite <= 549))) ||//bloco 1
              ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 569) &&//posicao em y dos blocos 3 e 4
              ((goku.pos_x_sprite >= 592 && goku.pos_x_sprite <= 737) ||//bloco 4
              (goku.pos_x_sprite >= 378 && goku.pos_x_sprite <= 549)))||//bloco 3
              ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 77) &&//posicao em y dos blocos 5 e 7
              (goku.pos_x_sprite >= 310 && goku.pos_x_sprite <= 360))||//bloco 5 e 7
              ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 81) &&//posicao em y dos blocos 6 e 8
              (goku.pos_x_sprite >= 770 && goku.pos_x_sprite <= 815))||//blocos 6 e 8
              ((goku.pos_y_sprite <= 649 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 9 e 10
              (goku.pos_x_sprite >= 310 && goku.pos_x_sprite <= 360))||//blocos 9 e 10
              ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 11 e 12
              (goku.pos_x_sprite >= 776 && goku.pos_x_sprite <= 815))||//blocos 11 e 12
              ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 15
              (goku.pos_x_sprite >= 701 && goku.pos_x_sprite <= 820))||//bloco 15
              ((goku.pos_y_sprite <= 539 && goku.pos_y_sprite >= 481) &&//posicao em y do bloco 18
              (goku.pos_x_sprite >= 701 && goku.pos_x_sprite <= 820))||//bloco 18
              ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 14
              (goku.pos_x_sprite >= 491 && goku.pos_x_sprite <= 649))||//bloco 14
              ((goku.pos_y_sprite <= 519 && goku.pos_y_sprite >= 461) &&//posicao em y do bloco 17
              (goku.pos_x_sprite >= 491 && goku.pos_x_sprite <= 649))||//bloco 17
              ((goku.pos_y_sprite <= 409 && goku.pos_y_sprite >= 289) &&//posicao em y do bloco 20
              (goku.pos_x_sprite >= 780 && goku.pos_x_sprite <= 890))||//bloco 20
              ((goku.pos_y_sprite <= 429 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
              ((goku.pos_x_sprite >= 390 && goku.pos_x_sprite <= 450)||
              (goku.pos_x_sprite >= 675 && goku.pos_x_sprite <= 749)))||//bloco 21
              ((goku.pos_y_sprite <= 289 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
              (goku.pos_x_sprite >= 600 && goku.pos_x_sprite <= 749))))){// bloco 21
                goku.pos_x_sprite += 2;
                goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
                goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
              }//else if


              desenha=1;
            }//if evento timer
            if (evento.type == ALLEGRO_EVENT_TIMER && desenha == 3) {
              /* code */
            }
            colisao = verificaColisao(goku,inimigo,inimigo2,inimigo3,semente1,semente2,semente3,captacolisao_goku,captacolisao_semente1,captacolisao_semente2,captacolisao_semente3);
            if (colisao == 1) {
              captacolisao_goku=1;
            }//if
            if (colisao == 2){
              captacolisao_semente1=1;
            }//if
            if (colisao == 3) {
              captacolisao_inimigo2=1;
            }//if
            if (colisao == 4) {
              captacolisao_inimigo1=1;
            }//if
            if (colisao == 5) {
              captacolisao_inimigo3=1;
            }//if
            if (colisao == 6) {
              captacolisao_semente2=1;
            }//if
            if (colisao == 7) {
              captacolisao_semente3=1;

            }//if
            else if (colisao == 0) {

            }//if

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
              //caixa de texto para confirmar sair do jogo
              //exibe o titulo do jogo
              char tcaixa[50] = "Menu";
              //exibe se deseja sair do jogo
              char titulo[100] = "Atenção";
              //exibe mensagem de aviso
              char texto[200] = "Deseja mesmo sair?";
              //mostra a caixa de texto
              int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
              //al_get_current_display() - retorna a janela ativa;
              printf("%i",r);
              if(r==true){
                return 0;
              }//if
            }//else do X na tela
            if (captacolisao_goku > 0) {
              desenha = 3;
            }

            if ((captacolisao_inimigo1>0 && captacolisao_inimigo2 > 0 )&& captacolisao_inimigo3 > 0) {
              desenha = 2;
            }//if

            /* -- ATUALIZA TELA -- */
            if(desenha && al_is_event_queue_empty(fila_eventos)) {
              al_get_keyboard_state(&keys);
              //desenha o fim do jogo - WIINNN
              if (desenha == 2) {
                al_draw_bitmap_region(fundofim,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);


                  if (!concluido){
                    nomeRanking(evento);
                    if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                      concluido = true;
                    }//if
                    al_draw_textf(fonte,al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "VITÓRIA! ");
                    al_draw_textf(fonte,al_map_rgb(0,0, 0), 100,
                    (ALTURA_TELA / 2 - al_get_font_ascent(fonte)) / 2,
                    ALLEGRO_ALIGN_LEFT, "Nome:");
                    if (strlen(str) > 0){
                      al_draw_text(fonte,al_map_rgb(0, 0, 0), 300, 160,
                      // (ALTURA_TELA - al_get_font_ascent(fonte)) / 2,
                      ALLEGRO_ALIGN_LEFT, str);
                    }//if
                  }//if
                  else{
                    al_draw_textf(fonte, al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "Ranking");
                    al_draw_textf(fonte, al_map_rgb(0,0,0),100 , 150, ALLEGRO_ALIGN_LEFT, "1º Lugar");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 500, 150, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[0].nome);
                    al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 150, ALLEGRO_ALIGN_LEFT, "Tempo:");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 150, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[0].minutos,listaRanking[0].segundos);

                    al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 250, ALLEGRO_ALIGN_LEFT, "2º Lugar");
                    al_draw_textf(fonte, al_map_rgb(72,61,139),500,250, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[1].nome);
                    al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 250, ALLEGRO_ALIGN_LEFT, "Tempo:");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 250, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[1].minutos,listaRanking[1].segundos);

                    al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 350, ALLEGRO_ALIGN_LEFT, "3º Lugar");
                    al_draw_textf(fonte, al_map_rgb(72,61,139),500,350, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[2].nome);
                    al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 350, ALLEGRO_ALIGN_LEFT, "Tempo:");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 350, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[2].minutos,listaRanking[2].segundos);


                  }



                  //al_draw_bitmap(fundo, 0, 0, 0);

                  al_flip_display();

                }//if GAME OVER You winn

                if (desenha == 3) {
                  i--;
                  if (i>0) {
                    return i;
                  }
                  al_draw_bitmap_region(fundofim,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);

                  if (!concluido){

                    nomeRanking(evento);


                    if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                      concluido = true;
                    }//if
                  }//if

                  //al_draw_bitmap(fundo, 0, 0, 0);
                  if (!concluido){
                    al_draw_textf(fonte,al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "PERDEU! ");
                    al_draw_textf(fonte,al_map_rgb(0,0, 0), 100,
                    (ALTURA_TELA / 2 - al_get_font_ascent(fonte)) / 2,
                    ALLEGRO_ALIGN_LEFT, "Nome:");
                    if (strlen(str) > 0){
                      al_draw_text(fonte,al_map_rgb(0, 0, 0), 300, 160,
                      // (ALTURA_TELA - al_get_font_ascent(fonte)) / 2,
                      ALLEGRO_ALIGN_LEFT, str);
                    }//if
                  }//if
                  else{
                    al_draw_textf(fonte, al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "Ranking");
                    al_draw_textf(fonte, al_map_rgb(0,0,0),100 , 150, ALLEGRO_ALIGN_LEFT, "1º Lugar");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 500, 150, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[0].nome);
                    al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 150, ALLEGRO_ALIGN_LEFT, "Tempo:");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 150, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[0].minutos,listaRanking[0].segundos);

                    al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 250, ALLEGRO_ALIGN_LEFT, "2º Lugar");
                    al_draw_textf(fonte, al_map_rgb(72,61,139),500,250, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[1].nome);
                    al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 250, ALLEGRO_ALIGN_LEFT, "Tempo:");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 250, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[1].minutos,listaRanking[1].segundos);

                    al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 350, ALLEGRO_ALIGN_LEFT, "3º Lugar");
                    al_draw_textf(fonte, al_map_rgb(72,61,139),500,350, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[2].nome);
                    al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 350, ALLEGRO_ALIGN_LEFT, "Tempo:");
                    al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 350, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[2].minutos,listaRanking[2].segundos);


                  }
                  al_flip_display();
                }


                //else jogo rodando
                else if (desenha ==1 ){
                  desenha = 0;
                  milisegundos--;
                  if(milisegundos%60==0){
                    segundos--;
                    milisegundos=0;
                  }//if
                  if(segundos==-1){
                    minutos--;
                    segundos=59;
                  }//if
                  if (minutos == 0 && segundos == 0) {
                    desenha = 3;
                  }

                  //A cada 100 frames, atualiza cor do titulo
                  al_draw_bitmap_region(fundo,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);
                  al_draw_textf(fonte,al_map_rgb(0,0,0),10,40, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte,al_map_rgb(0,0,0),1100,40, ALLEGRO_ALIGN_RIGHT, "Vida:");
                  al_draw_textf(fonte,cor_tempo ,10,130, ALLEGRO_ALIGN_LEFT, "%02d:%02d",minutos, segundos);

                  al_draw_bitmap_region(folha_sprite4,
                    vidas.regiao_x_folha , vidas.regiao_y_folha,
                    vidas.largura_sprite - (80*(4-i)),vidas.altura_sprite,
                    vidas.pos_x_sprite,vidas.pos_y_sprite,1);
                    if(minutos<=1){
                      cor_tempo = al_map_rgb(rand()%255, rand()%34, rand()%34);
                    }//if
                    else cor_tempo= al_map_rgb(0,0,0);

                    //velocidade positiva (movendo para direita)
                    if (goku.vel_x_sprite>0){
                      if (captacolisao_goku<=0) {
                        al_draw_bitmap_region(folha_sprite,
                          goku.regiao_x_folha,goku.regiao_y_folha,
                          goku.largura_sprite,goku.altura_sprite,
                          goku.pos_x_sprite,goku.pos_y_sprite,0);
                        }//if
                      }//if

                      //faz virar pra esquerda invertendo a sprite
                      else{
                        if (captacolisao_goku<=0) {
                          al_draw_bitmap_region(folha_sprite,
                            goku.regiao_x_folha,goku.regiao_y_folha,
                            goku.largura_sprite,goku.altura_sprite,
                            goku.pos_x_sprite,goku.pos_y_sprite,1);
                          }//if
                        }//else
                        if (inimigo.vel_x_sprite > 0) {
                          if (captacolisao_inimigo1<=0) {
                            al_draw_bitmap_region(folha_sprite2,
                              inimigo.regiao_x_folha,inimigo.regiao_y_folha,
                              inimigo.largura_sprite,inimigo.altura_sprite,
                              inimigo.pos_x_sprite,inimigo.pos_y_sprite,0);
                            }//if
                          }//if
                          else{
                            if (captacolisao_inimigo1<=0) {
                              al_draw_bitmap_region(folha_sprite2,
                                inimigo.regiao_x_folha,inimigo.regiao_y_folha,
                                inimigo.largura_sprite,inimigo.altura_sprite,
                                inimigo.pos_x_sprite,inimigo.pos_y_sprite,1);
                              }//if
                            }//else
                            if (inimigo2.vel_x_sprite > 0) {
                              if (captacolisao_inimigo2 <=0) {
                                al_draw_bitmap_region(folha_sprite2,
                                  inimigo2.regiao_x_folha,inimigo2.regiao_y_folha,
                                  inimigo2.largura_sprite,inimigo2.altura_sprite,
                                  inimigo2.pos_x_sprite,inimigo2.pos_y_sprite,0);
                                }//if
                              }//if
                              else{
                                if (captacolisao_inimigo2 <=0) {
                                  al_draw_bitmap_region(folha_sprite2,
                                    inimigo2.regiao_x_folha,inimigo2.regiao_y_folha,
                                    inimigo2.largura_sprite,inimigo2.altura_sprite,
                                    inimigo2.pos_x_sprite,inimigo2.pos_y_sprite,1);
                                  }//if
                                }//else
                                if (inimigo3.vel_x_sprite > 0) {
                                  if (captacolisao_inimigo3 <=0) {
                                    al_draw_bitmap_region(folha_sprite2,
                                      inimigo3.regiao_x_folha,inimigo3.regiao_y_folha,
                                      inimigo3.largura_sprite,inimigo3.altura_sprite,
                                      inimigo3.pos_x_sprite,inimigo3.pos_y_sprite,0);
                                    }//if
                                  }//if
                                  else{
                                    if (captacolisao_inimigo3 <=0) {
                                      al_draw_bitmap_region(folha_sprite2,
                                        inimigo3.regiao_x_folha,inimigo3.regiao_y_folha,
                                        inimigo3.largura_sprite,inimigo3.altura_sprite,
                                        inimigo3.pos_x_sprite,inimigo3.pos_y_sprite,1);
                                      }//if
                                    }//else
                                    if (captacolisao_semente1 <=0) {
                                      al_draw_bitmap_region(folha_sprite3,
                                        semente1.regiao_x_folha,semente1.regiao_y_folha,
                                        semente1.largura_sprite,semente1.altura_sprite,
                                        semente1.pos_x_sprite,semente1.pos_y_sprite,0);
                                      }//if
                                      if (captacolisao_semente2 <=0) {
                                        al_draw_bitmap_region(folha_sprite3,
                                          semente2.regiao_x_folha,semente2.regiao_y_folha,
                                          semente2.largura_sprite,semente2.altura_sprite,
                                          semente2.pos_x_sprite,semente2.pos_y_sprite,0);
                                        }//if
                                        if (captacolisao_semente3 <=0) {
                                          al_draw_bitmap_region(folha_sprite3,
                                            semente3.regiao_x_folha,semente3.regiao_y_folha,
                                            semente3.largura_sprite,semente3.altura_sprite,
                                            semente3.pos_x_sprite,semente3.pos_y_sprite,0);
                                          }//if




                                          al_flip_display();
                                        }// else jogo rodando
                                      }//if desenha events
                                    }//while
                                    return 0;
                                  }//funcao atualizar Goku
int Atualizarjogo2(Personagem goku, Personagem inimigo, Personagem inimigo2,Personagem inimigo3,Personagem semente1,Personagem semente2,Personagem semente3,int j,Personagem vidas){
  int desenha = 1;
  //int sair = 0;
  int captacolisao_goku = 0;
  int captacolisao_inimigo1 = 0;
  int captacolisao_inimigo2 = 0;
  int captacolisao_inimigo3 = 0;
  int captacolisao_semente1 = 0;
  int captacolisao_semente2 = 0;
  int captacolisao_semente3 = 0;
  int atualiza_x_1 = 2;
  int atualiza_x_2 = 2;
  int atualiza_x_3 = 2;
  bool pressed_key[ALLEGRO_KEY_MAX]  ;
  pressed_key[ALLEGRO_KEY_UP] = false;
  pressed_key[ALLEGRO_KEY_DOWN] = false;
  pressed_key[ALLEGRO_KEY_LEFT] = false;
  pressed_key[ALLEGRO_KEY_RIGHT] = false;
  bool done = false;
  int colisao;
  int cont = 0;
  //largura e altura de cada sprite dentro da folha
  goku.altura_sprite=68; goku.largura_sprite=34;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  goku.colunas_folha=10, goku.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  goku.linha_atual=3, goku.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  goku.regiao_x_folha=0, goku.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  goku.frames_sprite=0, goku.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  goku.pos_x_sprite=300, goku.pos_y_sprite=260;
  //velocidade X Y que o sprite ira se mover pela janela
  goku.vel_x_sprite=13, goku.vel_y_sprite=13;
  // inicializarGoku(goku);

  //largura e altura de cada sprite dentro da folha
  inimigo.altura_sprite=80; inimigo.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo.colunas_folha=3, inimigo.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo.linha_atual=1, inimigo.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo.regiao_x_folha=0, inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo.frames_sprite=10, inimigo.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo.pos_x_sprite=400, inimigo.pos_y_sprite=50;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo.vel_x_sprite=13, inimigo.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  inimigo2.altura_sprite=80; inimigo2.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo2.colunas_folha=3, inimigo2.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo2.linha_atual=1, inimigo2.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo2.regiao_x_folha=0, inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo2.frames_sprite=10, inimigo2.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo2.pos_x_sprite=400, inimigo2.pos_y_sprite=550;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo2.vel_x_sprite=13, inimigo2.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  inimigo3.altura_sprite=80; inimigo3.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo3.colunas_folha=3, inimigo3.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo3.linha_atual=1, inimigo3.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo3.regiao_x_folha=0, inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo3.frames_sprite=10, inimigo3.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo3.pos_x_sprite=400, inimigo3.pos_y_sprite=210;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo3.vel_x_sprite=13, inimigo3.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente1.altura_sprite=30; semente1.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente1.colunas_folha=10, semente1.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente1.linha_atual=3, semente1.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente1.regiao_x_folha=0, semente1.regiao_y_folha=0;
  //quantos frames devem 480passar para atualizar pa 570o proximo sprite
  semente1.frames_sprite=0, semente1.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente1.pos_x_sprite= 740, semente1.pos_y_sprite=70;
  //velocidade X Y que o sprite ira se mover pela janela
  semente1.vel_x_sprite=13, semente1.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente2.altura_sprite=30; semente2.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente2.colunas_folha=10, semente2.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente2.linha_atual=3, semente2.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente2.regiao_x_folha=0, semente2.regiao_y_folha=0;
  //quantos frames devem 500passar para atualizar paa o proximo sprite
  semente2.frames_sprite=0, semente2.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente2.pos_x_sprite= 610, semente2.pos_y_sprite= 380;
  //velocidade X Y que o sprite ira se mover pela janela
  semente2.vel_x_sprite=13, semente2.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente3.altura_sprite=30; semente3.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente3.colunas_folha=10, semente3.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente3.linha_atual=3, semente3.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente3.regiao_x_folha=0, semente3.regiao_y_folha=0;
  //quantos frames devem 560passar para atualizar pa630 o proximo sprite
  semente3.frames_sprite=0, semente3.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente3.pos_x_sprite= 290, semente3.pos_y_sprite= 670;
  //velocidade X Y que o sprite ira se mover pela janela
  semente3.vel_x_sprite=13, semente3.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  vidas.altura_sprite=110; vidas.largura_sprite=270;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  vidas.colunas_folha=4, vidas.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  vidas.linha_atual=1, vidas.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  vidas.regiao_x_folha = 0;
  vidas.regiao_y_folha = 0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  vidas.frames_sprite=0, vidas.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  vidas.pos_x_sprite= 920, vidas.pos_y_sprite=40;
  //velocidade X Y que o sprite ira se mover pela janela
  vidas.vel_x_sprite=13, vidas.vel_y_sprite=13;

  if (j==2) {
    minutos = 2;
    segundos = 0;

  }

  ALLEGRO_KEYBOARD_STATE keys;

  while(!done){
    al_wait_for_event(fila_eventos, &evento);
    if(evento.type == ALLEGRO_EVENT_KEY_DOWN) {
      al_get_keyboard_state(&keys);
      char tcaixa[50] = "Pausado",titulo[100] = "Jogo Pausado", texto[200] = "Deseja fechar o jogo?";
      int r;
      //se apertar nas teclas
      switch(evento.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        if (goku.pos_y_sprite > 20 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_UP]=true;
          goku.linha_atual=3;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
        }//if
        break;

        case ALLEGRO_KEY_DOWN:
        if (goku.pos_y_sprite + goku.altura_sprite < ALTURA_TELA -20 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_DOWN]=true;
          goku.linha_atual=1;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
        }//if
        break;

        case ALLEGRO_KEY_LEFT:
        if (goku.pos_x_sprite > 270 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_LEFT]=true;
          goku.linha_atual = 2;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
          goku.vel_x_sprite = -1;
        }//if
        break;

        case ALLEGRO_KEY_RIGHT:
        if (goku.pos_x_sprite + goku.altura_sprite < 902 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_RIGHT]=true;
          goku.linha_atual = 2;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
          goku.vel_x_sprite = 1;
        }//if
        break;

        case ALLEGRO_KEY_ESCAPE:
        //mostra a caixa de texto
        r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        printf("%i",r);
        if(r==true){
          done=true;
        }//if
        break;
      }//switch
    }//if evento movimentação apertando teclado

    else if(evento.type == ALLEGRO_EVENT_KEY_UP) {
      al_get_keyboard_state(&keys);
      //se soltar a tecla
      switch(evento.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        pressed_key[ALLEGRO_KEY_UP]=false;
        break;

        case ALLEGRO_KEY_DOWN:
        pressed_key[ALLEGRO_KEY_DOWN]=false;
        break;

        case ALLEGRO_KEY_LEFT:
        pressed_key[ALLEGRO_KEY_LEFT]=false;
        break;

        case ALLEGRO_KEY_RIGHT:
        pressed_key[ALLEGRO_KEY_RIGHT]=false;
        break;
      }//switch

      desenha=1;
    }//if se solta a tecla

    if (evento.type == ALLEGRO_EVENT_TIMER) {
      if (atualiza_x_1 > 0 &&(inimigo.pos_x_sprite >= 790 || inimigo.pos_x_sprite < 270)){
        atualiza_x_1*= -1;
        inimigo.vel_x_sprite = 1;
        inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
      }//if
      else if(atualiza_x_1 < 0 &&(inimigo.pos_x_sprite >= 790 || inimigo.pos_x_sprite < 270)){
        atualiza_x_1*= -1;
        inimigo.vel_x_sprite = -1;
        inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
      }//else if
      if (atualiza_x_2 > 0 &&(inimigo2.pos_x_sprite >= 750 || inimigo2.pos_x_sprite < 350)){
        atualiza_x_2*= -1;
        inimigo2.vel_x_sprite = 1;
        inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
      }//if
      else if(atualiza_x_2 < 0 &&(inimigo2.pos_x_sprite >= 750 || inimigo2.pos_x_sprite < 350)){
        atualiza_x_2*= -1;
        inimigo2.vel_x_sprite = -1;
        inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
      }//else if
      if (atualiza_x_3 > 0 &&(inimigo3.pos_x_sprite >= 750 || inimigo3.pos_x_sprite < 350)){
        atualiza_x_3*= -1;
        inimigo3.vel_x_sprite = 1;
        inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
      }//if
      else if(atualiza_x_3 < 0 &&(inimigo3.pos_x_sprite >= 750 || inimigo3.pos_x_sprite < 350)){
        atualiza_x_3*= -1;
        inimigo3.vel_x_sprite = -1;
        inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
      }//else if
      //fantasma se move para direita
      //if(inimigo.pos_y_sprite + inimigo.altura_sprite > ALTURA_TELA -20 || inimigo.pos_y_sprite < 20/*&& !((inimigo.pos_y_sprite >= 201 && inimigo.pos_y_sprite <= 249) && (inimigo.pos_x_sprite >= 300 && inimigo.pos_x_sprite <= 400) || (inimigo.pos_x_sprite >= 150 && inimigo.pos_x_sprite <= 250))*/)
      inimigo.pos_x_sprite -= atualiza_x_1;
      inimigo2.pos_x_sprite -= atualiza_x_2;
      inimigo3.pos_x_sprite -= atualiza_x_3;
      cont++;
      if (cont == 15) {
        inimigo.coluna_atual = (inimigo.coluna_atual+1)% inimigo.colunas_folha;
        inimigo.regiao_x_folha = inimigo.coluna_atual * inimigo.largura_sprite;
        inimigo2.coluna_atual = (inimigo2.coluna_atual+1)% inimigo2.colunas_folha;
        inimigo2.regiao_x_folha = inimigo2.coluna_atual * inimigo2.largura_sprite;
        inimigo3.coluna_atual = (inimigo3.coluna_atual+1)% inimigo3.colunas_folha;
        inimigo3.regiao_x_folha = inimigo3.coluna_atual * inimigo3.largura_sprite;
        cont = 0;
      }//if
      //pressiona para cima (GOKU)
      if (pressed_key[ALLEGRO_KEY_UP]==true && goku.pos_y_sprite > 40 &&
        ((goku.pos_y_sprite != 118 || goku.pos_x_sprite < 580 || goku.pos_x_sprite > 745) &&//bloco 2
        (goku.pos_y_sprite != 118 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 540)) &&//bloco 1
        ((goku.pos_y_sprite != 642 || goku.pos_x_sprite < 595 || goku.pos_x_sprite > 744) &&//bloco 4
        (goku.pos_y_sprite != 642 || goku.pos_x_sprite < 398 || goku.pos_x_sprite > 549)) &&//bloco 3
        ((goku.pos_y_sprite != 260 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 5 e 7
        (goku.pos_y_sprite != 260 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//blocos 6 e 8
        ((goku.pos_y_sprite != 650 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 9 e 10
        (goku.pos_y_sprite != 630 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//bloco 11 e 12
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 13
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 15
        (goku.pos_y_sprite != 540 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 16
        (goku.pos_y_sprite != 540 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 18
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 14
        (goku.pos_y_sprite != 520 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 17
        (goku.pos_y_sprite != 410 || goku.pos_x_sprite < 290 || goku.pos_x_sprite > 360)  &&//bloco 19
        (goku.pos_y_sprite != 410 || goku.pos_x_sprite < 779 || goku.pos_x_sprite > 890)  &&//bloco 20
        (goku.pos_y_sprite != 430 || goku.pos_x_sprite < 410 || goku.pos_x_sprite > 735)  &&//bloco 21
        (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 420 || goku.pos_x_sprite > 549)  &&//bloco 21
        (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 599 || goku.pos_x_sprite > 735)){//bloco 21
          goku.pos_y_sprite -= 2;
          goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
          goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
        }//if

        //pressiona para baixo
        else if(pressed_key[ALLEGRO_KEY_DOWN]==true && goku.pos_y_sprite < 680 &&
          ((goku.pos_y_sprite != 50 || goku.pos_x_sprite < 580 || goku.pos_x_sprite > 745)  &&//bloco 2
          (goku.pos_y_sprite != 50 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 540))  &&//bloco 1
          ((goku.pos_y_sprite != 566 || goku.pos_x_sprite < 595 || goku.pos_x_sprite > 746) &&//bloco 4
          (goku.pos_y_sprite != 566 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 551)) &&//bloco 3
          ((goku.pos_y_sprite != 76 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361)  &&//blocos 5 e 7
          (goku.pos_y_sprite != 80 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830))  &&//blocos 6 e 8
          ((goku.pos_y_sprite != 444 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 9 e 10
          (goku.pos_y_sprite != 444 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//bloco 11 e 12
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 13
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 15
          (goku.pos_y_sprite != 480 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 16
          (goku.pos_y_sprite != 480 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 18
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 14
          (goku.pos_y_sprite != 460 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 17
          (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 290 || goku.pos_x_sprite > 360)  &&//bloco 19
          (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 779 || goku.pos_x_sprite > 890)  &&//bloco 20
          (goku.pos_y_sprite != 360 || goku.pos_x_sprite < 410 || goku.pos_x_sprite > 730)  &&//bloco 21
          (goku.pos_y_sprite != 220 || goku.pos_x_sprite < 420 || goku.pos_x_sprite > 549)  &&//bloco 21
          (goku.pos_y_sprite != 220 || goku.pos_x_sprite < 599 || goku.pos_x_sprite > 750)){//bloco 21
            goku.pos_y_sprite += 2;
            goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
            goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
          }//if

          //pressiona para esquerda
          if (pressed_key[ALLEGRO_KEY_LEFT]==true && goku.pos_x_sprite > 290 && !
            ((((goku.pos_y_sprite >= 51 && goku.pos_y_sprite <= 117)&&//posicao em y dos blocos 1 e 2
            ((goku.pos_x_sprite >= 589 && goku.pos_x_sprite <=745) ||//bloco 2
            (goku.pos_x_sprite >= 380 && goku.pos_x_sprite <=550))) ||//bloco 1
            ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 569) &&//posicao em y dos blocos 3 e 4
            ((goku.pos_x_sprite >= 594 && goku.pos_x_sprite <=745) ||//bloco 4
            (goku.pos_x_sprite >= 380 && goku.pos_x_sprite <= 550))) ||//bloco 3
            ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 77) &&//posicao em y dos blocos 5 e 7
            (goku.pos_x_sprite >= 315 && goku.pos_x_sprite <=366))||//bloco 5 e 7
            ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 81) &&//posicao em y dos blocos 6 e 8
            (goku.pos_x_sprite >= 775 && goku.pos_x_sprite <= 830))||//bloco 6 e 8
            ((goku.pos_y_sprite <= 649 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 9 e 10
            (goku.pos_x_sprite >= 315 && goku.pos_x_sprite <=366))||//bloco 9 e 10
            ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 11 e 12
            (goku.pos_x_sprite >= 785 && goku.pos_x_sprite <= 830))||//blocos 11 e 12
            ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 13
            (goku.pos_x_sprite >= 330 && goku.pos_x_sprite <= 441))||//bloco 13
            ((goku.pos_y_sprite <= 539 && goku.pos_y_sprite >= 480) &&//posicao em y do bloco 16
            (goku.pos_x_sprite >= 330 && goku.pos_x_sprite <=441))||//bloco 16
            ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 14
            (goku.pos_x_sprite >= 550 && goku.pos_x_sprite <= 651))||//bloco 14
            ((goku.pos_y_sprite <= 519 && goku.pos_y_sprite >= 461) &&//posicao em y do bloco 17
            (goku.pos_x_sprite >= 550 && goku.pos_x_sprite <= 651))||//bloco 17
            ((goku.pos_y_sprite <= 409 && goku.pos_y_sprite >= 289) &&//posicao em y do bloco 19
            (goku.pos_x_sprite >= 250 && goku.pos_x_sprite <= 363))||//bloco 19
            ((goku.pos_y_sprite <= 429 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
            ((goku.pos_x_sprite >= 690 && goku.pos_x_sprite <= 750) ||
            (goku.pos_x_sprite >= 420 && goku.pos_x_sprite <= 475)))||//bloco 21
            ((goku.pos_y_sprite <= 289 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
            (goku.pos_x_sprite >= 440 && goku.pos_x_sprite <= 550))))){//bloco 21
              goku.pos_x_sprite -= 2;
              goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
              goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
            }//if

            //pressiona para direita
            else if(pressed_key[ALLEGRO_KEY_RIGHT]==true && goku.pos_x_sprite < 850 && !
              ((((goku.pos_y_sprite >= 51 && goku.pos_y_sprite <= 117)&&//posicao em y dos blocos 1 e 2
              ((goku.pos_x_sprite >= 570 && goku.pos_x_sprite <= 737) ||//bloco 2
              (goku.pos_x_sprite >= 378 && goku.pos_x_sprite <= 549))) ||//bloco 1
              ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 569) &&//posicao em y dos blocos 3 e 4
              ((goku.pos_x_sprite >= 592 && goku.pos_x_sprite <= 737) ||//bloco 4
              (goku.pos_x_sprite >= 378 && goku.pos_x_sprite <= 549)))||//bloco 3
              ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 77) &&//posicao em y dos blocos 5 e 7
              (goku.pos_x_sprite >= 310 && goku.pos_x_sprite <= 360))||//bloco 5 e 7
              ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 81) &&//posicao em y dos blocos 6 e 8
              (goku.pos_x_sprite >= 770 && goku.pos_x_sprite <= 815))||//blocos 6 e 8
              ((goku.pos_y_sprite <= 649 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 9 e 10
              (goku.pos_x_sprite >= 310 && goku.pos_x_sprite <= 360))||//blocos 9 e 10
              ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 11 e 12
              (goku.pos_x_sprite >= 776 && goku.pos_x_sprite <= 815))||//blocos 11 e 12
              ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 15
              (goku.pos_x_sprite >= 701 && goku.pos_x_sprite <= 820))||//bloco 15
              ((goku.pos_y_sprite <= 539 && goku.pos_y_sprite >= 481) &&//posicao em y do bloco 18
              (goku.pos_x_sprite >= 701 && goku.pos_x_sprite <= 820))||//bloco 18
              ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 14
              (goku.pos_x_sprite >= 491 && goku.pos_x_sprite <= 649))||//bloco 14
              ((goku.pos_y_sprite <= 519 && goku.pos_y_sprite >= 461) &&//posicao em y do bloco 17
              (goku.pos_x_sprite >= 491 && goku.pos_x_sprite <= 649))||//bloco 17
              ((goku.pos_y_sprite <= 409 && goku.pos_y_sprite >= 289) &&//posicao em y do bloco 20
              (goku.pos_x_sprite >= 780 && goku.pos_x_sprite <= 890))||//bloco 20
              ((goku.pos_y_sprite <= 429 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
              ((goku.pos_x_sprite >= 390 && goku.pos_x_sprite <= 450)||
              (goku.pos_x_sprite >= 675 && goku.pos_x_sprite <= 749)))||//bloco 21
              ((goku.pos_y_sprite <= 289 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
              (goku.pos_x_sprite >= 600 && goku.pos_x_sprite <= 749))))){// bloco 21
                goku.pos_x_sprite += 2;
                goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
                goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
              }//else if

              desenha=1;
            }//if evento timer
            colisao = verificaColisao(goku,inimigo,inimigo2,inimigo3,semente1,semente2,semente3,captacolisao_goku,captacolisao_semente1,captacolisao_semente2,captacolisao_semente3);
            if (colisao == 1) {
              captacolisao_goku=1;
            }//if
            if (colisao == 2){
              captacolisao_semente1=1;
            }//if
            if (colisao == 3) {
              captacolisao_inimigo2=1;
            }//if
            if (colisao == 4) {
              captacolisao_inimigo1=1;
            }//if
            if (colisao == 5) {
              captacolisao_inimigo3=1;
            }//if
            if (colisao == 6) {
              captacolisao_semente2=1;
            }//if
            if (colisao == 7) {
              captacolisao_semente3=1;
            }//if
            else if (colisao == 0) {
            }//if

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
              //caixa de texto para confirmar sair do jogo
              //exibe o titulo do jogo
              char tcaixa[50] = "Menu";
              //exibe se deseja sair do jogo
              char titulo[100] = "Atenção";
              //exibe mensagem de aviso
              char texto[200] = "Deseja mesmo sair?";
              //mostra a caixa de texto
              int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
              //al_get_current_display() - retorna a janela ativa;
              printf("%i",r);
              if(r==true){
                return 0;
              }//if
            }//else do X na tela
            if (captacolisao_goku > 0) {
              desenha = 3;
            }

            if ((captacolisao_inimigo1>0 && captacolisao_inimigo2 > 0 )&& captacolisao_inimigo3 > 0) {
              desenha = 2;
            }//if

            /* -- ATUALIZA TELA -- */
            if(desenha && al_is_event_queue_empty(fila_eventos)) {
              al_get_keyboard_state(&keys);
              //desenha o fim do jogo - WIINNN
              if (desenha == 2) {
                al_draw_bitmap_region(fundofim,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);

                if (!concluido){
                  nomeRanking(evento);
                  if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                    concluido = true;
                  }//if
                }//if

                // al_draw_bitmap(fundo, 0, 0, 0);
                if (!concluido){
                  al_draw_textf(fonte,al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "VITÓRIA! ");
                  al_draw_textf(fonte,al_map_rgb(0,0, 0), 100,
                  (ALTURA_TELA / 2 - al_get_font_ascent(fonte)) / 2,
                  ALLEGRO_ALIGN_LEFT, "Nome:");
                  if (strlen(str) > 0){
                    al_draw_text(fonte,al_map_rgb(0, 0, 0), 300, 160,
                    // (ALTURA_TELA - al_get_font_ascent(fonte)) / 2,
                    ALLEGRO_ALIGN_LEFT, str);
                  }//if
                }//if
                else{
                  al_draw_textf(fonte, al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "Ranking");
                  al_draw_textf(fonte, al_map_rgb(0,0,0),100 , 150, ALLEGRO_ALIGN_LEFT, "1º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 500, 150, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[0].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 150, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 150, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[0].minutos,listaRanking[0].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 250, ALLEGRO_ALIGN_LEFT, "2º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,250, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[1].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 250, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 250, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[1].minutos,listaRanking[1].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 350, ALLEGRO_ALIGN_LEFT, "3º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,350, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[2].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 350, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 350, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[2].minutos,listaRanking[2].segundos);


                }
                al_flip_display();

              }//if GAME OVER You winn

              if (desenha == 3) {
                j--;
                if (j>0) {
                  return j;
                }
                al_draw_bitmap_region(fundofim,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);

                if (!concluido){
                  nomeRanking(evento);
                  if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                    concluido = true;
                  }//if
                }//if

                // al_draw_bitmap(fundo, 0, 0, 0);
                if (!concluido){
                  al_draw_textf(fonte,al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "PERDEU! ");
                  al_draw_textf(fonte,al_map_rgb(0,0, 0), 100,
                  (ALTURA_TELA / 2 - al_get_font_ascent(fonte)) / 2,
                  ALLEGRO_ALIGN_LEFT, "Nome:");
                  if (strlen(str) > 0){
                    al_draw_text(fonte,al_map_rgb(0, 0, 0), 300, 160,
                    // (ALTURA_TELA - al_get_font_ascent(fonte)) / 2,
                    ALLEGRO_ALIGN_LEFT, str);
                  }//if
                }//if
                else{
                  al_draw_textf(fonte, al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "Ranking");
                  al_draw_textf(fonte, al_map_rgb(0,0,0),100 , 150, ALLEGRO_ALIGN_LEFT, "1º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 500, 150, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[0].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 150, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 150, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[0].minutos,listaRanking[0].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 250, ALLEGRO_ALIGN_LEFT, "2º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,250, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[1].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 250, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 250, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[1].minutos,listaRanking[1].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 350, ALLEGRO_ALIGN_LEFT, "3º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,350, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[2].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 350, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 350, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[2].minutos,listaRanking[2].segundos);


                }
                al_flip_display();
              }



              //else jogo rodando
              else if (desenha ==1 ){
                desenha = 0;
                milisegundos--;
                if(milisegundos%60==0){
                  segundos--;
                  milisegundos=0;
                }//if
                if(segundos==-1){
                  minutos--;
                  segundos=59;
                }//if

                //A cada 100 frames, atualiza cor do titulo
                al_draw_bitmap_region(fundo,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);
                al_draw_textf(fonte,al_map_rgb(0,0,0),1100,40, ALLEGRO_ALIGN_RIGHT, "Vida:");
                al_draw_textf(fonte,al_map_rgb(0,0,0),10,40, ALLEGRO_ALIGN_LEFT, "Tempo:");
                al_draw_textf(fonte,cor_tempo ,10,130, ALLEGRO_ALIGN_LEFT, "%02d:%02d",minutos, segundos);

                al_draw_bitmap_region(folha_sprite4,
                  vidas.regiao_x_folha , vidas.regiao_y_folha,
                  vidas.largura_sprite - (80*(2-j)),vidas.altura_sprite,
                  vidas.pos_x_sprite,vidas.pos_y_sprite,1);

                  if(minutos<=1){
                    cor_tempo = al_map_rgb(rand()%255, rand()%34, rand()%34);
                  }//if
                  else cor_tempo= al_map_rgb(0,0,0);

                  //velocidade positiva (movendo para direita)
                  if (goku.vel_x_sprite>0){
                    if (captacolisao_goku<=0) {
                      al_draw_bitmap_region(folha_sprite,
                        goku.regiao_x_folha,goku.regiao_y_folha,
                        goku.largura_sprite,goku.altura_sprite,
                        goku.pos_x_sprite,goku.pos_y_sprite,0);
                      }//if
                    }//if

                    //faz virar pra esquerda invertendo a sprite
                    else{
                      if (captacolisao_goku<=0) {
                        al_draw_bitmap_region(folha_sprite,
                          goku.regiao_x_folha,goku.regiao_y_folha,
                          goku.largura_sprite,goku.altura_sprite,
                          goku.pos_x_sprite,goku.pos_y_sprite,1);
                        }//if
                      }//else
                      if (inimigo.vel_x_sprite > 0) {
                        if (captacolisao_inimigo1<=0) {
                          al_draw_bitmap_region(folha_sprite2,
                            inimigo.regiao_x_folha,inimigo.regiao_y_folha,
                            inimigo.largura_sprite,inimigo.altura_sprite,
                            inimigo.pos_x_sprite,inimigo.pos_y_sprite,0);
                          }//if
                        }//if
                        else{
                          if (captacolisao_inimigo1<=0) {
                            al_draw_bitmap_region(folha_sprite2,
                              inimigo.regiao_x_folha,inimigo.regiao_y_folha,
                              inimigo.largura_sprite,inimigo.altura_sprite,
                              inimigo.pos_x_sprite,inimigo.pos_y_sprite,1);
                            }//if
                          }//else
                          if (inimigo2.vel_x_sprite > 0) {
                            if (captacolisao_inimigo2 <=0) {
                              al_draw_bitmap_region(folha_sprite2,
                                inimigo2.regiao_x_folha,inimigo2.regiao_y_folha,
                                inimigo2.largura_sprite,inimigo2.altura_sprite,
                                inimigo2.pos_x_sprite,inimigo2.pos_y_sprite,0);
                              }//if
                            }//if
                            else{
                              if (captacolisao_inimigo2 <=0) {
                                al_draw_bitmap_region(folha_sprite2,
                                  inimigo2.regiao_x_folha,inimigo2.regiao_y_folha,
                                  inimigo2.largura_sprite,inimigo2.altura_sprite,
                                  inimigo2.pos_x_sprite,inimigo2.pos_y_sprite,1);
                                }//if
                              }//else
                              if (inimigo3.vel_x_sprite > 0) {
                                if (captacolisao_inimigo3 <=0) {
                                  al_draw_bitmap_region(folha_sprite2,
                                    inimigo3.regiao_x_folha,inimigo3.regiao_y_folha,
                                    inimigo3.largura_sprite,inimigo3.altura_sprite,
                                    inimigo3.pos_x_sprite,inimigo3.pos_y_sprite,0);
                                  }//if
                                }//if
                                else{
                                  if (captacolisao_inimigo3 <=0) {
                                    al_draw_bitmap_region(folha_sprite2,
                                      inimigo3.regiao_x_folha,inimigo3.regiao_y_folha,
                                      inimigo3.largura_sprite,inimigo3.altura_sprite,
                                      inimigo3.pos_x_sprite,inimigo3.pos_y_sprite,1);
                                    }//if
                                  }//else
                                  if (captacolisao_semente1 <=0) {
                                    al_draw_bitmap_region(folha_sprite3,
                                      semente1.regiao_x_folha,semente1.regiao_y_folha,
                                      semente1.largura_sprite,semente1.altura_sprite,
                                      semente1.pos_x_sprite,semente1.pos_y_sprite,0);
                                    }//if
                                    if (captacolisao_semente2 <=0) {
                                      al_draw_bitmap_region(folha_sprite3,
                                        semente2.regiao_x_folha,semente2.regiao_y_folha,
                                        semente2.largura_sprite,semente2.altura_sprite,
                                        semente2.pos_x_sprite,semente2.pos_y_sprite,0);
                                      }//if
                                      if (captacolisao_semente3 <=0) {
                                        al_draw_bitmap_region(folha_sprite3,
                                          semente3.regiao_x_folha,semente3.regiao_y_folha,
                                          semente3.largura_sprite,semente3.altura_sprite,
                                          semente3.pos_x_sprite,semente3.pos_y_sprite,0);
                                        }//if
                                        al_flip_display();
                                      }// else jogo rodando
                                    }//if desenha events
                                  }//while
                                  return 0;
}//funcao atualizar Goku

int Atualizarjogo3(Personagem goku, Personagem inimigo, Personagem inimigo2,Personagem inimigo3,Personagem semente1,Personagem semente2,Personagem semente3,Personagem vidas){
  int desenha = 1;
  //int sair = 0;
  int captacolisao_goku = 0;
  int captacolisao_inimigo1 = 0;
  int captacolisao_inimigo2 = 0;
  int captacolisao_inimigo3 = 0;
  int captacolisao_semente1 = 0;
  int captacolisao_semente2 = 0;
  int captacolisao_semente3 = 0;
  int atualiza_x_1 = 2;
  int atualiza_x_2 = 2;
  int atualiza_x_3 = 2;
  bool pressed_key[ALLEGRO_KEY_MAX]  ;
  pressed_key[ALLEGRO_KEY_UP] = false;
  pressed_key[ALLEGRO_KEY_DOWN] = false;
  pressed_key[ALLEGRO_KEY_LEFT] = false;
  pressed_key[ALLEGRO_KEY_RIGHT] = false;
  bool done = false;
  int colisao;
  int cont = 0;
  //largura e altura de cada sprite dentro da folha
  goku.altura_sprite=68; goku.largura_sprite=34;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  goku.colunas_folha=10, goku.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  goku.linha_atual=3, goku.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  goku.regiao_x_folha=0, goku.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  goku.frames_sprite=0, goku.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  goku.pos_x_sprite=300, goku.pos_y_sprite=260;
  //velocidade X Y que o sprite ira se mover pela janela
  goku.vel_x_sprite=13, goku.vel_y_sprite=13;
  // inicializarGoku(goku);

  //largura e altura de cada sprite dentro da folha
  inimigo.altura_sprite=80; inimigo.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo.colunas_folha=3, inimigo.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo.linha_atual=1, inimigo.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo.regiao_x_folha=0, inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo.frames_sprite=10, inimigo.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo.pos_x_sprite=400, inimigo.pos_y_sprite=50;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo.vel_x_sprite=13, inimigo.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  inimigo2.altura_sprite=80; inimigo2.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo2.colunas_folha=3, inimigo2.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo2.linha_atual=1, inimigo2.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo2.regiao_x_folha=0, inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo2.frames_sprite=10, inimigo2.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo2.pos_x_sprite=400, inimigo2.pos_y_sprite=550;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo2.vel_x_sprite=13, inimigo2.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  inimigo3.altura_sprite=80; inimigo3.largura_sprite=50;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  inimigo3.colunas_folha=3, inimigo3.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  inimigo3.linha_atual=1, inimigo3.linhas_folha=4;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  inimigo3.regiao_x_folha=0, inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
  //quantos frames devem se passar para atualizar para o proximo sprite
  inimigo3.frames_sprite=10, inimigo3.cont_frames=0;
  //posicao X Y da janela em que sera mostrado o sprite
  inimigo3.pos_x_sprite=400, inimigo3.pos_y_sprite=210;
  //velocidade X Y que o sprite ira se mover pela janela
  inimigo3.vel_x_sprite=13, inimigo3.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente1.altura_sprite=30; semente1.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente1.colunas_folha=10, semente1.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente1.linha_atual=3, semente1.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente1.regiao_x_folha=0, semente1.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  semente1.frames_sprite=0, semente1.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente1.pos_x_sprite= 480, semente1.pos_y_sprite= 570;
  //velocidade X Y que o sprite ira se mover pela janela
  semente1.vel_x_sprite=13, semente1.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente2.altura_sprite=30; semente2.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente2.colunas_folha=10, semente2.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente2.linha_atual=3, semente2.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente2.regiao_x_folha=0, semente2.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  semente2.frames_sprite=0, semente2.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente2.pos_x_sprite= 500, semente2.pos_y_sprite=70;
  //velocidade X Y que o sprite ira se mover pela janela
  semente2.vel_x_sprite=13, semente2.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  semente3.altura_sprite=30; semente3.largura_sprite=30;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  semente3.colunas_folha=10, semente3.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  semente3.linha_atual=3, semente3.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  semente3.regiao_x_folha=0, semente3.regiao_y_folha=0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  semente3.frames_sprite=0, semente3.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  semente3.pos_x_sprite= 560, semente3.pos_y_sprite=630;
  //velocidade X Y que o sprite ira se mover pela janela
  semente3.vel_x_sprite=13, semente3.vel_y_sprite=13;

  //largura e altura de cada sprite dentro da folha
  vidas.altura_sprite=110; vidas.largura_sprite=190;
  //quantos sprites tem em cada linha da folha, e a atualmente mostrada
  vidas.colunas_folha=4, vidas.coluna_atual=1;
  //quantos sprites tem em cada coluna da folha, e a atualmente mostrada
  vidas.linha_atual=1, vidas.linhas_folha=3;
  //posicoes X e Y da folha de sprites que serao mostradas na tela
  vidas.regiao_x_folha = 0;
  vidas.regiao_y_folha = 0;
  //quantos frames devem se passar para atualizar para o proximo sprite
  vidas.frames_sprite=0, vidas.cont_frames=2;
  //posicao X Y da janela em que sera mostrado o sprite
  vidas.pos_x_sprite= 920, vidas.pos_y_sprite=40;
  //velocidade X Y que o sprite ira se mover pela janela
  vidas.vel_x_sprite=13, vidas.vel_y_sprite=13;

  minutos = 1;
  segundos  = 0;



  ALLEGRO_KEYBOARD_STATE keys;

  while(!done){
    al_wait_for_event(fila_eventos, &evento);
    if(evento.type == ALLEGRO_EVENT_KEY_DOWN) {
      al_get_keyboard_state(&keys);
      char tcaixa[50] = "Pausado",titulo[100] = "Jogo Pausado", texto[200] = "Deseja fechar o jogo?";
      int r;
      //se apertar nas teclas
      switch(evento.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        if (goku.pos_y_sprite > 20 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_UP]=true;
          goku.linha_atual=3;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
        }//if
        break;

        case ALLEGRO_KEY_DOWN:
        if (goku.pos_y_sprite + goku.altura_sprite < ALTURA_TELA -20 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_DOWN]=true;
          goku.linha_atual=1;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
        }//if
        break;

        case ALLEGRO_KEY_LEFT:
        if (goku.pos_x_sprite > 270 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_LEFT]=true;
          goku.linha_atual = 2;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
          goku.vel_x_sprite = -1;
        }//if
        break;

        case ALLEGRO_KEY_RIGHT:
        if (goku.pos_x_sprite + goku.altura_sprite < 902 ){
          //diminui uma posição do sprite no y
          pressed_key[ALLEGRO_KEY_RIGHT]=true;
          goku.linha_atual = 2;
          goku.regiao_y_folha = (goku.linha_atual-1) * goku.altura_sprite;
          goku.vel_x_sprite = 1;
        }//if
        break;

        case ALLEGRO_KEY_ESCAPE:
        //mostra a caixa de texto
        r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
        //al_get_current_display() - retorna a janela ativa;
        printf("%i",r);
        if(r==true){
          done=true;
        }//if
        break;
      }//switch
    }//if evento movimentação apertando teclado

    else if(evento.type == ALLEGRO_EVENT_KEY_UP) {
      al_get_keyboard_state(&keys);
      //se soltar a tecla
      switch(evento.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        pressed_key[ALLEGRO_KEY_UP]=false;
        break;

        case ALLEGRO_KEY_DOWN:
        pressed_key[ALLEGRO_KEY_DOWN]=false;
        break;

        case ALLEGRO_KEY_LEFT:
        pressed_key[ALLEGRO_KEY_LEFT]=false;
        break;

        case ALLEGRO_KEY_RIGHT:
        pressed_key[ALLEGRO_KEY_RIGHT]=false;
        break;
      }//switch

      desenha=1;
    }//if se solta a tecla

    if (evento.type == ALLEGRO_EVENT_TIMER) {
      if (atualiza_x_1 > 0 &&(inimigo.pos_x_sprite >= 790 || inimigo.pos_x_sprite < 270)){
        atualiza_x_1*= -1;
        inimigo.vel_x_sprite = 1;
        inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
      }//if
      else if(atualiza_x_1 < 0 &&(inimigo.pos_x_sprite >= 790 || inimigo.pos_x_sprite < 270)){
        atualiza_x_1*= -1;
        inimigo.vel_x_sprite = -1;
        inimigo.regiao_y_folha=(inimigo.linha_atual-1) * inimigo.altura_sprite;
      }//else if
      if (atualiza_x_2 > 0 &&(inimigo2.pos_x_sprite >= 750 || inimigo2.pos_x_sprite < 350)){
        atualiza_x_2*= -1;
        inimigo2.vel_x_sprite = 1;
        inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
      }//if
      else if(atualiza_x_2 < 0 &&(inimigo2.pos_x_sprite >= 750 || inimigo2.pos_x_sprite < 350)){
        atualiza_x_2*= -1;
        inimigo2.vel_x_sprite = -1;
        inimigo2.regiao_y_folha=(inimigo2.linha_atual-1) * inimigo2.altura_sprite;
      }//else if
      if (atualiza_x_3 > 0 &&(inimigo3.pos_x_sprite >= 750 || inimigo3.pos_x_sprite < 350)){
        atualiza_x_3*= -1;
        inimigo3.vel_x_sprite = 1;
        inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
      }//if
      else if(atualiza_x_3 < 0 &&(inimigo3.pos_x_sprite >= 750 || inimigo3.pos_x_sprite < 350)){
        atualiza_x_3*= -1;
        inimigo3.vel_x_sprite = -1;
        inimigo3.regiao_y_folha=(inimigo3.linha_atual-1) * inimigo3.altura_sprite;
      }//else if
      //fantasma se move para direita
      //if(inimigo.pos_y_sprite + inimigo.altura_sprite > ALTURA_TELA -20 || inimigo.pos_y_sprite < 20/*&& !((inimigo.pos_y_sprite >= 201 && inimigo.pos_y_sprite <= 249) && (inimigo.pos_x_sprite >= 300 && inimigo.pos_x_sprite <= 400) || (inimigo.pos_x_sprite >= 150 && inimigo.pos_x_sprite <= 250))*/)
      inimigo.pos_x_sprite -= atualiza_x_1;
      inimigo2.pos_x_sprite -= atualiza_x_2;
      inimigo3.pos_x_sprite -= atualiza_x_3;
      cont++;
      if (cont == 15) {
        inimigo.coluna_atual = (inimigo.coluna_atual+1)% inimigo.colunas_folha;
        inimigo.regiao_x_folha = inimigo.coluna_atual * inimigo.largura_sprite;
        inimigo2.coluna_atual = (inimigo2.coluna_atual+1)% inimigo2.colunas_folha;
        inimigo2.regiao_x_folha = inimigo2.coluna_atual * inimigo2.largura_sprite;
        inimigo3.coluna_atual = (inimigo3.coluna_atual+1)% inimigo3.colunas_folha;
        inimigo3.regiao_x_folha = inimigo3.coluna_atual * inimigo3.largura_sprite;
        cont = 0;
      }//if
      //pressiona para cima (GOKU)
      if (pressed_key[ALLEGRO_KEY_UP]==true && goku.pos_y_sprite > 40 &&
        ((goku.pos_y_sprite != 118 || goku.pos_x_sprite < 580 || goku.pos_x_sprite > 745) &&//bloco 2
        (goku.pos_y_sprite != 118 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 540)) &&//bloco 1
        ((goku.pos_y_sprite != 642 || goku.pos_x_sprite < 595 || goku.pos_x_sprite > 744) &&//bloco 4
        (goku.pos_y_sprite != 642 || goku.pos_x_sprite < 398 || goku.pos_x_sprite > 549)) &&//bloco 3
        ((goku.pos_y_sprite != 260 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 5 e 7
        (goku.pos_y_sprite != 260 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//blocos 6 e 8
        ((goku.pos_y_sprite != 650 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 9 e 10
        (goku.pos_y_sprite != 630 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//bloco 11 e 12
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 13
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 15
        (goku.pos_y_sprite != 540 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 16
        (goku.pos_y_sprite != 540 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 18
        (goku.pos_y_sprite != 200 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 14
        (goku.pos_y_sprite != 520 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 17
        (goku.pos_y_sprite != 410 || goku.pos_x_sprite < 290 || goku.pos_x_sprite > 360)  &&//bloco 19
        (goku.pos_y_sprite != 410 || goku.pos_x_sprite < 779 || goku.pos_x_sprite > 890)  &&//bloco 20
        (goku.pos_y_sprite != 430 || goku.pos_x_sprite < 410 || goku.pos_x_sprite > 735)  &&//bloco 21
        (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 420 || goku.pos_x_sprite > 549)  &&//bloco 21
        (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 599 || goku.pos_x_sprite > 735)){//bloco 21
          goku.pos_y_sprite -= 2;
          goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
          goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
        }//if

        //pressiona para baixo
        else if(pressed_key[ALLEGRO_KEY_DOWN]==true && goku.pos_y_sprite < 680 &&
          ((goku.pos_y_sprite != 50 || goku.pos_x_sprite < 580 || goku.pos_x_sprite > 745)  &&//bloco 2
          (goku.pos_y_sprite != 50 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 540))  &&//bloco 1
          ((goku.pos_y_sprite != 566 || goku.pos_x_sprite < 595 || goku.pos_x_sprite > 746) &&//bloco 4
          (goku.pos_y_sprite != 566 || goku.pos_x_sprite < 400 || goku.pos_x_sprite > 551)) &&//bloco 3
          ((goku.pos_y_sprite != 76 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361)  &&//blocos 5 e 7
          (goku.pos_y_sprite != 80 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830))  &&//blocos 6 e 8
          ((goku.pos_y_sprite != 444 || goku.pos_x_sprite < 315 || goku.pos_x_sprite > 361) &&//blocos 9 e 10
          (goku.pos_y_sprite != 444 || goku.pos_x_sprite < 775 || goku.pos_x_sprite > 830)) &&//bloco 11 e 12
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 13
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 15
          (goku.pos_y_sprite != 480 || goku.pos_x_sprite < 330 || goku.pos_x_sprite > 440)  &&//bloco 16
          (goku.pos_y_sprite != 480 || goku.pos_x_sprite < 700 || goku.pos_x_sprite > 820)  &&//bloco 18
          (goku.pos_y_sprite != 140 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 14
          (goku.pos_y_sprite != 460 || goku.pos_x_sprite < 490 || goku.pos_x_sprite > 650)  &&//bloco 17
          (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 290 || goku.pos_x_sprite > 360)  &&//bloco 19
          (goku.pos_y_sprite != 290 || goku.pos_x_sprite < 779 || goku.pos_x_sprite > 890)  &&//bloco 20
          (goku.pos_y_sprite != 360 || goku.pos_x_sprite < 410 || goku.pos_x_sprite > 730)  &&//bloco 21
          (goku.pos_y_sprite != 220 || goku.pos_x_sprite < 420 || goku.pos_x_sprite > 549)  &&//bloco 21
          (goku.pos_y_sprite != 220 || goku.pos_x_sprite < 599 || goku.pos_x_sprite > 750)){//bloco 21
            goku.pos_y_sprite += 2;
            goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
            goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
          }//if

          //pressiona para esquerda
          if (pressed_key[ALLEGRO_KEY_LEFT]==true && goku.pos_x_sprite > 290 && !
            ((((goku.pos_y_sprite >= 51 && goku.pos_y_sprite <= 117)&&//posicao em y dos blocos 1 e 2
            ((goku.pos_x_sprite >= 589 && goku.pos_x_sprite <=745) ||//bloco 2
            (goku.pos_x_sprite >= 380 && goku.pos_x_sprite <=550))) ||//bloco 1
            ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 569) &&//posicao em y dos blocos 3 e 4
            ((goku.pos_x_sprite >= 594 && goku.pos_x_sprite <=745) ||//bloco 4
            (goku.pos_x_sprite >= 380 && goku.pos_x_sprite <= 550))) ||//bloco 3
            ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 77) &&//posicao em y dos blocos 5 e 7
            (goku.pos_x_sprite >= 315 && goku.pos_x_sprite <=366))||//bloco 5 e 7
            ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 81) &&//posicao em y dos blocos 6 e 8
            (goku.pos_x_sprite >= 775 && goku.pos_x_sprite <= 830))||//bloco 6 e 8
            ((goku.pos_y_sprite <= 649 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 9 e 10
            (goku.pos_x_sprite >= 315 && goku.pos_x_sprite <=366))||//bloco 9 e 10
            ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 11 e 12
            (goku.pos_x_sprite >= 785 && goku.pos_x_sprite <= 830))||//blocos 11 e 12
            ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 13
            (goku.pos_x_sprite >= 330 && goku.pos_x_sprite <= 441))||//bloco 13
            ((goku.pos_y_sprite <= 539 && goku.pos_y_sprite >= 480) &&//posicao em y do bloco 16
            (goku.pos_x_sprite >= 330 && goku.pos_x_sprite <=441))||//bloco 16
            ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 14
            (goku.pos_x_sprite >= 550 && goku.pos_x_sprite <= 651))||//bloco 14
            ((goku.pos_y_sprite <= 519 && goku.pos_y_sprite >= 461) &&//posicao em y do bloco 17
            (goku.pos_x_sprite >= 550 && goku.pos_x_sprite <= 651))||//bloco 17
            ((goku.pos_y_sprite <= 409 && goku.pos_y_sprite >= 289) &&//posicao em y do bloco 19
            (goku.pos_x_sprite >= 250 && goku.pos_x_sprite <= 363))||//bloco 19
            ((goku.pos_y_sprite <= 429 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
            ((goku.pos_x_sprite >= 690 && goku.pos_x_sprite <= 750) ||
            (goku.pos_x_sprite >= 420 && goku.pos_x_sprite <= 475)))||//bloco 21
            ((goku.pos_y_sprite <= 289 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
            (goku.pos_x_sprite >= 440 && goku.pos_x_sprite <= 550))))){//bloco 21
              goku.pos_x_sprite -= 2;
              goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
              goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
            }//if

            //pressiona para direita
            else if(pressed_key[ALLEGRO_KEY_RIGHT]==true && goku.pos_x_sprite < 850 && !
              ((((goku.pos_y_sprite >= 51 && goku.pos_y_sprite <= 117)&&//posicao em y dos blocos 1 e 2
              ((goku.pos_x_sprite >= 570 && goku.pos_x_sprite <= 737) ||//bloco 2
              (goku.pos_x_sprite >= 378 && goku.pos_x_sprite <= 549))) ||//bloco 1
              ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 569) &&//posicao em y dos blocos 3 e 4
              ((goku.pos_x_sprite >= 592 && goku.pos_x_sprite <= 737) ||//bloco 4
              (goku.pos_x_sprite >= 378 && goku.pos_x_sprite <= 549)))||//bloco 3
              ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 77) &&//posicao em y dos blocos 5 e 7
              (goku.pos_x_sprite >= 310 && goku.pos_x_sprite <= 360))||//bloco 5 e 7
              ((goku.pos_y_sprite <= 259 && goku.pos_y_sprite >= 81) &&//posicao em y dos blocos 6 e 8
              (goku.pos_x_sprite >= 770 && goku.pos_x_sprite <= 815))||//blocos 6 e 8
              ((goku.pos_y_sprite <= 649 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 9 e 10
              (goku.pos_x_sprite >= 310 && goku.pos_x_sprite <= 360))||//blocos 9 e 10
              ((goku.pos_y_sprite <= 629 && goku.pos_y_sprite >= 445) &&//posicao em y dos blocos 11 e 12
              (goku.pos_x_sprite >= 776 && goku.pos_x_sprite <= 815))||//blocos 11 e 12
              ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 15
              (goku.pos_x_sprite >= 701 && goku.pos_x_sprite <= 820))||//bloco 15
              ((goku.pos_y_sprite <= 539 && goku.pos_y_sprite >= 481) &&//posicao em y do bloco 18
              (goku.pos_x_sprite >= 701 && goku.pos_x_sprite <= 820))||//bloco 18
              ((goku.pos_y_sprite <= 199 && goku.pos_y_sprite >= 141) &&//posicao em y do bloco 14
              (goku.pos_x_sprite >= 491 && goku.pos_x_sprite <= 649))||//bloco 14
              ((goku.pos_y_sprite <= 519 && goku.pos_y_sprite >= 461) &&//posicao em y do bloco 17
              (goku.pos_x_sprite >= 491 && goku.pos_x_sprite <= 649))||//bloco 17
              ((goku.pos_y_sprite <= 409 && goku.pos_y_sprite >= 289) &&//posicao em y do bloco 20
              (goku.pos_x_sprite >= 780 && goku.pos_x_sprite <= 890))||//bloco 20
              ((goku.pos_y_sprite <= 429 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
              ((goku.pos_x_sprite >= 390 && goku.pos_x_sprite <= 450)||
              (goku.pos_x_sprite >= 675 && goku.pos_x_sprite <= 749)))||//bloco 21
              ((goku.pos_y_sprite <= 289 && goku.pos_y_sprite >= 221) &&//posicao em y do bloco 21
              (goku.pos_x_sprite >= 600 && goku.pos_x_sprite <= 749))))){// bloco 21
                goku.pos_x_sprite += 2;
                goku.coluna_atual = (goku.coluna_atual+1)% goku.colunas_folha;
                goku.regiao_x_folha = goku.coluna_atual * goku.largura_sprite;
              }//else if

              desenha=1;
            }//if evento timer
            colisao = verificaColisao(goku,inimigo,inimigo2,inimigo3,semente1,semente2,semente3,captacolisao_goku,captacolisao_semente1,captacolisao_semente2,captacolisao_semente3);
            if (colisao == 1) {
              captacolisao_goku=1;
            }//if
            if (colisao == 2){
              captacolisao_semente1=1;
            }//if
            if (colisao == 3) {
              captacolisao_inimigo2=1;
            }//if
            if (colisao == 4) {
              captacolisao_inimigo1=1;
            }//if
            if (colisao == 5) {
              captacolisao_inimigo3=1;
            }//if
            if (colisao == 6) {
              captacolisao_semente2=1;
            }//if
            if (colisao == 7) {
              captacolisao_semente3=1;
            }//if
            else if (colisao == 0) {
            }//if

            if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
              //caixa de texto para confirmar sair do jogo
              //mostra a caixa de texto
              //exibe o titulo do jogo
              char tcaixa[50] = "Menu";
              //exibe se deseja sair do jogo
              char titulo[100] = "Atenção";
              //exibe mensagem de aviso
              char texto[200] = "Deseja mesmo sair?";
              int r= al_show_native_message_box(al_get_current_display(), tcaixa, titulo, texto, NULL, ALLEGRO_MESSAGEBOX_YES_NO);
              //al_get_current_display() - retorna a janela ativa;
              printf("%i",r);
              if(r==true){
                return 0;
              }//if
            }//else do X na tela
            if (captacolisao_goku > 0) {
              desenha = 3;
            }

            if ((captacolisao_inimigo1>0 && captacolisao_inimigo2 > 0 )&& captacolisao_inimigo3 > 0) {
              desenha = 2;
            }//if

            /* -- ATUALIZA TELA -- */
            if(desenha && al_is_event_queue_empty(fila_eventos)) {
              al_get_keyboard_state(&keys);
              //desenha o fim do jogo - WIINNN
              if (desenha == 2) {
                al_draw_bitmap_region(fundofim,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);

                if (!concluido){
                  nomeRanking(evento);
                  if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                    concluido = true;
                  }//if
                }//if

                // al_draw_bitmap(fundo, 0, 0, 0);
                if (!concluido){
                  al_draw_textf(fonte,al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "VITÓRIA! ");
                  al_draw_textf(fonte,al_map_rgb(0,0, 0), 100,
                  (ALTURA_TELA / 2 - al_get_font_ascent(fonte)) / 2,
                  ALLEGRO_ALIGN_LEFT, "Nome:");
                  if (strlen(str) > 0){
                    al_draw_text(fonte,al_map_rgb(0, 0, 0), 300, 160,
                    // (ALTURA_TELA - al_get_font_ascent(fonte)) / 2,
                    ALLEGRO_ALIGN_LEFT, str);
                  }//if
                }//if
                else {
                  al_draw_textf(fonte, al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "Ranking");
                  al_draw_textf(fonte, al_map_rgb(0,0,0),100 , 150, ALLEGRO_ALIGN_LEFT, "1º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 500, 150, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[0].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 150, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 150, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[0].minutos,listaRanking[0].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 250, ALLEGRO_ALIGN_LEFT, "2º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,250, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[1].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 250, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 250, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[1].minutos,listaRanking[1].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 350, ALLEGRO_ALIGN_LEFT, "3º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,350, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[2].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 350, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 350, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[2].minutos,listaRanking[2].segundos);


                }
                al_flip_display();

              }//if GAME OVER You winn

              if (desenha == 3) {
                al_draw_bitmap_region(fundofim,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);

                if (!concluido){
                  nomeRanking(evento);
                  if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                    concluido = true;
                  }//if
                }//if

                // al_draw_bitmap(fundo, 0, 0, 0);
                if (!concluido){
                  al_draw_textf(fonte,al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "PERDEU! ");
                  al_draw_textf(fonte,al_map_rgb(0,0, 0), 100,
                  (ALTURA_TELA / 2 - al_get_font_ascent(fonte)) / 2,
                  ALLEGRO_ALIGN_LEFT, "Nome:");
                  if (strlen(str) > 0){
                    al_draw_text(fonte,al_map_rgb(0, 0, 0), 300, 160,
                    // (ALTURA_TELA - al_get_font_ascent(fonte)) / 2,
                    ALLEGRO_ALIGN_LEFT, str);
                  }//if
                }//if
                else{
                  al_draw_textf(fonte, al_map_rgb(0,0,0),500,40, ALLEGRO_ALIGN_LEFT, "Ranking");
                  al_draw_textf(fonte, al_map_rgb(0,0,0),100 , 150, ALLEGRO_ALIGN_LEFT, "1º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 500, 150, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[0].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 150, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 150, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[0].minutos,listaRanking[0].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 250, ALLEGRO_ALIGN_LEFT, "2º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,250, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[1].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 250, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 250, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[1].minutos,listaRanking[1].segundos);

                  al_draw_textf(fonte, al_map_rgb(0,0,0), 100, 350, ALLEGRO_ALIGN_LEFT, "3º Lugar");
                  al_draw_textf(fonte, al_map_rgb(72,61,139),500,350, ALLEGRO_ALIGN_CENTRE, "%s", listaRanking[2].nome);
                  al_draw_textf(fonte, al_map_rgb(0,0,0), 700, 350, ALLEGRO_ALIGN_LEFT, "Tempo:");
                  al_draw_textf(fonte, al_map_rgb(72,61,139), 950, 350, ALLEGRO_ALIGN_LEFT, "%d:%d",listaRanking[2].minutos,listaRanking[2].segundos);


                }
                al_flip_display();

              }


              //else jogo rodando
              else if (desenha ==1 ){
                desenha = 0;
                milisegundos--;
                if(milisegundos%60==0){
                  segundos--;
                  milisegundos=0;
                }//if
                if(segundos==-1){
                  minutos--;
                  segundos=59;
                }//if

                //A cada 100 frames, atualiza cor do titulo
                al_draw_bitmap_region(fundo,0,0,LARGURA_TELA,ALTURA_TELA,0,0,0);
                al_draw_textf(fonte,al_map_rgb(0,0,0),10,40, ALLEGRO_ALIGN_LEFT, "Tempo:");
                al_draw_textf(fonte,al_map_rgb(0,0,0),1100,40, ALLEGRO_ALIGN_RIGHT, "Vida:");
                al_draw_textf(fonte,cor_tempo ,10,130, ALLEGRO_ALIGN_LEFT, "%02d:%02d",minutos, segundos);
                al_draw_bitmap_region(folha_sprite4,
                  vidas.regiao_x_folha , vidas.regiao_y_folha,
                  vidas.largura_sprite ,vidas.altura_sprite,
                  vidas.pos_x_sprite,vidas.pos_y_sprite,1);
                  if(minutos<=1){
                    cor_tempo = al_map_rgb(rand()%255, rand()%34, rand()%34);
                  }//if
                  else cor_tempo= al_map_rgb(0,0,0);

                  //velocidade positiva (movendo para direita)
                  if (goku.vel_x_sprite>0){
                    if (captacolisao_goku<=0) {
                      al_draw_bitmap_region(folha_sprite,
                        goku.regiao_x_folha,goku.regiao_y_folha,
                        goku.largura_sprite,goku.altura_sprite,
                        goku.pos_x_sprite,goku.pos_y_sprite,0);
                      }//if
                    }//if

                    //faz virar pra esquerda invertendo a sprite
                    else{
                      if (captacolisao_goku<=0) {
                        al_draw_bitmap_region(folha_sprite,
                          goku.regiao_x_folha,goku.regiao_y_folha,
                          goku.largura_sprite,goku.altura_sprite,
                          goku.pos_x_sprite,goku.pos_y_sprite,1);
                        }//if
                      }//else
                      if (inimigo.vel_x_sprite > 0) {
                        if (captacolisao_inimigo1<=0) {
                          al_draw_bitmap_region(folha_sprite2,
                            inimigo.regiao_x_folha,inimigo.regiao_y_folha,
                            inimigo.largura_sprite,inimigo.altura_sprite,
                            inimigo.pos_x_sprite,inimigo.pos_y_sprite,0);
                          }//if
                        }//if
                        else{
                          if (captacolisao_inimigo1<=0) {
                            al_draw_bitmap_region(folha_sprite2,
                              inimigo.regiao_x_folha,inimigo.regiao_y_folha,
                              inimigo.largura_sprite,inimigo.altura_sprite,
                              inimigo.pos_x_sprite,inimigo.pos_y_sprite,1);
                            }//if
                          }//else
                          if (inimigo2.vel_x_sprite > 0) {
                            if (captacolisao_inimigo2 <=0) {
                              al_draw_bitmap_region(folha_sprite2,
                                inimigo2.regiao_x_folha,inimigo2.regiao_y_folha,
                                inimigo2.largura_sprite,inimigo2.altura_sprite,
                                inimigo2.pos_x_sprite,inimigo2.pos_y_sprite,0);
                              }//if
                            }//if
                            else{
                              if (captacolisao_inimigo2 <=0) {
                                al_draw_bitmap_region(folha_sprite2,
                                  inimigo2.regiao_x_folha,inimigo2.regiao_y_folha,
                                  inimigo2.largura_sprite,inimigo2.altura_sprite,
                                  inimigo2.pos_x_sprite,inimigo2.pos_y_sprite,1);
                                }//if
                              }//else
                              if (inimigo3.vel_x_sprite > 0) {
                                if (captacolisao_inimigo3 <=0) {
                                  al_draw_bitmap_region(folha_sprite2,
                                    inimigo3.regiao_x_folha,inimigo3.regiao_y_folha,
                                    inimigo3.largura_sprite,inimigo3.altura_sprite,
                                    inimigo3.pos_x_sprite,inimigo3.pos_y_sprite,0);
                                  }//if
                                }//if
                                else{
                                  if (captacolisao_inimigo3 <=0) {
                                    al_draw_bitmap_region(folha_sprite2,
                                      inimigo3.regiao_x_folha,inimigo3.regiao_y_folha,
                                      inimigo3.largura_sprite,inimigo3.altura_sprite,
                                      inimigo3.pos_x_sprite,inimigo3.pos_y_sprite,1);
                                    }//if
                                  }//else
                                  if (captacolisao_semente1 <=0) {
                                    al_draw_bitmap_region(folha_sprite3,
                                      semente1.regiao_x_folha,semente1.regiao_y_folha,
                                      semente1.largura_sprite,semente1.altura_sprite,
                                      semente1.pos_x_sprite,semente1.pos_y_sprite,0);
                                    }//if
                                    if (captacolisao_semente2 <=0) {
                                      al_draw_bitmap_region(folha_sprite3,
                                        semente2.regiao_x_folha,semente2.regiao_y_folha,
                                        semente2.largura_sprite,semente2.altura_sprite,
                                        semente2.pos_x_sprite,semente2.pos_y_sprite,0);
                                      }//if
                                      if (captacolisao_semente3 <=0) {
                                        al_draw_bitmap_region(folha_sprite3,
                                          semente3.regiao_x_folha,semente3.regiao_y_folha,
                                          semente3.largura_sprite,semente3.altura_sprite,
                                          semente3.pos_x_sprite,semente3.pos_y_sprite,0);
                                        }//if
                                        al_flip_display();
                                      }// else jogo rodando
                                    }//if desenha events
                                  }//while
                                  return 0;
}//funcao atualizar Goku
int main(){
  Personagem goku;
  Personagem inimigo;
  Personagem inimigo2;
  Personagem inimigo3;
  Personagem semente1;
  Personagem semente2;
  Personagem semente3;
  Personagem vidas;
  inicializaBiblioteca();

  // O nosso arquivo da imagem
  ALLEGRO_BITMAP *imagem = NULL;
  //botões
  ALLEGRO_BITMAP *btn_sair = NULL, *btn_iniciar = 0, *btn_tutorial = 0, *btn_configuracao = 0, *btn_creditos = 0;
  //criando os botoes do nivel de dificuldade
  ALLEGRO_BITMAP *btn_facil = 0, *btn_medio = 0, *btn_dificil = 0, *btn_voltar=0, *btn_som=0, *btn_prox=0, *btn_menu=0;
  //declarando a variavel responsável pela música
  //ALLEGRO_AUDIO_STREAM *musica ;

  musica = al_load_audio_stream("musicas/menu.ogg", 4, 1024);//deixei na main pois estava ocorrendo erro
    if (!musica)                                             //o jogo fechava mas algo ficava rodando e o cmd ficava aberto
    {
        mensagem_erro( "Audio nao carregado" );
        al_destroy_audio_stream(musica);
        al_destroy_display(janela);
        return 0;
    }//if musica

    //liga o stream no mixer
    al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());

    //define que o stream vai tocar no modo repeat
    al_set_audio_stream_playmode(musica, ALLEGRO_PLAYMODE_LOOP);


  // Flag que condicionará nosso looping
  int sair = 0;
  int voltar= 0;
  int inicio=0;
  int j = 2;
  int i = 4;

  inicio=geraMenu(janela, fonte, imagem, fila_eventos, btn_iniciar, btn_tutorial, btn_configuracao, btn_creditos, btn_sair, sair, musica);
  do {
    if (inicio==5) {
      inicio=geraMenu(janela, fonte, imagem, fila_eventos, btn_iniciar, btn_tutorial, btn_configuracao, btn_creditos, btn_sair, sair, musica);
    }//if

    if (inicio==1) {
      inicio=geraJogar(janela, fonte, imagem, fila_eventos, btn_medio, btn_facil, btn_dificil, btn_voltar, sair, voltar, musica);
    }//if

    if (inicio==2) {
      inicio=geraTutorial(janela, fonte, imagem, fila_eventos, btn_voltar,sair, voltar, musica, btn_prox, btn_menu);
    }//if

    if (inicio==3) {
      inicio=geraConfiguracao(janela, fonte, imagem, fila_eventos, btn_som, btn_voltar, sair, voltar, musica, volume);
    }//if

    if (inicio==4) {
      inicio=geraCredito(janela, fonte, imagem, fila_eventos, btn_voltar, sair, voltar, musica);
    }//if
    if (inicio==6) {
      //al_destroy_audio_stream(musica);
      timer = al_create_timer(0.9 / 60);
      if(!timer) {
        mensagem_erro("Falha ao criar temporizador");
        return 0;
      }//if
      al_start_timer(timer);
      al_register_event_source(fila_eventos, al_get_keyboard_event_source());
      al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
      do {
        i = Atualizarjogo1(goku,inimigo,inimigo2,inimigo3,semente1,semente2,semente3,i,vidas);
        timer = al_create_timer(0.9 / 60);
        if(!timer) {
          mensagem_erro("Falha ao criar temporizador");
          return 0;
        }//if
        al_start_timer(timer);
      } while(i!=0);
      inicio = 0;
    }//if
    if (inicio==7) {
      //al_destroy_audio_stream(musica);
      timer = al_create_timer(0.9 / 60);
      if(!timer) {
        mensagem_erro("Falha ao criar temporizador");
        return 0;
      }//if
      al_start_timer(timer);
      al_register_event_source(fila_eventos, al_get_keyboard_event_source());
      al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
      do {
        j = Atualizarjogo2(goku,inimigo,inimigo2,inimigo3,semente1,semente2,semente3,j,vidas);
        timer = al_create_timer(0.9 / 60);
        if(!timer) {
          mensagem_erro("Falha ao criar temporizador");
          return 0;
        }//if
        al_start_timer(timer);
      } while(j!=0);
      inicio = 0;
    }//if
    if (inicio==8) {
      //al_destroy_audio_stream(musica);
      timer = al_create_timer(0.9 / 60);
      if(!timer) {
        mensagem_erro("Falha ao criar temporizador");
        return 0;
      }//if
      al_start_timer(timer);
      al_register_event_source(fila_eventos, al_get_keyboard_event_source());
      al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
      Atualizarjogo3(goku,inimigo,inimigo2,inimigo3,semente1,semente2,semente3,vidas);
      inicio = 0;
    }//if
  } while(inicio!=0);



  al_destroy_event_queue(fila_eventos);
  al_destroy_bitmap(folha_sprite);
  al_destroy_bitmap(folha_sprite2);
  al_destroy_bitmap(folha_sprite3);
  al_destroy_bitmap(fundo);
  al_destroy_timer(timer);
  al_destroy_display(janela);

  return 0;
}//main
