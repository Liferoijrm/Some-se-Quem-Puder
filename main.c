//                                                                           //
//          Universidade de Brasilia - Instituto de Ciencias Exatas          //     
//                   Departamento de Ciencia da Computacao                   //
//                                                                           //
//             Algoritmos e Programação de Computadores - 2024.1             //
//                                                                           //
//                         Professora: Carla Castanho                        //
//                           Aluno: Pedro Marcinoni                          //
//                                                                           //               
//                            Some-se Quem Puder!                            //
//                                                                           //

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

///////////////////////////////////////////////////
// *** Definicao de macros de opcoes da main *** //
///////////////////////////////////////////////////

#define OP_JOGAR 1
#define OP_CONFIG 2
#define OP_INSTR 3
#define OP_RANK 4
#define OP_SAIR 5

////////////////////////////////////////////////////////////////////////////////
// *** Definicao das macros dos modos Iniciante, Intermediario e Avancado *** //
////////////////////////////////////////////////////////////////////////////////

#define MODO_INICIANTE 1
#define TAMANHO_INICIANTE 4

#define MODO_INTERMEDIARIO 2
#define TAMANHO_INTERMEDIARIO 6 

#define MODO_AVANCADO 3
#define TAMANHO_AVANCADO 7 

////////////////////////////////////////////////////////////////////////////////////////////
// *** Definicao das macros dos offsets, em bytes, entre as fases de cada dificuldade *** //
////////////////////////////////////////////////////////////////////////////////////////////

// OBS: Esses dados foram calculados previamente para reduzir o tamanho e aumentar a eficiencia do codigo
#define OFFSET_INICIANTE 60
#define OFFSET_INTERMEDIARIO 112
#define OFFSET_AVANCADO 144

//////////////////////////////////////////////////////////////////////////
// *** Definicao da macro CLEAR de acordo com o sistema operacional *** //
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32 
    #define CLEAR "cls" 
#else 
    #define CLEAR "clear" 
#endif

///////////////////////////////////////////////////////////////
// *** Definicao do padrao da struct de dados do jogador *** //
///////////////////////////////////////////////////////////////

typedef struct{
    char nome[21];
    int pontuacao;
} playerData;

///////////////////////////////////////////////////////
// *** Modulo de declaracao de variaveis globais *** //
///////////////////////////////////////////////////////

FILE* fd; // ponteiro para arquivos .txt (fases)
FILE* fp; // ponteiro para arquivos .bin (ranking)

char nickname[21], arquivo[21] = "iniciante.txt";

char matriz[7][8], matriz_espelho[7][8];
char somatorio_lin[15], somatorio_col[15];

char matriz_apagados[7][8];
char somatorios_apagados_lin[8], somatorios_apagados_col[8];

int fase_iniciante = 1, fase_intermediario = 1, fase_avancado = 1, fases_completas = 0, pontos = 0;
int modo = MODO_INICIANTE, tamanho_matriz = TAMANHO_INICIANTE;

int creditos = 0; 

////////////////////////////////////////////////////////////////
// *** Modulo de funcoes basicas reutilizadas no programa *** //
////////////////////////////////////////////////////////////////

// limpa a tela
void cleanScreen(){
    system(CLEAR);
}

// limpa o buffer de entrada
void cleanBuffer(){
    while (getchar() != '\n');
}

// configura os valores das variaveis para o Modo Iniciante
void modoIniciante(){
    modo = MODO_INICIANTE;
    tamanho_matriz = TAMANHO_INICIANTE;
    strcpy(arquivo, "iniciante.txt");
}

// configura os valores das variaveis para o Modo Intermediario
void modoIntermediario(){
    modo = MODO_INTERMEDIARIO;
    tamanho_matriz = TAMANHO_INTERMEDIARIO;
    strcpy(arquivo, "intermediario.txt");
}

// configura os valores das variaveis para o Modo Avancado
void modoAvancado(){
    modo = MODO_AVANCADO;
    tamanho_matriz = TAMANHO_AVANCADO;
    strcpy(arquivo, "avancado.txt");
}

// ordena um vetor de struct "playerData" em ordem decrescente de pontos e depois em ordem lexicografica de nomes
void multBubbleSort(playerData v[], int n){
    
    int i, j;
    
    for(j = 0; j < n; j++) {
        for(i = 0; i < n-1; i++) {
            
            int troca = 0;
            
            if(v[i].pontuacao < v[i+1].pontuacao){
                troca = 1;
            } 
            else if(v[i].pontuacao == v[i+1].pontuacao && strcmp(v[i].nome, v[i+1].nome) > 0){
                troca = 1;
            }

            if(troca == 1){
                playerData aux = v[i];
                v[i] = v[i+1];
                v[i+1] = aux;
            }
        }
    }
}

