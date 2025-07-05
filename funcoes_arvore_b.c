#include "funcoes_arvore_b.h"
#include "funcoes_tabela.h"

// Função para criar uma struct de nó de árvore B
// Argumentos: Tipo do nó da árvore
// Retorna: struct do nó de árvore B
NO_ARVORE *criar_no(int tipo){
    // Aloca espaço para os nós e inicializa os campos de maneira conveniente
    NO_ARVORE *no = malloc(sizeof(NO_ARVORE));
    no->tipoNo = tipo;
    no->nroChaves = 0;

    // Inicializa os valores dos vetores como -1
    for (int i = 0; i < ORDEM_ARVORE; i++){
        no->chaves[i] = -1;
        no->byteOffsets[i] = -1;
        no->ponteirosNos[i] = -1;
    }
    no->ponteirosNos[ORDEM_ARVORE] = -1;
    return no;
}

// Função para criar árvore B (arquivo de indices) a partir de um arquivo binário
// Argumentos: Arquivo binário e o arquivo de indices
// Retorno: 0 caso ocorreu algum erro, 1 caso tudo foi executado corretamente
int criar_arvore(FILE *arquivo_entrada, FILE *arquivo_arvore){
    // Ler o header do arquivo binário
    HEADER header = ler_header(arquivo_entrada);
    // Checa se arquivo está consistente
    if (header.status == '0') return 0;
    
    // Incia os valores do header do arquivo de indice 
    HEADER_ARVORE header_arvore;
    header_arvore.status = '0';
    header_arvore.noRaiz = -1;
    header_arvore.nroNos = 0;
    header_arvore.proxRRN = 0;
    for (int i = 0; i < 31; i++) header_arvore.lixo[i] = '$';

    // Escreve esse header no arquivo
    escreve_header_arvore(arquivo_arvore, &header_arvore);

    // Loop de inserções dos registros
    for (int i = 0; i < header.nroRegArq + header.nroRegRem; i++){
        // Pega o byteoffset do registro atual
        int long byteoffset = ftell(arquivo_entrada);
        // Le o registro atual
        REGISTRO reg = ler_registro(arquivo_entrada, &header);
        // Adiciona na árvore caso ele não esteja removido
        if (reg.removido == '1') continue;
        arvore_adicionar_chave(reg.idAttack, byteoffset, &header_arvore, arquivo_arvore);
    }
    // Atualiza o status para 1
    header_arvore.status = '1';
    // Atualiza o header do arquivo de indices
    escreve_header_arvore(arquivo_arvore, &header_arvore);
    return 1;
}

// Insere um nó na árvore B
// Argumentos: ponteiros de uma chave e um byteoffset a serem inseridos (os valores podem ser alterados na volta da recursão),
// RRN da esquerda e da direita da chave a ser inserida, nó onde ocorrerá inserção, header e ponteiro do arquivo de indice
// Retorno: 0 caso não tenha split, 1 caso tenha split
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

// Função para percorrer a árvore B inserindo uma chave e um byteoffset na posição correta
// Argumentos: ponteiros de uma chave e um byteoffset a serem inseridos (os valores podem ser alterados na volta da recursão),
// RRN do nó atual, header e ponteiro do arquivo de indices
int percorre_inserindo(int *chave, long int *byteoffset, int RRN, HEADER_ARVORE *header, FILE *arquivo){
    // Le o nó atual e checa se não é NULL
    NO_ARVORE *no = ler_no(RRN, arquivo);
    if (no == NULL) return 1;
    // Busca pela posição em que a chave deveria estar
    int index_busca = 0;
    while (index_busca < no->nroChaves && *chave > no->chaves[index_busca]) index_busca++;
    int prox_rrn = no->ponteirosNos[index_busca];
    // Libera o nó para não deixar mais de uma página de disco aberta ao mesmo tempo
    free(no);
    // Chamada recursiva usando o prox_rrn. Entra no if caso precise fazer uma inserção no nó atual
    if (percorre_inserindo(chave, byteoffset, prox_rrn, header, arquivo) == 1){
        // Ler o nó novamente
        no = ler_no(RRN, arquivo);
        // Insere no nó e escreve no arquivo de indices
        int teve_split = no_inserir(chave, byteoffset, prox_rrn, header->proxRRN - 1, no, header, arquivo);
        escrever_no(no, RRN, arquivo);
        free(no);
        return teve_split;
    }
    return 0;

}

// Adiciona uma chave no arquivo de indices (Árvore B)
// Argumentos: idAttack e byteoffset do registro a ser inserido, header e ponteiro do arquivo de indices
void arvore_adicionar_chave(int idAttack, long int byteoffset, HEADER_ARVORE *header, FILE *arquivo){

    // Chama a função de percorrer inserindo passando a raiz como RRN. Entra no if caso tenha que mudar a raiz
    if (percorre_inserindo(&idAttack, &byteoffset, header->noRaiz, header, arquivo)){
        NO_ARVORE *no;
        // Caso em que a raiz é o nó folha
        if (header->noRaiz == -1) no = criar_no(-1);
        // Caso a raiz não é o nó folha
        else no = criar_no(0);
        // Insere o valor na raiz e escreve no arquivo
        no_inserir(&idAttack, &byteoffset, header->noRaiz, header->proxRRN - 1, no, header, arquivo);
        escrever_no(no, header->proxRRN, arquivo);
        // Atualiza os valores no header
        header->noRaiz = header->proxRRN;
        (header->proxRRN)++;
        (header->nroNos)++;
        free(no);
    }
}

