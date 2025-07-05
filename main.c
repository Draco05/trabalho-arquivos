// Grupo 9
// Caio Draco Araújo Albuquerque Galvão - 15573731
// Luis Guilherme Zanetti - 15652750
// Pedro Bizon Dania - 11812667

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "funcoes_tabela.h"
#include "estrutura_arquivo.h"
#include "io_arquivo.h"
#include "funcao_fornecida.h"
#include "funcoes_arvore_b.h"

int checar_arquivos(int quantidade_arquivos, FILE *arquivo1, FILE *arquivo2){
    if (quantidade_arquivos == 1){
        if (arquivo1 == NULL){
            printf("Falha no processamento do arquivo.\n");
            return 1;
        }
    }
    else{
        if (arquivo1 == NULL || arquivo2 == NULL){ 
            printf("Falha no processamento do arquivo.\n");
            if (arquivo1 != NULL) fclose(arquivo1);
            if (arquivo2 != NULL) fclose(arquivo2);
            return 1;
        }
    }
    return 0;
}

int loop_busca(int n, int modo_operacao, int modo_modelo, FILE *arquivo_binario, HEADER *header){
    for (int i = 0; i < n; i++){
        REGISTRO reg_modelo = cria_modelo(modo_modelo);
    
        if (modo_operacao != INSERIR && !busca_condicional(arquivo_binario, reg_modelo, header, modo_operacao)){
            desaloca_struct_registro(reg_modelo);
            fclose(arquivo_binario);
            return 1;
        }
        else if (modo_operacao == INSERIR && inserir_registro(arquivo_binario, &reg_modelo, header) == -1){
            desaloca_struct_registro(reg_modelo);
            fclose(arquivo_binario);
            return 1;
        }
        desaloca_struct_registro(reg_modelo);
    }
    return 0;
}

