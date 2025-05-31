#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "funcoes_tabela.h"
#include "estrutura_arquivo.h"
#include "io_arquivo.h"
#include "funcao_fornecida.h"

int main(){
    int modo, n;
    scanf("%d", &modo); // Lê a funcionalidade que será executada
    char nome_csv[25], nome_binario[25]; // Variáveis para armazenar o nome dos arquivos
    FILE *arquivo_csv, *arquivo_binario; // Ponteiros para os arquivos
    HEADER header;
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
            if (arquivo_binario != NULL) fclose(arquivo_binario);
            if (arquivo_csv != NULL) fclose(arquivo_csv);
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
        scanf(" %s %d", nome_binario, &n); // Le o número de buscas
        arquivo_binario = fopen(nome_binario, "rb");
        // Verifica se o arquivo está correto
        if (arquivo_binario == NULL){
            printf("Falha no processamento do arquivo.\n");
            break;
        }
        header = ler_header(arquivo_binario);
        // Loop que realiza as buscas
        for (int i = 0; i < n; i++){
            // Cria um registro modelo que define as condições
            REGISTRO reg_modelo = cria_modelo(); 
            // Chama a função para busca condicional  
            busca_condicional(arquivo_binario, reg_modelo, &header, IMPRIMIR);
            // Desaloca o registro modelo
            desaloca_struct_registro(reg_modelo);
        }
        // Fecha o arquivo
        fclose(arquivo_binario);
        break;
    
    // Deletar registros
    case 4:
        scanf(" %s %d", nome_binario, &n); // Le o número de buscas
        arquivo_binario = fopen(nome_binario, "rb+");
        // Verifica se o arquivo está correto
        if (arquivo_binario == NULL){
            printf("Falha no processamento do arquivo.\n");
            break;
        }
        header = ler_header(arquivo_binario);
        //o status no arquivo é 0, mas na struct é 1
        header.status = '0';
        update_header(arquivo_binario, header);
        header.status = '1'; 
        for (int i = 0; i < n; i++){
            REGISTRO reg_modelo = cria_modelo();
            busca_condicional(arquivo_binario, reg_modelo, &header, DELETAR);
            desaloca_struct_registro(reg_modelo);
        }
        update_header(arquivo_binario, header);
        fclose(arquivo_binario);
        binarioNaTela(nome_binario);
        break;
    // funcionalidade inexistente
    default:
        printf("Funcionalidade %d inexistente!", modo);
    }
    return 0;
}