// reseta os dados contidos nas matrizes e strings que constituem uma fase apos o seu encerramento
void resetMatrixes(){

    int i, j;

    for (i = 0; i < 15; i++){
        somatorio_lin[i] = '\0', somatorio_col[i] = '\0';
    }

    for (i = 0; i < 8; i++){
        somatorios_apagados_lin[i] = '\0', somatorios_apagados_col[i] = '\0';
    }

    for (i = 0; i < 7; i++){
        for (j = 0; j < 8; j++){
            matriz[i][j] = '\0';
            matriz_apagados[i][j] = '\0';
            matriz_espelho[i][j] = '\0';
        }
    }
}

// realiza uma comparacao entre as colunas de duas matrizes de char (como um "strcmp" que considera colunas como strings)
int strcmpcol(char matriz1[7][8], char matriz2[7][8], int col){

    int i;

    for (int i = 0; i < 7; i++) {
        if (matriz1[i][col] != matriz2[i][col]) {
            return 1;
        }
    }

    return 0;
}

// le os dados do "arquivo.txt" para a matriz a ser mostrada na tela
void scanMatrix(){

    int i, j;

    for(i = 0; i < tamanho_matriz; i++){
        fscanf(fd, "%s", matriz[i]);
        fgetc(fd); // capta o caracter '\n' no final da string
    }
}

// le os dados "arquivo.txt" para as strings de somatorios a serem mostradas na tela
void scanSomatorios(){
    fscanf(fd, "%s", somatorio_col);
    fgetc(fd); // capta o caracter '\n' no final da string
    fscanf(fd, "%s", somatorio_lin);
    fgetc(fd); // capta o caracter '\n' no final da string
}

// le os dados do "arquivo.txt" para a matriz-espelho
void scanMirrorMatrix(){

    int i, j;

    for(i = 0; i < tamanho_matriz; i++){
        fscanf(fd, "%s", matriz_espelho[i]);
        fgetc(fd); // capta o caracter '\n' no final da string
    }
}

// adiciona os dados do jogador no ranking ou atualiza a pontuacao do jogador se ja estiver registrado
void add2Rank(){

    playerData jogador; // Estrutura para armazenar os dados do jogador
    int achou = 0; // Flag para verificar se o jogador já está no ranking

    fp = fopen("ranking.bin", "r+b"); // abre o arquivo "ranking.bin" para leitura ou escrita caso exista

    if (fp == NULL){ // Se o arquivo não existir

        fp = fopen("ranking.bin", "w+b"); // cria e abre um novo arquivo "ranking.bin" para leitura e escrita

        if (fp == NULL) {
            printf("Erro em criar o arquivo de ranking");
            return;
        }
    }

    // fread le um elemento de "tipo" playerData do arquivo e retorna 1 caso o elemento seja lido corretmente. Se nao houver mais elementos, encerra o loop ao retornar 0
    while (fread(&jogador, sizeof(playerData), 1, fp) == 1){ 
        if (strcmp(jogador.nome, nickname) == 0){ // Se o jogador já estiver registrado
            achou = 1; 
            jogador.pontuacao = pontos; // Atualiza a pontuação do jogador
            fseek(fp, -sizeof(playerData), SEEK_CUR); // move o ponteiro para a posicao anterior ao registro do jogador (-sizeof(playerData) = -quantidade de bits no struct e SEEK_CUR = posicao atual)
            fwrite(&jogador, sizeof(playerData), 1, fp); // sobreescreve os dados do jogador
            break; 
        }
    }

    if (!achou) { // se o jogador for novo
        fseek(fp, 0, SEEK_END); // move o ponteiro para o final do arquivo
        strcpy(jogador.nome, nickname); 
        jogador.pontuacao = pontos;
        fwrite(&jogador, sizeof(playerData), 1, fp); // escreve os dados do jogador
    }

    fclose(fp); // fecha o arquivo
}

//////////////////////////////////////////////////////
// *** Inicio da divisao modular real do codigo *** //
//      OBS: a int main() esta no fim do codigo     //
//////////////////////////////////////////////////////

