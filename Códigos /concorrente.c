#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
//estrutura de dados 
typedef struct{
    double x0;
    double x1;
    int n_func;
}ret;

//Globais
int nThreads;
int tam_buffer; // Indica a capacidade total do buffer
int tam_ocupado = 0; //Valor eh incrementado e decrementado conforme as threads insere e retira do buffer, tambem serve como stack pointer do buffer.
ret * buffer;
double soma = 0;
double Epsilon;
pthread_mutex_t mutex;
pthread_cond_t cond;
int count = 0; // Conta quantas threads estão em espera para a retirar do buffer

//Funcao chamada quando uma thread percebe que o buffer encheu (tam_ocupado = tam_buffer)
//Tracar exclusao mutua antes de chamar a funcao 
void aloca_buffer ( ) {
    int new_tam_buffer = tam_buffer*2;
    ret *new_buffer;
    
    new_buffer = malloc(sizeof(ret) * new_tam_buffer); if(!new_buffer) exit(-1);

    for (int i = 0; i < tam_buffer; i++) {
        new_buffer[i] = buffer[i];
    }

    tam_buffer = new_tam_buffer;
    buffer = new_buffer;

}
//recebe do usuario qual função ele quer analizar
int menu(){
    int numero_funcao;
    printf("Escolha qual das seguintes funções abaixo você gostaria de utilizar:\n");
    printf("Se for a funcao f(x) = 1 + x -> escreva 1 \n ");
    printf("Se for a funcao f(x) = raiz(1 - x^2) -> escreva 2 \n ");
    printf("Se for a funcao f(x) = raiz(1 + x^4) -> escreva 3 \n ");
    printf("Se for a funcao f(x) = sen(x^2) -> escreva 4 \n ");
    printf("Se for a funcao f(x) = cos(e^-x) -> escreva 5 \n ");
    printf("Se for a funcao f(x) = cos(e^-x)*x -> escreva 6 \n ");
    printf("Se for a funcao f(x) = cos(e^-x)*(0,005*x^3 + 1) -> escreva 7 \n ");
    scanf("%d", &numero_funcao);
    
    return numero_funcao;
    
}
//funcões dispopniveis
double funcao1(double x){
    return x+1.0;
}
double funcao2(double x){
    return sqrt(1.0-pow(x,2));
}
double funcao3(double x){
    return sqrt(1.0+pow(x,4));
}
double funcao4(double x){
    return sin(pow(x,2));
}
double funcao5(double x){
    return cos(pow(exp(x), -1));
}
double funcao6(double x){
    return cos(pow(exp(x), -1))*x;
}
double funcao7(double x){
    return cos(pow(exp(x), -1))*(0.005*pow(x,3)+1.0);
}
//Faz o calculo da funcao indicada de acordo com o x passado para ela;
double chama_funcao(int y, double x){
    double resposta;
    if (y==1)
        resposta =funcao1(x);
    if(y==2)
        resposta =funcao2(x);
    if(y==3)
        resposta = funcao3(x);
    if(y==4)
        resposta = funcao4(x);
    if(y==5)
        resposta = funcao5(x);
    if(y==6)
        resposta = funcao6(x);
    if(y==7)
        resposta = funcao7(x);
    return resposta;
}
// Estruturas auxiliares para o calculo das areas.
typedef struct{
    double x0;
    double x1;
    double result_func;
}ajuda;

