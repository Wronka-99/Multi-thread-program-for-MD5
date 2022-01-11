 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <openssl/evp.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <signal.h>
 #include <time.h>
 #include <ctype.h>

#define SIZE_MD5 35
#define L_HASEL_MD5 441
#define L_SLOW_SLOWNIK 1000
#define DL_SLOWA 32
#define NUM_THREADS  6


struct dane_wejsciowe
{
    char tab_MD5_wej[L_HASEL_MD5][SIZE_MD5];
    char* tab_slow_wej[L_SLOW_SLOWNIK];

}dane_wej;

struct dane_wyjsciowe
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready;
    int licznik;
    char tab_MD5_wyj[100][33];
    char tab_slow_wyj[100][64];
} dane_wyj = {PTHREAD_MUTEX_INITIALIZER,
            PTHREAD_COND_INITIALIZER};

int praca=1;
int count=0;
char tab_prawdy[L_SLOW_SLOWNIK]={0};
pthread_mutex_t mutex_1;
pthread_t threads[NUM_THREADS];

void sighandler(int signum);
void koduj_MD5(char *slowo_pobrane, char kod[33]);
void *watek_1();
void *watek_2();
void *watek_3();
void *watek_4();
void *watek_5();
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
     dane_wyj.licznik=0;
    //######################################

    signal(SIGHUP, sighandler);//FUNKCJA OBSLUGUJACA SYGNAL

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
        tmp_wsk=fgets (tab_pom,DL_SLOWA,slownik);   //ODCZYT Z PLIKU SLOWNIK DO TAB_POM
        dane_wej.tab_slow_wej[index]=(char *)calloc(strlen(tab_pom),sizeof(char)); //PRZYPISANIE ODPOWIEDNIEJ ILOSCI PAMIECI
        strncpy(dane_wej.tab_slow_wej[index],tab_pom,strlen(tab_pom)-1);// PRZYPISANIE SLOWA Z PLIKU DO DANYCH WEJSCIOWYCH

        if (tmp_wsk != NULL) 
        {      
            if (feof (slownik))
                printf ("\n%d koniec pliku\n", index);
        }
        else
            printf ("\n%d blad odczytu\n", index);
    }
    //######################################


    //######################################
    

      /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&dane_wyj.mutex, NULL);
    pthread_mutex_init(&mutex_1, NULL);
    pthread_cond_init (&dane_wyj.cond, NULL);

       /* For portability, explicitly create threads in a joinable state */
    pthread_create(&threads[0], NULL, konsument, (void *) &dane_wyj); 
    pthread_create(&threads[1], NULL, watek_1, NULL);
    pthread_create(&threads[2], NULL, watek_2, NULL); 
    pthread_create(&threads[3], NULL, watek_3, NULL); 
    pthread_create(&threads[4], NULL, watek_4, NULL);
    pthread_create(&threads[5], NULL, watek_5, NULL); 

    //############## CZYSZCZENIE PAMIECI I ZAMYKANIE PLIKOW ############## 
    fclose(slownik);
    fclose(hasla_MD5);

    //pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutex_1);
    pthread_mutex_destroy(&dane_wyj.mutex);
    pthread_cond_destroy(&dane_wyj.cond);
    pthread_exit (NULL);


    for(int i=0; i<L_SLOW_SLOWNIK;i++)
    {
       free(dane_wej.tab_slow_wej[i]); 
    }
    //######################################

    return 0;
 }
 //######################################################  


 void koduj_MD5(char *slowo_pobrane, char kod[33])
 {
    EVP_MD_CTX *mdctx; //tworzenie struktury ktora zawiera rodzaj kodowania
    const EVP_MD *md;  //przechowuje skrot kodowania MD5 MD4 i to co zawiera openssl
    unsigned char md_value[32];
    unsigned int md_len;
    char znak_pom;
    char tab_pom[3];
    md = EVP_get_digestbyname("MD5");

     mdctx = EVP_MD_CTX_new(); 
    EVP_DigestInit_ex(mdctx, md, NULL);//ustawia kontekst kodowania w strukturze mdctx, na MD5 lub inne dostepne w openssl
    EVP_DigestUpdate(mdctx, slowo_pobrane, strlen(slowo_pobrane));// tutaj odbywa się kodowanie naszego tekstu do wybranego klucza (MD5)
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);//pobiera wartosc mdctx i zapisuje do wartosci md_value, md_len tutaj zostaje zapisana liczba bajtow, ktore zostaly zapisane 
    EVP_MD_CTX_free(mdctx);//zwalnia pamiec zajmowana przez strukture mdctx

    for (int i = 0; i < md_len; i++)
    {
        znak_pom=(char)md_value[i];
        sprintf(kod+2*i,"%02X",znak_pom & 0xff);
    }
 }


