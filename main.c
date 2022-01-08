 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <openssl/evp.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <signal.h>

#define SIZE_MD5 35
#define L_HASEL_MD5 420
#define L_SLOW_SLOWNIK 550
#define DL_SLOWA 32
#define NUM_THREADS  4
#define TCOUNT 10
#define COUNT_LIMIT 8


struct dane_wejsciowe
{
    char tab_MD5_wej[L_HASEL_MD5][SIZE_MD5];
    char* tab_slow_wej[L_SLOW_SLOWNIK];

};

struct dane_wyjsciowe
{
    char tab_MD5_wyj[100][33];
    char tab_slow_wyj[100][64];
};

int count=0;
int licznik=0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;
pthread_t threads[4];
pthread_attr_t attr;
struct dane_wyjsciowe dane_wyj;
struct dane_wejsciowe dane_wej;

void sighandler(int signum);
void *inc_count(void *t);
void *watch_count(void *t);
char *wczytaj(char *z, int ile, FILE *plik);
void *watek_1(void *dane);
void *watek_1(void *dane);
void *watek_2(void *dane);




//############## MAIN ############## 

 int main(int argc, char *argv[])
 {

     //############## ZMIENNE ############## 

     FILE *slownik;
     FILE *hasla_MD5;

     char tab_pom[DL_SLOWA];
     char tab_pom2[SIZE_MD5];
     char *tmp_wsk;
     char *tmp_wsk2;
     char *znak="\0";

     
    


     int i, rc; 
     long t1=1, t2=2, t3=3;
     


    //######################################

    signal(SIGHUP, sighandler);

    //############## SPRAWDZENIE LICZBY ARGUMENTOW ############## 
     if(argc!=3)
     {
         puts("Podano niewlasciwa liczbe argumentow.");
         puts("Poprawna liczba to dwa argumenty.");
     }
    //######################################



    //############## PROBA OTWARCIA PLIKOW SLOWNIK I KODY MD5
     if((slownik=fopen(argv[1],"r"))==NULL)
     {
        printf("Nie mozna otworzyc pliku %s\n",argv[1]);
        exit(EXIT_FAILURE);
     }

     if((hasla_MD5=fopen(argv[2],"r"))==NULL)
     {
        printf("Nie mozna otworzyc pliku %s\n",argv[2]);
        exit(EXIT_FAILURE);
     }
    //######################################

 

    //############## POBIERANIE LISTY HASEL MD5 ############## 
      for (int index = 0; index < L_HASEL_MD5; index++) 
    {
        tmp_wsk=fgets (tab_pom2, SIZE_MD5, hasla_MD5);   // czytamy ze standardowego wejścia
        
        tmp_wsk2=tab_pom2;
        strncpy(dane_wej.tab_MD5_wej[index],tmp_wsk2,SIZE_MD5);
        strcpy(dane_wej.tab_MD5_wej[index]+32,znak);

       // strncpy(dane_wej.tab_MD5_wej[index],tab_hasla_MD5[index],SIZE_MD5);//############################

        if (tmp_wsk != NULL) 
        {      
            if (feof (hasla_MD5))
                printf ("\n%d koniec pliku\n", index);
        }
        else
            printf ("\n%d blad odczytu\n", index);
    }
    //######################################



    //############## POBIERANIE LISTY SLOW ZE SLOWNIKA ############## 
      for (int index = 0; index < L_SLOW_SLOWNIK ; index++) 
    {
        tmp_wsk=fgets (tab_pom,DL_SLOWA,slownik);   // czytamy ze standardowego wejścia
        dane_wej.tab_slow_wej[index]=(char *)calloc(strlen(tab_pom),sizeof(char));

        dane_wej.tab_slow_wej[index]=(char *)calloc(strlen(tab_pom),sizeof(char));

        wczytaj(tab_pom,DL_SLOWA,slownik);
        strncpy(dane_wej.tab_slow_wej[index],tab_pom,strlen(tab_pom));
        
        if (tmp_wsk != NULL) 
        {      
            if (feof (slownik))
                printf ("\n%d koniec pliku\n", index);
        }
        else
            printf ("\n%d blad odczytu\n", index);
    }
    //######################################



    //############## WYPISYWANIE LISTY SLOW ZE SLOWNIKA I HASEL MD5 PO 10 ZEBY NIE BYLO ZA DUZO ############## 
    for(int index=L_HASEL_MD5-10; index<L_HASEL_MD5; index++)
    {
        printf("Kod nr:%d wyglada nastepujaco: %s, dlugosc tego slowa to %ld. \n",index,dane_wej.tab_MD5_wej[index], strlen(dane_wej.tab_MD5_wej[index]));
    }

      for(int index=L_SLOW_SLOWNIK-10; index<L_SLOW_SLOWNIK; index++)
    {
        printf("Slowo nr:%d wyglada nastepujaco: %s, dlugosc tego slowa to %ld. \n",index,dane_wej.tab_slow_wej[index], strlen(dane_wej.tab_slow_wej[index]));
    }
    //######################################
    

      /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init (&count_threshold_cv, NULL);

       /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, watch_count, (void *)t1);
    pthread_create(&threads[1], &attr, inc_count, (void *)t2);
    pthread_create(&threads[2], &attr, watek_1, (void *) &dane_wej);
    pthread_create(&threads[3], &attr, watek_2, (void *) &dane_wej); 



    /* Wait for all threads to complete */
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf ("Main(): Waited and joined with %d threads. Final value of count = %d. Done.\n", 
          NUM_THREADS, count);


    

    for(int i=0; i<licznik; i++)
    {
        printf("Wartosc licznika to: %d, slowo to: %s, a kod to: %s\n", i, dane_wyj.tab_slow_wyj[i],dane_wyj.tab_MD5_wyj[i] );
    }

    sleep(20000);


    //############## CZYSZCZENIE PAMIECI I ZAMYKANIE PLIKOW ############## 
    fclose(slownik);
    fclose(hasla_MD5);

    
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_exit (NULL);

    

    for(int i=0; i<L_SLOW_SLOWNIK;i++)
    {
       //free(tab_slownik[i]);
       free(dane_wej.tab_slow_wej[i]); 
    }
    //######################################

    return 0;
 }
 //######################################################  



