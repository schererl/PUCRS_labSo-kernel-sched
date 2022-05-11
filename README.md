# kkk eae man bota publico e deleta isso 

# labSo-escalonador
Victor Putrich e Lucca Dornelles

Tabela de conteúdos
=================
<!--ts-->
   * [Sobre](#Sobre)
   * [Escalonador](#Escalonador)
   * [Biblioteca-list](#Biblioteca-list)
   * [Implementação](#Implementação)
   * [Execução](#Execução)
<!--te-->

# Sobre
Iremos apresentar neste trabalho uma implementação da política de escalonamento SSTF (Shortest Seek Time First). O módulo será compilado junto ao Kernel Linux 4.13.9, usando o Buildroot. Para isto, utilizaremos chamadas de sistema disponíveis nas bibliotecas do Kernel em C, incluindo a biblioteca "list.h" que foi utilizada como estrutura de armazenamento das requisições feitas de acesso ao disco. Por fim, apresentaremos uma comparação entre a nossa implementação em relação a uma versão que também usa a estrutura de filas encadeadas, mas não implementa uma política específica de escalonamento chamada Noop.

# Escalonador
O escalonador é responsável por organizar o recebimento e leitura dos processos em disco. É trabalho dele fazer bom uso dos recursos disponíveis através de uma política de escalonamento. Para este trabalho iremos fazer uma comparação entre uma política simples chamada noop e outra política mais elaborada busca otimizar o acesso ao disco a partir da distânica entre a última leitura em disco e as seguintes.

## sstf
A política SSTF busca diminuir o seek time de disco que é o tempo que a cabeça de leitura do disco leva para se deslocar de um trilha até o destino, ou seja, a próxima requisição de leitura ou escrita. Para implementar tal política, comparamos duas possíveis soluções, a primeira que chamamos de Naive e a outra que chamamos de CAD (Closest Absolute Distance).

   - **SSTF-Naive**: Nesta versão, não usamos nenhum critério de ordenamento das requisições que chegam, elas são adicionadas a fila por ordem de chegada. Durante a fase de Dispatch, iteramos ao longo de toda a lista para pegar a requisição que tenha posição de leitura/escrita mais próxima da última requisição atendida.
    
   - **SSTF-CAD**: Depois de implementarmos uma política sem se preocupar com ordenamento, fizemos uma versão que passa a ordenar as requisições conforme chegam, garantindo que durante o dispatch sempre removamos o primeiro elemento da fila. Este processo simplifica o tempo de processamento do dispatch que deixa de iterar a lista inteira sempre e adiciona uma maior complexidade na operação de ADD da fila. Para o Add, usamos a posição absoluta das requisições, mas ordenamos pela distância entre as requisições conseguintes. Existem três cenários que podem acontecer na adição de uma nova requisição.
        
        * *Novo Head*: Se a distância da nova requisição com a última requisição atendida é menor que head e a última requisição, então atribuímos head para a nova requisição e o antigo head vira o próximo da fila.
         <p align="center"> <img src="https://github.com/schererl/labSo-escalonador/blob/main/artifacts/add-head.png" width="450"/></p>

        * *Meio da fila*: Iteramos a fila até encontrarmos uma situação como, a distânica entre a requisição da fila e a nova requisição é menor que a requisição da fila e a sua próxima requisição. Neste caso colocamos a nova requisição no "meio" da requisição e de sua antiga sucesora.
         <p align="center"><img src="https://github.com/schererl/labSo-escalonador/blob/main/artifacts/add-middle.png" width="450"/></p>
        
       * *Fim da fila*: Se em nenhum momento encontramos um espaço onde a nova requisição é mais próxima de algum dos elementos da fila e seu sucessor, ela é inserida no fim da fila.


# Biblioteca-list

A biblioteca list.h pode ser consultada em [https://github.com/torvalds/linux/blob/master/include/linux/list.h]. Ela fornece uma implementação de lista duplamente encadeada que pode ser usada sobre qualquer tipo de estrutura que deseja representar como uma fila. Para a nossa implementação de sstf, não foi necessário usarmos qualquer tipo de atributo adicional além do que se pode obter a partir da lista, extraímos a posição de seek de cada requisição usando o método blk_rq_pos(requsição) como parâmetro de ordenação.

Para operarmos sobre a lista, precisamos adicionar à nossa estrutura um componente chamado list_head que deve ser inicializado pelo método init_list_head que recebe de parâmetro a referência para a instância inicial da fila. A seguir, listamos os métodos utilizados da biblioteca e onde usamos eles:

   * **list_first_entry_or_null**(list_head \*entry, struct request, member): Função usado para pegar o primeiro elemento da lista, neste caso, retorna um tipo request.
   * **list_del_init**(list_head \*entry): Deleta o elemento passado por ponteiro 'entry'. Usado quando a requisição é consumida no dispatch.
   * **list_add**(list_head *new, list_head *head): Adiciona um elemento novo a partir de head. Usamos esta função no SSTF-CAD para inserir uma nova requisição no meio de duas já existentes na fila, também usamos para adicionar um novo head à lista.
   * **list_add_tail**(list_head \*new): Adiciona um novo elemento que contém uma requisição no final da fila. Usamos ela em ambas implementações do sstf.
   * **list_for_each**(list_head \*cur, list_head queue): Um macro fornecido pela lib que itera toda lista encadeada e atribui à 'cur' o elemento atual. Este método é usado para iterarmos a fila no método dispatch do SSTF-Naive.
   * **list_entry**(list_head \*cur, struct request, queuelist): Esta função é usada dentro de list_for_each para extrair de 'cur' o request da posição corrente. Foi usado em SSTF-Naive para saber a posição de seek de cada request para buscar a menor distânica com a última posição de seek lida.


# Implementação

### sstf_data
Para implementarmos as duas forma de SSTF usando a biblioteca "list.h", primeiro criamos uma struct chamada *sstf_data* que guarda um *list_head*, pré-requisito para fazer uso da lista.

### abslong
Precisamos medir a distância entre duas requisições, para isso precisamos pegar o valor absoluto e retornar um tipo *long long int*, criamos a função abslong para esta tarefa.

### printList
Para que pudessemos visualizar o que estava acontecendo com as listas de requisições, criamos a função printList que recebe um ponteiro para *SSTF_DATA* e retorna no kernel uma chamada de sistema de saída de toda lista.

## sstf_add -Naive
Os métodos de add são responsáveis por adicionar na fila novas requisições que estão chegando, usamos as funções 'list_add_tail' para fazê-lo.
Na versão *Naive* como não fazemos ordenamento, simplesmente vamos adicionando no final da fila as requisições que vão chegando.
```
static void sstf_add_request(struct request_queue *q, struct request *rq){
  struct sstf_data *nd = q->elevator->elevator_data;
	
	list_add_tail(&rq->queuelist, &nd->queue);
	}
```
Para adicionarmos o novo request que chega, precisamos passar por parâmetro a entrada do tipo 'list_head' que está armazenada no parâmetro 'queuelist' em request e também precisamos passar a início da fila que está em 'sstf_data'.

## sstf_add -Closest Absolute Distance
Na versão CAD, a operação de inserção na fila é a mais importante, porque é onde é feita a lógica da política. Dividimos ela ao explicaros o CAD como sendo em três partes, porém não foi mencionado um caso especial que pode ser considerado trivial, quando a lista está vazia sem requisições, simplesmente adicionamos em TAIL a requisição.
 
```
 static void sstf_add_request(struct request_queue *q, struct request *rq){
	struct sstf_data *nd = q->elevator->elevator_data;
	struct list_head *ptr; 
	long long unsigned int req_pos = blk_rq_pos(rq);	
	struct request *head_req = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	long long unsigned int head_pos = (long long unsigned int) blk_rq_pos(head_req);

	// Lista vazia: adiciona em tail
	if(head_req == NULL){
		list_add_tail(&rq->queuelist, &nd->queue);
        
		return;
	}
. . .
```

Em seguida, é necessário avaliar se a distância entre a posição da nova requisição e a última requisição lida é menor que a distância entre 'head' e a última posição lida, se for, quer dizer que a nova requisição deve ser o 'head' da lista:
```
. . .
	if(abslong(req_pos - last_pos) < abslong(head_pos  - last_pos)){
		struct list_head *old_head = &nd->queue;
        struct list_head *new_head = &rq->queuelist;
        list_add(new_head, old_head);
        
		return;
	} 
 
. . .
```
	
 Caso nenhuma das situações anteiores se verifiquem, iremos tentar posicionar a nossa requisição entre duas requisições da fila, faremos isso comparado as distâncias da requisição corrente *curE* e de seu sucessor *nextE*, caso a distância entre eles seja maior que entre *curE* e a nova requisição, inserimos ela no meio através do método 'list_add', passando a requisição e um ponteiro do tipo 'list_head' para a posição corrente:
 ```
	. . .
	
	ptr = (&(nd -> queue))->next; //pula head porque já foi avaliado anteriormente.
	while(ptr->next != &(nd -> queue)){
		 struct list_head *next = ptr->next;
		 
		 struct request *curE = list_entry(ptr, struct request, queuelist); 
		 struct request *nextE = list_entry(next, struct request, queuelist); 
		 long long unsigned int curPos = blk_rq_pos(curE);
		 long long unsigned int nextPos = blk_rq_pos(nextE);

		 if(abslong(curPos - nextPos) > abslong(curPos - req_pos)){
			  list_add(&rq->queuelist, ptr); 
      
      			  return;
		 }

		 ptr = next;
	} 
 . . .
```
Se a nova requisição ainda não encontrou o seu lugar, não precisamos nos apavorar, basta adicioná-la no final da lista que é o seu devido lugar.
```
. . .
	//tail
	list_add_tail(&rq->queuelist, &nd->queue);
}
```


## sstf_dispatch -Naive
Um dos métodos principais para as duas abordagens, o sstf_dispatch recebe como parâmetro um ponteiro para o request do tipo 'request_queue', a partir dele extraímos a nosssa struct criada *sstf_data*.
Existem duas abordagens deste método, primeiro iremos presentar como é feita o dispatch no *sstf-Naive* e depois no *sstf-CAD*.

O código abaixo mostra a implementação do dispatch com uma fila encadeada sem ordenamento:
```
static int sstf_dispatch(struct request_queue *q, int force){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	struct request *rq;

	struct request *closest_rq = NULL;
	struct list_head *closest_head = NULL;
	long long unsigned int closest_dist = 23372036854775807;
	struct list_head *ptr;
	list_for_each(ptr, &nd->queue){
		rq = list_entry(ptr, struct request, queuelist); 
		long long unsigned int new_pos = blk_rq_pos(rq);
		
		if ( abslong(new_pos - last_pos) < closest_dist ) {
			closest_rq = rq;
			closest_head = ptr;
			closest_dist = abslong(new_pos - last_pos); 
		}
	}
	
	if(closest_rq){
		last_pos = blk_rq_pos(closest_rq);
		list_del(closest_head);
		elv_dispatch_sort(q, closest_rq);
		
		return 1;
	}

	return 0;
 }

```
Perceba que para que possamos dar dispatch efetivamente em uma requisição precisamos iterar toda lista com o método *list_for_each* e toda vez que encontramos um elemento que tenha uma distância entre a última posição lida (*last_pos*) consigo, menor que a armazenada em *closest_distance*, a sobreescrevemos com o novo valor achado. Um último detalhe é que no final é preciso atualizar a última posição lida como sendo a que está em 'closes_distance'.

## sstf_dispatch -Closest Absolute Distance

Como explicado anteriormente, no *SSTF-CAD* a complexidade da política fica para o momento de adição de novas requisições na fila, logo tudo que é preciso fazer no dispatch é remover e processar o primeiro da fila:
```
struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	struct request *rq;
	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	if (rq) {
		last_pos = blk_rq_pos(rq);
   		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		
		return 1;
	}
	return 0;
```
Removemos o 'HEAD' da fila e se este não for null, atualizamos a última posição lida e damos dispatch na requisição.


# Execução

## Resultados
 Para que possamos visualizar a diferença nas políticas de leitura ativamos no nosso Kernel Linux a política desejada Noop ou SSTF e rodamos o script sector_read para disparar diversas requisições de acesso ao disco. 
 
 Apresentaremos a seguir uma comparação de acesso entre o Noop e o *SSTF-Naive*, ou seja, a ordem de acesso ao disco é referente a sequência de dispatch das duas políticas. Executamos um test de stress de acesso ao disco, produzimos 4 forks em 'sector_read' produzindo 10 operações de acesso a disco. Os gráficos tem uma relação de números de requisições atendidas por posição de seek:
 
 <p float="left">
  <img src="https://github.com/schererl/labSo-escalonador/blob/main/output/execucao-noop.png" width="400" alt="Noop"/>
  
  <img src="https://github.com/schererl/labSo-escalonador/blob/main/output/execucao-sstf-naive.png" width="400" alt="SSTF-Naive"/>
 </p>
 
 O gráfico a esquerda representa a operação de *Noop* e o gráfico à direita *SSTF-Naive*. A medida que mais requisições vão se acumulado no SSTF ele vai deixando de apresentar variações bruscas de sentido e sua declividade fica cada vez mais suave, atendendo uma grande parcela de requisições em um único movimento de subida ou descida. Isso ocorre porque a medida que as requisições se acumulam, elas vão sendo atendidas visando minimizar a distância que o disco deve percorrer até a próxima requisição. 
 
 *As tabelas com os resultados que originaram os gráficos se encontram neste repositório nos links:*
  * https://github.com/schererl/labSo-escalonador/blob/main/output/log-noop.txt
  * https://github.com/schererl/labSo-escalonador/blob/main/output/log-sstf-Naive_4f.txt
 
 Para que fosse possível visualizar de forma mais clara a execução da política *SSTF-CAD* (figura logo abaixo), foi necessário aumentar o número de forks executados para 6 e ainda colocar os processos em espera antes de fazer Dispatch. O que ocorreu com os testes de estresse em disco foi que todas as requisições estavam entrando e saindo da fila diretamente. A explicação que encontramos para este comportamento, que ficou muito mais frequente no *CAD*, foi que como a complexidade da operação de ADD aumentou e o de Dispatch ficou o mais simples possível, o kernel conseguia consumir requisições muito mais rápido que adicioná-las, em outras palavras, o fluxo de entrada ficou muito mais rápido que o fluxo de saída, portanto, a política de ordenamento não estava conseguindo sequer entrar em ação.
 
 <p align="center"><img src="https://github.com/schererl/labSo-escalonador/blob/main/output/execucao-sstf-CAD-6FORKS.png" width="850"/></p>
 
 O gráfico acima mostra que mesmo aumentando o número de processos, logo, aumentando as chances de ocorrerem operações de adição simultâneas, a execução muitas vezes não consegue enfileirar uma grande quantidade de requisições. Observamos também os momentos aonde os processos são colocados para dormir e logo é desencadeado uma sequênica de dispatch que é feita em ordem, tanto com a inclinação virada para cima ou para baixo, ocorrendo periodicamente.
 
 Devido a discrepância entre as duas operações, tivemos dificuldade em fazer um teste que pudesse ser comparado com o *Noop* e o *SSTF-Naive*, não podemos determinar se o *SSTF-Naive* é menos eficiente em termos de tempo de execução que o *SSTF-CAD*, porém podemos deduzir que o *CAD* é mais eficiente por não precisar passar por toda lista toda vez que fora fazer uma operação de inserção na fila, ao contrário do Dispatch no naive. Presumimos que o *CAD* seja mais eficiente em termos de tempo de execução, mas nos testes que realizamos, o *SSTF-Naive* teve um melhor aproveitamento e utilização de sua política.
 
 Tentamos medir os tempos dos testes usando a biblioteca "linux/time.h" que em teoria pode ser usada pelo kernel para extrair os tempos de execução, porém não conseguimos usar as estruturas por um erro interno na hora de compilar. Dessa forma poderíamos comparar o *SSTF-Naive* com *SSTF-CAD*
avaliando não só a política, mas a eficiência de execução dos dois. 
 
