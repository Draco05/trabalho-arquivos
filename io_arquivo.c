#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estrutura_arquivo.h"
#include "io_arquivo.h"

// Descobre o tamanho da string que contem o dado
// Argumentos: string com os dados separados ou por '\0', ou por ',', ou por '|'
//             index inicial do dado a ser lido
int tamanho_str_dado(char *string, int index_inicial){
    int i = index_inicial;
    // incrementa o tamanho enquanto não achar um simbolo de "fim do dado"
    while (string[i] != ',' && string[i] != '\0' && string[i] != '|' && string[i] != '$') i++;
    return i - index_inicial;
}

// Transforma um dado do buffer para int. Pula para proxima posição de dado do buffer
// Argumentos: buffer e o ponteiro para a posição do buffer
int dado_buffer_em_int(char *string, int *posicao){
    int resultado;
    int tamanho = tamanho_str_dado(string, *posicao);

    // Caso o dado não exista, retornar -1
    if (!tamanho){
        (*posicao)++; //ir para o próximo dado
        return -1;
    }

    // Substitui a proxima virgula por um \0 (facilitar uso do atoi) 
    string[*posicao + tamanho] = '\0';
    // Transfomra a string que começa em string[posicao] em um int
    resultado = atoi(&string[*posicao]); 
    *posicao += tamanho + 1; // Pula para o proximo dado
    return resultado;
}

// Transforma um dado do buffer para float. Pula para proxima posição de dado do buffer
// Argumentos: buffer e o ponteiro para a posição do buffer
float dado_buffer_em_float(char *string, int *posicao){
    float resultado;
    int tamanho = tamanho_str_dado(string, *posicao);
    // Caso o dado não exista, retornar -1
    if (!tamanho){
        (*posicao)++; // pula para o próximo dado
        return -1;
    }
    // Substitui a proxima virgula por um \0 (facilitar uso do atof)
    string[*posicao + tamanho] = '\0';
    // Transfomra a string que começa em string[posicao] em um float
    resultado = atof(&string[*posicao]);
    *posicao += tamanho + 1; // Pula para o proximo dado
    return resultado;
    
}

// Transforma um dado do buffer para string. Pula para proxima posição de dado do buffer
// Argumentos: buffer, ponteiro para a posição do buffer e o ponteiro para o tamanho da string
char *dado_buffer_em_str(char *string, int *posicao, int *tamanho){
    char *resultado;
    *tamanho = tamanho_str_dado(string, *posicao);
    // Caso o dado não exista, retorna NULL
    if (!(*tamanho)){
        (*posicao)++; // Pula pra o próximo dado
        return NULL;
    }
    // Substitui a proxima virgula por um \0 (facilitar uso do strncpy)
    string[(*posicao) + (*tamanho)] = '\0';
    // Copia a string que começa em string[posicao]
    resultado = malloc(sizeof(char) * (*tamanho));
    strncpy(resultado, &string[*posicao], *tamanho);
    *posicao += *tamanho + 1; // Pula para o próximo dado
    return resultado; 
}

