#include "eserv/http.h"
#include "cgi_custom.h"

int main()
{
	char buf[16];

	ex_init();
	register_cgi();

	while (scanf("%16s", buf) > 0) {
		if (strncmp("quit", buf, 4) == 0)
			break;
		ex_sleep(200);
	}

	ex_uninit();

	return 0;
}