// Função para fazer uma operação usando um registro como base
// Argumentos: modo (IMPRIMIR ou UPDATE), registro base, header do arquivo binário, arquivos, 
// index usado pela arvore B, RRN na arvore B, novo registro (usado só no update), nó da arvore
void operacao_registro(int modo, REGISTRO reg, HEADER *header_bin, FILE *arquivo_bin, FILE *arquivo_indice, int index, int rrn, REGISTRO novo_reg, NO_ARVORE *no){
    // Imprime o registro no modo IMPRIMIR
    if (modo == IMPRIMIR) print_registro(reg, header_bin);
    // Atualiza o registro no modo UPDATE
    else if (modo == UPDATE){
        // Vai para posição do registro a ser atualizado
        fseek(arquivo_bin, no->byteOffsets[index], SEEK_SET);
        // Atualiza o registro e recebe o byteoffset dessa atualização
        long int byteoffset = update_registro(arquivo_bin, &reg, &novo_reg, header_bin);
        // Caso o byteoffset tenha sido alterado na atualização
        if (byteoffset != -1){
            // Atualiza a árvore B
            no->byteOffsets[index] = byteoffset;
            escrever_no(no, rrn, arquivo_indice);
        }
    }
}

// Percorre a árvore B em-ordem
// Argumentos: arquivos, registro usado como modelo da comparação, headers, modo da operação
// RRN do nó atual, novo registro (usado só no update), flag se encontrou ou não  
void percorre_comparando(FILE *arquivo_bin, FILE *arquivo_indice, REGISTRO reg_modelo, HEADER *header_bin, HEADER_ARVORE *header_indice, int modo, int rrn, REGISTRO novo_reg, int *encontrou){
    // Fim da recursão
    if (rrn == -1) {
        return;
    }
    // Le o nó no RRN atual
    NO_ARVORE *no = ler_no(rrn, arquivo_indice);
    for (int i = 0; i < no->nroChaves; i++){
        // Chama a função para o ponteiro da esquerda
        percorre_comparando(arquivo_bin, arquivo_indice, reg_modelo, header_bin, header_indice, modo, no->ponteirosNos[i], novo_reg, encontrou);
        // Vai para o byteoffset especificado e le o registro  
        fseek(arquivo_bin, no->byteOffsets[i], SEEK_SET);
        REGISTRO reg = ler_registro(arquivo_bin, header_bin);
        // Checa se o registro é o que está sendo buscado
        if (compara_registros(&reg, &reg_modelo)){
            // Faz a operação com o registro
            operacao_registro(modo, reg, header_bin, arquivo_bin, arquivo_indice, i, rrn, novo_reg, no);
            *encontrou = 1;
        }
        desaloca_struct_registro(reg);
    }
    // Chama a função para o ponteiro da direita
    percorre_comparando(arquivo_bin, arquivo_indice, reg_modelo, header_bin, header_indice, modo, no->ponteirosNos[no->nroChaves], novo_reg, encontrou);
    free(no);
} 

// Busca um registro na árvore B
// Argumentos: arquivos, registro usado como modelo de busca, headers e o modo da operação
void arvore_busca_condicional(FILE *arquivo_bin, FILE *arquivo_indice, REGISTRO reg_modelo, HEADER *header_bin, HEADER_ARVORE *header_indice, int modo){
    NO_ARVORE *no;
    REGISTRO novo_reg = {0};
    int encontrou = 0;

    // Cria um o registro novo no caso de update
    if (modo == UPDATE) novo_reg = cria_modelo(CAMPOS);

    // Primeiro RRN é o da raíz
    int rrn = header_indice->noRaiz;

    // Quando não é busca pelo idAttack, busca na árvore em-ordem
    if (reg_modelo.idAttack == -1){
        percorre_comparando(arquivo_bin, arquivo_indice, reg_modelo, header_bin, header_indice, modo, rrn, novo_reg, &encontrou);
    }
    // Busca pelo idAttack
    else{
        // Enquanto não for NULL
        while (rrn != -1){
            no = ler_no(rrn, arquivo_indice);
            // Vai para posição esperada da chave
            int index = 0;
            while (index < no->nroChaves && reg_modelo.idAttack > no->chaves[index]) index++;

            // encontrou o registro com mesmo idAttack
            if (index < no->nroChaves && no->chaves[index] == reg_modelo.idAttack){
                // Vai para o byteoffset especificado e le o registro  
                long int byteoffset = no->byteOffsets[index];
                fseek(arquivo_bin, byteoffset, SEEK_SET);
                REGISTRO reg = ler_registro(arquivo_bin, header_bin);
                // Checa se o registro é o que está sendo buscado
                if (compara_registros(&reg, &reg_modelo)){
                    // Faz a operação com o registro
                    operacao_registro(modo, reg, header_bin, arquivo_bin, arquivo_indice, index, rrn,novo_reg, no);
                    encontrou = 1;
                }
                desaloca_struct_registro(reg);
                free(no);
                // Para a busca, pois o idAttack é único
                break;
            }
            // Continua o loop para no próximo nó
            rrn = no->ponteirosNos[index];
            free(no);
        }
    }
    if (modo == IMPRIMIR){
        // Não encontrou nenhum registro igual
        if (!encontrou){
            printf("Registro inexistente.\n\n");
        }
        printf("**********\n");
    }
    else if (modo == UPDATE){
        // Libera memória
        desaloca_struct_registro(novo_reg);
    }
    
}