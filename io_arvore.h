#ifndef IO_ARVORE_H
    #define IO_ARVORE_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "estrutura_arquivo.h"
    #include "io_arquivo.h"
    void escreve_header_arvore(FILE *arquivo_arvore, HEADER_ARVORE *header_arvore);
    NO_ARVORE *ler_no(int rrn, FILE *arquivo_arvore);
    void escrever_no(NO_ARVORE *no, int rrn, FILE *arquivo_arvore);

    
#endif