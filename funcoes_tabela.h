// Funções para trabalhar com a tabela
#ifndef FUNCOES_TABELA
    #define FUNCOES_TABELA
    #define IMPRIMIR 0
    #define DELETAR 1
    #include <stdio.h>
    #include <stdlib.h>
    #include "estrutura_arquivo.h"
    #include "funcao_fornecida.h"
    void criar_tabela(FILE *arquivo_csv, FILE *arquivo_binario);
    void print_registro(REGISTRO reg, HEADER *header);
    void busca_simples(FILE *arquivo);
    void busca_condicional(FILE *arquivo, REGISTRO reg_modelo, HEADER *header, int modo);
    REGISTRO cria_modelo();

#endif