//############## WCZYTYWANIE DANYCH Z POMINIECIEM ZNAKU NOWEJ LINII ############## 
 char *wczytaj(char *z, int ile, FILE *plik)
 {
     char *wynik;
     int i=0;

     wynik=fgets(z,ile, plik);

     if(wynik)
     {
         while(z[i]!= '\n' && z[i]!= '\0')
            i++;
        if(z[i]=='\n')
            z[i]='\0';
        else
            while(getchar()!='\n')
                continue;
     }
 }
//######################################



 void *inc_count(void *t) 
{
  int i;
  long my_id = (long)t;

  for (i=0; i < TCOUNT; i++) {
    pthread_mutex_lock(&count_mutex);
    count++;

    /* 
    Check the value of count and signal waiting thread when condition is
    reached.  Note that this occurs while mutex is locked. 
    */
    if (count == COUNT_LIMIT) {
      printf("inc_count(): thread %ld, count = %d  Threshold reached. ",
             my_id, count);
      pthread_cond_signal(&count_threshold_cv);
      printf("Just sent signal.\n");
      }
    printf("inc_count(): thread %ld, count = %d, unlocking mutex\n", 
	   my_id, count);
    pthread_mutex_unlock(&count_mutex);

    /* Do some work so threads can alternate on mutex lock */
    sleep(1);
    }
  pthread_exit(NULL);
}
 
 


 void *watch_count(void *t) 
{
  long my_id = (long)t;

  printf("Starting watch_count(): thread %ld\n", my_id);

  /*
  Lock mutex and wait for signal.  Note that the pthread_cond_wait routine
  will automatically and atomically unlock mutex while it waits. 
  Also, note that if COUNT_LIMIT is reached before this routine is run by
  the waiting thread, the loop will be skipped to prevent pthread_cond_wait
  from never returning.
  */
  pthread_mutex_lock(&count_mutex);
  while (count < COUNT_LIMIT) {
    printf("watch_count(): thread %ld Count= %d. Going into wait...\n", my_id,count);
    pthread_cond_wait(&count_threshold_cv, &count_mutex);
    printf("watch_count(): thread %ld Condition signal received. Count= %d\n", my_id,count);
    }
  printf("watch_count(): thread %ld Updating the value of count...\n", my_id);
  count += 125;
  printf("watch_count(): thread %ld count now = %d.\n", my_id, count);
  printf("watch_count(): thread %ld Unlocking mutex.\n", my_id);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}