// mostra a matriz da fase atual do jogo na tela com informacoes de fase, modo e vida
void showMainMatrix(int vidas){

    cleanScreen();

    if (modo == MODO_INICIANTE){ // se o modo for Iniciante
        printf(" *** FASE %d - INICIANTE *** 50 pts\n\n", fase_iniciante); // mostra a fase e o modo atual Iniciante
    }   
    else if (modo == MODO_INTERMEDIARIO){ // se o modo for Intermediario
        printf(" *** FASE %d - INTERMEDIARIO *** 100 pts\n\n", fase_intermediario); // mostra a fase e o modo atual Intermediario
    }
    else if (modo == MODO_AVANCADO){ // se o modo for Avancado
        printf(" *** FASE %d - AVANCADO *** 200pts\n\n", fase_avancado); // mostra a fase e o modo atual Avancado
    } 

    int i, j;

    printf("    ");

    // mostra os somatorios das colunas
    for (i = 0; i < (tamanho_matriz*2); i+=2){
        if (somatorios_apagados_col[i/2] == '1'){ 
            printf(" %c%c", somatorio_col[i], somatorio_col[i+1]);
        }
        else { // se o somatorio tiver sido apagado (somatorios_apagados_col[i/2] != '1')
            printf("   "); // mostra apenas 3 espacos em branco
        }
    }

    printf("\n");
    printf("    ");

    // mostra a divisoria entre a matriz e os somatorios das colunas
    for (i = 0; i < tamanho_matriz; i++){
        printf("---");
    }

    printf("-\n");
    
    // mostra os somatorios das linhas e os elementos da matriz
    for (i = 0; i < (tamanho_matriz*2); i+=2){

        if (somatorios_apagados_lin[i/2] == '1'){
            printf(" %c%c | ", somatorio_lin[i], somatorio_lin[i+1]);
        }
        else { // se o somatorio tiver sido apagado (somatorios_apagados_lin[i/2] != '1')
            printf("    | "); // mostra apenas 3 espacos em branco e " | "
        }

        for (j = 0; j < tamanho_matriz; j++){

            if (matriz_apagados[i/2][j] == '1'){
                printf("%c  ", matriz[i/2][j]);
            }
            else { // se o elemento tiver sido apagado (matriz_apagados[i/2][j] != '1')
                printf("   "); // mostra apenas 3 espacos em branco
            }
        }

        printf("\n");
    }

    printf("\n *** VOCE TEM %d VIDAS(S) ***\n", vidas); // mostra as vidas restantes
    printf(" Digite a linha e a coluna do elemento a ser apagado: ");
}

