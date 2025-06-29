#ifndef ARVORE_B_H
    #define ARVORE_B_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "estrutura_arquivo.h"
    #include "io_arquivo.h"
    #include "io_arvore.h"
    int criar_arvore(FILE *arquivo_entrada, FILE *arquivo_arvore);
    void arvore_adicionar_chave(int idAttack, long int byteoffset, HEADER_ARVORE *header_arvore, FILE *arquivo);
    void arvore_busca_condicional(FILE *arquivo_bin, FILE *arquivo_indice, REGISTRO reg_modelo, HEADER *header_bin, HEADER_ARVORE *header_indice);

#endif