#define _XOPEN_SOURCE 600

#include <bits/stdc++.h>
#include <pthread.h>
#include <htmlcxx/html/ParserDom.h>
#include <curl/curl.h>

using namespace std;
using namespace htmlcxx;

#define NUMTHREADS 5




string master = "http://cse.iitkgp.ac.in";

queue <string>	toDo, toDoNext;
map<string, pair<int, int> > done;
int level = 1;
int maxlevel = 4;
pthread_mutex_t mutex;

pthread_barrier_t barrier1, barrier2;


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}



void parser(string url){
	// string url = master;
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
	tree <HTML::Node>::iterator it = dom.begin();
	tree <HTML::Node>::iterator end = dom.end();

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
				pthread_mutex_lock(&mutex);
				toDoNext.push(href);
				pthread_mutex_unlock(&mutex);
				// cout << href << endl;
			}else{
				if((strcmp(href.c_str(), "") == 0 )|| (href.find("#") == 0)){
					// continue;

				}else{

					if(href[0] == '/'){
						href = master + href;
					}else {
						href = master + "/" + href;	
					}

					

					pthread_mutex_lock(&mutex);
					toDoNext.push(href);
					pthread_mutex_unlock(&mutex);
					// cout << href << endl;
				}
			}
		}
	}
}


void levelIncrease(long id){
	pthread_barrier_wait(&barrier1);
	if(id == 1){
		pthread_mutex_lock(&mutex);
		while(!toDoNext.empty()){
			toDo.push(toDoNext.front());
			toDoNext.pop();
		}
		level++;
		cout << "Level Increased " << level << endl;
		pthread_mutex_unlock(&mutex);
	}
	pthread_barrier_wait(&barrier2);
}


void * crawler(void * args){
	long id = (long)args;

	while(1){
		pthread_mutex_lock(&mutex);


		if(level == maxlevel){
			pthread_mutex_unlock(&mutex);
			break;
		}else if(toDo.empty()){
			pthread_mutex_unlock(&mutex);
			cout << "Thread " << id << " found to-do queue empty" << endl;
			levelIncrease(id);

		}else{
			string url = toDo.front();
			toDo.pop();
			cout << "Thread " << id << " " << url << endl;
			if(done.find(url) == done.end()){ 	//url found in the done list

				done[url] = make_pair(id, level);
				pthread_mutex_unlock(&mutex);

				parser(url);

			}else{

			}
		}
	}

	return NULL;
}



int main(){
	pthread_t threads[NUMTHREADS];

	//init barriers here
	if(pthread_barrier_init(&barrier1, NULL, NUMTHREADS)){
        printf("Could not create a barrier 1\n");
        return -1;
    }

    if(pthread_barrier_init(&barrier2, NULL, NUMTHREADS)){
        printf("Could not create a barrier 2\n");
        return -1;
    }

    cout  << "Enter the number of levels:" << endl;
    cin >> maxlevel;

    toDo.push(master);

	for(int i = 1; i <= NUMTHREADS; i++){
		long x = i;
		if(pthread_create(&threads[i-1], NULL, &crawler, (void*)x))
        {
            printf("Could not create thread %d\n", i);
            return -1;
        }
	}

	for(int i = 1;i <= NUMTHREADS; i++){
		if(pthread_join(threads[i-1], NULL)){
			printf("Could not join thread %d\n", i);
			return -1;
		}
	}

	int size = done.size();

	cout << "depth thread url" << endl;
	for(map<string, pair<int, int> >::iterator it = done.begin(); it != done.end(); it++ ){
		// cout << it->first << " " << it->second << endl;
		cout << it->second.second << "\t " << it->second.first << "\t " << it->first << endl;
	}
	done.clear();


	while(!toDo.empty()){
		cout << toDo.front() << " 0" << endl;
		toDo.pop();
	}

	return 0;
}