#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes_tabela.h"
#include "io_arquivo.h"
#include "estrutura_arquivo.h"

// funções privadas
int compara_registros(REGISTRO *reg, REGISTRO *reg_modelo);

// Função CREATE TABLE
// Argumetos: Ponteiro dos arquivos csv e binário
void criar_tabela(FILE *arquivo_csv, FILE *arquivo_binario){
    HEADER header;
    long int tamanho;
    REGISTRO reg;

    fseek(arquivo_csv, 0, SEEK_END); // vai até o fim do arquivo csv
    tamanho = ftell(arquivo_csv); // descobre o tamanho desse arquivo
    fseek(arquivo_csv, 0, SEEK_SET); // volta ao inicio do arquivo csv

    header = ler_header_csv(arquivo_csv); // le primeira linha do csv, criando o header
    escreve_header(arquivo_binario, header); // escreve o header no arquivo binário

    // Loop da leitura e escrita de um registro
    while (ftell(arquivo_csv) < tamanho - 1){
        reg = ler_dado_csv(arquivo_csv);
        escreve_registro(arquivo_binario, reg, header);
        desaloca_struct_registro(reg);
        header.nroRegArq++;
    }

    header.proxByteOffset = ftell(arquivo_binario); // próximo byteoffset é o fim do arquivo
    header.status = '1'; // muda status para 1
    update_header(arquivo_binario, header); //atualiza o header
}

/// Função para printar um registro
// Argumentos: registro a ser printado e o header do arquivo
// Função que imprime um registro
void print_registro(REGISTRO reg, HEADER header){
    // Se for removido não printa
    // Verifica se o registro está removido
    if (reg.removido == '1'){ 
        return;
    }


    // Se não estiver removido, imprime o registro
    printf("%.23s: %d\n", header.descreveIdentificador, reg.idAttack);
    
    printf("%.27s: ", header.descreveYear);
    if (reg.year == -1) printf("NADA CONSTA\n");
    else printf("%d\n", reg.year);

    printf("%.26s: ", header.descreveCountry);
    if (reg.tmnCountry == 0) printf("NADA CONSTA\n");
    else printf("%s\n", reg.country);

    printf("%.38s: ", header.descreveTargetIndustry);
    if (reg.tmnTargetIndustry == 0 ) printf("NADA CONSTA\n");
    else printf("%s\n", reg.targetIndustry);

    printf("%.38s: ", header.descreveType);
    if (reg.tmnAttackType == 0) printf("NADA CONSTA\n");
    else printf("%s\n", reg.attackType);

    printf("%.28s: ", header.descreveFinancialLoss);
    if (reg.financialLoss == -1) printf("NADA CONSTA\n");
    else printf("%.2f\n", reg.financialLoss);

    printf("%.67s: ", header.descreveDefense);
    if (reg.tmnDefenseMechanism == 0) printf("NADA CONSTA\n");
    else printf("%s\n", reg.defenseMechanism);
    
    printf("\n"); // Pula uma linha no final do registro
}


// Função que busca e imprime todos os registros do arquivo
void busca_simples(FILE *arquivo){
    REGISTRO reg;
    HEADER header = ler_header(arquivo);
    // Verifica a consistência do arquivo
    if (header.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    // Verifica se há registros
    if (header.nroRegArq == 0){
        printf("Registro inexistente.\n");
        return;
    }
    // Loop para ler e imprimir
    for (int i = 0; i < header.nroRegArq; i++){
        reg = ler_registro(arquivo, header);
        print_registro(reg, header);
        desaloca_struct_registro(reg);
    }
}

// Função que busca e imprime todos os registros que atendam a uma certa condição
// Argumentos: o arquivo binario e um registro modelo para comparação
void busca_condicional(FILE *arquivo, REGISTRO reg_modelo){ 
    REGISTRO reg;
    HEADER header = ler_header(arquivo);
    int encontrou = 0;
    // Verifica a consistência do arquivo
    if (header.status == '0'){
        printf("Falha no processamento do arquivo.\n"); 
        return;
    }
    // Loop que compara os registros do arquivo com o registro modelo
    for (int i = 0; i < header.nroRegArq; i++){
        reg = ler_registro(arquivo, header);
        // Checa se o registro foi removido
        if (reg.removido == '1'){
            desaloca_struct_registro(reg);
            continue;
        }
        
        // Se atender às especificações, imprime o registro
        if (compara_registros(&reg,&reg_modelo)){
            print_registro(reg, header);
            encontrou = 1;
            if (reg_modelo.idAttack != -1) { // idAttack é único -> não precisa mais continuar a busca
                desaloca_struct_registro(reg);
                break; 
            } 
        }
        desaloca_struct_registro(reg);
    }
    
    // Se não encontrar nenhum registro com as condições, imprime "Registro inexistente"
    if (!encontrou){
        printf("Registro inexistente.\n\n");
    }
    printf("**********\n");
}

REGISTRO cria_modelo(){
    int m;
    // Inicializa os campos do registro modelo
    REGISTRO reg_modelo;
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
    return reg_modelo;
}

int compara_registros(REGISTRO *reg, REGISTRO *reg_modelo){
    // Checa se o tipo de busca é de ID e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->idAttack != -1 && reg_modelo->idAttack != reg->idAttack) return 0;
        // Checa se o tipo de busca é de Year e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->year != -1 && reg_modelo->year != reg->year) return 0;
        // Checa se o tipo de busca é de FinalcialLoss e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->financialLoss != -1 && reg_modelo->financialLoss != reg->financialLoss) return 0;
        // Checa se o tipo de busca é de Country e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->country != NULL && (!reg->tmnCountry || strcmp(reg_modelo->country, reg->country))) return 0;
        // Checa se o tipo de busca é de AttackType e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->attackType != NULL && (!reg->tmnAttackType || strcmp(reg_modelo->attackType, reg->attackType))) return 0;
        // Checa se o tipo de busca é de TargetIndustry e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->targetIndustry != NULL && (!reg->tmnTargetIndustry || strcmp(reg_modelo->targetIndustry, reg->targetIndustry))) return 0;
        // Checa se o tipo de busca é de DefenseMechanism e, se sim, se o valor é diferente do a ser buscado
        if (reg_modelo->defenseMechanism != NULL && (!reg->tmnDefenseMechanism || strcmp(reg_modelo->defenseMechanism, reg->defenseMechanism))) return 0;
        return 1;
}