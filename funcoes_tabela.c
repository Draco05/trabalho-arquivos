#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes_tabela.h"
#include "io_arquivo.h"
#include "estrutura_arquivo.h"
// valor usado para checar se o campo do modelo foi passado como NULO 
#define VALOR_MODELO_NULO -2 

// funções privadas
int compara_registros(REGISTRO *reg, REGISTRO *reg_modelo);
long int posicao_da_insercao(FILE *arquivo, int *tamanho, HEADER *header);
void update_registro(FILE *arquivo, REGISTRO *reg, REGISTRO *reg_template, HEADER *header);

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

// Função que busca todos os registros que atendam a uma certa condição e realiza a operação indicada pelo modo
// Argumentos: o arquivo binario, um registro modelo para comparação, o header, modo/operação realizada com os registros encontrados
// Retorno: 0 caso ocorra algum erro, 1 caso esteja tudo certo
int busca_condicional(FILE *arquivo, REGISTRO reg_modelo, HEADER *header, int modo){ 
    REGISTRO reg, novo_reg;
    int encontrou = 0;
    // Verifica a consistência do arquivo
    if (header->status == '0'){
        printf("Falha no processamento do arquivo.\n"); 
        return 0;
    }
    // altera status na struct nos modos que escrevem no arquivo
    if (modo == DELETAR || modo == UPDATE) header->status = '0';
    // no modo update, cria o modelo do registro a ser atualizado
    if (modo == UPDATE) novo_reg = cria_modelo(CAMPOS);
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
            if (modo == IMPRIMIR) print_registro(reg, header); // imprime no modo imprimir
            else if (modo == DELETAR){ // remoção lógica no modo deletar
                fseek(arquivo, -reg.tamanhoRegistro - 5, SEEK_CUR);
                remocao_logica(arquivo, header);
            }
            else if(modo == UPDATE){ // atualiza com os valores do novo_reg o registro atual
                fseek(arquivo, -reg.tamanhoRegistro - 5, SEEK_CUR);
                update_registro(arquivo, &reg, &novo_reg, header);
            }
            // indica que encontrou um registro na busca
            encontrou = 1;
            if (reg_modelo.idAttack != -1) { // idAttack é único -> não precisa mais continuar a busca
                desaloca_struct_registro(reg);
                break; 
            } 
        }
        desaloca_struct_registro(reg);
    }
    if (modo == UPDATE){
        desaloca_struct_registro(novo_reg);
    }

    if (modo == IMPRIMIR){
        // Se não encontrar nenhum registro com as condições, imprime "Registro inexistente"
        if (!encontrou){
            printf("Registro inexistente.\n\n");
        }
        printf("**********\n");
    }
    header->status = '1'; // atualiza o status na struct
    return 1;
}

