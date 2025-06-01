#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes_tabela.h"
#include "io_arquivo.h"
#include "estrutura_arquivo.h"


// funções privadas
int compara_registros(REGISTRO *reg, REGISTRO *reg_modelo);
long int percorre_lista(FILE *arquivo, int *tamanho, HEADER *header);

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
void print_registro(REGISTRO reg, HEADER *header){
    // Se for removido não printa
    // Verifica se o registro está removido
    if (reg.removido == '1'){ 
        // printf("REMOVIDO\nPROXIMO: %ld\n\n", reg.prox); //debug
        return;
    }


    // Se não estiver removido, imprime o registro
    printf("%.23s: %d\n", header->descreveIdentificador, reg.idAttack);
    
    printf("%.27s: ", header->descreveYear);
    if (reg.year == -1) printf("NADA CONSTA\n");
    else printf("%d\n", reg.year);

    printf("%.26s: ", header->descreveCountry);
    if (reg.tmnCountry == 0) printf("NADA CONSTA\n");
    else printf("%s\n", reg.country);

    printf("%.38s: ", header->descreveTargetIndustry);
    if (reg.tmnTargetIndustry == 0 ) printf("NADA CONSTA\n");
    else printf("%s\n", reg.targetIndustry);

    printf("%.38s: ", header->descreveType);
    if (reg.tmnAttackType == 0) printf("NADA CONSTA\n");
    else printf("%s\n", reg.attackType);

    printf("%.28s: ", header->descreveFinancialLoss);
    if (reg.financialLoss == -1) printf("NADA CONSTA\n");
    else printf("%.2f\n", reg.financialLoss);

    printf("%.67s: ", header->descreveDefense);
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

    // printf("TOPO: %ld\n\n", header.topo); //debug

    // Loop para ler e imprimir
    for (int i = 0; i < header.nroRegArq + header.nroRegRem; i++){
        // printf("Posição: %ld\n", ftell(arquivo)); //debug
        reg = ler_registro(arquivo, &header);
        print_registro(reg, &header);
        desaloca_struct_registro(reg);
    }
}

// Função que busca e imprime todos os registros que atendam a uma certa condição
// Argumentos: o arquivo binario e um registro modelo para comparação
int busca_condicional(FILE *arquivo, REGISTRO reg_modelo, HEADER *header, int modo){ 
    REGISTRO reg;
    int encontrou = 0;
    // Verifica a consistência do arquivo
    if (header->status == '0'){
        printf("Falha no processamento do arquivo.\n"); 
        return 0;
    }
    if (modo == DELETAR) header->status = '0';
    fseek(arquivo, 276, SEEK_SET); // pular para o inicio dos registros de dados
    // Loop que compara os registros do arquivo com o registro modelo
    for (int i = 0; i < header->nroRegArq + header->nroRegRem; i++){
        reg = ler_registro(arquivo, header);
        // Checa se o registro foi removido
        if (reg.removido == '1'){
            desaloca_struct_registro(reg);
            continue;
        }
        
        // Se atender às especificações, faz o que o modo especificar com o registro
        if (compara_registros(&reg, &reg_modelo)){
            if (modo == IMPRIMIR) print_registro(reg, header);
            else if (modo == DELETAR){
                fseek(arquivo, -reg.tamanhoRegistro - 5, SEEK_CUR);
                remocao_logica(arquivo, header);
            }
            encontrou = 1;
            if (reg_modelo.idAttack != -1) { // idAttack é único -> não precisa mais continuar a busca
                desaloca_struct_registro(reg);
                break; 
            } 
        }
        desaloca_struct_registro(reg);
    }
    
    if (modo == IMPRIMIR){
        // Se não encontrar nenhum registro com as condições, imprime "Registro inexistente"
        if (!encontrou){
            printf("Registro inexistente.\n\n");
        }
        printf("**********\n");
    }
    header->status = '1';
    return 1;
}

