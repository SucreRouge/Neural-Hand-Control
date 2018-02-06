#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include <math.h>
#include <fcntl.h>

#define NUMPAT 15
#define NUMIN  2
#define NUMHID 10
#define NUMOUT 4

#define rando() ((double)rand()/((double)RAND_MAX+1))

int main() {
FILE *afile=fopen("Databases/Training_Angles.csv","r");
FILE *cfile=fopen("Databases/Coordinates.csv","r");
	int i, j, k, p, np, op, ranpat[NUMPAT+1], epoch;
	int NumPattern = NUMPAT, NumInput = NUMIN, NumHidden = NUMHID, NumOutput = NUMOUT;
	double Input[NUMPAT+1][NUMIN+1];
	char tempstr[20];
	for(i=1;i<=NUMPAT;i++)
	{
		for(j=1;j<=NUMIN;j++)
		{
			fscanf(cfile,"%[^,],",tempstr);
            if(j==1)
                Input[i][j]=atof(tempstr)/650;
            else
                Input[i][j]=atof(tempstr)/500;
		}
	}
	//Specify The target Output
	double Target[NUMPAT+1][NUMOUT+1];
	for(i=1;i<=NUMPAT;i++)
	{
		for(j=1;j<=NUMOUT;j++)
		{
            fscanf(afile,"%[^,],",tempstr);
            Target[i][j]=atof(tempstr)/1000;
        }
    }


	double SumH[NUMPAT+1][NUMHID+1], WeightIH[NUMIN+1][NUMHID+1], Hidden[NUMPAT+1][NUMHID+1];
	double SumO[NUMPAT+1][NUMOUT+1], WeightHO[NUMHID+1][NUMOUT+1], Output[NUMPAT+1][NUMOUT+1];
	double DeltaO[NUMOUT+1], SumDOW[NUMHID+1], DeltaH[NUMHID+1];
	double DeltaWeightIH[NUMIN+1][NUMHID+1], DeltaWeightHO[NUMHID+1][NUMOUT+1];


	double Error, eta = 0.3, alpha = 0.9, smallwt = 0.5;


	for( j = 0 ; j <= NumHidden ; j++ ) 
	{
		for( i = 0 ; i <= NumInput ; i++ ) 
		{ 
			DeltaWeightIH[i][j] = 0.0 ;
			WeightIH[i][j] = 2.0 * ( rando() - 0.5 ) * smallwt ;
        	}
	}



	for( k = 1 ; k <= NumOutput ; k ++ ) 
	{    
		for( j = 0 ; j <= NumHidden ; j++ ) 
		{
			DeltaWeightHO[j][k] = 0.0 ;              
			WeightHO[j][k] = 2.0 * ( rando() - 0.5 ) * smallwt ;
		}
	}
     


	for( epoch = 0 ; epoch < 1000000 ; epoch++) 
	{

        	for( p = 1 ; p <= NumPattern ; p++ ) 
		{    
			ranpat[p] = p ;
        	}
		
		for( p = 1 ; p <= NumPattern ; p++) 
		{
			np = p + rando() * ( NumPattern + 1 - p ) ;
			op = ranpat[p] ; ranpat[p] = ranpat[np] ; ranpat[np] = op ;
		}

		Error = 0.0 ;



	for( np = 1 ; np <= NumPattern ; np++ ) 
	{    
		p = ranpat[np];


		for( j = 1 ; j <= NumHidden ; j++ ) 
		{    
                	SumH[p][j] = WeightIH[0][j] ;
                	for( i = 1 ; i <= NumInput ; i++ ) 
			{
                    		SumH[p][j] += Input[p][i] * WeightIH[i][j] ;
                	}
                	Hidden[p][j] = 1.0/(1.0 + exp(-SumH[p][j])) ;
            	}


            	for( k = 1 ; k <= NumOutput ; k++ ) 
		{    
                	SumO[p][k] = WeightHO[0][k] ;	//Bias b
                	for( j = 1 ; j <= NumHidden ; j++ ) 
			{
                    		SumO[p][k] += Hidden[p][j] * WeightHO[j][k] ;	//Weighted Inputs Wi*Xi
                	}
                	Output[p][k] = 1.0/(1.0 + exp(-SumO[p][k])) ;   
                	Error += 0.5 * (Target[p][k] - Output[p][k]) * (Target[p][k] - Output[p][k]) ; 
                	DeltaO[k] = (Target[p][k] - Output[p][k]) * Output[p][k] * (1.0 - Output[p][k]) ;
            	}

		for( j = 1 ; j <= NumHidden ; j++ ) 
		{    
	                SumDOW[j] = 0.0 ;
	                for( k = 1 ; k <= NumOutput ; k++ ) 
			{
	                    SumDOW[j] += WeightHO[j][k] * DeltaO[k] ;
	                }
	                DeltaH[j] = SumDOW[j] * Hidden[p][j] * (1.0 - Hidden[p][j]) ;
		}
            	
		for( j = 1 ; j <= NumHidden ; j++ ) 
		{     
	                DeltaWeightIH[0][j] = eta * DeltaH[j] + alpha * DeltaWeightIH[0][j] ;
	                WeightIH[0][j] += DeltaWeightIH[0][j] ;
	                for( i = 1 ; i <= NumInput ; i++ ) 
			{ 
				DeltaWeightIH[i][j] = eta * Input[p][i] * DeltaH[j] + alpha * DeltaWeightIH[i][j];
				WeightIH[i][j] += DeltaWeightIH[i][j] ;
			}
		}

		for( k = 1 ; k <= NumOutput ; k ++ ) 
		{    
	                DeltaWeightHO[0][k] = eta * DeltaO[k] + alpha * DeltaWeightHO[0][k] ;
	                WeightHO[0][k] += DeltaWeightHO[0][k] ;
	                for( j = 1 ; j <= NumHidden ; j++ ) 
			{
                    		DeltaWeightHO[j][k] = eta * Hidden[p][j] * DeltaO[k] + alpha * DeltaWeightHO[j][k] ;
				WeightHO[j][k] += DeltaWeightHO[j][k] ;
			}
		}
	}
        
	if( epoch%10000 == 0 ) fprintf(stdout, "\nEpoch %-5d :   Error = %f", epoch, Error) ;
        if( Error < 0.00001 ) break ;
}
    
fprintf(stdout, "\n\nNETWORK DATA - EPOCH %d\n\nPat\t", epoch) ; 
for( p = 1 ; p <= NumPattern ; p++ ) 
{        
	fprintf(stdout, "\n%d\t", p) ;
        for( k = 1 ; k <= NumOutput ; k++ ) 
	{
		fprintf(stdout, "%f  ", Output[p][k]) ;
        }
	printf("\n");
}
FILE *fwih=fopen("WeightIH.csv","w");
FILE *fwho=fopen("WeightHO.csv","w");


for(j=0;j<=NumHidden;j++)
{
	for(i=0;i<=NumInput;i++)
	{
		fprintf(fwih,"%lf,",WeightIH[i][j]);
	}
	fprintf(fwih,"\n");
}


for(k=0;k<=NumOutput;k++)
{
    for(j=0;j<=NumHidden;j++)
	{
		fprintf(fwho,"%lf,",WeightHO[j][k]);
	}
	fprintf(fwho,"\n");
}
return 1 ;
}