// Escreve um registro no arquivo
// Argumentos: arquivo binário, o registro a ser escrito e o header
void escreve_registro(FILE *arquivo, REGISTRO reg, HEADER header){
    // Escrevendo dados de tamanho fixo
    fwrite(&reg.removido, sizeof(char), 1, arquivo);
    fwrite(&reg.tamanhoRegistro, sizeof(int), 1, arquivo);
    fwrite(&reg.prox, sizeof(int long), 1, arquivo);
    fwrite(&reg.idAttack, sizeof(int), 1, arquivo);
    fwrite(&reg.year, sizeof(int), 1, arquivo);
    fwrite(&reg.financialLoss, sizeof(float), 1, arquivo);

    // Escrevendo dados de tamanho variáveis (caso existam)
    if (reg.tmnCountry){
        fwrite(&header.codDescreveCountry, sizeof(char), 1, arquivo);
        fwrite(reg.country, sizeof(char), reg.tmnCountry, arquivo);
        fwrite("|", sizeof(char), 1, arquivo);
    }
    if (reg.tmnAttackType){
        fwrite(&header.codDescreveType, sizeof(char), 1, arquivo);
        fwrite(reg.attackType, sizeof(char), reg.tmnAttackType, arquivo);
        fwrite("|", sizeof(char), 1, arquivo);
    }
    if (reg.tmnTargetIndustry){
        fwrite(&header.codDescreveTargeIndustry, sizeof(char), 1, arquivo);
        fwrite(reg.targetIndustry, sizeof(char), reg.tmnTargetIndustry, arquivo);
        fwrite("|", sizeof(char), 1, arquivo);
    }
    if (reg.tmnDefenseMechanism){
        fwrite(&header.codDescreveDefense, sizeof(char), 1, arquivo);
        fwrite(reg.defenseMechanism, sizeof(char), reg.tmnDefenseMechanism, arquivo);
        fwrite("|", sizeof(char), 1, arquivo);
    }
    int tamanho_dinamico = 0;
    // O (+2) representa o código do campo e o delimitador
    if (reg.tmnCountry) tamanho_dinamico += reg.tmnCountry + 2;
    if (reg.tmnAttackType) tamanho_dinamico += reg.tmnAttackType + 2;
    if (reg.tmnTargetIndustry) tamanho_dinamico += reg.tmnTargetIndustry + 2;
    if (reg.tmnDefenseMechanism) tamanho_dinamico += reg.tmnDefenseMechanism + 2; 
    // escrever lixo no espaço não utilizado
    for (int i = tamanho_dinamico; i < reg.tamanhoRegistro - 20; i++){
        char lixo = '$';
        fwrite(&lixo, sizeof(char), 1, arquivo);
    }

}
// Le a a primeira linha do arquivo .csv, retornando um header inicial
// Argumentos: arquivo csv
HEADER ler_header_csv(FILE *arquivo){
    HEADER header;
    // Garante que o ponteiro está no inicio do arquivo
    if (ftell(arquivo) != 0){
        fseek(arquivo, 0, SEEK_SET);
    }

    // Le os dados relacionados aos campos "descreve"
    fscanf(arquivo, "%[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^\n]", header.descreveIdentificador, header.descreveYear, header.descreveFinancialLoss, header.descreveCountry, header.descreveType, header.descreveTargetIndustry, header.descreveDefense);

    // Inicializa os valores do header
    header.status = '0';
    header.topo = -1;
    header.proxByteOffset = 0;
    header.nroRegArq = 0;
    header.nroRegRem = 0;
    header.codDescreveCountry = '1';
    header.codDescreveType = '2';
    header.codDescreveTargeIndustry = '3';
    header.codDescreveDefense = '4';
    return header;
}

// Escreve o header no início do arquivo
// Argumentos: arquivo binário e o header
void escreve_header(FILE *arquivo, HEADER header){
    // Garante que o ponteiro está no inicio do arquivo
    if (ftell(arquivo) != 0){
        fseek(arquivo, 0, SEEK_SET);
    }
    // Escreve dados do header
    fwrite(&header.status, sizeof(char), 1, arquivo);
    fwrite(&header.topo, sizeof(int long), 1, arquivo);
    fwrite(&header.proxByteOffset, sizeof(int long), 1, arquivo);
    fwrite(&header.nroRegArq, sizeof(int), 1, arquivo);
    fwrite(&header.nroRegRem, sizeof(int), 1, arquivo);
    fwrite(header.descreveIdentificador, sizeof(char), 23, arquivo);
    fwrite(header.descreveYear, sizeof(char), 27, arquivo);
    fwrite(header.descreveFinancialLoss, sizeof(char), 28, arquivo);
    fwrite(&header.codDescreveCountry, sizeof(char), 1, arquivo);
    fwrite(header.descreveCountry, sizeof(char), 26, arquivo);
    fwrite(&header.codDescreveType, sizeof(char), 1, arquivo);
    fwrite(header.descreveType, sizeof(char), 38, arquivo);
    fwrite(&header.codDescreveTargeIndustry, sizeof(char), 1, arquivo);
    fwrite(header.descreveTargetIndustry, sizeof(char), 38, arquivo);
    fwrite(&header.codDescreveDefense, sizeof(char), 1, arquivo);
    fwrite(header.descreveDefense, sizeof(char), 67, arquivo);
}

// Atualiza no header somente os dados que podem ter sofrido variação
// Argumentos: arquivo binário e o header
void update_header(FILE *arquivo, HEADER header){
    // Garante que o ponteiro esteja no inicio do arquivo
    if (ftell(arquivo) != 0){
        fseek(arquivo, 0, SEEK_SET);
    }
    // Escreve os dados que podem ter mudado no header
    fwrite(&header.status, sizeof(char), 1, arquivo);
    fwrite(&header.topo, sizeof(int long), 1, arquivo);
    fwrite(&header.proxByteOffset, sizeof(int long), 1, arquivo);
    fwrite(&header.nroRegArq, sizeof(int), 1, arquivo);
    fwrite(&header.nroRegRem, sizeof(int), 1, arquivo);
    fseek(arquivo, 276, SEEK_SET);
}