void *watek_1()
{

    char znak_pom;
    char tab_pom[3];
    char kod_md5[33];
    char *nowe_slowo;
    int sprawdzenie=1;

    
    
    while(praca)
    {
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        { 
            for(int c=0; c<1000; c++)
            {
                if(c<10)
                    nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+2, sizeof(char));
                else if(c<100)
                    nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+3, sizeof(char));
                else
                    nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+4, sizeof(char));

                sprintf(nowe_slowo,"%d",c);
                strcat(nowe_slowo, dane_wej.tab_slow_wej[a]);

                koduj_MD5(nowe_slowo, kod_md5);
                
                //printf("Wygenerowane slowo %s, kod to: %s .\n",nowe_slowo,kod_md5);

                for(int b=0; b<L_HASEL_MD5; b++)
                {
                    
                    sprawdzenie=strncmp(kod_md5, dane_wej.tab_MD5_wej[b],32);//ZWRACA 0 JESLI LANCUCHY SA TAKIE SAME

                     if(sprawdzenie==0&&tab_prawdy[a]==0)
                    {   
                        pthread_mutex_lock(&dane_wyj.mutex); //POCZATEK SEKCJI KRYTYCZNEJ
                        strncpy(dane_wyj.tab_slow_wyj[dane_wyj.licznik],nowe_slowo,strlen(nowe_slowo));
                        strncpy(dane_wyj.tab_MD5_wyj[dane_wyj.licznik],kod_md5, strlen(kod_md5));
                        //printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[dane_wyj.licznik], dane_wyj.tab_MD5_wyj[dane_wyj.licznik]); 
                        puts("Watek pierwszy wykryl slowo.");
                        dane_wyj.nready++;
                        
                        tab_prawdy[a]=1;
                        
                        
                        pthread_cond_signal(&dane_wyj.cond);
                        pthread_mutex_unlock(&dane_wyj.mutex);//KONIEC SEKCJI KRYTYCZNEJ
                        sleep(1);
                        break;
                    }  
                }
                free(nowe_slowo);
            }
        }
    }
    pthread_exit(NULL);       
}




void *watek_2()
{
   
    char znak_pom;
    char tab_pom[3];
    char kod_md5[33];
    int sprawdzenie=1;
    char *nowe_slowo;
    
    
     
    while(praca)
    {
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        {   
            
                nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+3, sizeof(char));

                
                strcpy(nowe_slowo, dane_wej.tab_slow_wej[a]);

                koduj_MD5(nowe_slowo, kod_md5);
            
            for(int b=0; b<L_HASEL_MD5; b++)
            {
                 
                sprawdzenie=strncmp(kod_md5, dane_wej.tab_MD5_wej[b],32);

               if(sprawdzenie==0&&tab_prawdy[a]==0)
                {   
                        pthread_mutex_lock(&dane_wyj.mutex); //POCZATEK SEKCJI KRYTYCZNEJ
                        strncpy(dane_wyj.tab_slow_wyj[dane_wyj.licznik],nowe_slowo,strlen(nowe_slowo));
                        strncpy(dane_wyj.tab_MD5_wyj[dane_wyj.licznik],kod_md5, strlen(kod_md5));
                        //printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[dane_wyj.licznik], dane_wyj.tab_MD5_wyj[dane_wyj.licznik]); 
                        puts("Watek drugi wykryl slowo.");
                        dane_wyj.nready++;
                        
                        tab_prawdy[a]=1;
                        
                        
                        pthread_cond_signal(&dane_wyj.cond);
                        pthread_mutex_unlock(&dane_wyj.mutex);//KONIEC SEKCJI KRYTYCZNEJ
                        sleep(1);
                        break;
                }
                
            }
        }
    }
    pthread_exit(NULL);
}
 
