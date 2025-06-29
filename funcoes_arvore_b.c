#include "funcoes_arvore_b.h"

NO_ARVORE *criar_no(int tipo){
    NO_ARVORE *no = malloc(sizeof(NO_ARVORE));
    no->tipoNo = tipo;
    no->nroChaves = 0;
    for (int i = 0; i < ORDEM_ARVORE; i++){
        no->chaves[i] = -1;
        no->byteOffsets[i] = -1;
    }
    for (int i = 0; i < ORDEM_ARVORE + 1; i++) no->ponteirosNos[i] = -1;
    return no;
}

void criar_arvore(FILE *arquivo_entrada, FILE *arquivo_arvore){
    HEADER header = ler_header(arquivo_entrada);
    HEADER_ARVORE header_arvore;
    header_arvore.status = '0';
    header_arvore.noRaiz = -1;
    header_arvore.nroNos = 0;
    header_arvore.proxRRN = 0;
    for (int i = 0; i < 31; i++) header_arvore.lixo[i] = '$';

    escreve_header_arvore(arquivo_arvore, &header_arvore);

    for (int i = 0; i < header.nroRegArq + header.nroRegRem; i++){
        int long byteoffset = ftell(arquivo_entrada);
        REGISTRO reg = ler_registro(arquivo_entrada, &header);
        if (reg.removido == '1') continue;
        arvore_adicionar_chave(reg.idAttack, byteoffset, &header_arvore, arquivo_arvore);
    }
    header_arvore.status = '1';
    escreve_header_arvore(arquivo_arvore, &header_arvore);
}

int no_inserir(int *chave, long int *byteoffset, int rrn_esq, int rrn_dir, NO_ARVORE *no, HEADER_ARVORE *header, FILE *arquivo){
    // Encontra a posição para ser inserido
    int index = 0;
    while (index < no->nroChaves && *chave > no->chaves[index]) index++;

    // Shift para direita dos valores antigos
    for (int i = no->nroChaves; i > index; i--){
        no->chaves[i] = no->chaves[i - 1];
        no->byteOffsets[i] = no->byteOffsets[i - 1];
        no->ponteirosNos[i + 1] = no->ponteirosNos[i];
    }

    // Escreve a chave na posição
    no->chaves[index] = *chave;
    no->byteOffsets[index] = *byteoffset;
    if (no->tipoNo != -1){
        no->ponteirosNos[index + 1] = rrn_dir;
        if (index == 0) no->ponteirosNos[0] = rrn_esq;
    }
    
    // Cabe na pagina
    if (no->nroChaves < ORDEM_ARVORE - 1){
        (no->nroChaves)++;
        return 0;
    }   
    // Precisa fazer um split
    else{
        if (no->tipoNo == 0) no->tipoNo = 1;
        NO_ARVORE *novo_no = criar_no(no->tipoNo);
        // Pela ordem ser 2, o maior dos valores vai ir a nova pagina criada e o do meio será promovido
        no_inserir(&(no->chaves[2]), &(no->byteOffsets[2]), no->ponteirosNos[2], no->ponteirosNos[3], novo_no, header, arquivo);
        escrever_no(novo_no, header->proxRRN, arquivo);
        (header->proxRRN)++;
        (header->nroNos)++;

        free(novo_no);

        // Chave/Byteoffset promovidos
        *chave = no->chaves[1];
        *byteoffset = no->byteOffsets[1];

        no->chaves[1] = -1;
        no->byteOffsets[1] = -1;
        no->ponteirosNos[2] = -1;
        (no->nroChaves)--;

        return 1;
    }
}

int percorre_inserindo(int *chave, long int *byteoffset, int RRN, HEADER_ARVORE *header, FILE *arquivo){
    NO_ARVORE *no = ler_no(RRN, arquivo);
    if (no == NULL) return 1;
    int index_busca = 0;
    while (index_busca < no->nroChaves && *chave > no->chaves[index_busca]) index_busca++;
    int prox_rrn = no->ponteirosNos[index_busca];
    free(no);
    if (percorre_inserindo(chave, byteoffset, prox_rrn, header, arquivo) == 1){
        no = ler_no(RRN, arquivo);
        int teve_split = no_inserir(chave, byteoffset, prox_rrn, header->proxRRN - 1, no, header, arquivo);
        escrever_no(no, RRN, arquivo);
        free(no);
        return teve_split;
    }
    return 0;

}

void arvore_adicionar_chave(int idAttack, long int byteoffset, HEADER_ARVORE *header, FILE *arquivo){

    if (percorre_inserindo(&idAttack, &byteoffset, header->noRaiz, header, arquivo)){
        NO_ARVORE *no;
        if (header->noRaiz == -1) no = criar_no(-1);
        else no = criar_no(0);
        no_inserir(&idAttack, &byteoffset, header->noRaiz, header->proxRRN - 1, no, header, arquivo);
        escrever_no(no, header->proxRRN, arquivo);
        header->noRaiz = header->proxRRN;
        (header->proxRRN)++;
        (header->nroNos)++;
        free(no);
    }
}
