#include <stdio.h>
#include <stdlib.h>

int randint(int min, int max){
    return min+rand()%((max-min)+1);
}

int existe(int k,int v[],int n){
	for(int i=0;i<n;i++){
		if(v[i]==k){
			return 1;
		}
	}
	return 0;
	
}



int main()
{

    int participantes;
    printf("Informe o numero de participantes: ");
    scanf("%d",&participantes);
    
    printf("Informe o nome dos participantes: \n");
    
    int count=0;
    char names[participantes][50];
    
    for(int i=0;i<participantes;i++){
        scanf("%s",names[i]);
        
    }

	int retirados[participantes];
    printf("\nOs retirados sao:\n");
	int i=0,k=0,rt=0;
	
    while(i<participantes){
        rt=randint(0,participantes-1);
		if(existe(rt,retirados,i)==1){
			continue;
		}
		else{
			retirados[k]=rt;
		}
		if(i==retirados[k]){
			continue;
		}
		printf("%s - %s\n",names[i],names[retirados[k]]);
        i++;
		k++;
        
    }
    
	system("pause");
    
    
    

    return 0;
}