void *watek_3()
{

    char znak_pom;
    char tab_pom[4];
    char kod_md5[33];
    char *nowe_slowo;
    int sprawdzenie=1;

    
    while(praca)
    {  
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        { 
            for(int c=0; c<1000; c++)
            {
                if(c<10)
                    nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+2, sizeof(char));
                else if(c<100)
                    nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+3, sizeof(char));
                else
                    nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+4, sizeof(char));

                sprintf(nowe_slowo,"%s",dane_wej.tab_slow_wej[a]);
                
                sprintf(tab_pom,"%d",c);
                strcat(nowe_slowo,tab_pom);

               
                koduj_MD5(nowe_slowo, kod_md5);
                //printf("Wygenerowane slowo %s, kod to: %s .\n",nowe_slowo,kod_md5);

                for(int b=0; b<L_HASEL_MD5; b++)
                {
                    
                    sprawdzenie=strncmp(kod_md5, dane_wej.tab_MD5_wej[b],32);//ZWRACA 0 JESLI LANCUCHY SA TAKIE SAME

                     if(sprawdzenie==0&&tab_prawdy[a]==0)
                    {   
                        pthread_mutex_lock(&dane_wyj.mutex); //POCZATEK SEKCJI KRYTYCZNEJ
                        strncpy(dane_wyj.tab_slow_wyj[dane_wyj.licznik],nowe_slowo,strlen(nowe_slowo));
                        strncpy(dane_wyj.tab_MD5_wyj[dane_wyj.licznik],kod_md5, strlen(kod_md5));
                        //printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[dane_wyj.licznik], dane_wyj.tab_MD5_wyj[dane_wyj.licznik]); 
                        
                        dane_wyj.nready++;
                        
                        tab_prawdy[a]=1;
                        puts("Watek trzeci wykryl slowo.");
                        
                        pthread_cond_signal(&dane_wyj.cond);
                        pthread_mutex_unlock(&dane_wyj.mutex);//KONIEC SEKCJI KRYTYCZNEJ
                        sleep(1);
                        break;
                    }  
                }
                free(nowe_slowo);
            }
        }
    }
    pthread_exit(NULL);       
}


void *watek_4()
{

    char znak_pom;
    char tab_pom[4];
    char kod_md5[33];
    char *nowe_slowo;
    int sprawdzenie=1;

    
    while(praca)
    {  
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        { 
                nowe_slowo=(char*)calloc(strlen(dane_wej.tab_slow_wej[a])+1, sizeof(char));
                sprintf(nowe_slowo,"%s",dane_wej.tab_slow_wej[a]);
                
                strcat(nowe_slowo,tab_pom);

                for(int c=0; c<strlen(nowe_slowo); c++)
                {  
                    nowe_slowo[c]=toupper(nowe_slowo[c]);
                }

               
                koduj_MD5(nowe_slowo, kod_md5);
                //printf("Wygenerowane slowo %s, kod to: %s .\n",nowe_slowo,kod_md5);
                
                for(int b=0; b<L_HASEL_MD5; b++)
                {
                    
                    sprawdzenie=strncmp(kod_md5, dane_wej.tab_MD5_wej[b],32);//ZWRACA 0 JESLI LANCUCHY SA TAKIE SAME

                     if(sprawdzenie==0&&tab_prawdy[a]==0)
                    {   
                        pthread_mutex_lock(&dane_wyj.mutex); //POCZATEK SEKCJI KRYTYCZNEJ
                        strncpy(dane_wyj.tab_slow_wyj[dane_wyj.licznik],nowe_slowo,strlen(nowe_slowo));
                        strncpy(dane_wyj.tab_MD5_wyj[dane_wyj.licznik],kod_md5, strlen(kod_md5));
                        //printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[dane_wyj.licznik], dane_wyj.tab_MD5_wyj[dane_wyj.licznik]); 
                        
                        dane_wyj.nready++;
                        
                        tab_prawdy[a]=1;
                        puts("Watek czwarty wykryl slowo.");
                        
                        pthread_cond_signal(&dane_wyj.cond);
                        pthread_mutex_unlock(&dane_wyj.mutex);//KONIEC SEKCJI KRYTYCZNEJ
                        sleep(1);
                        break;
                    }  
                }
                free(nowe_slowo);
            
        }
    }
    pthread_exit(NULL);       
}