// Le apenas um dado do arquivo .csv e transforma isso em um registro
// Argumentos: arquivo csv
REGISTRO ler_dado_csv(FILE *arquivo){
    REGISTRO reg;
    char buffer[200];
    int pos = 0;
    int tamanho;
    
    // Le a linha inteira e salva no buffer
    fscanf(arquivo, " %[^\n]", buffer);
    // Transforma o buffer nos dados respectivos
    reg.idAttack = dado_buffer_em_int(buffer, &pos);
    reg.year = dado_buffer_em_int(buffer, &pos);
    reg.financialLoss = dado_buffer_em_float(buffer, &pos);
    reg.country = dado_buffer_em_str(buffer, &pos, &reg.tmnCountry);
    reg.attackType = dado_buffer_em_str(buffer, &pos, &reg.tmnAttackType);
    reg.targetIndustry = dado_buffer_em_str(buffer, &pos, &reg.tmnTargetIndustry);
    reg.defenseMechanism = dado_buffer_em_str(buffer, &pos, &reg.tmnDefenseMechanism);
    
    // Inicia registro como não removido
    reg.removido = '0';
    reg.prox = -1;

    reg.tamanhoRegistro = 20; // tamanhos fixos
    // Adiciona o tamanho de cada campo variável se ele existe
    // O (+2) representa o código do campo e o delimitador
    if (reg.tmnCountry) reg.tamanhoRegistro += reg.tmnCountry + 2;
    if (reg.tmnAttackType) reg.tamanhoRegistro += reg.tmnAttackType + 2;
    if (reg.tmnTargetIndustry) reg.tamanhoRegistro += reg.tmnTargetIndustry + 2;
    if (reg.tmnDefenseMechanism) reg.tamanhoRegistro += reg.tmnDefenseMechanism + 2;

    return reg;
}

// Le o header do arquivo binário
// Argumentos: arquivo binário 
HEADER ler_header(FILE *arquivo){
    HEADER header;
    // Garante que o ponteiro está no inicio do arquivo
    if (ftell(arquivo) != 0){
        fseek(arquivo, 0, SEEK_SET);
    }
    // Le dados do header
    fread(&header.status, sizeof(char), 1, arquivo);
    fread(&header.topo, sizeof(int long), 1, arquivo);
    fread(&header.proxByteOffset, sizeof(int long), 1, arquivo);
    fread(&header.nroRegArq, sizeof(int), 1, arquivo);
    fread(&header.nroRegRem, sizeof(int), 1, arquivo);
    fread(header.descreveIdentificador, sizeof(char), 23, arquivo);
    fread(header.descreveYear, sizeof(char), 27, arquivo);
    fread(header.descreveFinancialLoss, sizeof(char), 28, arquivo);
    fread(&header.codDescreveCountry, sizeof(char), 1, arquivo);
    fread(header.descreveCountry, sizeof(char), 26, arquivo);
    fread(&header.codDescreveType, sizeof(char), 1, arquivo);
    fread(header.descreveType, sizeof(char), 38, arquivo);
    fread(&header.codDescreveTargeIndustry, sizeof(char), 1, arquivo);
    fread(header.descreveTargetIndustry, sizeof(char), 38, arquivo);
    fread(&header.codDescreveDefense, sizeof(char), 1, arquivo);
    fread(header.descreveDefense, sizeof(char), 67, arquivo);
    return header;
}

// Função que desaloca a memória alocada em um struct REGISTRO
void desaloca_struct_registro(REGISTRO reg){
    // Se o campo existe, a memória é desalocada
    if (reg.tmnCountry) free(reg.country);
    if (reg.tmnAttackType) free(reg.attackType);
    if (reg.tmnTargetIndustry) free(reg.targetIndustry);
    if (reg.tmnDefenseMechanism) free(reg.defenseMechanism);
}

