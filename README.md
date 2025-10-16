# Glurr'ik e CocoonVM

Este repositório contém a linguagem de programação Glurr'ik, que tem como finalidade simular as funcionalidades biológicas dos casulos - estruturas biológicas que os monstros de gosma conseguem criar para entrar em seu estado mais vulnerável e entrar em contato com a mente coletiva - como uma forma de explorar e controlar o ciclo de transformação dos casulos, sem colocar uma criatura em risco.

A CocoonVM é uma máquina virtual que simula artificialmente um casulo e todas as funções biológicas que consegue realizar, como consumir energia sombria, regular a entrada e saída de informações da mente coletiva, fusão, separação, transformação e o crescimento do indivíduo dentro do casulo.

## Motivação

Na ficção, os monstros de gosma surgiram quando uma espécie de alienígenas que coletavam amostras de espécies de vários planetas chegam à Terra e se deparam com uma Pedra Sombria - uma rocha extremamente perigosa, abundante em energia sombria - e quando ela entrou em contato com os exemplares e o líquido essencial para a sobrevivência dos alienígenas, o resultado foi uma explosão radioativa que transformou o líquido em gosma e todos os habitantes da ilha que estavam em monstros de gosma. Após a transformação, todos os monstros foram interligados a uma mente coletiva, chamada de Colmeia, e são capazes de se transformar em qualquer coisa, mas tornaram-se dependentes de energia sombria para sobreviver. Ao decorrer dos milhares de anos, a Colmeia foi se expandindo e os monstros de gosma criaram uma sociedade tão desenvolvida quanto a humana, com cultura e língua própria, tecnologias avançadas e sistemas de classificação social baseado na força. 

Um pesquisador, que é um monstro de gosma, curioso em estudar o funcionamento dos casulos e da mente coletiva, decidiu criar uma linguagem de programação que pudesse simulá-los e os pensamentos da mente coletiva, sem colocar algum monstro de gosma em risco. E para isso, ele utilizou uma versão artificial e hipotética de um casulo como uma VM para criar um ambiente controlado para os experiementos, já que um casulo é um container e um sistema ao mesmo tempo, como se fosse uma VM orgânica.

Escolhi utilizar uma história fictícia para utilizar como base da línguagem, pois há alguns meses atrás, que foi o momento que de fato comecei a estruturar a história, percebi que criei uma sociedade tão complexa quanto a humana, ou seja, eles tinham cultura e identidade própria, um território exclusivo deles e uma estrutura social, com base em níveis de poder. Então, naturalmente, aproveitei que tinha a base fictícia e decidi montar a linguagem de programação em cima dela.

## EBNF

<pre>```
PROGRAM         = { STATEMENT } ;

STATEMENT       = PROCESS_DECL | LINK_STMT | MUTATE_STMT | MERGE_STMT
                | IF_STMT| LOOP_STMT | RETURN_STMT ;
PROCESS_DECL    = "Srrl" IDENTIFIER "(" [ PARAM_LIST ] ")" "{" { STATEMENT } "}" ;
PARAM_LIST      = IDENTIFIER { "," IDENTIFIER } ;
LINK_STMT       = "Hrrash" IDENTIFIER "->" IDENTIFIER ;
MUTATE_STMT     = "Vleth" IDENTIFIER EXPRESSION ;
MERGE_STMT      = "Drazh" IDENTIFIER "," IDENTIFIER "Esshl" IDENTIFIER ;
IF_STMT         = "Frral" "(" EXPRESSION ")" "{" { STATEMENT } "}" 
                  [ "Shrelk" "{" { STATEMENT } "}" ] ;
LOOP_STMT       = "Zrran" "(" EXPRESSION ")" "{" { STATEMENT } "}" ;
RETURN_STMT     = "Srryl" EXPRESSION ;

EXPRESSION      = LITERAL | IDENTIFIER | "(" EXPRESSION ")" | EXPRESSION OPERATOR EXPRESSION ;
LITERAL         = NUMBER | STRING | "Esshl" ;
OPERATOR        = "+" | "-" | "*" | "/" ;

IDENTIFIER      = SYLLABLE { SYLLABLE } ;
SYLLABLE        = CONSONANT VOWEL [ CONSONANT ] ;
CONSONANT       = "s" | "r" | "l" | "h" | "v" | "d" | "z" | "k" | "m" | "g" ;
VOWEL           = "a" | "e" | "i" | "o" | "u" ;
```</pre>

## Arquitetura da CocoonVM

A CocoonVM possui uma arquitetura minimalista, com quatro registradores principais:

| Registrador | Função |
|--------------|--------|
| R1 | Massa (quantidade de gosma / vitalidade) |
| R2 | Energia Sombria (fonte vital e combustível) |
| R3 | Mutação (forma atual) |
| R4 | Fluxo Coletivo (nível de conexão com a Colmeia) |

Além disso, há sensores de ambiente como `sensor_energia`, `sensor_calor`, `sensor_fluxo` e `sensor_impacto`, que representam estímulos externos lidos pelos casulos.

## Exemplo

### Código Glurr'ik

<pre>```glurrik
Srrl CasuloPrincipal() {
  Vleth massa Esshl
  Frral (sensor_energia > 50) {
    Hrrash gosma -> colmeia;
  } Shrelk {
    Drazh gosma, energia Esshl fusao;
  }
  Srryl massa;
}
```</pre>

## Saida em Assembly

<pre>```LABEL CasuloPrincipal
MUTAR massa
CMP sensor_energia, 50
IFGT bloco1
DISSOLVER gosma, energia -> fusao
JMP fim
LABEL bloco1
PARTILHAR gosma, colmeia
LABEL fim
RETURN R1
HALT```</pre>

