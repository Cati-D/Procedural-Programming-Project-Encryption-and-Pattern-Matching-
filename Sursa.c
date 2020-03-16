#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

typedef struct
{
    unsigned int dimensiune;
    unsigned int inaltime;
    unsigned int latime;
    unsigned int padding;
    unsigned int* pixel;
    unsigned int*header;
}detaliiImagine;

typedef struct
{
    unsigned int dimensiune;
    unsigned int inaltime;
    unsigned int latime;
    unsigned int **pixel;
    unsigned int *header;
    unsigned int padding;
}detaliiImagine2;

typedef struct
{
    double R;
    double G;
    double B;
}chiTest;

typedef struct
{
    unsigned int linie;
    unsigned int coloana;
    double corr;
    unsigned int nrElem;
    unsigned int sablon;
}detectii;

 union culoare
{
    int i;
    int ch[4];
};

//FUNCTIA XORSHIFT=>//Generarea unei secvente de numere intregi aleatoare fara semn pe 32 de biti
uint32_t* XORSHIFT32( unsigned int dim, uint_fast32_t *R, uint32_t R0)
{
    unsigned int i;
    for (i=0; i<dim; i++)
    {
        R[i]= R0;
        R[i]^= R[i] << 13;
        R[i]^= R[i]>> 17;
        R[i]^= R[i] << 5;
        R0= R[i];
    }

    return R;
}

//Retin detaliile imaginii
detaliiImagine  detalii(char* nume_fisier_sursa)
{
    FILE *fin;
    unsigned int dim_img, latime_img, inaltime_img;

    printf("nume_fisier_sursa = %s \n", nume_fisier_sursa);
    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

    //calculez padding-ul pentru o linie
    unsigned int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding =0;

    detaliiImagine imagine_sursa;
    imagine_sursa.padding=padding;
    imagine_sursa.latime=latime_img;
    imagine_sursa.inaltime=inaltime_img;
    imagine_sursa.dimensiune=dim_img;

    fseek(fin, 0, SEEK_SET);
    imagine_sursa.header=(unsigned char*)malloc(54* sizeof(unsigned char));
    fread(imagine_sursa.header, 54, 1, fin);

    fseek(fin, 54, SEEK_SET);
    imagine_sursa.pixel=(unsigned int*)calloc(latime_img * inaltime_img, sizeof(unsigned int));
    fread(imagine_sursa.pixel, latime_img * inaltime_img, sizeof(unsigned int), fin);
    fclose(fin);

    return imagine_sursa;
}

///LINIARIZARE IMAGINE
unsigned char* liniarizareImagine(char* nume_fisier_sursa, detaliiImagine imagine_sursa)
{
    FILE *fin;

    printf("nume_fisier_sursa = %s \n", nume_fisier_sursa);
    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

    unsigned char* tablouLiniarizat;
    tablouLiniarizat=(unsigned char*)malloc((imagine_sursa.dimensiune)*sizeof(unsigned int));

    //copiez pixelii in vector
    unsigned int i;
    for (i=0; i< imagine_sursa.dimensiune; i++)
        fread(&tablouLiniarizat[i], 1, 1, fin);

    fclose(fin);
    return tablouLiniarizat;

}

///SALVAREA IN MEMORIE EXTERNA A UNEI IMAGINI STOCATE IN MEMORIA INTERNA IN FORMA LINIARIZATA
void liniarizareImagineMeorieExterna(char* nume_fisier_imagine_liniarizata, unsigned char* tablouLiniarizat, detaliiImagine imagine_sursa)
{
    FILE* fout;

    printf("nume_fisier_imagine_liniarizata = %s \n",nume_fisier_imagine_liniarizata);

    fout=fopen(nume_fisier_imagine_liniarizata, "wb+");
    if(fout == NULL)
    {
        printf("Nu am putut deschide corect fisierul pentru scriere\n");
        return;
    }

    unsigned int i;
    for(i=0; i<imagine_sursa.dimensiune; i++)
        fwrite(&tablouLiniarizat[i], 1, 1, fout);

    fclose(fout);
}