// executa a gameplay, condicao de vitoria e de derrota do jogo
void gameplay(){

    int perdeu = 0, venceu = 0, vidas = 5, somatorios_fechados = 0;
    int i, j;

    // inicializa as matrizes e strings de apagados com caracteres '1' (indica que o numero daquela posicao ainda nao foi apagado)
    for (i = 0; i < tamanho_matriz; i++){

        somatorios_apagados_lin[i] ='1', somatorios_apagados_col[i] = '1';

        for (j = 0; j < tamanho_matriz; j++){
            matriz_apagados[i][j] = '1';
        }
    }

    // loop do jogo
    while (1){

        showMainMatrix(vidas); // mostra a matriz formatada com elementos e somatorios, alem de informacoes de fase, modo e vida

        int linha = 0, coluna = 0;

        scanf("%d %d", &linha, &coluna); // le o input do usuario de linha e coluna
        cleanBuffer();

        while (linha < 1 || linha > (tamanho_matriz) || coluna < 1 || coluna > (tamanho_matriz)){ // garante que linha/coluna seja um numero de 1 a tamanho_matriz
            showMainMatrix(vidas);
            printf("\n\n");
            printf(" Elemento invalido selecionado. Por favor, insira linha e coluna novamente: ");
            scanf("%d %d", &linha, &coluna);
            cleanBuffer();
        }

        linha--;
        coluna--; // reduz em 1 o valor de linha e coluna para se adequar ao padrao posicional de vetores

        if (matriz_espelho[linha][coluna] == '0' && matriz_apagados[linha][coluna] == '1'){ // se o elemento da posicao escolhida pelo usuario na matriz-espelho for 0 (deve ser apagado) e se o elemento dessa posicao na matriz dos apagados for 1 (ainda nao foi apagado)

            matriz_apagados[linha][coluna] = '0'; // apaga o elemento na matriz dos apagados

            if (strcmp(matriz_espelho[linha], matriz_apagados[linha]) == 0 && somatorios_apagados_lin[linha] == '1'){ // se a linha da matriz dos apagados for igual a linha da matriz-espelho (fechou linha)
                somatorios_apagados_lin[linha] = '0'; // apaga o somatorio
                somatorios_fechados++; // incrementa o numero de somatorios fechados
                printf("\n Parabens, voce fechou a linha %d!", (linha+1));
            }
            if (strcmpcol(matriz_espelho, matriz_apagados, coluna) == 0 && somatorios_apagados_col[coluna] == '1'){ // se a coluna da matriz dos apagados for igual a coluna da matriz-espelho (fechou coluna)
                somatorios_apagados_col[coluna] = '0'; // apaga o somatorio
                somatorios_fechados++; // incrementa o numero de somatorios fechados
                printf("\n Parabens, voce fechou a coluna %d!", (coluna+1));
            }

            printf("\n Muito bem! Pressione <ENTER> para apagar o proximo numero ");
            cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna ao inicio do loop de buffer limpo
        }
        else if (matriz_espelho[linha][coluna] == '0' && matriz_apagados[linha][coluna] == '0'){ // se o elemento da posicao escolhida pelo usuario na matriz-espelho for 0 (deve ser apagado) e se o elemento dessa posicao na matriz dos apagados for 0 (ja foi apagado)
            printf("\n Voce ja apagou esse numero. Pressione <ENTER> para tentar novamente ");
            cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna ao inicio do loop de buffer limpo
        } 
        else{ // se o elemento nao deveria ser apagado
            vidas--; // reduz o numero de vidas
            if (vidas != 0){ // mostra mensagem de erro se o jogo nao tiver acabado
                printf("\n Ops, parece que esse numero deve ser mantido. Pressione <ENTER> para tentar novamente ");
                cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna ao inicio do loop de buffer limpo
            }
        }

        if (vidas == 0){ // se vidas = 0
            perdeu = 1; // marca que o usuario perdeu
            break; // sai do loop
        }
        else if (somatorios_fechados == (tamanho_matriz*2)){ // se todos os somatorios forem apagados
            venceu = 1; // marca que o usuario venceu
            fases_completas++; // incrementa o numero de fases completas
            break; // sai do loop
        }
    }

    if (perdeu){ // se o usuario perdeu o jogo

        showMainMatrix(vidas); 

        printf("\n\n Que pena, voce perdeu! Pressione <ENTER> para retornar ao Menu Principal ");

        resetMatrixes(); // reseta as matrizes e strings

        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna ao menu principal de buffer limpo
    }
    else if (venceu){ // se o usuario venceu o jogo

        showMainMatrix(vidas);

        printf("\n\n Parabens, voce venceu! Pressione <ENTER> para retornar ao Menu Principal ");

        if (modo == MODO_INICIANTE && fase_iniciante < 4){ // se nao for a ultima fase do Iniciante
            fase_iniciante++; // incrementa a fase atual do Iniciante
            pontos+=50; // adiciona 50 pontos a pontuacao do usuario
        }
        else if (modo == MODO_INICIANTE && fase_iniciante == 4){ // se for a ultima fase do Iniciante
            fase_iniciante++; // incrementa a fase atual do Iniciante
            modoIntermediario(); // muda para o modo Intermediario automaticamente na transicao das fases
            pontos+=50; // adiciona 50 pontos a pontuacao do usuario
        }   
        else if (modo == MODO_INTERMEDIARIO && fase_intermediario < 4){ // se nao for a ultima fase do Intermediario
            fase_intermediario++; // incrementa a fase atual do Intermediario
            pontos+=100; // adiciona 100 pontos a pontuacao do usuario
        }
        else if (modo == MODO_INTERMEDIARIO && fase_intermediario == 4){ // se for a ultima fase do Intermediario
            fase_intermediario++; // incrementa a fase atual do Intermediario
            modoAvancado(); // muda para o modo Avancado automaticamente na transicao das fases
            pontos+=100; // adiciona 100 pontos a pontuacao do usuario
        }
        else if (modo == MODO_AVANCADO && fase_avancado < 5){ // se for o modo Avancado
            fase_avancado++; // incrementa a fase atual do Avancado
            pontos+=200; // adiciona 200 pontos a pontuacao do usuario
        }

        add2Rank(); // atualiza a pontuacao do jogador no Ranking

        resetMatrixes(); // reseta as matrizes e strings

        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna ao menu principal de buffer limpo
    }
}

