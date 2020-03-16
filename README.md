# Programare-Procedurala
Documentatia Proiectului la Programare Procedurala
Proiectul ce urmeaza a fi prezentat combina criptografia cu procesarea digitala, fiind divizat în doua parti:
-prima parte ce are drept scop criptarea si decriptarea unei imagini “pappers.bmp”;
-partea a doua ce are drept scop recunoasterea de pattern-uri intr-o imagine cu cifre scrise de mana, folosind tehnica template matching.
In cele ce urmeaza voi prezenta atat rolul functiilor utilizate:
1. XORSHIFT32, cu antetul: uint32_t* XORSHIFT32( unsigned int dim, uint_fast32_t *R, uint32_t R0)
 Returneaza o secventa de numere intregi aleatoare fara semn pe 32 de biti;
 Parametrul dim reprezinta dimeniunea secventei de numere;
 Parametrul *R reprezinta secventa de numere, transmisa sub forma unui vector alocat dynamic;
 Parametrul R0 reprezinta seed-ul la care vom reveni ulterior.
2. detalii, cu antetul: detaliiImagine detalii(char* nume_fisier_sursa)
 Returneaza o variabila de tip detaliiImagine, unde detaliiImagine este o structura ce contine campurile: dimensiune, inaltime, latime, padding, *pixel, *header;
 Parametrul nume_fisier_sursa este calea imaginii sursa;
 Functia calculeaza si retine in variabila imagine_sursa paddingul imaginii, latimea, dimensiunea si inaltimea (in pixeli), un vector de pixeli alocat dinamic -pixel- si un vector cu primii 54 de octeti reprezentand header-ul imaginii.
3. liniarizareImagine, cu antetul: unsigned char* liniarizareImagine(char* nume_fisier_sursa, detaliiImagine imagine_sursa)
 Returneaza un vector de caractere alocat dinamic;
 Paramterul nume_fisier_sursa este calea imaginii sursa;
 Parametrul imagine_sursa este o variabila de tip detaliiImagine ce contine toate detaliile calculate in functie anterioara;
 Functie copiaza pixelii octet cu octet in vector, returnandu-l ulterior.
4. liniarizareImagineMeorieExterna, cu antetul: void liniarizareImagineMeorieExterna(char* nume_fisier_imagine_liniarizata, unsigned char* tablouLiniarizat, detaliiImagine imagine_sursa)
 Furnizeaza in memoria exterrna o imagine utilizand vectorul calcula la fucntai anterioara;
 Parametrul nume_fisier_imagine_liniarizata reprezinta imagineaz obtinuta.
5. Durstenfeld, cu antetul: unsigned int* Durstenfeld( detaliiImagine imagine_sursa, uint32_t* R)
 Returneaza o permutare aleatoare folosind algoritmul lui Durstenfeld;
 Parametrul uint32_t* R este de fapt vectorul generat cu ajutorul functiei XORSHIFT32.
Druță Cati-Grupa 132
*5 ianuarie 2019
6. criptare, cu antetul: void criptare(char* nume_fisier_sursa, char* nume_fisier_imagine_criptata, char* nume_fisier_cheie_secreta, detaliiImagine imagine_sursa)
 Cripteaza o imagine;
 Parametrul nume_fisier_imagine_criptata reprezinta imagiena obtinuta;
 Parametrul nume_fisier_cheie_secreta reprezinta fisierul text din care citesc seedul- R0 (transmis ca parametru in functia XORSHIFT32) si cheia secreta SV;
 Creez secventa de numere intregi aleatoare pe 32 de biti folosind functia XORSHIFT32;
 Generez permutarea aleatoare folosind Durstenfeld;
 Permut pixelii imaginii P conform permutarii anterioare;
 Obtin vectoul specific imaginii criptate:
C[0]= SV^P[0]^R[imagine_sursa.inaltime * imagine_sursa.latime]
C[i]=C[i]^P[i]^R[imagine_sursa.latime * imagine_sursa.inaltime+i], unde i ia valori de la 1 la imagine_sursa.inaltime * imagine_sursa.latime;
 Apelez functia liniarizareImagineMeorieExterna pentru a transforma vectorul caracteristic imaginii criptate in format bmp.
7. decriptare, cu antetul: void decriptare(char* nume_fisier_sursa, char* nume_fisier_imagine_decriptata, char* nume_fisier_cheie_secreta, detaliiImagine imagine_sursa)
 Parcurg aceeasi pasi ca la functia criptare;
 Calculez C_inv:
In prima istanta C_inv[i]=D[i]=C[i], unde i are valori de la 0 la imagine_sursa.inaltime * imagine_sursa.latime;
C_inv[0]=SV^C[0]^R[imagine_sursa.latime * imagine_sursa.inaltime]
C_inv[i]=C[i-1]^C[i]^R[imagine_sursa.latime * imagine_sursa.inaltime + i], unde i are valori de la 1 la imagine_sursa.inaltime * imagine_sursa.latime
 Generez permuatrea inversa
