#ifndef __CGI_CUSTOM_H__
#define __CGI_CUSTOM_H__

#include "eserv/cgi.h"

extern int cgi_page_sum(ExHttp *pHttp);
extern int cgi_page_txt(ExHttp *pHttp);
extern int cgi_page_login(ExHttp *pHttp);
extern int cgi_page_gallery(ExHttp *pHttp);

/* customized page handler declare here */
cgi_page cgi_pages[] = {
	{
		.name = "sum.cgi",
		.callback = cgi_page_sum,
	},

	{
		.name = "txt.cgi",
		.callback = cgi_page_txt,
	},

	{
		.name = "login.cgi",
		.callback = cgi_page_login,
	},

	{
		.name = "gallery.cgi",
		.callback = cgi_page_gallery,
	},
};

void register_cgi(){
	size_t i;
	for (i = 0; i < sizeof(cgi_pages) / sizeof(cgi_page); i++)
		cgi_page_add(cgi_pages[i].name,
			cgi_pages[i].callback);

}

#endif