// estabelece os dados necessarios para o inicio da gameplay
void jogar(){

    int jogar = 1; // flag que marca se o usuario pode ou nao jogar mais uma fase do modo atual

    if (modo == MODO_INICIANTE && fase_iniciante == 5){ // se o modo for Iniciante e a fase for 5 (indica que todas as 4 fases foram completas)
        cleanScreen();
        printf(" *** VOCE ZEROU O MODO INICIANTE! ***\n\n"); // mostra uma mensagem de que o modo foi zerado
        printf(" Caso queira continuar jogando, troque as configuracoes para outra dificuldade\n\n");
        printf(" Pressione <ENTER> para voltar ao Menu Principal ");
        jogar = 0;
        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
    }
    else if (modo == MODO_INICIANTE){ // se nao se o modo for Iniciante
        fseek(fd, ((fase_iniciante - 1)*OFFSET_INICIANTE), SEEK_SET); // pula fd para o endereço dos dados da fase atual do modo Iniciante
    }
    else if (modo == MODO_INTERMEDIARIO && fase_intermediario == 5){ // se o modo for Intermediario e a fase for 5 (indica que todas as 4 fases foram completas)
        cleanScreen();
        printf(" *** VOCE ZEROU O MODO INTERMEDIARIO! ***\n\n"); // mostra uma mensagem de que o modo foi zerado
        printf(" Caso queira continuar jogando, troque as configuracoes para outra dificuldade\n\n");
        printf(" Pressione <ENTER> para voltar ao Menu Principal ");
        jogar = 0;
        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
    }   
    else if (modo == MODO_INTERMEDIARIO){ // se nao se o modo for Intermediario
        fseek(fd, ((fase_intermediario - 1)*OFFSET_INTERMEDIARIO), SEEK_SET); // pula fd para o endereço dos dados da fase atual do modo Intermediario
    }
    else if (modo == MODO_AVANCADO && fase_avancado == 5){ // se o modo for Avancado e a fase for 5 (indica que todas as 4 fases foram completas)
        cleanScreen();
        printf(" *** VOCE ZEROU O MODO AVANCADO! ***\n\n"); // mostra uma mensagem de que o modo foi zerado
        printf(" Caso queira continuar jogando, troque as configuracoes para outra dificuldade\n\n");
        printf(" Pressione <ENTER> para voltar ao Menu Principal ");
        jogar = 0;
        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
    }
    else if (modo == MODO_AVANCADO){ // se nao se o modo for Avancado
        fseek(fd, ((fase_avancado - 1)*OFFSET_AVANCADO), SEEK_SET); // pula fd para o endereço dos dados da fase atual do modo Avancado
    }

    if (jogar == 1){ // se jogar = 1 (as 4 fases do modo atual nao foram completas)
        scanMatrix(); // le os dados da matriz de "arquivo.txt" para a memoria
        scanSomatorios(); // le os dados dos somatorios de "arquivo.txt" para a memoria
        scanMirrorMatrix(); // le os dados da matriz-espelho de "arquivo.txt" para a memoria

        gameplay(); // executa a gameplay do jogo
    }

    fclose(fd); // ao fim da gameplay, retorna ao menu principal de arquivo fechado
}

// apaga todos os dados contidos no Ranking
void zerarRanking(){
    fp = fopen("Ranking.bin", "w+b"); // abre o arquivo "Ranking.bin" com "w+b", o que apaga os dados de "Ranking.bin" caso ele ja exista
    fclose(fp); // fecha o arquivo
    printf("\n Ranking zerado!\n");
    printf(" Pressione <ENTER> para voltar as Configuracoes ");
    cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
}

// limpa a tela e mostra a mensagem de zerar ranking
void showConfirmaZerar(){

    cleanScreen();

    printf(" *** ATENCAO ***\n\n");

    printf(" Zerar o Ranking deletara todos os nicknames e pontuacoes de\n");
    printf(" jogadores anteriores, tem certeza que deseja continuar?\n\n");

    printf(" Digite \"s\" para confirmar ou \"n\" para cancelar: ");
}

