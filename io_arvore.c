#include "io_arvore.h"

HEADER_ARVORE ler_header_arvore(FILE *arquivo_arvore){
    HEADER_ARVORE header_arvore;
    // Vai para o inicio do arquivo
    if (ftell(arquivo_arvore) != 0){
        fseek(arquivo_arvore, 0, SEEK_SET);
    }

    // Faz leitura dos dados
    fread(&(header_arvore.status), sizeof(char), 1, arquivo_arvore); //Status
    fread(&(header_arvore.noRaiz), sizeof(int), 1, arquivo_arvore); //Raiz
    fread(&(header_arvore.proxRRN), sizeof(int), 1, arquivo_arvore); //Prox RRN
    fread(&(header_arvore.nroNos), sizeof(int), 1, arquivo_arvore); //Numero de nos
    fread(header_arvore.lixo, sizeof(char), 31, arquivo_arvore); //Lixo

    return header_arvore;
}

void escreve_header_arvore(FILE *arquivo_arvore, HEADER_ARVORE *header_arvore){
    // Vai para o inicio do arquivo
    if (ftell(arquivo_arvore) != 0){
        fseek(arquivo_arvore, 0, SEEK_SET);
    }

    // Faz escrita dos dados
    fwrite(&(header_arvore->status), sizeof(char), 1, arquivo_arvore); //Status
    fwrite(&(header_arvore->noRaiz), sizeof(int), 1, arquivo_arvore); //Raiz
    fwrite(&(header_arvore->proxRRN), sizeof(int), 1, arquivo_arvore);//Proximo RRN
    fwrite(&(header_arvore->nroNos), sizeof(int), 1, arquivo_arvore); //Numero de nos
    fwrite(header_arvore->lixo, sizeof(char), 31, arquivo_arvore); //Lixo
}

void escrever_no(NO_ARVORE *no, int rrn, FILE *arquivo_arvore){
    if (rrn == -1) return; //Posicao invalida

    // Vai ao local correto, escreve o tipo e o numero de chaves
    fseek(arquivo_arvore, (rrn + 1) * 44, SEEK_SET); 
    fwrite(&(no->tipoNo), sizeof(int), 1, arquivo_arvore); 
    fwrite(&(no->nroChaves), sizeof(int), 1, arquivo_arvore); 

    for (int i = 0; i < ORDEM_ARVORE - 1; i++){
        fwrite(&(no->ponteirosNos[i]), sizeof(int), 1, arquivo_arvore);
        fwrite(&(no->chaves[i]), sizeof(int), 1, arquivo_arvore);
        fwrite(&(no->byteOffsets[i]), sizeof(long int), 1, arquivo_arvore);
    }
    fwrite(&(no->ponteirosNos[ORDEM_ARVORE - 1]), sizeof(int), 1, arquivo_arvore);

}

NO_ARVORE *ler_no(int rrn, FILE *arquivo_arvore){
    if (rrn == -1) return NULL; // Posicao invalida

    // Vai ao local correto, le o tipo e o numero de dados
    NO_ARVORE *no = malloc(sizeof(NO_ARVORE));
    fseek(arquivo_arvore, (rrn + 1) * 44, SEEK_SET);
    fread(&(no->tipoNo), sizeof(int), 1, arquivo_arvore);
    fread(&(no->nroChaves), sizeof(int), 1, arquivo_arvore);
    
    for (int i = 0; i < ORDEM_ARVORE - 1; i++){
        fread(&(no->ponteirosNos[i]), sizeof(int), 1, arquivo_arvore);
        fread(&(no->chaves[i]), sizeof(int), 1, arquivo_arvore);
        fread(&(no->byteOffsets[i]), sizeof(long int), 1, arquivo_arvore);
    }
    fread(&(no->ponteirosNos[ORDEM_ARVORE - 1]), sizeof(int), 1, arquivo_arvore);
    
    no->ponteirosNos[ORDEM_ARVORE] = -1;
    no->byteOffsets[ORDEM_ARVORE - 1] = -1;
    no->chaves[ORDEM_ARVORE - 1] = -1;


    return no;
}