// Função que cria um modelo de registro a partir de entradas do usuário
// Argumento: o modo em que a entrada do usuário será feita:
//  -CAMPOS: m NomeCampo1 ValorCampo1 ... NomeCampom ValorCampom
//  -COMPLETO: idAttack year finantialLoss country attackType targetIndustry defenseMechanism
// Retorno: registro com os parâmetros passados pelo usuário
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
    reg_modelo.tamanhoRegistro = 20;

    int m;
    char valor_campo[100]; // Armazena o valor do campo temporariamente
    char nome_campo[25]; // Armazena o nome do campo
    // Inicia o valor de m dependendo do modo
    if (modo == CAMPOS) scanf("%d", &m);
    else if (modo == COMPLETO) m = 7;

    // Loop para ler os valores do registro
    for (int j = 0; j < m; j++){
        int tamanho;
        memset(nome_campo, '\0', 25);
        memset(valor_campo, '\0', 100);
        if (modo == CAMPOS) scanf(" %s", nome_campo);
        scan_quote_string(valor_campo);
        tamanho = strlen(valor_campo);

        if ((modo == CAMPOS && !strcmp(nome_campo, "idAttack")) || (modo == COMPLETO && j == 0)){
            reg_modelo.idAttack = atoi(valor_campo);
        }
        else if ((modo == CAMPOS && !strcmp(nome_campo, "year")) || (modo == COMPLETO && j == 1)){
            reg_modelo.year = tamanho ? atoi(valor_campo) : (modo==CAMPOS ? VALOR_MODELO_NULO : -1);
        }
        else if ((modo == CAMPOS && !strcmp(nome_campo, "financialLoss")) || (modo == COMPLETO && j == 2)){
            reg_modelo.financialLoss = tamanho ? atof(valor_campo) : (modo==CAMPOS ? ((float) VALOR_MODELO_NULO) : -1.0f);
        }
        else if ((modo == CAMPOS && !strcmp(nome_campo, "country")) || (modo == COMPLETO && j == 3)){
            reg_modelo.tmnCountry = tamanho;
            if (tamanho) {
                reg_modelo.country = malloc(sizeof(char) * tamanho);
                strcpy(reg_modelo.country, valor_campo);
                reg_modelo.tamanhoRegistro += tamanho + 2;
            }
            else {
                reg_modelo.tmnCountry = modo == CAMPOS ? VALOR_MODELO_NULO : 0;
                reg_modelo.country = NULL;
            }
        }
        else if ((modo == CAMPOS && !strcmp(nome_campo, "attackType")) || (modo == COMPLETO && j == 4)){
            reg_modelo.tmnAttackType = tamanho;
            if (tamanho) {
                reg_modelo.attackType = malloc(sizeof(char) * tamanho);
                strcpy(reg_modelo.attackType, valor_campo);
                reg_modelo.tamanhoRegistro += tamanho + 2;
            }
            else{
                reg_modelo.tmnAttackType = modo == CAMPOS ? VALOR_MODELO_NULO : 0;
                reg_modelo.attackType = NULL;
            }
        }
        else if ((modo == CAMPOS && !strcmp(nome_campo, "targetIndustry")) || (modo == COMPLETO && j == 5)){
            reg_modelo.tmnTargetIndustry = tamanho;
            if (tamanho) {
                reg_modelo.targetIndustry = malloc(sizeof(char) * tamanho);
                strcpy(reg_modelo.targetIndustry, valor_campo);
                reg_modelo.tamanhoRegistro += tamanho + 2;
            }
            else{
                reg_modelo.tmnTargetIndustry = modo == CAMPOS ? VALOR_MODELO_NULO : 0;
                reg_modelo.targetIndustry = NULL;
            }
        }
        else if ((modo == CAMPOS && !strcmp(nome_campo, "defenseMechanism")) || (modo == COMPLETO && j == 6)){
            reg_modelo.tmnDefenseMechanism = tamanho;
            if (tamanho) {
                reg_modelo.defenseMechanism = malloc(sizeof(char) * tamanho);
                strcpy(reg_modelo.defenseMechanism, valor_campo);
                reg_modelo.tamanhoRegistro += tamanho + 2;
            }
            else {
                reg_modelo.tmnDefenseMechanism = modo == CAMPOS ? VALOR_MODELO_NULO : 0;
                reg_modelo.defenseMechanism = NULL;
            }
        }
    }
    return reg_modelo;
}

// Função que insere registro no arquivo de dados
// Argumentos: Arquivo binário a ser manipulado, registro a ser inserido, header do arquivo
// Retorno: 0 caso ocorra algum erro, 1 caso esteja tudo certo
int inserir_registro(FILE *arquivo, REGISTRO *registro, HEADER *header){
    // checa consistência do arquivo
    if (header->status == '0'){
        printf("Falha no processamento do arquivo.\n"); 
        return 0;
    }
    // atualiza o status da struct
    header->status = '0';

    // descobre a posição em que o dado será inserido e o escreve nessa posição
    long int posicao = posicao_da_insercao(arquivo, &(registro->tamanhoRegistro), header);
    fseek(arquivo, posicao, SEEK_SET);
    escreve_registro(arquivo, *registro, *header);
    
    // atualiza a struct header
    header->status = '1';
    header->nroRegArq++;
    return 1;
}

// Função que compara dois registros
// Argumentos: registros a serem comparados
// Retorno: 0 caso sejam diferentes, 1 caso sejam iguais
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


// Função que descobre a posição em que um registro deve ser inserido
// Argumentos: arquivo binário, tamanho do registro a ser inserido, header do arquivo
// Retorno: posição em que o registro deve ser inserido
long int posicao_da_insercao(FILE *arquivo, int *tamanho, HEADER *header){
    long int posicao_anterior = -1; // posição do registro anterior na lista de removidos
    long int posicao = header->topo; // posição atual do registro na lista de removidos
    // Checar até o fim da lista de removidos
    while (posicao != -1){
        // Colocar o ponteiro na posição do registro atual da lista
        fseek(arquivo, posicao, SEEK_SET);
        REGISTRO reg = ler_registro(arquivo, header);
        // checar se é possível inserir na posição atual
        if (reg.tamanhoRegistro >= *tamanho) {
            // guarda o tamanho do maior registro na struct
            *tamanho = reg.tamanhoRegistro;
            header->nroRegRem--;
            // checa se está no meio da lista
            if (posicao_anterior != -1){
                // atualiza o próximo do registro anterior
                fseek(arquivo, posicao_anterior + 5, SEEK_SET);
                fwrite(&reg.prox, sizeof(long int), 1, arquivo);
            }
            else header->topo = reg.prox; // caso a posição indicada seja a que está no topo (header)
            desaloca_struct_registro(reg);
            break;
        }
        // atualiza a posição
        posicao_anterior = posicao;
        posicao = reg.prox;
        desaloca_struct_registro(reg);
    }
    // caso não foi possível reaproveitar um espaço removido, inserir no fim do arquivo
    if (posicao == -1){
        posicao = header->proxByteOffset;
        header->proxByteOffset += *tamanho + 5; 
    }
    return posicao;
}

