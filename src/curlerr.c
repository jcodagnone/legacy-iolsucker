#include <curl/curl.h>

const char *
curl_strerror(CURLcode code)
{	const char *r;

	switch(code)
	{
		case CURLE_OK:
			r = "CURLE_OK";
			break;
		case CURLE_UNSUPPORTED_PROTOCOL:
			r = "CURLE_UNSUPPORTED_PROTOCOL";
			break;
		case CURLE_FAILED_INIT:
			r = "CURLE_FAILED_INIT";
			break;
		case CURLE_URL_MALFORMAT:
			r = "CURLE_URL_MALFORMAT";
			break;
		case CURLE_URL_MALFORMAT_USER:
			r = "CURLE_URL_MALFORMAT_USER";
			break;
		case CURLE_COULDNT_RESOLVE_PROXY:
			r = "CURLE_COULDNT_RESOLVE_PROXY";
			break;
		case CURLE_COULDNT_RESOLVE_HOST:
			r = "CURLE_COULDNT_RESOLVE_HOST";
			break;
		case CURLE_COULDNT_CONNECT:
			r = "CURLE_COULDNT_CONNECT";
			break;
		case CURLE_FTP_WEIRD_SERVER_REPLY:
			r = "CURLE_FTP_WEIRD_SERVER_REPLY";
			break;
		case CURLE_FTP_ACCESS_DENIED:
			r = "CURLE_FTP_ACCESS_DENIED";
			break;
		case CURLE_FTP_USER_PASSWORD_INCORRECT:
			r = "CURLE_FTP_USER_PASSWORD_INCORRECT";
			break;
		case CURLE_FTP_WEIRD_PASS_REPLY:
			r = "CURLE_FTP_WEIRD_PASS_REPLY";
			break;
		case CURLE_FTP_WEIRD_USER_REPLY:
			r = "CURLE_FTP_WEIRD_USER_REPLY";
			break;
		case CURLE_FTP_WEIRD_PASV_REPLY:
			r = "CURLE_FTP_WEIRD_PASV_REPLY";
			break;
		case CURLE_FTP_WEIRD_227_FORMAT:
			r = "CURLE_FTP_WEIRD_227_FORMAT";
			break;
		case CURLE_FTP_CANT_GET_HOST:
			r = "CURLE_FTP_CANT_GET_HOST";
			break;
		case CURLE_FTP_CANT_RECONNECT:
			r = "CURLE_FTP_CANT_RECONNECT";
			break;
		case CURLE_FTP_COULDNT_SET_BINARY:
			r = "CURLE_FTP_COULDNT_SET_BINARY";
			break;
		case CURLE_PARTIAL_FILE:
			r = "CURLE_PARTIAL_FILE";
			break;
		case CURLE_FTP_COULDNT_RETR_FILE:
			r = "CURLE_FTP_COULDNT_RETR_FILE";
			break;
		case CURLE_FTP_WRITE_ERROR:
			r = "CURLE_FTP_WRITE_ERROR";
			break;
		case CURLE_FTP_QUOTE_ERROR:
			r = "CURLE_FTP_QUOTE_ERROR";
			break;
		case CURLE_HTTP_NOT_FOUND:
			r = "CURLE_HTTP_NOT_FOUND";
			break;
		case CURLE_WRITE_ERROR:
			r = "CURLE_WRITE_ERROR";
			break;
		case CURLE_MALFORMAT_USER:
			r = "user name is illegally specified";
			break;
		case CURLE_FTP_COULDNT_STOR_FILE:
			r = "failed FTP upload";
			break;
		case CURLE_READ_ERROR:
			r = "could open/read from file";
			break;
		case CURLE_OUT_OF_MEMORY:
			r = "CURLE_OUT_OF_MEMORY";
			break;
		case CURLE_OPERATION_TIMEOUTED:
			r = "the timeout time was reached";
			break;
		case CURLE_FTP_COULDNT_SET_ASCII:
			r = "TYPE A failed";
			break;
		case CURLE_FTP_PORT_FAILED:
			r = "FTP PORT operation failed";
			break;
		case CURLE_FTP_COULDNT_USE_REST:
			r = "the REST command failed";
			break;
		case CURLE_FTP_COULDNT_GET_SIZE:
			r = "the SIZE command failed";
			break;
		case CURLE_HTTP_RANGE_ERROR:
			r = "RANGE \"command\" didn't work";
			break;
		case CURLE_HTTP_POST_ERROR:
			r = "CURLE_HTTP_POST_ERROR";
			break;
		case CURLE_SSL_CONNECT_ERROR:
			r = "wrong when connecting with SSL";
			break;
		case CURLE_BAD_DOWNLOAD_RESUME:
			r = "couldn't resume download";
			break;
		case CURLE_FILE_COULDNT_READ_FILE:
			r = "CURLE_FILE_COULDNT_READ_FILE";
			break;
		case CURLE_LDAP_CANNOT_BIND:
			r = "CURLE_LDAP_CANNOT_BIND";
			break;
		case CURLE_LDAP_SEARCH_FAILED:
			r = "CURLE_LDAP_SEARCH_FAILED";
			break;
		case CURLE_LIBRARY_NOT_FOUND:
			r = "CURLE_LIBRARY_NOT_FOUND";
			break;
		case CURLE_FUNCTION_NOT_FOUND:
			r = "CURLE_FUNCTION_NOT_FOUND";
			break;
		case CURLE_ABORTED_BY_CALLBACK:
			r = "CURLE_ABORTED_BY_CALLBACK";
			break;
		case CURLE_BAD_FUNCTION_ARGUMENT:
			r = "CURLE_BAD_FUNCTION_ARGUMENT";
			break;
		case CURLE_BAD_CALLING_ORDER:
			r = "CURLE_BAD_CALLING_ORDER";
			break;
		case CURLE_HTTP_PORT_FAILED:
			r = "HTTP Interface operation failed";
			break;
		case CURLE_BAD_PASSWORD_ENTERED:
			r = "my_getpass() returns fail";
			break;
		case CURLE_TOO_MANY_REDIRECTS:
			r = "- catch endless re-direct loops";
			break;
		case CURLE_UNKNOWN_TELNET_OPTION:
			r = "User specified an unknown option";
			break;
		case CURLE_TELNET_OPTION_SYNTAX:
			r = "- Malformed telnet option";
			break;
		case CURLE_OBSOLETE:
			r = "removed after 7.7.3";
			break;
		case CURLE_SSL_PEER_CERTIFICATE:
			r = "peer's certificate wasn't ok";
			break;
		case CURLE_GOT_NOTHING:
			r = "when this is a specific error";
			break;
		case CURLE_SSL_ENGINE_NOTFOUND:
			r = "SSL crypto engine not found";
			break;
		case CURLE_SSL_ENGINE_SETFAILED:
			r = "can not set SSL crypto engine";
			break;
		case CURLE_SEND_ERROR:
			r = "failed sending network data";
			break;
		case CURLE_RECV_ERROR:
			r = "failure in receiving network data";
			break;
		case CURLE_SHARE_IN_USE:
			r = "share is in use";
			break;
		case CURLE_SSL_CERTPROBLEM:
			r = "problem with the local certificate";
			break;
		case CURLE_SSL_CIPHER:
			r = "couldn't use specified cipher";
			break;
		case CURLE_SSL_CACERT:
			r = "problem with the CA cert (path?)";
			break;
		case CURLE_BAD_CONTENT_ENCODING:
			r = "Unrecognized transfer encoding";
			break;
		default:
			r = "";
	}

	return r;
}
