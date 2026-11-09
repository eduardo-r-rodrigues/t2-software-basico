#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

static size_t alinha(size_t size)
{
    return (size + 7) / 8 * 8;
}

static size_t ocupado(mymemory_t *memory, allocation_t *a)
{
    size_t pos = (char *)a->start - (char *)memory->pool;
    size_t tam = alinha(a->size);
    if (pos + tam > memory->total_size)
        tam = memory->total_size - pos;
    return tam;
}

mymemory_t* mymemory_init(size_t size)
{
    if (size == 0)
        return NULL;

    mymemory_t *memory = malloc(sizeof(mymemory_t));
    if (memory == NULL)
        return NULL;

    memory->pool = malloc(size);
    if (memory->pool == NULL) {
        free(memory);
        return NULL;
    }

    memory->total_size = size;
    memory->head = NULL;
    memory->strategy = FIRST_FIT;
    return memory;
}

void mymemory_set_strategy(mymemory_t *memory, strategy_t strategy)
{
    memory->strategy = strategy;
}

void* mymemory_alloc(mymemory_t *memory, size_t size)
{
    if (memory == NULL || size == 0 || size > memory->total_size)
        return NULL;

    size_t precisa = alinha(size);

    int achou = 0;
    size_t melhor_pos = 0;
    size_t melhor_tam = 0;
    allocation_t *melhor_ant = NULL;

    size_t inicio_lacuna = 0;
    allocation_t *ant = NULL;
    allocation_t *atual = memory->head;

    while (1) {
        size_t fim_lacuna;
        if (atual != NULL)
            fim_lacuna = (char *)atual->start - (char *)memory->pool;
        else
            fim_lacuna = memory->total_size;

        size_t lacuna = fim_lacuna - inicio_lacuna;

        int cabe = lacuna >= precisa || (atual == NULL && lacuna >= size);

        if (cabe) {
            int escolhe = 0;
            if (!achou)
                escolhe = 1;
            else if (memory->strategy == BEST_FIT && lacuna < melhor_tam)
                escolhe = 1;
            else if (memory->strategy == WORST_FIT && lacuna > melhor_tam)
                escolhe = 1;

            if (escolhe) {
                achou = 1;
                melhor_pos = inicio_lacuna;
                melhor_tam = lacuna;
                melhor_ant = ant;
            }

            if (memory->strategy == FIRST_FIT)
                break;
        }

        if (atual == NULL)
            break;

        inicio_lacuna = fim_lacuna + ocupado(memory, atual);
        ant = atual;
        atual = atual->next;
    }

    if (!achou)
        return NULL;

    allocation_t *novo = malloc(sizeof(allocation_t));
    if (novo == NULL)
        return NULL;

    novo->start = (char *)memory->pool + melhor_pos;
    novo->size = size;

    if (melhor_ant == NULL) {
        novo->next = memory->head;
        memory->head = novo;
    } else {
        novo->next = melhor_ant->next;
        melhor_ant->next = novo;
    }

    return novo->start;
}

void mymemory_free(mymemory_t *memory, void *ptr)
{
    if (memory == NULL || ptr == NULL)
        return;

    allocation_t *ant = NULL;
    allocation_t *atual = memory->head;

    while (atual != NULL && atual->start != ptr) {
        ant = atual;
        atual = atual->next;
    }

    if (atual == NULL)
        return;

    if (ant == NULL)
        memory->head = atual->next;
    else
        ant->next = atual->next;

    free(atual);
}

void mymemory_display(mymemory_t *memory)
{
    printf("Alocacoes (pool de %zu bytes em %p):\n", memory->total_size, memory->pool);

    if (memory->head == NULL)
        printf("  nenhuma alocacao\n");

    int i = 0;
    allocation_t *a = memory->head;
    while (a != NULL) {
        size_t pos = (char *)a->start - (char *)memory->pool;
        printf("  %d: inicio=%p (offset %zu) tamanho=%zu\n", i, a->start, pos, a->size);
        a = a->next;
        i++;
    }
}

void mymemory_stats(mymemory_t *memory)
{
    int total = 0;
    size_t alocado = 0;
    size_t usado = 0;
    size_t maior_livre = 0;
    int fragmentos = 0;

    size_t inicio_lacuna = 0;
    allocation_t *atual = memory->head;

    while (1) {
        size_t fim_lacuna;
        if (atual != NULL)
            fim_lacuna = (char *)atual->start - (char *)memory->pool;
        else
            fim_lacuna = memory->total_size;

        size_t lacuna = fim_lacuna - inicio_lacuna;
        if (lacuna > 0) {
            fragmentos++;
            if (lacuna > maior_livre)
                maior_livre = lacuna;
        }

        if (atual == NULL)
            break;

        total++;
        alocado += atual->size;
        usado += ocupado(memory, atual);
        inicio_lacuna = fim_lacuna + ocupado(memory, atual);
        atual = atual->next;
    }

    printf("Estatisticas:\n");
    printf("  alocacoes: %d\n", total);
    printf("  memoria alocada: %zu bytes (%zu com alinhamento)\n", alocado, usado);
    printf("  memoria livre: %zu bytes\n", memory->total_size - usado);
    printf("  maior bloco livre: %zu bytes\n", maior_livre);
    printf("  fragmentos livres: %d\n", fragmentos);
}

void mymemory_cleanup(mymemory_t *memory)
{
    if (memory == NULL)
        return;

    allocation_t *atual = memory->head;
    while (atual != NULL) {
        allocation_t *prox = atual->next;
        free(atual);
        atual = prox;
    }

    free(memory->pool);
    free(memory);
}