// le a opcao do usuario, verifica se ela eh valida e apaga o ranking se necessario
void scanConfirmaZerar(){ 

    /* preferi uma abordagem de leitura por string por possibilitar que o usuario consiga escrever "s/S" ou "n/N" de varias maneiras,
    inclusive precedidos\sucedidos de espacos em branco, mas ainda garantindo seguranca para casos de um input qualquer digitado sem querer que comece com "s"/"n" */
    
    char option[100]; 
    int i = 0, invalid = 0, sim = 0, nao = 0;

    scanf("%[^\n]", option);  // le a string incluindo caracteres "espaco"
    cleanBuffer(); // le o '\n' ao final da string

    while (option[i] != '\0'){ // verifica quantos "s"\"n" a string tem, e verifica a presenca de caracteres invalidos (diferentes de "s", "n" ou " ")
        if (option[i] != ' ' && option[i] != 's' && option[i] != 'S' && option[i] != 'n' && option[i] != 'N'){
            invalid = 1;
            break;
        }
        if (option[i] == 's' || option[i] == 'S'){
            sim+=1;
        }
        if (option[i] == 'n' || option[i] == 'N'){
            nao+=1;
        }
        i++;
    }                   

    while (invalid == 1 || sim == nao || sim > 1 || nao > 1){ // garante que option contenha, obrigatoriamente, apenas 1 caracter "n" ou apenas 1 caracter "s", desconsiderando os espacos

        // apenas repete o codigo acima ate que a entrada seja valida
        showConfirmaZerar();
        printf("\n\n");
        printf(" Opcao invalida identificada. Por favor, insira os dados novamente: ");

        i = 0, invalid = 0, sim = 0, nao = 0;

        scanf("%[^\n]", option);
        cleanBuffer();
        
        while (option[i] != '\0'){
            if (option[i] != ' ' && option[i] != 's' && option[i] != 'S' && option[i] != 'n' && option[i] != 'N'){
                invalid = 1;
                break;
            }
            if (option[i] == 's' || option[i] == 'S'){
                sim+=1;
            }
            if (option[i] == 'n' || option[i] == 'N'){
                nao+=1;
            }
            i++;
        }
    }

    if (sim) zerarRanking(); // se a opcao for s, zera o Ranking
    // se a opcao escolhida for "n", apenas retorna ao loop das configuracoes
}

// mostra a mensagem de confirmacao de zerar Ranking e le um input do usuario
void msgZerarRanking(){
    showConfirmaZerar();
    scanConfirmaZerar();
}

// limpa a tela e mostra as opcoes de dificuldade
void showDificuldade(){

    cleanScreen();

    printf(" *** SELECIONE A DIFICULDADE DE JOGO ***\n\n");

    printf(" 1 - Iniciante\n");
    printf(" 2 - Intermediario\n");
    printf(" 3 - Avancado\n\n");
    printf(" 4 - Voltar ao Menu de Configuracoes\n\n");

    printf(" Insira a opcao desejada: ");
}

// le a opcao do usuario, verifica se ela eh valida e modifica a dificuldade do jogo caso desejado
void scanDificuldade(){

    int option = 0; // inicializa option com 0 para garantir que o usuario escolha uma opcao valida por conta propria

    scanf("%d", &option);
    cleanBuffer();                    

    while (option < 1 || option > 4){ // garante que option seja um numero de 1 a 3
        showDificuldade();
        printf("\n\n");
        printf(" Opcao invalida identificada. Por favor, insira os dados novamente: ");
        scanf("%d", &option);
        cleanBuffer();
    }

    if (option == 1){
        modoIniciante(); // configura o jogo para o modo iniciante
        printf("\n");
        printf(" Configurado para a dificuldade INICIANTE\n");
        printf(" Pressione <ENTER> para voltar as Configuracoes: ");
        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
    }
    else if (option == 2){
        modoIntermediario(); // configura o jogo para o modo intermediario
        printf("\n");
        printf(" Configurado para a dificuldade INTERMEDIARIO\n");
        printf(" Pressione <ENTER> para voltar as Configuracoes: ");
        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
    } 
    else if (option == 3) {
        modoAvancado(); // configura o jogo para o modo avancado
        printf("\n");
        printf(" Configurado para a dificuldade AVANCADO\n");
        printf(" Pressione <ENTER> para voltar as Configuracoes: ");
        cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
    }
    // se a opcao escolhida for 4, apenas retorna ao loop das configuracoes
}

// mostra as opcoes de dificuldade na tela e le um input do usuario
void dificuldade(){
    showDificuldade();
    scanDificuldade();
}

// limpa a tela e mostra o menu de configuracoes
void showConfig(){

    cleanScreen();

    printf(" *** CONFIGURACOES ***\n\n");

    printf(" 1 - Zerar Ranking\n");
    printf(" 2 - Modo de Dificuldade\n");
    printf(" 3 - Voltar ao Menu Principal\n\n");

    printf(" Insira a opcao desejada: ");
}

