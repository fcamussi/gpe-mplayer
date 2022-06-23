
#include "window.h"
#include <stdlib.h>
//#include <gpe/init.h>



int main(int argc, char *argv[])
{
	//gpe_application_init(&argc, &argv);
	gtk_init(&argc, &argv);
	
	setenv("G_FILENAME_ENCODING", "iso-8859-1", 1);

	Window_Create();
	
	gtk_main();

	return 0;
}
