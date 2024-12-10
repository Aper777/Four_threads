#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define SIZE 200
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_cond_t cond1;
pthread_cond_t cond2;

volatile int state = 0;
volatile int state1 = 0;

volatile int state2 = 0;
int flag = 0;

int key = 3;

void atbash(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (isupper(str[i]))
        {
            str[i] = 'Z' - (str[i] - 'A');
        }
        else if (islower(str[i]))
        {
            str[i] = 'z' - (str[i] - 'a');
        }
    }
}

void caesar_encrypt(char *text, int shift)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (isalpha(text[i]))
        {
            char base = (isupper(text[i])) ? 'A' : 'a';
            text[i] = (text[i] - base + shift) % 26 + base;
        }
    }
}
void caesar_decrypt(char *text, int shift)
{
    caesar_encrypt(text, 26 - shift);
}

void *work_thread1(void *arg)
{
    FILE *file1 = fopen("file1.txt", "r");
    if (file1 == NULL)
    {
        perror("file1 fopen failed");
        exit(EXIT_FAILURE);
    }
    FILE *file2 = fopen("file2.txt", "w+");
    if (file2 == NULL)
    {
        perror("file2 fopen failed");
        exit(EXIT_FAILURE);
    }
    char buff[SIZE] = {0};
    while (1)
    {
        while (fgets(buff, SIZE, file1))
        {
            pthread_mutex_lock(&mutex);
            while (state != 0)
            {
                pthread_cond_wait(&cond, &mutex);
            }
            printf("Thread1 start\n");
            atbash(buff);
            fprintf(file2, "%s", buff);
            fflush(file2);
            memset(buff, 0, strlen(buff));
            state = 1;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        if (feof(file1))
        {
            flag = 1;
            fclose(file1);
            fclose(file2);
            printf("thread1 finished\n");
            pthread_exit(NULL);
        }
    }

    return NULL;
}

void *work_thread2(void *arg)
{

    FILE *file3 = fopen("file3.txt", "w");
    if (file3 == NULL)
    {
        perror("file3 fopen failed");
        exit(EXIT_FAILURE);
    }
    FILE *file2 = fopen("file2.txt", "r");

    if (file2 == NULL)
    {
        perror("file2 fopen failed");
        exit(EXIT_FAILURE);
    }

    char buff[SIZE] = {0};
    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (state != 1)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        printf("Thread2 start\n");
        fgets(buff, SIZE, file2);
        caesar_encrypt(buff, key);

        state = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex1);

        while (state1 != 0)
        {
            pthread_cond_wait(&cond1, &mutex1);
        }

        
        fprintf(file3, "%s", buff);
        fflush(file3);
        memset(buff, 0, strlen(buff));
        state1 = 1;
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&mutex1);
        if (feof(file2) && flag == 1)
        {
            flag = 2;
            fclose(file2);
            fclose(file3);
            printf("thread2 finished\n");
            pthread_exit(NULL);
        }
    }

    return NULL;
}

void *work_thread3(void *arg)
{

    FILE *file4 = fopen("file4.txt", "w");
    if (file4 == NULL)
    {
        perror("file4 fopen failed");
        exit(EXIT_FAILURE);
    }
    FILE *file3 = fopen("file3.txt", "r");
    if (file3 == NULL)
    {
        perror("file3 fopen failed");
        exit(EXIT_FAILURE);
    }
    char buff[SIZE] = {0};
    while (1)
    {
        pthread_mutex_lock(&mutex1);
        while (state1 != 1)
        {
            printf("Thread3 lock\n");
            pthread_cond_wait(&cond1, &mutex1);
        }

        printf("thread3 start\n");
        fgets(buff, SIZE, file3);
        caesar_decrypt(buff, key);

        state1 = 0;
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&mutex1);

        pthread_mutex_lock(&mutex2);
        while (state2 != 0)
        {
            pthread_cond_broadcast(&cond2);
        }
        fprintf(file4, "%s", buff);
        fflush(file4);
        memset(buff, 0, strlen(buff));
        state2 = 1;
        
        pthread_cond_signal(&cond2);
        pthread_mutex_unlock(&mutex2);
        if (feof(file3) && flag == 2)
        {
            flag = 3;
            fclose(file3);
            fclose(file4);
            printf("thread3 finished\n");
            pthread_exit(NULL);
        }
    }

    return NULL;
}

void *work_thread4(void *arg)
{

    FILE *file5 = fopen("file5.txt", "w+");
    if (file5 == NULL)
    {
        perror("file5 fopen failed");
        exit(EXIT_FAILURE);
    }
    FILE *file4 = fopen("file4.txt", "w+");
    if (file4 == NULL)
    {
        perror("file4 fopen failed");
        exit(EXIT_FAILURE);
    }
    char buff[SIZE] = {0};
    while (1)
    {
        pthread_mutex_lock(&mutex2);
        while (state2 != 1)
        {
            printf("Thread4 lock\n");
            pthread_cond_wait(&cond2, &mutex2);
        }

        printf("Thread4 start\n");
        fgets(buff, SIZE, file4);
        atbash(buff);
        fprintf(file5, "%s", buff);
        fflush(file5);
        memset(buff, 0, strlen(buff));
        state2 = 0;
        pthread_cond_signal(&cond2);
        pthread_mutex_unlock(&mutex2);
        if (feof(file4) && flag == 3)
        {
            
            fclose(file4);
            fclose(file5);
            printf("thread4 finished\n");
            pthread_exit(NULL);
        }
    }

    return NULL;
}

int main()
{

    pthread_t thr[4];
    pthread_create(&thr[0], NULL, work_thread1, NULL);
    pthread_create(&thr[1], NULL, work_thread2, NULL);
    pthread_create(&thr[2], NULL, work_thread3, NULL);
    pthread_create(&thr[3], NULL, work_thread4, NULL);

    for (int i = 0; i < 4; i++)
    {
        if (pthread_join(thr[i], NULL) != 0)
        {
            perror("pthread_join failed");
            exit(EXIT_FAILURE);
        }
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    return 0;
}