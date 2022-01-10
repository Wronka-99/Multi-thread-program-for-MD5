 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <openssl/evp.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <signal.h>

#define SIZE_MD5 35
#define L_HASEL_MD5 420
#define L_SLOW_SLOWNIK 10000
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

int praca=1;
int count=0;
int licznik=0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;
pthread_t threads[4];
pthread_attr_t attr;
struct dane_wyjsciowe dane_wyj;
struct dane_wejsciowe dane_wej;

void sighandler(int signum);
void *watek_1(void *dane);
void *watek_2(void *dane);
void *konsument(void *dane);



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
        strncpy(dane_wej.tab_MD5_wej[index],tab_pom2,SIZE_MD5);
        strcpy(dane_wej.tab_MD5_wej[index]+32,znak);

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
        strncpy(dane_wej.tab_slow_wej[index],tab_pom,strlen(tab_pom)-1);
        //strcpy(dane_wej.tab_MD5_wej[index]+strlen(tab_pom)-2,znak);

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
    for(int index=0; index<10; index++)
    {
        printf("Kod nr:%d wyglada nastepujaco: %s, dlugosc tego kodu to %ld, slowo wyglada nastepujaco: %s, dlugosc tego slowa to %ld\n",
        index,dane_wej.tab_MD5_wej[index], strlen(dane_wej.tab_MD5_wej[index]),dane_wej.tab_slow_wej[index], strlen(dane_wej.tab_slow_wej[index]));
    }


    //######################################
    

      /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init (&count_threshold_cv, NULL);

       /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[2], &attr, konsument, (void *) &dane_wyj);
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

    //sleep(20000);


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


 


void *watek_1(void * dane)
{
        
    struct dane_wejsciowe* info = (struct dane_wejsciowe*)dane;


    EVP_MD_CTX *mdctx; //tworzenie struktury ktora zawiera rodzaj kodowania
    const EVP_MD *md;  //przechowuje skrot kodowania MD5 MD4 i to co zawiera openssl
    unsigned char md_value[32];
    unsigned int md_len;
    unsigned int index=0;
    char znak_pom;
    char tab_pom[3];
    char tab_pom2[33];
    int sprawdzenie=1;

    md = EVP_get_digestbyname("MD5");
    
     
    while(praca)
    {
        puts("Watek pierwszy.");
        
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        {
            mdctx = EVP_MD_CTX_new(); 
       
            puts("Watek pierwszy. 1");
            EVP_DigestInit_ex(mdctx, md, NULL);//ustawia kontekst kodowania w strukturze mdctx, na MD5 lub inne dostepne w openssl
            EVP_DigestUpdate(mdctx, (* info).tab_slow_wej[a], strlen((* info).tab_slow_wej[a]));// tutaj odbywa się kodowanie naszego tekstu do wybranego klucza (MD5)
            EVP_DigestFinal_ex(mdctx, md_value, &md_len);//pobiera wartosc mdctx i zapisuje do wartosci md_value, md_len tutaj zostaje zapisana liczba bajtow, ktore zostaly zapisane 
            EVP_MD_CTX_free(mdctx);//zwalnia pamiec zajmowana przez strukture mdctx

            for (int i = 0; i < md_len; i++)
            {
                znak_pom=(char)md_value[i];
                sprintf(tab_pom,"%02X",znak_pom & 0xff);
                strcpy(tab_pom2+2*i,tab_pom);
            }
            puts("Watek pierwszy 3.");
            //printf("Slowo: %s , kod: %s\n", (* info).tab_slow_wej[a],tab_pom2);

            for(int b=0; b<L_HASEL_MD5; b++)
            {
                sprawdzenie=strncmp(tab_pom2, (* info).tab_MD5_wej[b],32);
                
                printf("Slowo: %s, kod ze slownika: %s, wygenerowany kod %s. \n",(*info).tab_slow_wej[a],(* info).tab_MD5_wej[b],tab_pom2);
                if(sprawdzenie==0)
                {
                    
                    pthread_mutex_lock(&count_mutex);  
                    strncpy(dane_wyj.tab_slow_wyj[licznik],(* info).tab_slow_wej[a],strlen((* info).tab_slow_wej[a]));
                    strncpy(dane_wyj.tab_MD5_wyj[licznik],tab_pom2, strlen(tab_pom2));

                    printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[licznik], dane_wyj.tab_MD5_wyj[licznik]);  

                    pthread_cond_signal(&count_threshold_cv);
                    pthread_mutex_unlock(&count_mutex);
                    

                    
                }
            }
        }

    
       // strncpy(dane_wyj.tab_MD5_wyj[licznik],tab_pom2, strlen(tab_pom2));
       // printf(" Wartosci MD5 po konwersji do wyjsciowych danych: %s \n", dane_wyj.tab_MD5_wyj[licznik]);
       // printf("\n");
       // strncpy(dane_wyj.tab_slow_wyj[licznik],(* info).tab_slow_wej[licznik],strlen((* info).tab_slow_wej[index]));

        sleep(1);
    }
    pthread_exit(NULL);
}




