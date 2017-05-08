#include<math.h>
#include<stdio.h>
#include<conio.h>

int getmaxcol(int);
int getmincol(int);

int num=10; /*num will indicate the number of nodes = 10*/
int original[10][10],a[10][10],tran[10][10];	/*	original[10][10]:	record of input adjacency matrix obtained from topology

													a[10][10]:			adjacency matrix for arrival time calculation

													tran[10][10]:		new transformed adjacency matrix used for
																		required time calculation

												 */


int main(int argc, char **argv)
 {
     int n=10,i,j,z,max,min,max_arrival_time,out[10],arrival[10],required[10]={0,0,0,0,0,23,0,0,23,0},slack[10];
     char nodes[] = {'A','B','C','D','E','F','G','H','I'};
     required[6]=required[9]=23;
     printf("\nEnter the number of nodes:"); /*It will take the number of nodes from the user*/
     scanf("%d",&n);
     num=n;
     printf("\nEnter the matrix: \n"); /* It will ask user to enter the matrix */
     for(i=0;i<n;i++)
      {
         for(j=0;j<n;j++)
         {
            scanf("%d",&original[i][j]);	/* Input adjacency matrix here */
            a[i][j]=original[i][j];
			tran[i][j]=a[i][j];
         }
      }



/*Calculation for the arrival time*/

      for(i=0;i<n;i++)
      {
        max=getmaxcol(i);
        for(j=0;j<n;j++){
         if(a[i][j]!=0)
           a[i][j]=(a[i][j]+max);
       }
      }


/*Calculation for the required time*/

      for(i=9;i>1;i--)
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

    printf("\n The input matrix is\n");
    for(i=0;i<n;i++)
    {
       for(j=0;j<n;j++)
        {
         printf("%d    ", original[i][j]);
        }
        printf("\n");
     }
/*Code for printing all the outputs*/

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


    printf("\n\nFinal Results:\nNodes         Arrival      Required        Slack\n");  /*This will print the number of nodes, arrival time, required time and slack time as an output */
     for(i=0;i<n-1;i++)
	 {
     	slack[i]=required[i+1]-arrival[i];    /*Here slack time is calculated*/
       printf("  %c\t\t%d\t\t%d\t\t%d\n",nodes[i],arrival[i],required[i+1],slack[i]); /*To print in the required fashion*/

     }

 }




 /*=============================== Sub-functions ===============================*/


/* The function for finding the maximum value from the matrix column */
 int getmaxcol(int colnum)
  {
    int i,max;

     max=a[0][colnum];
    for(i=1;i<num;i++)
          if(a[i][colnum] > max)
          max = a[i][colnum];

     return max;
  }


/* The function for finding the minimum value from the matrix column */
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

