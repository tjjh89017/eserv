#ifndef __CGI_CUSTOM_H__
#define __CGI_CUSTOM_H__

#include "cgi.h"

extern int cgi_page_sum(ExHttp *pHttp);
extern int cgi_page_txt(ExHttp *pHttp);
extern int cgi_page_login(ExHttp *pHttp);
extern int cgi_page_gallery(ExHttp *pHttp);

extern int cgi_hash_test(ExHttp *pHttp);

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
	{
		.name = "hash_test.cgi",
		.callback = cgi_hash_test,
	},
};
#endif