// le a opcao escolhida pelo usuario, verifica se ela eh vaida e redireciona-o para uma das 3 opcoes das configuracoes
void scanConfig(){

    int option = 0; // inicializa option com 0 para garantir que o usuario escolha uma opcao valida por conta propria

    scanf("%d", &option);
    cleanBuffer();                    

    while (option < 1 || option > 3){ // garante que option seja um numero de 1 a 3
        showConfig();
        printf("\n\n");
        printf(" Opcao invalida identificada. Por favor, insira os dados novamente: ");
        scanf("%d", &option);
        cleanBuffer();
    }

    if (option == 1) msgZerarRanking(); // mostra a mensagem de confirmacao de zerar Ranking na tela
    else if (option == 2) dificuldade(); // mostra as opcoes de dificuldade
    
    if (option != 3){ // se a opcao for 3 (voltar ao menu principal), apenas retorna ao loop do menu principal. Caso contrario, a recursao garante o reinicio do loop das configuracoes
        showConfig();
        scanConfig();
    }
}

// mostra o menu de configuracoes e le o input do usuario
void configuracoes(){
    showConfig();
    scanConfig();
}

// limpa a tela e mostra as instrucoes do jogo
void showInstr(){

    cleanScreen();

    printf(" ########################################################################\n");

    printf("                           Some-se Quem Puder!                         \n\n");

    printf("  Arquimedes, sequestrador famoso por usar a aritmetica para pressionar\n");
    printf("          suas vitimas, esta a solta na Cidade de Soma-lia...           \n\n");

    printf("                      E ELE SEQUESTROU SUA FAMILIA!                    \n\n");

    printf(" Para salva-la, ele exige a resolucao de 3 desafios envolvendo matrizes,\n");
    printf("           e enviou uma carta para voce contendo as instrucoes:         \n\n");

    printf("  >  Disposicao das matrizes: as matrizes possuem tamanhos que variam de\n");
    printf(" acordo com a dificuldade (iniciante: 4x4; intermediaria: 6x6; avancada:\n");
    printf(" 7x7). Na extremidade superior de cada coluna e na extremidade esquerda\n");
    printf(" de cada linha estao numeros de dois digitos denominados \"SOMATORIOS\".\n\n");

    printf("  >  Acoes: o desafiado pode apagar numeros da matriz indicando linha e\n");
    printf(" coluna do numero desejado (ex: o numero contido na linha 1 e na coluna\n");
    printf(" 2 pode ser apagado ao digitar \"1 2\"). Se errar, perde 1 de 5 vidas.\n\n");

    printf("  >  Objetivo: o desafiado deve apagar numeros especificos da matriz ate\n");
    printf(" satisfazer a Condicao de Arquimedes: a soma dos numeros restantes numa\n");
    printf(" linha deve ser igual ao \"SOMATORIO\" daquela linha, e a soma dos numeros\n");
    printf(" restantes numa coluna deve ser igual ao \"SOMATORIO\" daquela coluna. Isso\n");
    printf(" deve acontecer em todas as linhas e colunas da matriz.\n\n");

    printf("            PS: seja rapido, Arquimedes nao eh de esperar...           \n\n");

    printf(" ########################################################################\n\n");

    printf(" Pressione <ENTER> para voltar ao Menu Principal ");
}

// mostra as instrucoes na tela e le o input do ususario
void instrucoes(){
    showInstr();
    cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna ao loop do menu principal de buffer limpo
}

void ranking(){

    playerData ranking[500]; // Estrutura para armazenar os dados dos jogadores
    int n = 0; // quantidade de nomes lidos
    int i, j; 

    // reseta o struct
    for (i = 0; i < 500; i++){
        for (j = 0; j < 21; j++){
            ranking[i].nome[j] = '\0';
        }
        ranking[i].pontuacao = 0;
    }

    fp = fopen("Ranking.bin", "rb"); // abre o arquivo para leitura

    if (fp == NULL){ 
        printf ("Erro em abrir o arquivo (o arquivo \"Ranking.bin\" nao existe)"); 
        return;
    }

    while (fread(&ranking[n], sizeof(playerData), 1, fp) == 1){ // fread retorna 1 caso o elemento seja lido corretmente, o que garante a continuidade do loop ate o ultimo jogador ser lido
        n++; // incrementa o contador de jogadores lidos
        if (n == 500){ // interrompe a leitura caso 500 jogadores sejam lidos
            break;
        }
    }

    multBubbleSort(ranking, n); // ordena o vetor de struct em ordem decrescente de pontos e depois em ordem lexicografica

    if (n > 0){ // se n for maior que 0 (pelo menos 1 jogador esta gravado em "Ranking.bin")
        printf("\n *** RANKING *** \n\n");
        for (i = 0; i < n; i++){
            printf(" %d - %s %dpts\n", (i+1), ranking[i].nome, ranking[i].pontuacao); // mostra o ranking ordenado na tela
        }
    }
    else{ // se n for 0 (nenhum nome lido)
        printf("\n *** O RANKING ESTA VAZIO *** \n\n");
        printf(" Pontuacao de %s - %dpts\n", nickname, pontos); // mostra somente o nome do usuaruio atual e sua pontuacao
        printf("\n Venca uma partida para cadastrar sua pontuacao no Ranking!\n");
    }

    printf("\n Pressione <ENTER> para ocultar o Ranking ");

    cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo

    fclose(fp); // fecha o arquivo
}

