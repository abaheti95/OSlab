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

CURLcode download_page(string url)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
		res = curl_easy_perform(curl);

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	return res;
}

int main ()
{
// Parsing for some HTML code
	string html = "<html> <body> hey </ body> </ html>";
	HTML::ParserDom parser;
	tree <HTML::Node> dom = parser.parseTree (html);
// Output the whole grain DOM tree
	cout << dom << endl;
// Output tree hyperlink node
	tree <HTML::Node>::iterator it = dom.begin ();
	tree <HTML::Node>::iterator end = dom.end ();
	for (; it != end; ++it)
	{
		if (strcasecmp (it->tagName().c_str (), "A") == 0)
		{
			it-> parseAttributes();
			cout << it->attribute("href").second << endl;
		}
	}
// Output all text nodes
	it = dom.begin ();
	end = dom.end ();
	for (; it != end; ++it)
	{
		if ((!it->isTag()) && (! it->isComment()))
		{
			cout << it-> text ();
		}
	}
	cout << endl;
}