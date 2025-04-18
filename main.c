#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "funcoes_tabela.h"
#include "estrutura_arquivo.h"
#include "io_arquivo.h"
#include "funcao_fornecida.h"

int main(){
    int modo;
    scanf("%d", &modo); // Lê a funcionalidade que será executada
    char nome_csv[25], nome_binario[25]; // Variáveis para armazenar o nome dos arquivos
    FILE *arquivo_csv, *arquivo_binario; // Ponteiros para os arquivos
    HEADER header;
    REGISTRO reg;
    // Switch case para executar a funcionalidade especificada 
    switch (modo){
    // criar arquivo da tabela
    case 1:
        scanf("%s  %s", nome_csv, nome_binario); //Le o nome do arquivo csv de entrada e o arquivo binário de saida
        
        // Abre os arquivos
        arquivo_csv = fopen(nome_csv, "r");
        arquivo_binario = fopen(nome_binario, "wb");

        // Verifica se os dois arquivos estão corretos
        if (arquivo_binario == NULL || arquivo_csv == NULL){ 
            printf("Falha no processamento do arquivo.\n");
            break;
        }
        
        // Chama a função para criar a tabela
        criar_tabela(arquivo_csv, arquivo_binario);

        // Fecha os arquivos
        fclose(arquivo_csv);
        fclose(arquivo_binario);
        
        // Chama a função fornecida binarioNaTela
        binarioNaTela(nome_binario);
        break;

    // selecionar dados da tabela
    case 2:
        scanf(" %s", nome_binario);
        arquivo_binario = fopen(nome_binario, "rb");
        // Verifica se o arquivo binário abriu corretamente
        if (arquivo_binario == NULL){
            printf("Falha no processamento do arquivo.\n");
            break;
        }
        // Chama a função para buscar e imprimir os registros do arquivo  
        busca_simples(arquivo_binario);
        // Fecha o arquivo
        fclose(arquivo_binario);
        
        break;

    // selecionar dados da tabela com condição
    case 3:
        int n; // Variável que armazena o número de buscas a serem realizadas
        scanf(" %s %d", nome_binario, &n); // Le o número de buscas
        arquivo_binario = fopen(nome_binario, "rb");
        // Verifica se o arquivo está correto
        if (arquivo_binario == NULL){
            printf("Falha no processamento do arquivo.\n");
            break;
        }
        // Loop que realiza as buscas
        for (int i = 0; i < n; i++){
            int m; // Variável que armazena o número de condições na busca
            REGISTRO reg_modelo; // Cria um registro modelo que define as condições
            // Inicializa os campos do registro modelo
            reg_modelo.attackType = NULL;
            reg_modelo.country = NULL;
            reg_modelo.targetIndustry = NULL;
            reg_modelo.defenseMechanism = NULL;
            reg_modelo.idAttack = -1;
            reg_modelo.financialLoss = -1;
            reg_modelo.year = -1;
            reg_modelo.tmnCountry = 0;
            reg_modelo.tmnAttackType = 0;
            reg_modelo.tmnTargetIndustry = 0;
            reg_modelo.tmnDefenseMechanism = 0;

            scanf("%d", &m); // Le o valor de m
            
            // Loop para definir as condições da busca
            for (int j = 0; j < m; j++){
                char nome_campo[25]; // Armazena o valor do campo em que será definido a condição
                scanf(" %s", nome_campo);
                // Armazena no campo especificado qual é a condição
                if (!strcmp(nome_campo, "idAttack")){
                    scanf("%d", &reg_modelo.idAttack);
                }
                else if (!strcmp(nome_campo, "year")){
                    scanf("%d", &reg_modelo.year);
                }
                else if (!strcmp(nome_campo, "financialLoss")){
                    scanf("%f", &reg_modelo.financialLoss);
                }
                else if (!strcmp(nome_campo, "country")){
                    reg_modelo.country = malloc(sizeof(char) * 100);
                    reg_modelo.tmnCountry = 100;
                    scan_quote_string(reg_modelo.country);
                }
                else if (!strcmp(nome_campo, "attackType")){
                    reg_modelo.attackType = malloc(sizeof(char) * 100);
                    reg_modelo.tmnAttackType = 100;
                    scan_quote_string(reg_modelo.attackType);
                }
                else if (!strcmp(nome_campo, "targetIndustry")){
                    reg_modelo.targetIndustry = malloc(sizeof(char) * 100);
                    reg_modelo.tmnTargetIndustry = 100;
                    scan_quote_string(reg_modelo.targetIndustry);
                }
                else if (!strcmp(nome_campo, "defenseMechanism")){
                    reg_modelo.defenseMechanism = malloc(sizeof(char) * 100);
                    reg_modelo.tmnDefenseMechanism = 100;
                    scan_quote_string(reg_modelo.defenseMechanism);
                }

            }
            // Chama a função para busca condicional  
            busca_condicional(arquivo_binario, reg_modelo);
            // Desaloca o registro modelo
            desaloca_struct_registro(reg_modelo);
        }
        // Fecha o arquivo
        fclose(arquivo_binario);
        break;
    // funcionalidade inexistente
    default:
        printf("Funcionalidade %d inexistente!", modo);
    }
    return 0;
}