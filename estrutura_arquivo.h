#ifndef TABELA
    #define TABELA
    #define ORDEM_ARVORE 3
    // Struct do header
    typedef struct{
        char status;
        long int topo;
        long int proxByteOffset;
        int nroRegArq;
        int nroRegRem;
        char descreveIdentificador[23];
        char descreveYear[27];
        char descreveFinancialLoss[28];
        char codDescreveCountry;
        char descreveCountry[26];
        char codDescreveType;
        char descreveType[38];
        char codDescreveTargeIndustry;
        char descreveTargetIndustry[38];
        char codDescreveDefense;
        char descreveDefense[67];
    } HEADER;

    // Struct de um registro
    typedef struct{
        int idAttack;
        int year;
        float financialLoss;
        char *country;
        char *attackType;
        char *targetIndustry;
        char *defenseMechanism;
        char removido;
        int tamanhoRegistro;
        int long prox;
        // Armazena os tamanhos dos campos -> não é escrito no arquivo
        int tmnCountry;
        int tmnAttackType;
        int tmnTargetIndustry;
        int tmnDefenseMechanism;
    } REGISTRO;

    // Struct do header da Árvore B
    typedef struct{
        char status;
        int noRaiz;
        int proxRRN;
        int nroNos;
        char lixo[31];
    } HEADER_ARVORE;

    // Struct de um nó / página da Árvore B    
    typedef struct{
        int tipoNo;
        int nroChaves;
        // Os vetores tem 1 a mais de tamanho para facilitar os casos de split
        int ponteirosNos[ORDEM_ARVORE + 1];
        int chaves[ORDEM_ARVORE];
        long int byteOffsets[ORDEM_ARVORE];
    } NO_ARVORE;
    
#endif