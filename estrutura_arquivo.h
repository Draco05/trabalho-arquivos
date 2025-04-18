#ifndef TABELA
    #define TABELA
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
#endif