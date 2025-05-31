// Funções relacionadas a manipulação dos arquivos
#ifndef IO_ARQUIVO
    #define IO_ARQUIVO
    #include <stdio.h>
    #include "estrutura_arquivo.h"
    HEADER ler_header_csv(FILE *arquivo);
    void escreve_header(FILE *arquivo, HEADER header);
    REGISTRO ler_dado_csv(FILE *arquivo);
    void escreve_registro(FILE *arquivo, REGISTRO reg, HEADER header);
    HEADER ler_header(FILE *arquivo);
    REGISTRO ler_registro(FILE *arquivo, HEADER *header);
    void desaloca_struct_registro(REGISTRO reg);
    void update_header(FILE *arquivo, HEADER header);
    void remove_registro(FILE *arquivo, HEADER *header);
#endif