// Função que lê um registro do arquivo
// Argumentos: arquivo binário e header
REGISTRO ler_registro(FILE *arquivo, HEADER *header){
    REGISTRO reg;
    // Lendo os dados estáticos do registro
    fread(&reg.removido, sizeof(char), 1, arquivo);
    fread(&reg.tamanhoRegistro, sizeof(int), 1, arquivo);
    fread(&reg.prox, sizeof(long int), 1, arquivo);
    fread(&reg.idAttack, sizeof(int), 1, arquivo);
    fread(&reg.year, sizeof(int), 1, arquivo);
    fread(&reg.financialLoss, sizeof(float), 1, arquivo);

    // Inicia o tamanho dos campos variáveis como 0
    reg.tmnCountry = 0;
    reg.tmnAttackType = 0;
    reg.tmnTargetIndustry = 0;
    reg.tmnDefenseMechanism = 0;
    // Caso não exista nenhum dado nos campos variáveis
    if (reg.tamanhoRegistro - 20 == 0) return reg; 
    
    // Cria um buffer que armazena todos os campos de tamanho variável
    char *buffer = malloc(sizeof(char) * (reg.tamanhoRegistro - 20));
    // Lê todos os campos de tamanho variável    
    fread(buffer, sizeof(char), reg.tamanhoRegistro - 20, arquivo);

    int deslocamento = 0; // Variável que controla o número de bytes lidos, para garantir que não leia dados de outro registro ou campo
    // Loop para leitura dos dados do registro
    while (deslocamento < reg.tamanhoRegistro - 20){
        char codigo = buffer[deslocamento]; // codigo do campo
        // Checa qual campo o código pertence
        if (codigo == header->codDescreveCountry){
            // Descobre o tamanho do campo
            reg.tmnCountry = tamanho_str_dado(buffer, deslocamento + 1);
            if (reg.tmnCountry){
                // Aloca espaço para o campo na struct
                reg.country = malloc(sizeof(char) * (reg.tmnCountry + 1));
                // Copia o dado no buffer para a struct
                strncpy(reg.country, &buffer[deslocamento + 1], reg.tmnCountry);
                // Coloca um '\0' no fim da string para o uso do printf
                reg.country[reg.tmnCountry] = '\0';
            }
            // desloca para o próximo campo
            deslocamento += reg.tmnCountry + 2;
        }
        else if (codigo == header->codDescreveType){
            // Descobre o tamanho do campo
            reg.tmnAttackType = tamanho_str_dado(buffer, deslocamento + 1);
            if (reg.tmnAttackType){
                // Aloca espaço para o campo na struct
                reg.attackType = malloc(sizeof(char) * (reg.tmnAttackType + 1));
                // Copia o dado no buffer para a struct
                strncpy(reg.attackType, &buffer[deslocamento + 1], reg.tmnAttackType);
                // Coloca um '\0' no fim da string para o uso do printf
                reg.attackType[reg.tmnAttackType] = '\0';
            }
            // desloca para o próximo campo
            deslocamento += reg.tmnAttackType + 2;
        }
        else if (codigo == header->codDescreveTargeIndustry){
            // Descobre o tamanho do campo
            reg.tmnTargetIndustry = tamanho_str_dado(buffer, deslocamento + 1);
            if (reg.tmnTargetIndustry){
                // Aloca espaço para o campo na struct
                reg.targetIndustry = malloc(sizeof(char) * (reg.tmnTargetIndustry + 1));
                // Copia o dado no buffer para a struct
                strncpy(reg.targetIndustry, &buffer[deslocamento + 1], reg.tmnTargetIndustry);
                // Coloca um '\0' no fim da string para o uso do printf
                reg.targetIndustry[reg.tmnTargetIndustry] = '\0';
            }
            // desloca para o próximo campo
            deslocamento +=  reg.tmnTargetIndustry + 2;
        }
        else if (codigo == header->codDescreveDefense){
            // Descobre o tamanho do campo
            reg.tmnDefenseMechanism = tamanho_str_dado(buffer, deslocamento + 1);
            if (reg.tmnDefenseMechanism){
                // Aloca espaço para o campo na struct
                reg.defenseMechanism = malloc(sizeof(char) * (reg.tmnDefenseMechanism + 1));
                // Copia o dado no buffer para a struct
                strncpy(reg.defenseMechanism, &buffer[deslocamento + 1], reg.tmnDefenseMechanism);
                // Coloca um '\0' no fim da string para o uso do printf
                reg.defenseMechanism[reg.tmnDefenseMechanism] = '\0';
            }
            // desloca para o próximo campo
            deslocamento += reg.tmnDefenseMechanism + 2;
        }
        else if(codigo == '$') break;
    }
    // Desaloca a memória alocada no buffer
    free(buffer);
    // Retorna o registro lido
    return reg;
}

void remocao_logica(FILE *arquivo, HEADER *header){
    long int posicao_registro = ftell(arquivo);
    int tamanho;
    char removido = '1';
    // escreve que foi removido
    fwrite(&removido, sizeof(char), 1, arquivo);
    fread(&tamanho, sizeof(int), 1, arquivo);
    // guarda a posição do topo como próxima na lista de removidos
    fwrite(&(header->topo), sizeof(long int), 1, arquivo);
    // seek para o proximo registro
    fseek(arquivo, tamanho - sizeof(long int), SEEK_CUR);
    // atualiza o topo
    header->topo = posicao_registro;
    header->nroRegArq--;
    header->nroRegRem++;
}