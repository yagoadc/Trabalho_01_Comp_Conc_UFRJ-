#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "timer.h"
double Epsilon;
typedef struct{
    double x0;
    double x1;
    double result_func;
}ret;
//Menu no qual o Usuario informa qual é a função que ele deseja usar
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

//Funcoes disponiveis para o calculo da integral

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

//Chama a funcao informada pelo usuario na hora desejada

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

//Funcao Recursiva que faz o calculo da integral

double Recursiva(double x0, double x1, int n_func){
    double soma, AreaMaior,AreaMenor_esq,AreaMenor_dir;
    ret maior;
    maior.x0 = x0;
    maior.x1 = x1;
    maior.result_func = chama_funcao(n_func,(x1+x0)/2.0);
    AreaMaior = fabs((maior.x1-maior.x0))*maior.result_func;
    ret menor_esq;
    menor_esq.x0 = x0;
    menor_esq.x1 = (x1  + x0)/2.0;
    menor_esq.result_func = chama_funcao(n_func,(menor_esq.x1 +menor_esq.x0)/2.0);
    AreaMenor_esq = fabs((menor_esq.x1-menor_esq.x0))*menor_esq.result_func;
    ret menor_dir;
    menor_dir.x0 = (x1 + x0)/2.0;
    menor_dir.x1 = x1;
    menor_dir.result_func = chama_funcao(n_func,(menor_dir.x1+menor_dir.x0)/2.0);
    AreaMenor_dir = fabs((menor_dir.x1-menor_dir.x0))*menor_dir.result_func;
    
    double diferenca = AreaMaior - ( AreaMenor_esq  + AreaMenor_dir);
        
    if( fabs(diferenca) > Epsilon )
        soma = Recursiva(menor_esq.x0, menor_esq.x1, n_func) + Recursiva(menor_dir.x0, menor_dir.x1, n_func);
    else
        soma = AreaMaior;
    return soma;
}
int main( int argc, char *argv[] ){
    //inicializações
    double intervalo_inicial, intervalo_final;
    int numero_funcao;
    double resposta, inicio, fim, delta1;

    if(argc < 3) {
        printf("Use: %s <intervalo inicial> <intervalo final> <Erro>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Epsilon = atof(argv[3]);
    intervalo_inicial = atof(argv[1]);
    intervalo_final = atof(argv[2]);

    /*printf("Entre o invervalo inicial: ");
    scanf("%lf", &intervalo_inicial);
    printf("Entre o invervalo final: ");
    scanf("%lf", &intervalo_final);
    printf("Entre o valor de Epsilon: ");
    scanf("%lf", &Epsilon);*/

    numero_funcao = menu();
    GET_TIME(inicio);
    //Chamada da Função Recursiva
    resposta = Recursiva(intervalo_inicial, intervalo_final, numero_funcao);
    GET_TIME(fim);
    delta1=fim-inicio;
    printf("O valor da Integral é : %lf.\n", resposta);
    printf("O Tempo total durante todo o processo eh de: %lf segundos.\n",delta1 );
    return 0;
}