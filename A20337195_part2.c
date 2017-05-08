#include<stdio.h>
#include<math.h>
#include<conio.h>

int getmaxcol(int);
int getmincol(int);

int num;
int original[14][14],a[14][14],tran[14][14];	/*	original[14][14]:	record of input adjacency matrix

													a[14][14]:			adjacency matrix for arrival time calculation

													tran[14][14]:		new transformed adjacency matrix used for
																		required time calculation
																		(whether use it or not depends on your algorithm)
												 */


int main(int argc, char **argv)
 {
     int n=14,i,j,z,max,min,max_arrival_time,out[14],arrival[14],required[14]={0,0,0,0,0,0,0,0,0,0,0,0,29,29},slack[14];
     char nodes[] = {'G','I','A','B','D','H','K','J','C','L','E','M','F'};
     required[12]=required[13]=29;
     printf("\nEnter the number of nodes:");
     scanf("%d",&n);
     num=n;
FILE *fp, *output;
	fp=fopen("inputs.txt","r");
	while (!feof(fp)){

		for(i=0;i<14;i++){
			for(j=0;j<14;j++){
				fscanf(fp,"%d ",&a[i][j]);
				tran[i][j]=a[i][j];
					}
}
}
            fclose( fp );
            for(i=0;i<14;i++){
		for(j=0;j<14;j++){

            printf("%d ",a[i][j]);
        }
        printf("\n");
		}
 /*  printf("\nEnter the matrix: \n");
     for(i=0;i<n;i++)
      {
         for(j=0;j<n;j++)
         {
            scanf("%d",&original[i][j]);	/* Input adjacency matrix here */
         /*   a[i][j]=original[i][j];
			tran[i][j]=a[i][j];
         }
      }
     for(i=0;i<n;i++)
      {
         for(j=0;j<n;j++)
         {
            printf("2%d\t",tran[i][j]);
         }
		 printf("\n");
      }

     printf("\nInput matrix is:\n" ); /* Print out the input matrix for your double check */

    /* printf("\n\nCalculate the arrival time: \n");*/

     for(i=0;i<n;i++)
      {
        max=getmaxcol(i);
        for(j=0;j<n;j++){
         if(a[i][j]!=0)
           a[i][j]=(a[i][j]+max);
       }
      }


/*=================== Required Time and Slack Calculation to be filled ==================*/

      for(i=13;i>1;i--)
      {
        min=getmincol(i);
        for(j=0;j<n;j++)
		{
         if(tran[j][i]!=0)
		 {
         		if(required[j]!=0&&required[j]<required[i]-min)
         			break;
         		else
           			required[j]=(required[i]-min);

         }
        }
	  }

/*=================== Printing the Results ========================*/

/* 5. The arrival time is already printed by this program. You have to print the required time and slack time similarly*/

    max=0;
    min=10000;

    for(i=1;i<n;i++)
     {
         for(j=0;j<n;j++)
         {
            if(a[j][i] > max)
            max=a[j][i];
         }

         arrival[i-1]=max;
         max=0;
     }

    output=fopen("outputs.txt","w");
    fprintf(output,"\n\nFinal Results:\nNodes         Arrival      Required        Slack\n");
     for(i=0;i<n-1;i++)
	 {
     	slack[i]=required[i+1]-arrival[i];
       fprintf(output,"  %c\t\t%d\t\t%d\t\t%d\n",nodes[i],arrival[i],required[i+1],slack[i]);

     }
     fclose(output);
 }




 /*=============================== Sub-functions ===============================*/
 /* Get the maximum value from a matrix column with the specified column number */
 int getmaxcol(int colnum)
  {
    int i,max;

     max=a[0][colnum];
    for(i=1;i<num;i++)
          if(a[i][colnum] > max)
          max = a[i][colnum];

     return max;
  }



int getmincol(int colnum)
  {
  	int i, min;
  	if(tran[0][colnum]==0)
	  min=100;
	else min=tran[0][colnum];
  	for (i=0;i<num;i++){
  		if(tran[i][colnum]!=0&&tran[i][colnum]<min)
  		min=tran[i][colnum];
	  }
	  return min;
  }

