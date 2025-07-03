// Funções para trabalhar com a tabela
#ifndef FUNCOES_TABELA
    #define FUNCOES_TABELA
    #define IMPRIMIR 0
    #define DELETAR 1
    #define UPDATE 2
    #define INSERIR 3
    #define CAMPOS 0 
    #define COMPLETO 1
    #include <stdio.h>
    #include <stdlib.h>
    #include "estrutura_arquivo.h"
    #include "funcao_fornecida.h"
    int compara_registros(REGISTRO *reg, REGISTRO *reg_modelo);
    void criar_tabela(FILE *arquivo_csv, FILE *arquivo_binario);
    void print_registro(REGISTRO reg, HEADER *header);
    void busca_simples(FILE *arquivo);
    int busca_condicional(FILE *arquivo, REGISTRO reg_modelo, HEADER *header, int modo);
    long int inserir_registro(FILE *arquivo, REGISTRO *registro, HEADER *header);
    REGISTRO cria_modelo(int modo);
    long int update_registro(FILE *arquivo, REGISTRO *reg, REGISTRO *reg_template, HEADER *header);

#endif