REGISTRO cria_modelo(int modo){
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
    reg_modelo.removido = '0';
    reg_modelo.prox = -1;

    if (modo == CAMPOS){
        int m;
        scanf("%d", &m); // Le o valor de m
        // Loop para definir as condições da busca
        for (int j = 0; j < m; j++){
            char nome_campo[25]; // Armazena o valor do campo em que será definido a condição
            scanf(" %s", nome_campo);
            // Armazena no campo especificado qual é a condição
            if (!strcmp(nome_campo, "idAttack")){
                scanf("%d", &(reg_modelo.idAttack));
            }
            else if (!strcmp(nome_campo, "year")){
                scanf("%d", &(reg_modelo.year));
            }
            else if (!strcmp(nome_campo, "financialLoss")){
                scanf("%f", &(reg_modelo.financialLoss));
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
    }
    else if (modo == COMPLETO){
        char *string = calloc(50, sizeof(char));
        int tamanho;
        //leitura idAttack
        scanf("%d", &reg_modelo.idAttack);
        //leitura year
        scan_quote_string(string);
        tamanho = strlen(string);
        reg_modelo.year = tamanho ? atoi(string) : -1;
        for (int j = 0; j < tamanho; j++) string[j] = '\0';
        //leitura financialLoss
        scan_quote_string(string);
        tamanho = strlen(string);
        reg_modelo.financialLoss = tamanho ? atof(string) : -1.0f;
        for (int j = 0; j < tamanho; j++) string[j] = '\0';
        tamanho = 0;
        // leitura country
        scan_quote_string(string);
        reg_modelo.tmnCountry = strlen(string);
        if (reg_modelo.tmnCountry){
            reg_modelo.country = malloc(sizeof(char) * reg_modelo.tmnCountry);
            strcpy(reg_modelo.country, string);
            tamanho += reg_modelo.tmnCountry + 2;
        }
        else reg_modelo.country = NULL;
        for (int j = 0; j < reg_modelo.tmnCountry; j++) string[j] = '\0';
        // leitura attackType
        scan_quote_string(string);
        reg_modelo.tmnAttackType = strlen(string);
        if (reg_modelo.tmnAttackType){
            reg_modelo.attackType = malloc(sizeof(char) * reg_modelo.tmnAttackType);
            strcpy(reg_modelo.attackType, string);
            tamanho += reg_modelo.tmnAttackType + 2;
        }
        else reg_modelo.attackType = NULL;
        for (int j = 0; j < reg_modelo.tmnAttackType; j++) string[j] = '\0';
        // leitura targetIndustry
        scan_quote_string(string);
        reg_modelo.tmnTargetIndustry = strlen(string);
        if (reg_modelo.tmnTargetIndustry){
            reg_modelo.targetIndustry = malloc(sizeof(char) * reg_modelo.tmnTargetIndustry);
            strcpy(reg_modelo.targetIndustry, string);
            tamanho += reg_modelo.tmnTargetIndustry + 2;
        }
        else reg_modelo.targetIndustry = NULL;
        for (int j = 0; j < reg_modelo.tmnTargetIndustry; j++) string[j] = '\0';
        // leitura defenseMechanism
        scan_quote_string(string);
        reg_modelo.tmnDefenseMechanism = strlen(string);
        if (reg_modelo.tmnDefenseMechanism){
            reg_modelo.defenseMechanism = malloc(sizeof(char) * reg_modelo.tmnDefenseMechanism);
            strcpy(reg_modelo.defenseMechanism, string);
            tamanho += reg_modelo.tmnDefenseMechanism + 2;
        }
        else reg_modelo.defenseMechanism = NULL;
        reg_modelo.tamanhoRegistro = tamanho + 20;
        free(string);
    }
    
    return reg_modelo;
}

int inserir_registro(FILE *arquivo, REGISTRO *registro, HEADER *header){
    if (header->status == '0'){
        printf("Falha no processamento do arquivo.\n"); 
        return 0;
    }
    header->status = '0';
    long int posicao = percorre_lista(arquivo, &(registro->tamanhoRegistro), header);
    fseek(arquivo, posicao, SEEK_SET);
    escreve_registro(arquivo, *registro, *header);
    header->status = '1';
    header->nroRegArq++;
    return 1;
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

long int percorre_lista(FILE *arquivo, int *tamanho, HEADER *header){
    long int posicao_anterior = -1;
    long int posicao = header->topo;
    while (posicao != -1){
        fseek(arquivo, posicao, SEEK_SET);
        REGISTRO reg = ler_registro(arquivo, header);
        if (reg.tamanhoRegistro >= *tamanho) {
            *tamanho = reg.tamanhoRegistro;
            header->nroRegRem--;
            if (posicao_anterior != -1){
                fseek(arquivo, posicao_anterior + 5, SEEK_SET);
                fwrite(&reg.prox, sizeof(long int), 1, arquivo);
            }
            else header->topo = reg.prox;
            desaloca_struct_registro(reg);
            break;
        }
        posicao_anterior = posicao;
        posicao = reg.prox;
        desaloca_struct_registro(reg);
    }
    if (posicao == -1){
        posicao = header->proxByteOffset;
        header->proxByteOffset += *tamanho + 5; 
    }
    return posicao;
}