typedef struct{
    double menor_esq;
    double menor_dir;
    double maior;
}Areas;
Areas calculaAreas(double x0, double x1, int n_func){
    double AreaMaior,AreaMenor_esq,AreaMenor_dir;
    ajuda maior;
    Areas area;
    maior.x0 = x0;
    maior.x1 = x1;
    maior.result_func = chama_funcao(n_func,(x1+x0)/2.0);
    AreaMaior = fabs((maior.x1-maior.x0)) * maior.result_func;
    ajuda menor_esq;
    menor_esq.x0 = x0;
    menor_esq.x1 = (x1 + x0)/2.0;
    menor_esq.result_func = chama_funcao(n_func,(menor_esq.x1 + menor_esq.x0)/2.0);
    AreaMenor_esq = fabs((menor_esq.x1-menor_esq.x0)) * menor_esq.result_func;
    ajuda menor_dir;
    menor_dir.x0 = (x1 + x0)/2.0;
    menor_dir.x1 = x1;
    menor_dir.result_func = chama_funcao(n_func,(menor_dir.x1 + menor_dir.x0)/2.0);
    AreaMenor_dir = fabs((menor_dir.x1-menor_dir.x0))*menor_dir.result_func;
    area.menor_esq = AreaMenor_esq;
    area.menor_dir = AreaMenor_dir;
    area.maior =   AreaMaior;
    return area;
}
void adiciona_Buffer(double x0, double x1, int n_func){
        
        if(tam_ocupado == tam_buffer){
            aloca_buffer();
        }
       
        double delta = (x0 + x1)/2;
        
        ret aux;
        aux.x0= x0;
        aux.x1 = delta;
        aux.n_func = n_func;
        
        ret aux2;
        aux2.x0= delta;
        aux2.x1 = x1;
        aux2.n_func = n_func;
       
        buffer[tam_ocupado]= aux;
        tam_ocupado++;
        buffer[tam_ocupado]=aux2;
        tam_ocupado++;

}
void *concorrente(void *arg){

    double soma_local, AreaMaior, AreaMenor_esq, AreaMenor_dir;
    Areas area_return;
    //int tid = *(int*)arg;

    while ( 1 ) {

        //Parte consumidora ( retira elemento do buffer e verifica se pode finalizar )
        pthread_mutex_lock(&mutex);
        while ( tam_ocupado == 0) {
            count++;
            if ( count == nThreads){ // Finalizacao em cadeia, se for a ultima thread a ficar em espera e nao tiver nada no buffer.
                pthread_cond_signal (&cond);
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&cond, &mutex);
            count--;
        }
        tam_ocupado = tam_ocupado - 1;
        ret area_local = buffer[tam_ocupado];
        pthread_mutex_unlock(&mutex);
     
        //Parte em paralelo
        area_return = calculaAreas(area_local.x0, area_local.x1, area_local.n_func);
        AreaMenor_esq = area_return.menor_esq;
        AreaMenor_dir = area_return.menor_dir;
        AreaMaior = area_return.maior;

        double diferenca = AreaMaior - (AreaMenor_esq + AreaMenor_dir);
  
        if ( fabs(diferenca) > Epsilon ) { 
            
            //Parte produtora ( insere dois elementos no buffer)
            pthread_mutex_lock(&mutex);
            adiciona_Buffer(area_local.x0, area_local.x1, area_local.n_func);
            pthread_cond_signal (&cond);
            pthread_cond_signal (&cond);
            pthread_mutex_unlock(&mutex);
            
        } else {

            soma_local = AreaMaior;
            
            //adiciona o valor da soma no valor total;
            pthread_mutex_lock(&mutex);
            soma = soma + soma_local;
            pthread_mutex_unlock(&mutex);
        
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    
    //Inicialização
    double inicio, fim, delta1, delta2, delta3;
    
    //valida e recebe os valores de entrada
    if(argc < 5) {
        printf("Use: %s <numero de threads> <intervalo inicial> <intervalo final> <Erro>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int numero_funcao = menu();
    
    GET_TIME(inicio);
    //converte valores recebidos
    nThreads = atoi(argv[1]);
    Epsilon = atof(argv[4]);
    double intervalo_inicial = atof(argv[2]);
    double intervalo_final = atof(argv[3]);
    pthread_t tid[nThreads];
    //aloca inialmente espaço para o buffer
    buffer = malloc(sizeof(ret) * (nThreads*2)); if(!buffer) exit(-1);
    tam_buffer = nThreads*2;
    //recebe do usuario qual funcao ele quer analizar
    
    int *t;
    //divide o dominio da funcao de acordo com o numero de threads 
    for (int i =0; i<nThreads;i++){
        ret aux;
        if(i==nThreads-1){
            aux.x0=intervalo_final - ((intervalo_final-intervalo_inicial)/nThreads);
            aux.x1=intervalo_final;
        }
        else{
            aux.x0 = intervalo_inicial + ((intervalo_final-intervalo_inicial)/nThreads)*i;
            aux.x1 = aux.x0 + ((intervalo_final-intervalo_inicial)/nThreads);
        }
        aux.n_func = numero_funcao;
        buffer[i]=aux;
        tam_ocupado++;
    }
    //inicializa as variaveis de condição e mutex
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    GET_TIME(fim);
    delta1 = fim-inicio;
    //////////////////////////////////////////////

    //Parte que vai ser paralelizada
   
    //cria as nThreads threads
    GET_TIME(inicio);
    for(int i =0;i<nThreads; i++){
        t = malloc(sizeof(int)); if(t==NULL) return -1;
        *t = i;
        if (pthread_create(&tid[i], NULL,concorrente,(void *)t)) {
            printf("--ERRO: pthread_create()\n"); exit(-1); }
    }    
    for (int i=0; i<nThreads; i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1); 
        }

    }
    GET_TIME(fim);
    delta2 = fim-inicio;
    ///////////////////////////////////////////////////

    //Finalização
    GET_TIME(inicio);
    //destroi todas as variaveis de condição e mutex
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    free(buffer);
    GET_TIME(fim);
    delta3 = fim-inicio;
    ////////////////////////////////////////////////////

    //printa o valor da integral de acordo com esse metodo do ponto medio
    printf("O valor da integral da funcao numero %d \nNo intervalo inicial = %lf e no intervalo final = %lf será igual a: %lf\n", numero_funcao, intervalo_inicial, intervalo_final, soma);
    printf("O tempo das inicializações foi de: %lf segundos\n", delta1);
    printf("O tempo da parte paralelizada foi de: %lf segundos\n", delta2);
    printf("O tempo da finalização foi de: %lf segundos\n", delta3); 

    pthread_exit(NULL);

}