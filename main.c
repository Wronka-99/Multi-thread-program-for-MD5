 #include <stdio.h>
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