perm_inv[permutare[i]]=i, i are valori de la 1 la imagine_sursa.inaltime * imagine_sursa.latime
 Obtin vectorul specific imaginii decriptate:
D[perm_inv[i]]=C_inv[i], unde , i are valori de la 1 la imagine_sursa.inaltime * imagine_sursa.latime
 Apelez functia liniarizareImagineMeorieExterna pentru a transforma vectorul characteristic imaginii decriptate in format bmp.
8. chi_patrat, cu antetul: chiTest chi_patrat(char* nume_fisier_sursa, detaliiImagine imagine_sursa )
 Returneaza o variabila de tip chiTest, unde chiTest, este o structura de date cu 3 campuri double R,G,B unde retin valoarea testului chi_patrat
 Calculez frecventa valorilor pe fiecare canal
B[tablouLiniarizat[3*i]]++;
G[tablouLiniarizat[3*i+1]]++;
R[tablouLiniarizat[3*i+2]]++; unde i are valori de la 1 la imagine_sursa.inaltime * imagine_sursa.latime
 Calzulez sumele
Druță Cati-Grupa 132
*5 ianuarie 2019
x.R=x.R+((R[i]-f)*(R[i]-f))/f;
x.G=x.G+((G[i]-f)*(G[i]-f))/f;
x.B=x.B+((B[i]-f)*(B[i]-f))/f; unde i are valori de la 1 la imagine_sursa.inaltime * imagine_sursa.latime, iar f este (imagine_sursa.latime * imagine_sursa.inaltime)/256, iar x este o variabila de tip chiTest.
9. grayscale_image, cu antetul: void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
10. detalii2, cu antetul: detaliiImagine2 detalii2(char* nume_fisier_sursa, detaliiImagine2 img).
 Returneaza o variabila de tip detaliiImagine2, unde detaliiImagine2 este o structura ce contine campurile: dimensiune, inaltime, latime, padding, **pixel, *header, ;
 Parametrul nume_fisier_sursa este calea imaginii sursa;
 Functia calculeaza si retine in variabila imagine_sursa paddingul imaginii, latimea, dimensiunea si inaltimea (in pixeli), o matrice de pixeli alocata dinamic -pixel- si un vector cu primii 54 de octeti reprezentand header-ul imaginii.
11. bordare, cu antetul detaliiImagine2 bordare( detaliiImagine2 image, detaliiImagine2 pattern)
 Returneaza cracteristicile imaginii dupa bordare prin intermediul unei variabile de tipul detaliiImagine2 ;
 Iau o variabila auxiliara aux in care retin matricea bordata cu dimensiunile unui sablon;
 Copiez in matricea caracteristica imaginii sursa valorile din aux.
12. average, cu antetul: float average(detaliiImagine2 x, unsigned line, unsigned column, union culoare intensitate)
 Returnez media valorilor intensitatilor grayscale a pixelilor în fereastra transmisa ca parametru prin intermediul parametrului x;
 Patrametrii line si column reprezinta indicii ferestrei;
 Parametrul intensitate este de tip union culoare, fiind o uniune ce prezinta campurile i, ch[4] in cel dintai retinandu-se un int(valoarea unui pixel), iar il cel de-al doilea un cir de charuri, cu ajutorul caruia accesez octetii pixelului.
13. deviation, cu antetul: double deviation(detaliiImagine2 x, unsigned int line, unsigned int column, union culoare intensitate, unsigned int n)
 Returneaza deviatia standard din fereastra din matricea cu proprietatile variabilei x;
 Parametrii line si column reprezinta indicii ferestrei;
 Parametrul n reprezinta numarul de pixeli din sablonul pe care lucram
 Calculam deviatia:
dev+=(intensitate.ch[0]-avr)*(intensitate.ch[0]-avr), unde ch[0] este primul octet al pixelului de pe pozitia i si j, unde i si j sunt contorii cu ajutorul carora parcurgem toti pixelii din fereastra, iar avr este media calculata cu functia anterioara pentru fereastra curenta
sqrt(1/(n-1)* dev)
14. correlation, cu antetul: double correlation(detaliiImagine2 gray, detaliiImagine2 pattern, unsigned int line, unsigned int column, float avgS, float devS, unsigned int n)
Druță Cati-Grupa 132
*5 ianuarie 2019
 Returnez corelatia ferestrei date de coordonatele line si column;
 Parametrul gray reprezinta o variabila de tip detaliiImagine2 care contine detaliile imaginii sursa grayscale;
 Parametrul avgS reprezinta rezultatul functiei average pentru un sablon S, ale carui proprietati sunt transmise prin intermediul parametrului pattern;
 Prametrul devS reprezinta rezultatul fucntiei deviation pentru un sablon S, ale carui proprietati sunt transmise prin intermediul parametrului pattern;
 Despre paramterul n am vornbit la functia anterioara;
 Calculez corelatia:
