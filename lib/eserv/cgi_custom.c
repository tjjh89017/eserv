#include "cgi.h"
#include "session.h"

int cgi_page_sum(ExHttp *pHttp)
{
	const char *lAdd, *rAdd;
	int sum;
	char buf[32];
	printf("\n--add.cgi--\n");

	print_param(pHttp);
	lAdd = get_param_info(pHttp, "lAdd");
	rAdd = get_param_info(pHttp, "rAdd");
	sum = atoi(lAdd) + atoi(rAdd);

	sprintf(buf, "%d", sum);
	ex_send_msg(pHttp, NULL, buf, strlen(buf));
	return 0;
}

int cgi_page_txt(ExHttp *pHttp)
{
	printf("\n--txt.cgi--\n");
	print_param(pHttp);
	ex_send_file(pHttp, "hello.txt");

	return 0;
}

// session test

int cgi_page_login(ExHttp *pHttp)
{
	const char *smsg = "login success";
	const char *emsg = "login error";
	const char *gss = "get session success";
	const char *ss = "send session";
	//static const char *user = "eserv";
	//static const char *passwd = "passwd";

	const char *pRet = emsg;

	const char *pUser , *pPasswd, *pSession;
	printf("\n--login.cgi--\n");
	print_param(pHttp);

	pSession = sessionFromHeader(get_head_info(pHttp, "Cookie"));
	pUser = get_param_info(pHttp, "user");
	pPasswd = get_param_info(pHttp, "passwd");
	/*
	if (strcmp(user, pUser) == 0 && strcmp(passwd, pPasswd) == 0) {
		pRet = smsg;
	}
	*/
	//ex_send_msg(pHttp, NULL, pRet, strlen(pRet));
	void *data = sessionCheck(pSession);
	if(data != NULL){
		ex_send_msg(pHttp, NULL, (char*)data, strlen((char*)data));
	}
	else{
		char *session_id = sessionCreate(pUser, "something");
		pRet = ss;
		ex_send_msg_session(pHttp, NULL, pRet, strlen(pRet), session_id);
	}

	return 0;
}

int cgi_page_gallery(ExHttp *pHttp)
{
	static int count = 0;
	char buf[40];
	printf("\n--gallery.cgi--\n");

	print_param(pHttp);

	count++;
	if (count > 3) {
		sprintf(buf, "All of pictures are shown. Reset");
		count = 0;
	} else {
		sprintf(buf, "No. %d<br /><img src='%d.jpg' />", count, count);
	}
	ex_send_msg(pHttp, NULL, buf, strlen(buf));
	return 0;
}

/*
 * Test ex_hash_next if works
 */
int cgi_hash_test(ExHttp *pHttp){

	printf("\n---cgi_hash_test---\n");

	ex_hashmap *head_map = &pHttp->headMap;
	ex_hashlist *cList = ex_hash_next(head_map);
	
	while(cList != NULL){
		printf("%s:%s\n", (char*)cList->key, (char*)cList->value);
		cList = cList->next;
	}

	return 0;
}
