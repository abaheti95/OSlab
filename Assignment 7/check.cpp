#include <string>
#include <iostream>
#include <sstream>
#include <htmlcxx/html/ParserDom.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>

using namespace std;
using namespace htmlcxx;
string master = "http://cse.iitkgp.ac.in/";

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


int main ()
{
	string url = master;
	string readBuffer;
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}curl_global_cleanup();

	string html = readBuffer;
	HTML::ParserDom parser;
	tree <HTML::Node> dom = parser.parseTree (html);


	// Output tree hyperlink node
	tree <HTML::Node>::iterator it = dom.begin ();
	tree <HTML::Node>::iterator end = dom.end ();

	for (; it != end; ++it)
	{
		if (strcasecmp (it->tagName().c_str (), "a") == 0)
		{
			it-> parseAttributes();
			string href = it->attribute("href").second;
			// cout << it->attribute("href").second << endl;

			//mutex lock here
			size_t found = href.find("http");
			if(found == 0){

			}else{
				if((strcmp(href.c_str(), "") == 0 )|| (href.find("#") == 0)){
					// continue;
				}else{
					href = master + href;

					cout << href << endl;
				}
			}

		}
	}
}