f=intensitate.ch[0];unde ch[0] este primul octet al pixelului de pe pozitia data de i si j, iar i si j sunt contorii care ajuta la parcurgerea ferestrei, si deci f este intensitatea pixelului din fereastra data de parametrii line si column
S=intensitate.ch[0]; retine intensitatea pixelului din sablon dat de pozitia i si j;
corr=corr+(1/(devS * devFij))*(f-avgFij)*(S-avgS); unde corr este o variabila de tip double
dupa parcurgerea ferestrei returnam corr/n;
15. template_matching, cu antetul: detectii* template_matching(char nume_img_sursa, char nume_sablon, unsigned int x, detaliiImagine2 gray, double ps, detectii* Det)
 Returneaza un vector de tip detectii, unde detectii este o structura ce prezinta campurile: line, column(reprezantand adresa de inceput a unei ferestre din matrice), corr(retine corelatia ferestrei date de indicii anteriori), nrElem(reprezinta numaruld e elemente pe care le are vectorul pana in momentul curent), sablon(reprezinta sablonul curent, avnd valori de la 0 la 9);
 Parametrul x imi spune pe ce sablon trebuie sa lucrez;
 Parametrul ps reprezinta un prag pentru corelatii;
 Parametrul Det este un vector in care urmeaza sa stochez detectiile ce au valoarea corelatiei >ps
 Transform sablonul in grayscale;
 Aflu proprietatile sablonului;
 Glisez sablonul pe matrice si aflu corelatiile pentru fiecare ferestra, retinandu-le doar pe cele ale caror corelatie >ps, totodata completez si restul campurilor cu informatiile necesare;
 Stochez in D[0].nrElem numarul de detectii ce indeplinesc acea conditie impusa pentru a sti ulterior cate elemente are vectorul.
16. color, cu antetul: detaliiImagine2 color(char nume_imag_sursa,detectii* det, union culoare C, unsigned int clr, detaliiImagine2 pattern, detaliiImagine2 image)
 Returneaza o variabila de tip detaliiImagine2 ce contine detaliile imaginii dupa ce au fost colorate contururile ferestrelor in vectorul de detectii;
 Parametrul C reprezinta valoarea pe care pixelii de pe conturul ferestrei trebuie sa o ia;
 Parametrul clr imi spune ce sablon am primit;
 Calculez C in fucntie cu care lucrez;
 Parcurg vecorul de detectii colorand mai inatai fereastra pe lateral apoi superior si inferior;
17. cmp, cu antetul: int cmp(const void* a, const void*b)
Druță Cati-Grupa 132
*5 ianuarie 2019
 E functia standard pentru qsort;
18. sorteaza, cu antetul: detectii* sorteaza(detectii *D,char nume_img_sursa, detaliiImagine2 gray, double ps)
 Returnez o variabila de tip detectii ordonata descrescator in fucntie de corelatie;
 Copiez vectorul de detectii intr-un auxiliar;
 Folosesc qsort pentru a sorta corelatiile din vectorul de detectii;
 Folosesc cautarea binara pentru a gasi pozitia pe care se afla acum fiecare detectie si ii asociez caracteristicile specifice (coloana, linie, sablon) ;
19. suprapunere, cu antetul: float suprapunere(unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2, detaliiImagine2 pattern)
 Returnez aria de suprapunere dintre 2 ferestre, a caror pozitie este data de primii 4 parametri ai functiei;
 Calzulez suprafatd e intersectie data de fereastra cu indicii(max(i1, i2), min(j1, j2))
 arie_d1 si arie_d2 reprezinta de fapt aria sablonului transmis ca paramteru;
 aplic formula;
20. eliminSuprapuneri, cu antetul: detectii* eliminSuprapuneri(detectii* D, char nume_img_sursa, detaliiImagine2 gray, double ps, detaliiImagine2 pattern)
 Returnez vectorul de detectii dupa eliminarea suprafetelor de suprapunere;
 Utilizez functia definita anterior pentru a testa daca 2 elemnte definite anterior au o suprapunere mai mare de 0.2, eliminandu-le ulterior;
21. non_maxime, cu antetul: detectii* non_maxime(char nume_img_sursa, detaliiImagine2 gray, double ps, detectii *D, detaliiImagine2 pattern)
 Citesc sablonul;
 Sortez vectorul de detectii folosind functia sorteaza;
 Elimin suprapunerile folosind fucntia eliminSuprapuneri ;