int main(){
    int modo, n;
    scanf("%d", &modo); // Lê a funcionalidade que será executada
    char nome_csv[30], nome_binario[30], nome_indice[30]; // Variáveis para armazenar o nome dos arquivos
    FILE *arquivo_csv, *arquivo_binario, *arquivo_indice; // Ponteiros para os arquivos
    HEADER header;
    HEADER_ARVORE header_indice;
    // Switch case para executar a funcionalidade especificada 
    switch (modo){
    // criar arquivo da tabela
    case 1:
        scanf("%s  %s", nome_csv, nome_binario); //Le o nome do arquivo csv de entrada e o arquivo binário de saida
        
        // Abre os arquivos
        arquivo_csv = fopen(nome_csv, "r");
        arquivo_binario = fopen(nome_binario, "wb");

        if (checar_arquivos(2, arquivo_binario, arquivo_csv)) break;
        
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
        if (checar_arquivos(1, arquivo_binario, NULL)) break;

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
        if (checar_arquivos(1, arquivo_binario, NULL)) break;

        header = ler_header(arquivo_binario);
        // checa consistência do arquivo
        if (header.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            break;
        }
        // Loop que realiza as buscas
        if (loop_busca(n, IMPRIMIR, CAMPOS, arquivo_binario, &header)) break;

        // Fecha o arquivo
        fclose(arquivo_binario);
        break;
    
    // Deletar registros
    case 4:
        scanf(" %s %d", nome_binario, &n); // Le o número de buscas
        arquivo_binario = fopen(nome_binario, "rb+");
        // Verifica se o arquivo está correto
        if (checar_arquivos(1, arquivo_binario, NULL)) break;

        header = ler_header(arquivo_binario);
        // checa consistência do arquivo
        if (header.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            break;
        }
        //atualiza o status no arquivo, mas mantém como 1 na struct
        header.status = '0';
        update_header(arquivo_binario, header);
        header.status = '1'; 

        // Loop que realiza as buscas
        if (loop_busca(n, DELETAR, CAMPOS, arquivo_binario, &header)) break;
        
        // atualiza os valores do header no arquivo
        update_header(arquivo_binario, header);
        fclose(arquivo_binario);
        binarioNaTela(nome_binario);
        break;

    // Inserir Registros
    case 5:
        scanf(" %s %d", nome_binario, &n); // Le o número de inserções
        arquivo_binario = fopen(nome_binario, "rb+");
        // Verifica se o arquivo está correto
        if (checar_arquivos(1, arquivo_binario, NULL)) break;

        header = ler_header(arquivo_binario);
        // checa consistência do arquivo
        if (header.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            break;
        }
        //atualiza o stauts no arquivo, mas mantém como 1 na struct
        header.status = '0';
        update_header(arquivo_binario, header);
        header.status = '1'; 

        // Loop das inserções
        if (loop_busca(n, INSERIR, COMPLETO, arquivo_binario, &header)) break;

        // atualiza os valores do header no arquivo
        update_header(arquivo_binario, header);
        fclose(arquivo_binario);
        binarioNaTela(nome_binario);
        break;
    
    // Atualizar registros
    case 6:
        scanf(" %s %d", nome_binario, &n); // Le o número de inserções
        arquivo_binario = fopen(nome_binario, "rb+");
        // Verifica se o arquivo está correto
        if (checar_arquivos(1, arquivo_binario, NULL)) break;

        header = ler_header(arquivo_binario);
        // checa consistência do arquivo
        if (header.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            break;
        }
        //atualiza o status no arquivo, mas mantém como 1 na struct
        header.status = '0';
        update_header(arquivo_binario, header);
        header.status = '1';

        // Loop para realizar as buscas
        if (loop_busca(n, UPDATE, CAMPOS, arquivo_binario, &header)) break;

        // atualiza valores do header no arquivo
        update_header(arquivo_binario, header);
        fclose(arquivo_binario);
        binarioNaTela(nome_binario);
        break;
    // Criar arquivo de indices    
    case 7:
        // Ler arquivos de entrada
        scanf(" %s  %s", nome_binario, nome_indice);
        arquivo_binario = fopen(nome_binario, "rb");
        arquivo_indice = fopen(nome_indice, "wb+");

        // Checa se os arquivos foram abertos corretamente
        if (checar_arquivos(2, arquivo_binario, arquivo_indice)) break;
        
        // Chama função de criar árvore e checa se foi executada corretamente
        if (criar_arvore(arquivo_binario, arquivo_indice) == 0){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            fclose(arquivo_indice);
            break;
        }
        // Fecha arquivos e faz o binarioNaTela
        fclose(arquivo_binario);
        fclose(arquivo_indice);
        binarioNaTela(nome_indice);
        break;

    // Busca de registros usando o arquivo de indices
    case 8:
        // Ler arquivos de entrada e número de checagens
        scanf(" %s  %s %d", nome_binario, nome_indice, &n);
        arquivo_binario = fopen(nome_binario, "rb");
        arquivo_indice = fopen(nome_indice, "rb");

        // Checa se os arquivos foram abertos corretamente
        if (checar_arquivos(2, arquivo_binario, arquivo_indice)) break;

        // Lê os headers dos arquivos e checa se os arquivos estão consistentes
        header = ler_header(arquivo_binario);
        header_indice = ler_header_arvore(arquivo_indice);
        if (header.status == '0' || header_indice.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            fclose(arquivo_csv);
            break;
        }
        // Loop das buscas
        for (int i = 0; i < n; i++){
            // Cria um registro com o modelo especificado pelo usuário
            REGISTRO reg_modelo = cria_modelo(CAMPOS);
            // Busca usando a árvore B usando modo de IMPRIMIR
            arvore_busca_condicional(arquivo_binario, arquivo_indice, reg_modelo, &header, &header_indice, IMPRIMIR);
            desaloca_struct_registro(reg_modelo);
        }

        // Fecha os arquivos
        fclose(arquivo_binario);
        fclose(arquivo_indice);
        break;
    case 10:
        // Ler arquivos de entrada e número de inserções
        scanf(" %s  %s %d", nome_binario, nome_indice, &n);
        arquivo_binario = fopen(nome_binario, "rb+");
        arquivo_indice = fopen(nome_indice, "rb+");

        // Checa se os arquivos foram abertos corretamente
        if (checar_arquivos(2, arquivo_binario, arquivo_indice)) break;

        // Lê os headers dos arquivos e checa se os arquivos estão consistentes
        header = ler_header(arquivo_binario);
        header_indice = ler_header_arvore(arquivo_indice);
        if (header.status == '0' || header_indice.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            fclose(arquivo_csv);
            break;
        }

        //atualiza o stauts nos arquivos, mas mantém como 1 na struct
        header.status = '0';
        header_indice.status = '0';
        update_header(arquivo_binario, header);
        escreve_header_arvore(arquivo_indice, &header_indice);
        header.status = '1'; 
        header_indice.status = '1';

        // Loop das inserções
        for (int i = 0; i < n; i++){
            // Cria modelo especificado pelo usuário
            REGISTRO registro = cria_modelo(COMPLETO);
            // Insere o registro no arquivo binário
            long int byteoffset = inserir_registro(arquivo_binario, &registro, &header);
            int chave = registro.idAttack;
            // Insere o registro especificado. Caso ocorra erro, termina o programa
            if (byteoffset == -1){
                fclose(arquivo_binario);
                fclose(arquivo_indice);
                desaloca_struct_registro(registro);
                return 0;
            }
            // Adiciona o novo idAttack no arquivo de indices
            arvore_adicionar_chave(chave, byteoffset, &header_indice, arquivo_indice);
            desaloca_struct_registro(registro);
        }
        // atualiza os valores dos headers nos arquivos
        update_header(arquivo_binario, header);
        escreve_header_arvore(arquivo_indice, &header_indice);
        // Fecha o arquivo o faz o binarioNaTela
        fclose(arquivo_binario);
        fclose(arquivo_indice);
        binarioNaTela(nome_binario);
        binarioNaTela(nome_indice);
        break;

    case 11:
        // Ler arquivos de entrada e número de updates
        scanf(" %s  %s %d", nome_binario, nome_indice, &n);
        arquivo_binario = fopen(nome_binario, "rb+");
        arquivo_indice = fopen(nome_indice, "rb+");
        
        // Checa se os arquivos foram abertos corretamente
        if (checar_arquivos(2, arquivo_binario, arquivo_indice)) break;

        // Lê os headers dos arquivos e checa se os arquivos estão consistentes
        header = ler_header(arquivo_binario);
        header_indice = ler_header_arvore(arquivo_indice);
        if (header.status == '0' || header_indice.status == '0'){
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivo_binario);
            fclose(arquivo_csv);
            break;
        }

        // Loop dos updates
        for (int i = 0; i < n; i++){
            // Cria modelo especificado pelo usuário
            REGISTRO reg_modelo = cria_modelo(CAMPOS);
            // Busca na árvore usando o modo de UPDATE
            arvore_busca_condicional(arquivo_binario, arquivo_indice, reg_modelo, &header, &header_indice, UPDATE);
            desaloca_struct_registro(reg_modelo);
        }

        // Atualizad os headers nos arquivos
        update_header(arquivo_binario, header);
        escreve_header_arvore(arquivo_indice, &header_indice);
        // Fecha o arquivo o faz o binarioNaTela
        fclose(arquivo_binario);
        fclose(arquivo_indice);
        binarioNaTela(nome_binario);
        binarioNaTela(nome_indice);
        break;
    
    // funcionalidade inexistente
    default:
        printf("Funcionalidade %d inexistente!\n", modo);
    }
    return 0;
}