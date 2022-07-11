/* Trabalho 1 - Inf1018 - 2022.1
	
	Luca Ribeiro - 2112662 - 3WA
	Thiago Becker - 2110449 - 3WA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gravacomp.h"

int gravacomp (int nstructs, void* valores, char* descritor, FILE* arquivo){
    unsigned char cont, tipo, tamanho, cab; /* variáveis para cálculo e gravação do cabeçalho de cada campo */
    unsigned char temInt; /* verificar se tem ou não int ou unsigned int para corrigir deslocamento na memória */
    int *auxInt,tamInt; /* ponteiro para typcasting de tipo int e auxiliar para calcular o tamanho necessário para gravar o inteiro */
	int digito, charDezena, charUnidade, i, j; 
    int sucesso, numCampos; /* verificar se não houve erro */
    char *auxString; /* ponteiro para typecasting de string */
    unsigned int *auxUns, tamUns; /* ponteiro para typcasting de tipo unsigned int e auxiliar para calcular o tamanho necessário para gravar o unsigned */
    
    fwrite(&nstructs, 1, 1, arquivo); /* gravar número de estruturas (até 255) */

    temInt = 0;
    sucesso = 0;
    numCampos = 0; 
    
    for(j = 0; j < nstructs; j++){
        i = 0;
        cont = 0; /* zerar cont antes de cada loop */
        while(descritor[i] != '\0'){
            if(descritor[i] == 's'){ /* tipo string */
                            
                tipo = 0x40; 
				
				/* calcular quantos bytes a string ocupa na struct */
                
                charDezena = descritor[i+1] - '0';
                charUnidade = descritor[i+2] - '0';
                digito = charDezena*10 + charUnidade;
				
                i += 2;
                
                auxString = (char*)valores; /* typecasting */
                tamanho = strlen(auxString);
              
                if(descritor[i+1] == '\0'){ /* verifica se é o último campo da estrutura */
                    cont = 0x80;
                }
              
                cab = cont + tipo + tamanho;
				
				/* gravação de cabeçalho e string no arquivo */
                fwrite(&cab, 1, 1, arquivo);
                sucesso += fwrite(auxString, tamanho, 1, arquivo);
               
                /* verificar se precisa corrigir padding */
                if(descritor[i+1] == 'i' || descritor[i+1] == 'u' || (descritor[i+1] == '\0' && temInt != 0)){ 
                    if(digito %4 != 0){
                        digito += 4 - (digito % 4);
                    }
                }
                valores += digito; /* passar para o próximo campo da struct ou finalizar o loop */
            }
            else{
                temInt = 1;

                if(descritor[i+1] == '\0'){ /* verifica se é o último campo da estrutura */
                    cont = 0x80;
                }
                
                if(descritor[i] == 'i'){ /* tipo int */
                    tipo = 0x20;
                    
                    auxInt = (int*)valores; /* typecasting */
                    
                    tamInt = *auxInt;
                    
                    tamanho = 1;
                    
					/* com 1 byte de int é possível escrever de -128 até 127
					para valores fora desse intervalo será preciso mais de 1 byte */
                    if((tamInt & 0x80000000) == 0){ /* bit mais significativo é 0: int positivo */
                        while(tamInt >= 128){ /* entra no loop se não for possível escrever com somente 1 byte */
                            tamanho++;
                            tamInt = tamInt >> 8; /* move um byte para a direita */
                        }
                    }
                    else{ /* bit mais significativo é 1: int negativo */
                        while(tamInt < -128){ /* entra no loop se não for possível escrever com somente 1 byte */
                            tamanho++;
                            tamInt = tamInt >> 8; /* move um byte para a direita */
                        }
                    }
                    
                    cab = cont + tipo + tamanho;
					
                    /* gravação de cabeçalho e inteiro no arquivo */
                    fwrite(&cab, 1, 1, arquivo);
                    sucesso += fwrite(auxInt, tamanho, 1, arquivo);
                    
                    valores += 4; /* passar para o próximo campo da struct ou finalizar o loop */
                  
                }
                else if(descritor[i] == 'u'){ /* tipo unsigned */
                    tipo = 0x00;
                    
                    auxUns = (unsigned int*)valores; /* typecasting */
                    
                    tamanho = 1;
                    tamUns = *auxUns;
                    
					/* com 1 byte de unsigned int é possível escrever de 0 até 255
					para valores maiores 255 será preciso mais de 1 byte */
                    while(tamUns > 255){ /* entra no loop se não for possível escrever com somente um byte */
                        tamanho++;
                        tamUns = tamUns >> 8; /* move um byte para a direita */
                    }
                                     				  
					cab = cont + tipo + tamanho;
					
                    /* gravação de cabeçalho e unsigned no arquivo */
                    fwrite(&cab, 1, 1, arquivo);
                    sucesso += fwrite(auxUns, tamanho, 1, arquivo);
                    
                    valores += 4; /* passar para o próximo campo da struct ou finalizar o loop */
                    
                }
            }
            
            numCampos++; /* contador para verificar quantos loops são feitos e comparar com quantos elementos são gravados no arquivo */
            i++;
        }
        
    }
    
	/* verifica se o contador de loops é diferente do contador de elementos gravados no arquivo */
    if(numCampos != sucesso){
        return -1;
    }
    else{
        return 0;
    }
}