// Função de atualizar o valor de um registro
// Argumentos: arquivo binário a ser manipulao, registro original, registro com os valores com update, header
void update_registro(FILE *arquivo, REGISTRO *reg, REGISTRO *reg_template, HEADER *header){

    // cria uma struct do registro com os valores atualizados
    REGISTRO reg_novo;
    reg_novo.removido = '0';
    reg_novo.prox = -1;
    reg_novo.tamanhoRegistro = 20;
    
    // para cada IF, é checado se o valor no template é:
    //  -1: indica que esse campo não será atualizado
    //  VALOR_MODELO_NULO: indica que esse campo será atualizado para nulo
    //  outo valor: indica que esse campo será atualizado para esse novo valor

    if (reg_template->idAttack != -1) reg_novo.idAttack = reg_template->idAttack;
    else reg_novo.idAttack = reg->idAttack;
    if (reg_template->year != -1) {
        reg_novo.year = (reg_template->year == VALOR_MODELO_NULO) ? -1 : reg_template->year;
    }
    else reg_novo.year = reg->year;
    if (reg_template->financialLoss != -1){
        reg_novo.financialLoss = (reg_template->financialLoss == ((float) VALOR_MODELO_NULO)) ? -1.0f : reg_template->financialLoss;
    }
    else reg_novo.financialLoss = reg->financialLoss;
    if (reg_template->tmnCountry){
        reg_novo.country = reg_template->country;
        reg_novo.tmnCountry = (reg_template->tmnCountry == VALOR_MODELO_NULO) ? 0 : reg_template->tmnCountry;
    }
    else {
        reg_novo.country = reg->country;
        reg_novo.tmnCountry = reg->tmnCountry;
    }
    if (reg_template->tmnAttackType){
        reg_novo.attackType = reg_template->attackType;
        reg_novo.tmnAttackType = (reg_template->tmnAttackType == VALOR_MODELO_NULO) ? 0 : reg_template->tmnAttackType;

    }
    else {
        reg_novo.attackType = reg->attackType;
        reg_novo.tmnAttackType = reg->tmnAttackType;
    }
    if (reg_template->tmnTargetIndustry){
        reg_novo.targetIndustry = reg_template->targetIndustry;
        reg_novo.tmnTargetIndustry = (reg_template->tmnTargetIndustry == VALOR_MODELO_NULO) ? 0 : reg_template->tmnTargetIndustry;
    }
    else {
        reg_novo.targetIndustry = reg->targetIndustry;
        reg_novo.tmnTargetIndustry = reg->tmnTargetIndustry;
    }
    if (reg_template->tmnDefenseMechanism){
        reg_novo.defenseMechanism = reg_template->defenseMechanism;
        reg_novo.tmnDefenseMechanism = (reg_template->tmnDefenseMechanism == VALOR_MODELO_NULO) ? 0 : reg_template->tmnDefenseMechanism;
    }
    else {
        reg_novo.defenseMechanism = reg->defenseMechanism;
        reg_novo.tmnDefenseMechanism = reg->tmnDefenseMechanism;
    }

    // calcula o tamanho do registro após os updates
    if (reg_novo.tmnCountry) reg_novo.tamanhoRegistro += reg_novo.tmnCountry + 2;
    if (reg_novo.tmnAttackType) reg_novo.tamanhoRegistro += reg_novo.tmnAttackType + 2;
    if (reg_novo.tmnTargetIndustry) reg_novo.tamanhoRegistro += reg_novo.tmnTargetIndustry + 2;
    if (reg_novo.tmnDefenseMechanism) reg_novo.tamanhoRegistro += reg_novo.tmnDefenseMechanism + 2;

    // checa se pode reutilizar o mesmo espaço do registro original
    if (reg_novo.tamanhoRegistro <= reg->tamanhoRegistro){
        reg_novo.tamanhoRegistro = reg->tamanhoRegistro;
        escreve_registro(arquivo, reg_novo, *header);
    }
    else{
        // remove registro original e adiciona o novo
        header->status = '1';
        remocao_logica(arquivo, header);
        inserir_registro(arquivo, &reg_novo, header);
    } 
}