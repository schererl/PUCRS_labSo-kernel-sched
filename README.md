# labSo-escalonador
Victor Putrich e Lucca Alguma Coisa

 - TODO: Descobrir como adiciona a biblioteca time.h no kernel pelo buildroot

 - TODO2: Capturar na execução do escalonador a média de tempo de execução do add, dispatch e do programa como um todo pra poder comparar eles depois

## Sobre
Iremos apresentar neste trabalho uma implementação da política de escalonamento SSTF (Shortest Seek Time First). O módulo será compilado junto ao Kernel Linux 4.13.9, usando o Buildroot. Para isto, utilizaremos chamadas de sistema disponíveis nas bibliotecas nativas de C, incluindo a biblioteca "list.h" que foi utilizada como estrutura de armazenamento das requisições feitas de acesso ao disco. Por fim, apresentaremos uma comparação entre a nossa implementação em relação a uma versão que também usa a estrutura de filas encadeadas, mas não implementa uma política específica de escalonamento que não seja FIFO (First In First Out), chamada Noop.

## Escalonador
O escalonador é responsável por organizar o recebimento e leitura dos processos em disco. É trabalho dele fazer bom uso dos recursos disponíveis através de uma política
de escalonamento. Para este trabalho iremos fazer uma comparação entre uma política simples chamada noop e outra política mais elaborada busca otimizar o acesso ao disco
a partir da distânica entre a última leitura em disco e as seguintes.

### noop

### sstf
A política SSTF busca diminuir o seek time de disco que é o tempo que a cabeça de leitura do disco leva para se deslocar de um trilha até o destino, ou seja, a próxima requisição de leitura ou escrita. Para implementar tal política, comparamos duas possíveis soluções, a primeira que pode ser chamada de Naive e a outra que chamamos de CAD (Closest Absolute Distance).

   - SSTF-Naive: Nesta versão não fizemos não usamos nenhum critério de ordenamento das requisições que chegam, elas são adicionadas a fila por ordem de chegada. Durante a fase de Dispatch, iteramos ao longo de toda a lista para pegar a requisição que tenha posição de leitura/escrita mais próxima da última requisição atendida.
    
   - SSTF-CAD: Depois de implementarmos uma política sem se preocupar com ordenamento, fizemos uma versão que passa a ordenar as requisições conforme chegam, garantindo que durante o dispatch sempre removemos o primeiro elemento da fila. Este processo simplifica o tempo de processamento do dispatch que deixa de iterar a lista inteira sempre e adiciona uma maior complexidade na operação de ADD da fila. Para o Add, usamos a posição absoluta das requisições, mas ordenamos pela distância entre as requisições conseguintes. Existem três cenários que podem acontecer na adiçãode uma nova requisição.
        
        * Novo Head: Se a distância da nova requisição e head é menor que head e seu suscessor, então atribuímos head para a nova requisição e o antigo head vira o próximo da fila.
                       <p align="center"> <img src="https://github.com/schererl/labSo-escalonador/blob/main/artifacts/add-head.png" width="350"/></p>

        * Meio da fila: Iteramos a fila até que encontremos uma situação como, a distânica entre a requisição da fila e a nova requisição é menor que a requisição da fila e a sua próxima requisição. Neste caso colocamos a nova requisição no "meio" da requisição e de sua antiga suscesora.
         <p align="center"><img src="https://github.com/schererl/labSo-escalonador/blob/main/artifacts/add-middle.png" width="350"/></p>

        
       * Fim da fila: Se em nenhum momento encontramos um espaço onde a nova requisição é mais próxima de algum dos elementos da fila e seu suscessor, ela é inserida no fim da fila.


## Biblitoeca "list.h"
A biblioteca list.h pode ser consultada em [https://github.com/torvalds/linux/blob/master/include/linux/list.h]. Ela fornece uma estrutura de lista que pode ser usada sobre qualquer tipo de estrutura que deseja representar como uma fila. Para usar ela, precisamos adicionar à nossa estrutura um componente chamado list_head que deve ser inicializado pelo método init_list_head que recebe de parâmetro a referência para a instância inicial da fila.


## Arquitetura e Implementação


Aqui falar com detalhes como fizemos o nosso escalonador
- Os scriptos que geramos para execução
- Compilar e ativar o escalonador no Sistema Operacional
- Falar sobre os métodos usados (dispatch, add, init, exit..)
- Falar sobre o original
- Falar sobre a nossa versão mais simples
- Falar sobre como vai ser a nossa versão com ordenamento no método add

## Resultados
 Por enquanto apresentar os dois gráficos que temos e compará-los (noop e sstf-sem ordenamento)
 
 