void *watek_2(void * dane)
{
    
    struct dane_wejsciowe* info = (struct dane_wejsciowe*)dane;


    EVP_MD_CTX *mdctx; //tworzenie struktury ktora zawiera rodzaj kodowania
    const EVP_MD *md;  //przechowuje skrot kodowania MD5 MD4 i to co zawiera openssl
    unsigned char md_value[32];
    unsigned int md_len;
    unsigned int index=0;
    char znak_pom;
    char tab_pom[3];
    char tab_pom2[33];
    int sprawdzenie=1;

    md = EVP_get_digestbyname("MD5");
    
     
    while(praca)
    {
        puts("Watek drugi.");
        
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        {
            
            mdctx = EVP_MD_CTX_new(); 
       
            EVP_DigestInit_ex(mdctx, md, NULL);//ustawia kontekst kodowania w strukturze mdctx, na MD5 lub inne dostepne w openssl
            EVP_DigestUpdate(mdctx, (* info).tab_slow_wej[a], strlen((* info).tab_slow_wej[a]));// tutaj odbywa się kodowanie naszego tekstu do wybranego klucza (MD5)
            EVP_DigestFinal_ex(mdctx, md_value, &md_len);//pobiera wartosc mdctx i zapisuje do wartosci md_value, md_len tutaj zostaje zapisana liczba bajtow, ktore zostaly zapisane 
            EVP_MD_CTX_free(mdctx);//zwalnia pamiec zajmowana przez strukture mdctx

            for (int i = 0; i < md_len; i++)
            {
                znak_pom=(char)md_value[i];
                sprintf(tab_pom,"%02X",znak_pom & 0xff);
                strcpy(tab_pom2+2*i,tab_pom);
            }
            
            //printf("Slowo: %s , kod: %s\n", (* info).tab_slow_wej[a],tab_pom2);

            for(int b=0; b<L_HASEL_MD5; b++)
            {
                sprawdzenie=strncmp(tab_pom2, (* info).tab_MD5_wej[b],32);
                //printf("Slowo: %s, kod ze slownika: %s, wygenerowany kod %s. \n",(*info).tab_slow_wej[a],(* info).tab_MD5_wej[b],tab_pom2);
                if(sprawdzenie==0)
                {
                    
                    pthread_mutex_lock(&count_mutex);  
                    strncpy(dane_wyj.tab_slow_wyj[licznik],(* info).tab_slow_wej[a],strlen((* info).tab_slow_wej[a]));
                    strncpy(dane_wyj.tab_MD5_wyj[licznik],tab_pom2, strlen(tab_pom2));

                    printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[licznik], dane_wyj.tab_MD5_wyj[licznik]);  

                    pthread_cond_signal(&count_threshold_cv);
                    pthread_mutex_unlock(&count_mutex);
                    

                    
                }
            }
        }

    
       // strncpy(dane_wyj.tab_MD5_wyj[licznik],tab_pom2, strlen(tab_pom2));
       // printf(" Wartosci MD5 po konwersji do wyjsciowych danych: %s \n", dane_wyj.tab_MD5_wyj[licznik]);
       // printf("\n");
       // strncpy(dane_wyj.tab_slow_wyj[licznik],(* info).tab_slow_wej[licznik],strlen((* info).tab_slow_wej[index]));

        sleep(1);
    }
    pthread_exit(NULL);
}
 


 void *konsument(void *dane) 
{
  struct dane_wyjsciowe* info = (struct dane_wyjsciowe*)dane;

  

  /*
  Lock mutex and wait for signal.  Note that the pthread_cond_wait routine
  will automatically and atomically unlock mutex while it waits. 
  Also, note that if COUNT_LIMIT is reached before this routine is run by
  the waiting thread, the loop will be skipped to prevent pthread_cond_wait
  from never returning.*/
  
  pthread_mutex_lock(&count_mutex);

  while (praca)
   {
    printf("Konsument czeka na odebranie znalezionego hasla.\n");
    pthread_cond_wait(&count_threshold_cv, &count_mutex);
    printf("Znalezionym haslem nr. %d jest: %s .\n",licznik,dane_wyj.tab_slow_wyj[licznik]);
    
    memset(dane_wej.tab_slow_wej[licznik], 0, strlen(dane_wej.tab_slow_wej[licznik]));
    licznik++;
    
    //free(dane_wyj.tab_slow_wyj[licznik]);
    }

  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
  
}



void sighandler(int signum) 
{
   printf("Przechwycono sygnal %d, konczenie pracy.\n", signum);
   praca=0;

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
