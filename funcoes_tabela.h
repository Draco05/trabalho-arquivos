// Funções para trabalhar com a tabela
#ifndef FUNCOES_TABELA
    #define FUNCOES_TABELA
    #include <stdio.h>
    #include <stdlib.h>
    #include "estrutura_arquivo.h"
    void criar_tabela(FILE *arquivo_csv, FILE *arquivo_binario);
    void print_registro(REGISTRO reg, HEADER header);
    void busca_simples(FILE *arquivo);
    void busca_condicional(FILE *arquivo, REGISTRO reg_modelo);

#endif