void *watek_5()
{

    char znak_pom;
    char tab_pom[3];
    char kod_md5[33];
    char *nowe_slowo;
    int sprawdzenie=1;
    int dl_slowa;
    
    while(praca)
    {
        for(int a=0; a<L_SLOW_SLOWNIK; a++)
        { 
            for(int c=0; c<1000; c++)
            {
                for(int d=0; d<L_SLOW_SLOWNIK;d++)
                {
                    if(c<10)
                        dl_slowa=strlen(dane_wej.tab_slow_wej[a])+2+strlen(dane_wej.tab_slow_wej[d]);
                    else if(c<100)
                        dl_slowa=strlen(dane_wej.tab_slow_wej[a])+3+strlen(dane_wej.tab_slow_wej[d]);
                    else
                        dl_slowa=strlen(dane_wej.tab_slow_wej[a])+4+strlen(dane_wej.tab_slow_wej[d]);

                    nowe_slowo=(char*)calloc(dl_slowa, sizeof(char));


                    strcat(nowe_slowo,dane_wej.tab_slow_wej[a]);
                
                    sprintf(tab_pom,"%d",c);
                    strcat(nowe_slowo,tab_pom);
                    strcat(nowe_slowo,dane_wej.tab_slow_wej[d]);

                    koduj_MD5(nowe_slowo, kod_md5);
                    
                    //printf("Wygenerowane slowo %s, kod to: %s .\n",nowe_slowo,kod_md5);
                    //sleep(1);
                    for(int b=0; b<L_HASEL_MD5; b++)
                    {
                        
                        sprawdzenie=strncmp(kod_md5, dane_wej.tab_MD5_wej[b],32);//ZWRACA 0 JESLI LANCUCHY SA TAKIE SAME
                        //printf("Wygenerowane slowo %s, kod to: %s, kod ze slownika to: %s, warosc sprawdzenia to %d.\n",nowe_slowo,kod_md5,dane_wej.tab_MD5_wej[b],sprawdzenie);
                        //sleep(1);
                        
                        if(sprawdzenie==0)
                        {   
                            pthread_mutex_lock(&dane_wyj.mutex); //POCZATEK SEKCJI KRYTYCZNEJ
                            strncpy(dane_wyj.tab_slow_wyj[dane_wyj.licznik],nowe_slowo,strlen(nowe_slowo));
                            strncpy(dane_wyj.tab_MD5_wyj[dane_wyj.licznik],kod_md5, strlen(kod_md5));
                            //printf("Znaleziono slowo: %s i kod: %s .\n",dane_wyj.tab_slow_wyj[dane_wyj.licznik], dane_wyj.tab_MD5_wyj[dane_wyj.licznik]); 
                            puts("Watek piaty wykryl slowo.");
                            dane_wyj.nready++;
                            
                            //tab_prawdy[a]=1;
                            pthread_cond_signal(&dane_wyj.cond);
                            pthread_mutex_unlock(&dane_wyj.mutex);//KONIEC SEKCJI KRYTYCZNEJ
                            sleep(1);
                            break;
                        }  
                    }
                    free(nowe_slowo);
                }
            }
        }
    }
    pthread_exit(NULL);       
}




 void *konsument(void *dane) 
{
    struct dane_wyjsciowe* info = (struct dane_wyjsciowe*)dane;
    

    while (praca)
    {
        pthread_mutex_lock(&dane_wyj.mutex);
        //printf("Konsument czeka na odebranie znalezionego hasla.\n");
        while(dane_wyj.nready==0)
            pthread_cond_wait(&dane_wyj.cond, &dane_wyj.mutex);
        
        
        printf("Znalezionym haslem nr. %d jest: %s o kodzie %s .\n",dane_wyj.licznik,dane_wyj.tab_slow_wyj[dane_wyj.licznik],dane_wyj.tab_MD5_wyj[dane_wyj.licznik]);
        dane_wyj.nready--;
        dane_wyj.licznik++;
        pthread_mutex_unlock(&dane_wyj.mutex);
    }
    pthread_exit(NULL);
}



void sighandler(int signum) 
{
   printf("Przechwycono sygnal %d, konczenie pracy.\n", signum);
   praca=0;

    for(int i=0; i<dane_wyj.licznik; i++)
    {
        printf("Wartosc licznika to: %d, slowo to: %s, a kod to: %s\n", i, dane_wyj.tab_slow_wyj[i],dane_wyj.tab_MD5_wyj[i] );
    }
    
    for(int i=0; i<L_SLOW_SLOWNIK;i++)
    {
       free(dane_wej.tab_slow_wej[i]); 
    }
   exit(1);
}