// Generarea unei permuatari σ aleatoare folosind algoritmul lui Durstenfeld
unsigned int* Durstenfeld( detaliiImagine imagine_sursa, uint32_t* R)
{
    unsigned int r, i, k=0;
    unsigned int aux, *pr;
    pr=(unsigned int *)malloc(imagine_sursa.inaltime * imagine_sursa.latime*sizeof(unsigned int));

    for (i=0; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
        pr[i]=i;

    for (i= imagine_sursa.latime * imagine_sursa.inaltime -1; i>0; i--)
    {
        r=R[k++]%(i+1);
        aux=pr[i];
        pr[i]=pr[r];
        pr[r]=aux;
    }

    return pr;
}

///CRIPTAREA UNEI IMAGINI
void criptare(char* nume_fisier_sursa, char* nume_fisier_imagine_criptata, char* nume_fisier_cheie_secreta, detaliiImagine imagine_sursa)
{
    FILE *fin;
    printf("nume_fisier_sursa = %s \n", nume_fisier_sursa);
    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

    //Citirea din fisierul ce contine cheia secreta
    FILE *fins;

    printf("nume_fisier_cheie_secreta = %s \n", nume_fisier_cheie_secreta);
    fins = fopen(nume_fisier_cheie_secreta, "r");
    if(fins == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

    uint32_t R0, SV;
    fscanf(fins, "%u", R0);
    fscanf(fins, "%d", SV);
    fclose(fins);

    //Creez secventa de numere intregi aleatoare pe 32 de biti
    uint32_t*R;
    R=(uint32_t*)calloc(imagine_sursa.latime * imagine_sursa.inaltime*2 - 1, sizeof(uint32_t));
    XORSHIFT32(imagine_sursa.latime* imagine_sursa.inaltime *2 - 1, R, R0);

    //Generez permutarea aleatoare
    unsigned int *permutare;
    permutare=Durstenfeld(imagine_sursa, R);

    //Permutarea pixelilor imaginii P conform permutarii σ
    unsigned int*P;
    P=(unsigned int*)calloc(imagine_sursa.latime * imagine_sursa.inaltime, sizeof(unsigned int));

    unsigned int i;
    for(i=0; i< imagine_sursa.latime * imagine_sursa.inaltime; i++)
        P[permutare[i]]=imagine_sursa.pixel[i];

    //Obtinerea imaginii criptate
    unsigned int *C;
    C=(unsigned int*)malloc(imagine_sursa.dimensiune* sizeof(unsigned int));

    C[0]=SV^P[0]^R[imagine_sursa.inaltime * imagine_sursa.latime];

    for(i=1; i<imagine_sursa.inaltime * imagine_sursa.latime; i++)
        C[i]=C[i]^P[i]^R[imagine_sursa.latime * imagine_sursa.inaltime+i];

    FILE* fout;
    printf("nume_fisier_imagine_criptata = %s \n",nume_fisier_imagine_criptata);
    fout=fopen(nume_fisier_imagine_criptata, "wb");
    fwrite(imagine_sursa.header, 54, 1, fout);
    fflush(fout);
    fseek(fout, 54, SEEK_SET);
    for (i=0; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
    {
        fwrite(&C[i], 1, 3, fout);
    }
    fflush(fout);
    fclose(fout);
    fclose(fin);

    free(R);
    free(C);
    free(P);
    free(permutare);

}
///DECRIPTAREA UNEI IMAGINI
void decriptare(char* nume_fisier_sursa, char* nume_fisier_imagine_decriptata, char* nume_fisier_cheie_secreta, detaliiImagine imagine_sursa)
{
     FILE *fin;
    printf("nume_fisier_sursa = %s \n", nume_fisier_sursa);
    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

 //Citirea din fisierul ce contine cheia secreta
    FILE *fins;

    printf("nume_fisier_cheie_secreta = %s \n", nume_fisier_cheie_secreta);
    fins = fopen(nume_fisier_cheie_secreta, "r");
    if(fins == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

    uint32_t R0, SV;
    fscanf(fins, "%u", R0);
    fscanf(fins, "%u", SV);
    fclose(fins);

    //Creez secventa de numere intregi aleatoare pe 32 de biti
    uint32_t*R;
    R=(uint32_t*)calloc(imagine_sursa.latime * imagine_sursa.inaltime*2 - 1, sizeof(uint32_t));
    unsigned int i;
    for(i=0; i<imagine_sursa.latime * imagine_sursa.inaltime*2 - 1; i++)
        R[i]=0;
    XORSHIFT32(imagine_sursa.latime* imagine_sursa.inaltime *2 - 1, R, R0);

    //Generez permutarea aleatorie
    unsigned int *permutare;
    permutare=Durstenfeld(imagine_sursa, R);

    //Permutarea pixelilor imaginii P conform permutarii σ
    unsigned int*P;
    P=(unsigned int*)calloc(imagine_sursa.latime * imagine_sursa.inaltime, sizeof(unsigned int));

    for(i=0; i< imagine_sursa.latime * imagine_sursa.inaltime; i++)
        P[permutare[i]]=imagine_sursa.pixel[i];

    //Obtinerea imaginii criptate
    unsigned int *C;
    C=(unsigned int*)malloc(imagine_sursa.dimensiune* sizeof(unsigned int));

    C[0]=SV^P[0]^R[imagine_sursa.inaltime * imagine_sursa.latime];

    for(i=1; i<imagine_sursa.inaltime * imagine_sursa.latime; i++)
        C[i]=C[i]^P[i]^R[imagine_sursa.latime * imagine_sursa.inaltime+i];

    //Obtinere imagine decriptata
    unsigned int *C_inv, *D;
    D=(unsigned int*)malloc(imagine_sursa.inaltime * imagine_sursa.latime * sizeof(unsigned int));
    C_inv=(unsigned int*)malloc(imagine_sursa.dimensiune*sizeof(unsigned int));

    for(i=0;i<imagine_sursa.inaltime * imagine_sursa.latime; i++)
        C_inv[i]=D[i]=C[i];

    C_inv[0]=SV^C[0]^R[imagine_sursa.latime * imagine_sursa.inaltime];
    for(i=1; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
        C_inv[i]=C[i-1]^C[i]^R[imagine_sursa.latime * imagine_sursa.inaltime + i];

    //Genereze permutarea inversa
    unsigned *perm_inv;
    perm_inv=(unsigned int*)malloc(sizeof(unsigned int) * imagine_sursa.inaltime * imagine_sursa.latime);
    for(i=0; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
        perm_inv[permutare[i]]=i;

    for (i=0; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
        D[perm_inv[i]]=C_inv[i];

    FILE* fout;
    printf("nume_fisier_imagine_decriptata = %s \n",nume_fisier_imagine_decriptata);
    fout=fopen(nume_fisier_imagine_decriptata, "wb");
    fwrite(imagine_sursa.header, 54, 1, fout);
    fflush(fout);
    fseek(fout, 54, SEEK_SET);
    for (i=0; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
    {
        fwrite(&D[i], 1, 3, fout);
    }
    fflush(fout);
    fclose(fout);

    free(R);
    free(C);
    free(C_inv);
    free(P);
    free(permutare);
    free(perm_inv);
    free(D);
    fclose(fin);
}

///TESTUL CHI-PATRAT
chiTest chi_patrat(char* nume_fisier_sursa, detaliiImagine imagine_sursa )
{
    unsigned char *tablouLiniarizat;
    tablouLiniarizat=liniarizareImagine(nume_fisier_sursa, imagine_sursa);

    //Calculez frecventa valorilor pe fiecare canal
    double *R, *G, *B;
    B=(double*)calloc(256, sizeof(double));
    G=(double*)calloc(256, sizeof(double));
    R=(double *)calloc(256, sizeof(double));
    unsigned int i;

    for(i=0; i<=255; i++)
        R[i]=G[i]=B[i]=0;
    for(i=0; i<imagine_sursa.latime * imagine_sursa.inaltime; i++)
    {
        B[tablouLiniarizat[3*i]]++;
        G[tablouLiniarizat[3*i+1]]++;
        R[tablouLiniarizat[3*i+2]]++;
    }

    //calculez sumele
    chiTest x;
    x.R=x.B=x.G=0;
    float f=(imagine_sursa.latime * imagine_sursa.inaltime)/256;

    for (i=0; i<=255; i++)
    {
        x.R=x.R+((R[i]-f)*(R[i]-f))/f;
        x.G=x.G+((G[i]-f)*(G[i]-f))/f;
        x.B=x.B+((B[i]-f)*(B[i]-f))/f;

    }

    return x;
}

void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
   FILE *fin, *fout;
   unsigned int dim_img, latime_img, inaltime_img;
   unsigned char pRGB[3], header[54], aux;

   printf("nume_fisier_sursa = %s \n",nume_fisier_sursa);

   fin = fopen(nume_fisier_sursa, "rb");
   if(fin == NULL)
   	{
   		printf("nu am gasit imaginea sursa din care citesc");
   		return;
   	}

   fout = fopen(nume_fisier_destinatie, "wb+");

   fseek(fin, 2, SEEK_SET);
   fread(&dim_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

   fseek(fin, 18, SEEK_SET);
   fread(&latime_img, sizeof(unsigned int), 1, fin);
   fread(&inaltime_img, sizeof(unsigned int), 1, fin);
   printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

   //copiaza octet cu octet imaginea initiala in cea noua
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);

	//calculam padding-ul pentru o linie
	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

    printf("padding = %d \n",padding);

	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			//citesc culorile pixelului
			fread(pRGB, 3, 1, fout);
			//fac conversia in pixel gri
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}

//Calculez si retin proprietatile imaginii
detaliiImagine2  detalii2(char* nume_fisier_sursa, detaliiImagine2 img)
{
    FILE *fin;
    unsigned int dim_img, latime_img, inaltime_img;

    printf("nume_fisier_sursa = %s \n", nume_fisier_sursa);
    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("Nu am gasit imaginea sursa din care citesc\n");
        return;
    }

    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

    unsigned int padding;
    img.padding=padding;
    img.latime=latime_img;
    img.inaltime=inaltime_img;
    img.dimensiune=dim_img;

    fseek(fin, 0, SEEK_SET);
    img.header=(unsigned char*)malloc(54* sizeof(unsigned char));
    fread(img.header, 54, 1, fin);

    fseek(fin, 54, SEEK_SET);
    unsigned int i, j, aux;
    img.pixel=(unsigned int**)malloc(sizeof(unsigned int *)* img.inaltime);
    for (i=0; i<img.inaltime; i++)
            img.pixel[i]=(unsigned int*)malloc(sizeof(unsigned int)* img.latime);

    for(i=0; i<img.inaltime; i++)
        for(j=0; j<img.latime; j++)
                {fread(&aux, 3, 1, fin);
                img.pixel[i][j]=aux;
                }
    fclose(fin);

    return img;
}

//Bordez matricea de pixeli
detaliiImagine2 bordare( detaliiImagine2 image, detaliiImagine2 pattern)
{
    unsigned int i, **aux, j, k=0;
    aux=(unsigned int**)malloc( sizeof(unsigned int*) * (image.inaltime + 2 * pattern.inaltime));
    for(j=0; j<image.inaltime+ 2 * pattern.inaltime; j++)
        aux[i]=(unsigned int*)malloc((image.latime+ 2 * pattern.latime)*sizeof(unsigned int));
    for (i=0; i<pattern. inaltime; i++)
        for (j=0; j<image.latime+ 2 * pattern.latime; j++)
            {aux[i][j]=0;
            aux[image.inaltime + pattern.inaltime-i-1][j]=0;
            printf("%d\t", k);
            k++;
            }
   for (i=pattern.inaltime; i<image.inaltime + pattern.inaltime; i++)
        for(j=0; j<pattern.latime; j++)
        {
            aux[i][j]=0;
            aux[i][image.latime + pattern.latime-j-1]=0;
        }

    for(i=0; i<image.inaltime; i++)
        for(j=0; j<image.latime; j++)
            aux[pattern.latime+i][pattern.latime+j]=image.pixel[i][j];

    image.pixel=realloc(image.pixel, sizeof(unsigned int*) * (image.inaltime + 2 * pattern.inaltime));
    for(j=0; j<image.inaltime+ 2 * pattern.inaltime; j++)
        image.pixel[i]=realloc(image.pixel[i], (image.latime+ 2 * pattern.latime)*sizeof(unsigned int));

    //Modific atat detaliile imaginii cat si valorii acesteia
    image.inaltime=image.inaltime+ 2 * pattern.inaltime;
    image.latime=image.latime+ 2 * pattern.latime;
    for(i=0; i<image.inaltime; i++)
        for(j=0; j<image.latime; j++)
        image.pixel[i][j]=aux[i][j];

    for(i=0; i<image.inaltime; i++)
        free(aux[i]);
    free(aux);
    return image;
}

//Glisez sablonul
unsigned int slide(detaliiImagine2 gray, detaliiImagine2 pattern, unsigned int *i, unsigned int *j )
{
    if(*i==gray.inaltime-1 && *j==gray.latime-pattern.latime-1)
        {printf("S-a ajuns la finalul imaginii\n");
        return 0;
        }
    if(*j==gray.latime-pattern.latime-1)
    {
        ++(*i);
        *j=0;
    }
    else
        ++(*j);

    return 1;
}

//Calculez media intensitatilor
float average(detaliiImagine2 x, unsigned line, unsigned column, union culoare intensitate)
{
    unsigned int i, j;
    float avr=0;
    for(i=line; i< line+x.inaltime; i++)
        for (j=column; j<column+x.latime; j++)
           {   intensitate.i=x.pixel[i][j];
               avr+=intensitate.ch[0];
           }
    avr/=(x.inaltime * x.latime);

    return avr;

}

//Calculez deviatia standard
double deviation(detaliiImagine2 x, unsigned int line, unsigned int column, union culoare intensitate, unsigned int n)
{
    unsigned int i, j;
    double dev =0, avr= average(x, line, column,intensitate);
    for(i=line; i< line+x.inaltime; i++)
        for (j=column; j<x.latime; j++)
        {
            intensitate.i=x.pixel[i][j];
            dev+=(intensitate.ch[0]-avr)*(intensitate.ch[0]-avr);
        }

    return sqrt(1/(n-1)* dev);
}

//Calculez corelatia
double correlation(detaliiImagine2 gray, detaliiImagine2 pattern, unsigned int line, unsigned int column, float avgS, float devS, unsigned int n)
{
     union culoare intensitate;
     float avgFij=average(gray, line, column, intensitate);
     float devFij=deviation(gray, line, column, intensitate, n);
     unsigned char S, f;
     double corr=0;

     unsigned int i, j;
     for(i=line; i<=line + pattern.inaltime-1; i++)
        for(j=column; j<column + pattern.latime - 1; j++)
     {
        intensitate.i=gray.pixel[i][j];
        f=intensitate.ch[0];
        intensitate.i=pattern.pixel[i][j];
        S=intensitate.ch[0];
        corr=corr+(1/(devS * devFij))*(f-avgFij)*(S-avgS);
     }

     return corr/n;

}

detectii* template_matching(char nume_img_sursa, char nume_sablon, unsigned int x, detaliiImagine2 gray, double ps, detectii* Det)
{
    FILE *fin;
    fin=fopen(nume_img_sursa, "rb+");
    detaliiImagine2 image;
    image=detalii2(nume_img_sursa, image);
    fclose(fin);
    //Transformarea sablonului in grayscale
    char nume_sablon_grayscale[]="cifrax_grayscale.bmp";
    nume_sablon_grayscale[5]='0'+x;
    grayscale_image(nume_sablon, nume_sablon_grayscale);

    //Aflarea propeietatilor sablonului
    detaliiImagine2 pattern;
    pattern=detalii2(nume_sablon_grayscale, pattern);

   //Glisarea sablonului si aflarea corelatiei
    union culoare intensitate;
    unsigned int n=pattern.latime * pattern.inaltime, *line, *column;
    float avgS=average(pattern, 0, 0, intensitate);
    float devS=deviation(pattern, 0, 0, intensitate, n);
    double corr;
    unsigned int k=0;
    while(slide(gray, pattern, &line, &column) == 1)
    {
        corr=correlation(gray, pattern, *line, *column, avgS, devS, n);
        if (corr > ps)
        {
            Det[k].linie=*line;
            Det[k].coloana=*column;
            Det[k].corr=corr;
            Det[k].sablon=x;
            Det[k++].nrElem=k;
            Det=realloc(Det, sizeof(detectii)*(k+1));
        }
    }
    Det[0].nrElem=k;

    return Det;

}
//Creez o functie care deseneaza conturul unei ferestre Fij
detaliiImagine2 color(char nume_imag_sursa,detectii* det, union culoare C, unsigned int clr, detaliiImagine2 pattern, detaliiImagine2 image)
{
    switch(clr)
    {
        case 0:
            C.ch[0]=255;
            C.ch[3]=C.ch[2]=C.ch[1]=0;
            break;
        case 1:
            C.ch[1]=C.ch[0]=255;
            C.ch[4]=C.ch[2]=0;
            break;
        case 2:
            C.ch[3]=C.ch[2]=C.ch[0]=0;
            C.ch[1]=255;
            break;
        case 3:
            C.ch[3]=C.ch[0]=0;
            C.ch[2]=C.ch[1]=255;
            break;
        case 4:
            C.ch[2]=C.ch[0]=255;
            C.ch[3]=C.ch[1]=0;
            break;
        case 5:
            C.ch[3]=C.ch[1]=C.ch[0]=0;
            C.ch[2]=255;
            break;
        case 6:
            C.ch[2]=C.ch[1]=C.ch[0]=192;
            C.ch[3]=0;
            break;
        case 7:
            C.ch[3]=C.ch[2]=0;
            C.ch[0]=255;
            C.ch[1]=140;
            break;
        case 8:
            C.ch[2]=C.ch[0]=128;
            C.ch[3]=C.ch[1]=0;
            break;
        case 9:
            C.ch[3]=C.ch[2]=C.ch[1]=0;
            C.ch[0]=128;
            break;
        default:
            return;
    }

    unsigned int i, j, k;
    for (k=0; k<det[0].nrElem; k++)
    {
        for(i=det[k].linie; i<det[k].linie+pattern.inaltime; i++)
        {
            image.pixel[i][det[k].coloana]=C.i;
            image.pixel[i][det[k].coloana+pattern.latime-1]=C.i;
        }
        for(j=det[k].coloana+1; j<det[k].coloana+pattern.latime-1; j++)
        {
            image.pixel[det[k].linie][j]=C.i;
            image.pixel[det[k].linie+pattern.inaltime-1]=C.i;
        }
    }

    return image;
}

//Sortez descrescator tabloul D
int cmp(const void* a, const void*b)
{
    detectii va, vb;
    va=*(detectii*)a;
    vb=*(detectii*)b;
    if(va.corr>vb.corr) return -1;
    if(va.corr<vb.corr) return 1;
    return 0;
}

detectii* sorteaza(detectii *D,char nume_img_sursa, detaliiImagine2 gray, double ps)
{
    //Copiez vectorul de detectii intr-un alt vector
    detectii *aux;
    aux=(detectii*)malloc(sizeof(D));
    unsigned int i;
    for(i=0; i<D[0].nrElem; i++)
    {
        aux[i].coloana=D[i].coloana;
        aux[i].linie=D[i].linie;
        aux[i].corr=D[i].corr;
        aux[i].sablon=D[i].sablon;
        aux[i].nrElem=D[i].nrElem;
    }
    qsort(D, D[0].nrElem, sizeof(double), cmp);

    //Asociez fiecarui element din vector fereastra corecta
    for(i=0; i<aux[0].nrElem; i++)
    {
        unsigned int st=0, dr=aux[0].nrElem, sw=0;
        while (st<=dr && sw==0)
        {
            unsigned int mij=st+(dr-st)/2;
            if (D[mij].corr==aux[i].corr)
                {
                   D[mij].coloana=aux[i].coloana;
                   D[mij].linie=aux[i].linie;
                   D[mij].sablon=aux[i].sablon;
                   sw=1;
                }
            if (D[mij].corr>aux[i].corr)
                dr=mij-1;
            else
                st=mij+1;
        }
    }

    free(aux);
    return D;
}

float suprapunere(unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2, detaliiImagine2 pattern)
{
    unsigned int  x, y, lim_x, lim_y;
    if (i1 < i2)
        {
            x=i2;
            lim_x=i1+pattern.inaltime-1;
        }
        else
        {
            x=i1;
            lim_x=i2+pattern.inaltime-1;
        }

    if(j1 < j2)
    {
        y=j2;
        lim_y=j1+pattern.latime-1;
    }
    else
    {
        y=j1;
        lim_y=j2+pattern.latime-1;
    }

    float arie_intersectie, arie_d1, arie_d2;
    arie_intersectie=(lim_y-y) * (lim_x-x);
    arie_d1=arie_d2=pattern.inaltime * pattern.latime;
    return arie_intersectie/(arie_d1 + arie_d2 - arie_intersectie);

}

detectii* eliminSuprapuneri(detectii* D, char nume_img_sursa, detaliiImagine2 gray, double ps, detaliiImagine2 pattern)
{
    unsigned int i, j;
    for(j=0; j< D[0].nrElem; j++)
        for(i=0; i<j; i++)
        {
            if(suprapunere(D[i].linie, D[i].coloana, D[j].linie, D[j].coloana, pattern)>0.2)
            {
                unsigned k;
                for(k=j; k< D[0].nrElem-1; k++)
                    {
                        D[k].coloana=D[k+1].coloana;
                        D[k].linie=D[k+1].linie;
                        D[k].corr=D[k+1].corr;
                        D[k].sablon=D[k+1].sablon;
                    }
                D[0].nrElem--;
                j--;
                D=realloc(D, sizeof(detectii) * D[0].nrElem);
            }
        }
    return D;
}

//Eliminarea non-maximelor
detectii* non_maxime(char nume_img_sursa, detaliiImagine2 gray, double ps, detectii *D, detaliiImagine2 pattern)
{
    char nume_sablon[]="cifrax.bmp";
    D=sorteaza(D, nume_img_sursa, gray, ps);
    unsigned int i, k;
    D=eliminSuprapuneri(D, nume_img_sursa, gray, ps, pattern);

    return D;
}
int main()
{
    detaliiImagine imagine_sursa;

   ///criptare imagine
    char nume_fisier_sursa[]="peppers.bmp";
    imagine_sursa=detalii(nume_fisier_sursa);
    unsigned char *tablouLiniarizat;
    tablouLiniarizat=liniarizareImagine(nume_fisier_sursa, imagine_sursa);
    printf("\n\n");

    char nume_fisier_imagine_liniarizata[]="pappersLiniarizat.bmp";
    liniarizareImagineMeorieExterna(nume_fisier_imagine_liniarizata, tablouLiniarizat, imagine_sursa);

    char nume_fisier_imagine_criptata[]="enc_peppers.bmp";
    char nume_fisier_cheie_secreta[]="secret_key.txt";
    criptare(nume_fisier_sursa, nume_fisier_imagine_criptata, nume_fisier_cheie_secreta, imagine_sursa);

    //decriptare imagine
    char nume_fisier_imagine_decriptata[]="pappers_decriptata.bmp";
    decriptare(nume_fisier_sursa, nume_fisier_imagine_decriptata, nume_fisier_cheie_secreta, imagine_sursa);

    //afisarea pe ecran a valorilor testlui chi patrat

    chiTest orig;
    chiTest cript;

    orig=chi_patrat(nume_fisier_sursa, imagine_sursa);
    cript=chi_patrat(nume_fisier_imagine_criptata, imagine_sursa);

    printf("Chi-squared test on RGB channels for peppers.bmp:\n R: %lf \n G: %lf \n B: %lf \n\n ", orig.R, orig.G, orig.B);
    printf("Chi-squared test on RGB channels for enc_peppers.bmp:\n R: %lf\n G: %lf\n B: %lf\n\n", cript.R, cript.G, cript.B);


    //Transforrmarea imaginii color in grayscale
    char nume_img_sursa[]="test.bmp";
    char nume_img_grayscale[] = "test_grayscale.bmp";
    char nume_sablon[]="cifrax.bmp";
    grayscale_image(nume_img_sursa, nume_img_grayscale);

    //Aflarea propeietatilor imaginii
    detaliiImagine2 image, gray, pattern;
    image=detalii2(nume_img_sursa, image);
    gray=detalii2(nume_img_grayscale, gray);
    unsigned int i;
    for(i=0; i<=9; i++)
    {
        nume_sablon[5]='0'+i;
        pattern=detalii2(nume_sablon, pattern);
    }

    //Bordare
    image=bordare(image, pattern);
    gray=bordare(gray, pattern);

    double ps=0.5;
    detectii *det, *D;
    det=(detectii*)malloc(sizeof(detectii));
    det=template_matching(nume_img_sursa, nume_sablon, 0, gray, ps, det);
    unsigned int n=det[0].nrElem;
    D=(detectii*)malloc(sizeof(detectii)* n);
    D[0].nrElem=0;
    unsigned int j=0, k;
    union culoare C;

    //Construirea unui tablou unidimensional D
    for (i=0; i<=9; i++)
    {
        for(i=0; i<det[0].nrElem; i++)
            {
                D[j].corr=det[i].corr;
                D[j].linie=det[i].linie;
                D[j].coloana=det[i].coloana;
                D[j].sablon=det[i].sablon;
                j++;
            }
        nume_sablon[5]='0'+i;
        det=template_matching(nume_img_sursa, nume_sablon, i, gray, ps, det);
        n=det[0].nrElem;
        D=realloc(D, sizeof(detectii) * j * n);

    }

    D[0].nrElem=j;

    //Functia de eliminare a non-maximelor
    D=non_maxime(nume_img_sursa, gray, ps, D, pattern);

    //Functia de colorare a ferestrelor
    for(i=0; i<=9; i++)
    {
        for(k=0; k<D[0].nrElem; k++)
            if(D[k].sablon==i)
                color(nume_img_sursa, D, C, i, pattern, image);
    }

     //Scriu matricea in fisier bmp
     unsigned int  *vect, c=0;
     vect=(unsigned int **)malloc(sizeof(unsigned int*) * image.inaltime);
     for (i=0; i<image.inaltime; i++)
     {
         vect[i]=(unsigned int*)malloc(sizeof(unsigned int) * image.latime);
         for(j=0; j<image.latime; j++)
            vect[k]=image.pixel[i][j];
     }
     unsigned char *scr;
     union culoare el;
     for(i=0; i<k ;i++)
     {
            el.i=vect[i];
            scr[c]=el.ch[0];
            scr[++c]=el.ch[1];
            scr[++c]=el.ch[2];
            c++;
     }
     FILE *fin;
     fin=fopen("tm.bmp", "wb+");
     fwrite(scr, c, 1, fin);
     fclose(fin);

	return 0;
}
