#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char **argv) {

	char arg1[10];
	char arg2[10];
	char arg3[10];

	int i = 5;
	// Create string for our first command line argument to 
	// the executable that we will exec	
	snprintf(arg1,10,"%d", i);

	// Create string for the second command line arg
	snprintf(arg2,10,"%s","-n");

	// Create string for the third command line arg
	snprintf(arg3,10,"%d",7);


	// lets fork off a copy
	if (fork() == 0) {
		// In child
		 
		// So we have our three command line arguments
		// now let us make exec call with our three args
		// In our case, acts as if executing:
		// ./worker 5 -n 7
		execlp("./worker","./worker",arg1,arg2,arg3,(char *)NULL);

		// If we get here, exec failed
		fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
		exit(-1);
	}

	// Rest of stuff parent needs to do
	
	return 0;
}
