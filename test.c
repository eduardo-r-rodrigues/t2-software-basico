#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymemory.h"

#define N 50
#define POOL 20000

int erros = 0;

void testa(int ok, const char *msg)
{
    if (!ok) {
        printf("ERRO: %s\n", msg);
        erros++;
    }
}

size_t tamanhos_base[] = {10, 25, 50, 100, 250, 500, 1000, 15, 75, 150};
const char *nome_estrategia[] = {"First Fit", "Best Fit", "Worst Fit"};
const char *nome_ordem[] = {"crescente", "decrescente", "aleatoria"};

int compara(const void *a, const void *b)
{
    size_t x = *(const size_t *)a;
    size_t y = *(const size_t *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

void gera_tamanhos(size_t *t, int ordem)
{
    for (int i = 0; i < N; i++)
        t[i] = tamanhos_base[i % 10];

    if (ordem == 0 || ordem == 1)
        qsort(t, N, sizeof(size_t), compara);

    if (ordem == 1) {
        for (int i = 0; i < N / 2; i++) {
            size_t aux = t[i];
            t[i] = t[N - 1 - i];
            t[N - 1 - i] = aux;
        }
    }

    if (ordem == 2) {
        for (int i = N - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            size_t aux = t[i];
            t[i] = t[j];
            t[j] = aux;
        }
    }
}

void confere_lista(mymemory_t *m)
{
    char *fim_anterior = m->pool;
    char *fim_pool = (char *)m->pool + m->total_size;

    for (allocation_t *a = m->head; a != NULL; a = a->next) {
        char *ini = a->start;
        testa(ini >= fim_anterior, "blocos sobrepostos");
        testa(ini + a->size <= fim_pool, "bloco passou do fim do pool");
        testa((ini - (char *)m->pool) % 8 == 0, "endereco desalinhado");
        fim_anterior = ini + a->size;
    }
}

int bloco_intacto(unsigned char *p, size_t tam, int valor)
{
    for (size_t i = 0; i < tam; i++)
        if (p[i] != (unsigned char)valor)
            return 0;
    return 1;
}

void cenario(strategy_t estrategia, int ordem)
{
    size_t tam[N], tam2[N];
    void *p[N], *p2[N];

    printf("\n=== %s, ordem %s ===\n", nome_estrategia[estrategia], nome_ordem[ordem]);

    mymemory_t *m = mymemory_init(POOL);
    mymemory_set_strategy(m, estrategia);

    srand(42 + ordem);
    gera_tamanhos(tam, ordem);

    for (int i = 0; i < N; i++) {
        p[i] = mymemory_alloc(m, tam[i]);
        testa(p[i] != NULL, "primeiros 50 blocos deveriam caber");
        if (p[i] != NULL)
            memset(p[i], i + 1, tam[i]);
    }
    confere_lista(m);

    for (int i = 0; i < N; i++) {
        if (rand() % 2 == 0) {
            mymemory_free(m, p[i]);
            p[i] = NULL;
        }
    }
    confere_lista(m);

    printf("depois de liberar metade:\n");
    mymemory_stats(m);

    gera_tamanhos(tam2, 2);
    int conseguiu = 0;
    for (int i = 0; i < N; i++) {
        p2[i] = mymemory_alloc(m, tam2[i] * 2);
        if (p2[i] != NULL) {
            memset(p2[i], 100 + i, tam2[i] * 2);
            conseguiu++;
        }
    }
    confere_lista(m);

    for (int i = 0; i < N; i++) {
        if (p[i] != NULL)
            testa(bloco_intacto(p[i], tam[i], i + 1), "bloco antigo foi sobrescrito");
        if (p2[i] != NULL)
            testa(bloco_intacto(p2[i], tam2[i] * 2, 100 + i), "bloco novo foi sobrescrito");
    }

    printf("segunda leva (dobro do tamanho): %d de %d alocados\n", conseguiu, N);
    mymemory_stats(m);

    mymemory_cleanup(m);
}

void casos_de_borda(void)
{
    printf("casos de borda\n");
    mymemory_t *m = mymemory_init(100);

    testa(mymemory_alloc(m, 0) == NULL, "alloc(0) deveria dar NULL");
    testa(mymemory_alloc(m, 101) == NULL, "alloc maior que o pool deveria dar NULL");

    void *a = mymemory_alloc(m, 40);
    void *b = mymemory_alloc(m, 40);
    testa(a != NULL && b != NULL, "dois blocos de 40 cabem em 100");
    testa(mymemory_alloc(m, 24) == NULL, "so sobram 20 bytes");

    int x;
    mymemory_free(m, &x);
    mymemory_free(m, (char *)a + 1);
    mymemory_free(m, NULL);
    testa(m->head != NULL && m->head->next != NULL, "free invalido mexeu na lista");

    mymemory_free(m, a);
    mymemory_free(m, a);
    testa(m->head != NULL && m->head->start == b && m->head->next == NULL, "free duplo");

    void *c = mymemory_alloc(m, 40);
    testa(c == a, "deveria reaproveitar o espaco liberado");

    mymemory_free(m, b);
    mymemory_free(m, c);
    testa(m->head == NULL, "lista deveria estar vazia");
    testa(mymemory_alloc(m, 100) != NULL, "pool inteiro deveria estar livre");

    mymemory_cleanup(m);
    testa(mymemory_init(0) == NULL, "init(0) deveria dar NULL");
}

void escolha_da_lacuna(void)
{
    printf("escolha da lacuna\n");
    size_t tam[7] = {200, 64, 300, 64, 100, 64, 136};

    for (int e = 0; e < 3; e++) {
        mymemory_t *m = mymemory_init(1000);
        mymemory_set_strategy(m, e);

        void *p[7];
        for (int i = 0; i < 7; i++)
            p[i] = mymemory_alloc(m, tam[i]);
        mymemory_free(m, p[0]);
        mymemory_free(m, p[2]);
        mymemory_free(m, p[4]);
        mymemory_free(m, p[6]);

        void *esperado = p[0];
        if (e == BEST_FIT)
            esperado = p[4];
        if (e == WORST_FIT)
            esperado = p[2];

        testa(mymemory_alloc(m, 80) == esperado, nome_estrategia[e]);
        mymemory_cleanup(m);
    }
}

int main(void)
{
    casos_de_borda();
    escolha_da_lacuna();

    for (int e = 0; e < 3; e++)
        for (int o = 0; o < 3; o++)
            cenario(e, o);

    if (erros == 0)
        printf("\nOK, todos os testes passaram\n");
    else
        printf("\n%d erros\n", erros);

    return erros != 0;
}