// encerra o programa
void sair(){
    add2Rank(); // adiciona o jogador e sua pontuacao no Ranking
    cleanScreen(); // limpa a tela
    exit(0); // fecha o programa
}

// mostra os creditos se o jogador completar as 12 fases do jogo
void showCreditos(){

    cleanScreen();

    printf(" ######################################\n\n");

    printf("                PARABENS!              \n\n");

    printf(" Voce zerou o jogo, obrigado por jogar!\n");
    printf(" Criado por: Pedro Marcinoni\n\n");

    printf(" ######################################\n\n");

    printf(" Pressione <ENTER> para retornar ao Menu Principal ");

    cleanBuffer(); // le um input qualquer do usuario e, quando encontrar um '\n' (< ENTER >), retorna de buffer limpo
}

// limpa a tela e mostra o menu principal
void showMainMenu(){

    cleanScreen();

    printf(" |~@ Some-se Quem Puder! @~|\n\n");

    printf(" 1 - Jogar\n");
    printf(" 2 - Configuracoes\n");
    printf(" 3 - Instrucoes\n");
    printf(" 4 - Ranking\n");
    printf(" 5 - Sair\n\n");

    printf(" Insira a opcao desejada: ");
}

// recebe a opcao do usuario, verifica se ela eh valida e redireciona-o para uma das 5 partes seguintes do programa
void scanMainMenu(){

    int option = 0; // inicializa option com 0 para garantir que o usuario escolha uma opcao valida por conta propria

    scanf("%d", &option);
    cleanBuffer();                    

    while (option < 1 || option > 5){ // garante que option seja um numero de 1 a 5
        showMainMenu();
        printf("\n\n");
        printf(" Opcao invalida identificada. Por favor, insira os dados novamente: ");
        scanf("%d", &option);
        cleanBuffer();
    }

    if (option == OP_JOGAR) { 
        fd = fopen(arquivo, "r"); // abre o "arquivo.txt" para leitura (o fechamento de arquivo ocorre na funcao "jogar()")

        if (fd == NULL){
            printf("Erro em abrir o arquivo (o arquivo nao existe)");
            getchar();
        }

        jogar(); // le as informacoes do "arquivo.txt" atual e organiza as configuracoes do jogo para o inicio da gameplay 
    }
    else if (option == OP_CONFIG) configuracoes(); // mostra o menu de configuracoes e le o input do usuario
    else if (option == OP_INSTR) instrucoes(); // mostra as instrucoes
    else if (option == OP_RANK) ranking(); // mostra o ranking
    else if (option == OP_SAIR) sair(); // fecha o programa

    if (fases_completas == 12 && creditos == 0){ // se as 12 fases estiverem completas e os creditos nao foram acessados
        showCreditos(); // mostra os creditos na tela
        creditos = 1; // marca que os creditos foram acessados
    }

    showMainMenu();
    scanMainMenu(); // se alguma funcao precisar retornar a scanMainMenu, essa recursao garante o reinicio do loop e a releitura dos inputs
}

int main(){

    cleanScreen(); // limpa a tela

    printf(" Bem-vindo(a) ao Some-se Quem Puder!\n\n");
    printf(" Insira seu nickname (serao considerados apenas os 20 primeiros caracteres sem espaco): \n\n");
    printf(" ");

    scanf("%20s", nickname); //le apenas os 20 primeiros caracteres do nome digitado
    cleanBuffer(); // limpa o resto do buffer de entrada se o nickname ultrapassar 20 caracteres

    add2Rank(); // adiciona o jogador e sua pontuacao no Ranking

    showMainMenu(); // mostra o menu principal
    scanMainMenu(); // le a opcao escolhida pelo jogador no menu principal

    return 0;
}

///////////////////////////
// *** Fim do codigo *** //
//   obrigado por ler!   //
///////////////////////////
