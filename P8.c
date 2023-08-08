#include <dirent.h> 
#include <stdio.h> 
#include <string.h>
#include <unistd.h>

int main(void) 
{
  DIR *d;
  struct dirent *dir;
  
  int contador = 0;
  
  printf("Imprimir P8 %d\n",getpid());
  
  while(1)
  {
  	char caminhoCompleto[1000] = "/home/matheus/Documents/INF-1316-Trab1-2023.1/Arquivos_InputOutput/P8/";
    	d = opendir("./Arquivos_InputOutput/P8");
  	if (d) 
  	{
    		while ((dir = readdir(d)) != NULL) 
    		{
    			if(strstr(dir->d_name,".txt"))
			{
    				strcat(caminhoCompleto,dir->d_name);
    				int status = remove(caminhoCompleto);

    				if(status == 0)
    				{
    					contador++;
    					printf("P8:Recebeu %d Arquivos Texto\n",contador);
    				}
    			}
    		}
    		
    	}
    	closedir(d);
    	
    	if(contador >= 15)
    		break;
    	
    	//sleep(1);
  }
    
  return(0);
}