void mostracomp (FILE *arquivo){
    int nStructs, i, j;
	int auxFF; /* variável auxiliar para fazer a correção de inteiros negativos lidos pela função */
	int valInt;
    unsigned char cont, tamanho, cab; 
	unsigned char vetInt[4]; /* vetor de bytes (tipo unsigned) para leitura do inteiro do arquivo, lendo byte por byte */
    char *string;
    unsigned int valUns, auxUns;
    
    fread(&nStructs, 1, 1, arquivo);
    
    printf("Estruturas: %d\n\n", nStructs);
    
    for(i = 0; i < nStructs; i++){
        cont = 0; /* zerar o cabeçalho antes de todo loop */
        
        while(cont == 0){
            fread(&cab, 1, 1, arquivo);
            
            cont = cab & 0x80; // 0x80 = 1000 0000
            
            if((cab & 0x40) != 0){ /* tipo string */   // 0x40 = 0100 0000
                tamanho = cab & 0x3f;                  // 0x3F = 0011 1111
                
              string = (char*)malloc(tamanho);
              if(!string){
                    printf("Erro na alocação de memória\n");
                    exit(1);
              }
                
              fread(string, tamanho, 1, arquivo);
                
              printf(" (str) %s\n", string);
              
            }
            else{
                tamanho = cab & 0x1f;                  // 0x1f = 0001 1111
              
                if((cab & 0x20) != 0){ /* tipo int */  // 0x20 = 0010 0000
                    fread(vetInt, tamanho, 1, arquivo);

                    valInt = 0;
                    auxFF = 0xffffffff; /* auxiliar para caso de inteiro negativo */

                    if((vetInt[tamanho-1] & 0x80) != 0){ /* inteiro negativo */
                        for(j = 0; j < tamanho; j++){                 
                            /* mover um byte para a direita */
							auxFF = auxFF << 8; 
                            valInt = valInt << 8; 
                            valInt += vetInt[tamanho-1-j]; /* corrigir ordem dos bytes */
                        
                        }
                        valInt += auxFF; /* correção de bits por ser negativo */
                        
                    }
                    else{ /* inteiro positivo */
                        for(j = 0; j < tamanho; j++){
                            valInt = valInt << 8;
                            valInt += vetInt[tamanho-1-j]; /* corrigir ordem dos bytes */
                        }
                    }

                    printf(" (int) %d (%08x)\n", valInt, valInt);
                }
                else{ /* tipo unsigned */
                    fread(&valUns, tamanho, 1, arquivo);
                    
                    auxUns = 0;
                    for(j = 0; j < tamanho; j++){ 
                        auxUns += ((valUns & 0x000000ff) << (8*j)); /* corrigir ordem dos bytes */
                        valUns = valUns >> 8; 
                    }
                    valUns = auxUns; 
                    
                    printf(" (uns) %u (%08x)\n", valUns, valUns);
                }
            }
            
        }
        printf("\n");
    }
}