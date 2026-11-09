# Trabalho 2 - Programacao de Software Basico

Matheus Zanchetta e Eduardo Rosa

Gerenciador de memoria em C que aloca e libera blocos dentro de um pool
pre-alocado.

## Compilar e rodar

    make
    ./main      exemplo do enunciado
    ./test      testes com 50 blocos e as 3 estrategias
    make check  roda os dois com AddressSanitizer (acusa erro de memoria e leak)

## Como funciona

A lista de `allocation_t` fica sempre ordenada pelo endereco de inicio. Com
isso, os espacos livres sao so os buracos entre uma alocacao e a proxima
(mais o comeco e o fim do pool). Nao precisamos de uma lista de livres
separada, e quando um bloco e liberado os buracos vizinhos se juntam sozinhos.

No `mymemory_alloc` percorremos esses buracos e escolhemos um de acordo com a
estrategia, definida com `mymemory_set_strategy`:

- First Fit: o primeiro buraco em que o bloco cabe
- Best Fit: o menor buraco em que o bloco cabe
- Worst Fit: o maior buraco

O novo no entra na lista logo depois do bloco anterior ao buraco, entao a
lista continua ordenada.

Os tamanhos sao arredondados para multiplo de 8 para os enderecos ficarem
alinhados (da pra guardar int, double, ponteiro...). O ultimo bloco do pool
pode ocupar so o que sobra ate o fim.

O `mymemory_free` procura o no com aquele endereco. Se nao achar (ponteiro
qualquer, meio de um bloco, NULL, free duplo), nao faz nada.

So usamos malloc/free para o pool, para o `mymemory_t` e para os nos da lista.

## Testes

O `test.c` testa os casos de borda (alloc 0, alloc maior que o pool, free
invalido, free duplo, reaproveitamento do espaco) e confere se cada estrategia
escolhe o buraco certo. Depois aloca 50 blocos de 10 a 1000 bytes em ordem
crescente, decrescente e aleatoria, libera mais ou menos metade e tenta alocar
mais 50 com o dobro do tamanho. Cada bloco e preenchido com um valor, e no fim
conferimos se nenhum foi sobrescrito.

Nos nossos testes, First Fit e Best Fit deixaram pouca memoria sobrando.
Worst Fit deixou mais memoria livre, mas picada em varios buracos pequenos.

## Uso de IA

Usamos o Claude (IA) como apoio para escrever parte do codigo e dos testes.
Revisamos e discutimos todo o codigo e sabemos explicar cada funcao.
