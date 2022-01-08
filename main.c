 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

#define SIZE_MD5 35
#define L_HASEL_MD5 420
#define L_SLOW_SLOWNIK 550
#define DL_SLOWA 32


char *wczytaj(char *z, int ile, FILE *plik);

 int main(int argc, char *argv[])
 {

     //############## ZMIENNE ############## 

     FILE *slownik;
     FILE *hasla_MD5;

     char tab_hasla_MD5[L_HASEL_MD5][SIZE_MD5];
     char* tab_slownik[L_SLOW_SLOWNIK];
     char tab_pom[DL_SLOWA];
     char tab_pom2[SIZE_MD5];
     char *tmp_wsk;
     char *tmp_wsk2;


    //######################################

    //############## SPRAWDZENIE LICZBY ARGUMENTOW ############## 
     if(argc!=3)
     {
         puts("Podano niewlasciwa liczbe argumentow.");
         puts("Poprawna liczba to dwa argumenty.");
     }

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


 

      for (int index = 0; index < L_HASEL_MD5; index++) 
    {
        tmp_wsk=fgets (tab_hasla_MD5[index], SIZE_MD5, hasla_MD5);   // czytamy ze standardowego wejścia
        
        //printf("Dlugos kodu nr. %d ze md5 %ld \n",index, strlen(tab_hasla_MD5[index]));

        if (tmp_wsk != NULL) 
        {      
            if (feof (hasla_MD5))
                printf ("\n%d koniec pliku\n", index);
        }
        else
            printf ("\n%d blad odczytu\n", index);
    }


      for (int index = 0; index < L_SLOW_SLOWNIK ; index++) 
    {
        tmp_wsk=fgets (tab_pom,DL_SLOWA,slownik);   // czytamy ze standardowego wejścia
        tab_slownik[index]=(char *)calloc(strlen(tab_pom),sizeof(char));

        wczytaj(tab_pom,DL_SLOWA,slownik);
        strncpy(tab_slownik[index],tab_pom,strlen(tab_pom));
        

        if (tmp_wsk != NULL) 
        {      
            if (feof (slownik))
                printf ("\n%d koniec pliku\n", index);
        }
        else
            printf ("\n%d blad odczytu\n", index);
    }

    for(int index=0; index<L_HASEL_MD5; index++)
    {
        printf("Kod nr:%d wyglada nastepujaco: %s ",index,tab_hasla_MD5[index]);
    }
        
    printf("Slowo ze slownika %s\n", tab_slownik[450]);

    fclose(slownik);
    fclose(hasla_MD5);

    for(int i=0; i<L_SLOW_SLOWNIK;i++)
    {
       free(tab_slownik[i]); 
    }
    
    

    return 0;
 }
 






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