void *watek_1(void * dane)
{
    int i;
    struct dane_wejsciowe* info = (struct dane_wejsciowe*)dane;

    for (i=0; i < TCOUNT; i++) 
    {
        pthread_mutex_lock(&count_mutex);

        puts("Watek pierwszy.");
         for(int index=L_HASEL_MD5-2; index<L_HASEL_MD5; index++)
        {
            printf("Kod nr:%d wyglada nastepujaco: %s, dlugosc tego slowa to %ld. \n",index,(* info).tab_MD5_wej[index], strlen((*info).tab_MD5_wej[index]));
        }
        
        strncpy(dane_wyj.tab_MD5_wyj[licznik],(* info).tab_MD5_wej[licznik],SIZE_MD5);
        licznik++;

        pthread_mutex_unlock(&count_mutex);

        /* Do some work so threads can alternate on mutex lock */
        sleep(1);
    }
    
    pthread_exit(NULL);
}




void *watek_2(void * dane)
{
    int i;
    struct dane_wejsciowe* info = (struct dane_wejsciowe*)dane;

    for (i=0; i < TCOUNT; i++) 
    {
        pthread_mutex_lock(&count_mutex);

        puts("Watek drugi.");
       

        for(int index=L_SLOW_SLOWNIK-3; index<L_SLOW_SLOWNIK; index++)
        {
             printf("Slowo nr:%d wyglada nastepujaco: %s, dlugosc tego slowa to %ld. \n",index,(* info).tab_slow_wej[index], strlen((*info).tab_slow_wej[index]));
        }

        strncpy(dane_wyj.tab_slow_wyj[licznik],(* info).tab_slow_wej[licznik],strlen((*info).tab_slow_wej[licznik]));
        licznik++;

        pthread_mutex_unlock(&count_mutex);

        /* Do some work so threads can alternate on mutex lock */
        sleep(1);
    }
    
    pthread_exit(NULL);
}
 



void sighandler(int signum) 
{
   printf("Przechwycono sygnal %d, konczenie pracy.\n", signum);

    for(int i=0; i<licznik; i++)
    {
        printf("Wartosc licznika to: %d, slowo to: %s, a kod to: %s\n", i, dane_wyj.tab_slow_wyj[i],dane_wyj.tab_MD5_wyj[i] );
    }
    
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_exit (NULL);

    

    for(int i=0; i<L_SLOW_SLOWNIK;i++)
    {
       free(dane_wej.tab_slow_wej[i]); 
    }
   exit(1);
}
 /*#include <stdio.h>
 #include <string.h>
 #include <openssl/evp.h>

 int main(int argc, char *argv[])
 {
     EVP_MD_CTX *mdctx; //tworzenie struktury ktora zawiera rodzaj kodowania
     const EVP_MD *md;  //przechowuje skrot kodowania MD5 MD4 i to co zawiera openssl
     char mess1[] = "Test Message\n";
     char mess2[] = "Hello World\n";
     unsigned char md_value[EVP_MAX_MD_SIZE];
     unsigned int md_len, i;

     if (argv[1] == NULL) {
         printf("Usage: mdtest digestname\n");
         exit(1);
     }

     md = EVP_get_digestbyname(argv[1]);
     if (md == NULL) {
         printf("Unknown message digest %s\n", argv[1]);
         exit(1);
     }

     mdctx = EVP_MD_CTX_new(); 
     //alokacja i zwracanie kontekstu skrotu,
     //czyli podajemy jak ma zostac zakodowane wszystko MD5 lub MD4
     //te argumenty podajemy podczas uruchomienia programu.

     EVP_DigestInit_ex(mdctx, md, NULL);//ustawia kontekst kodowania w strukturze mdctx, na MD5 lub inne dostepne w openssl
     EVP_DigestUpdate(mdctx, mess2, strlen(mess1));// tutaj odbywa się kodowanie naszego tekstu do wybranego klucza (MD5)
     //EVP_DigestUpdate(mdctx, mess1, strlen(mess2));
     EVP_DigestFinal_ex(mdctx, md_value, &md_len);//pobiera wartosc mdctx i zapisuje do wartosci md_value, md_len tutaj zostaje zapisana liczba bajtow,
     //ktore zostaly zapisane 
     EVP_MD_CTX_free(mdctx);//zwalnia pamiec zajmowana przez strukture mdctx

     printf("Digest is: ");
     for (i = 0; i < md_len; i++)
         printf("%02X", md_value[i]);// wyswietlanie liczb szesnastkowych małymi literami 
     printf("\n");

     exit(0);
 }

 */