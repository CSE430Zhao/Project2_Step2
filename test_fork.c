#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h> 

int main()
{
//    int i = 0;
    int pid = 0;
    
/*    while(i < 5)
    for(i = 0; i < 6; i++)
    {
        pid = fork();  
        printf("Forking\n");
        i++;
    }
*/
    pid = fork();  
    printf("Forking\n");
    pid = fork();  
    printf("Forking\n");
    pid = fork();  
    printf("Forking\n");
    pid = fork();  
    printf("Forking\n");
    pid = fork();  
    printf("Forking\n");
    
    while(1) {}
    return 0;
}
