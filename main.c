#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


/*----------------Structures--------------*/

struct layersRef
{
    char *text;
    struct layersRef *next;
}*firstLR, *firstDR;

struct layerData
{
    char *title;
    char *abstract;
    struct layerData *next;
}*firstLD;


/*----------GLOBAL VARIABLES---------*/
char *url = "http://test1:test1@www.idehn.tec.ac.cr/geoserver/rest/layers.xml";
char *docName = "layers.txt";
char *child = "layer";
char *subChild = "atom";


/*------------------------METHODS-----------------------*/

void fillLayerReferenceList(char *link, struct layersRef **list)
{
    struct layersRef *newNode = (struct layersRef*) malloc(sizeof(struct layersRef));
    newNode->text = link;
    newNode->next = NULL;

    if(*list == NULL)
        *list = newNode;
    else
    {
        struct layersRef *temp = *list;
        while(temp -> next != NULL)
            temp = temp -> next;
        temp -> next = newNode;
    }
}

void printList(struct layersRef **list)
{
    if(*list == NULL)
        printf("List is empty!!");
    else
    {
        struct layersRef * temp = *list;
        while(temp != NULL)
        {
            printf("Link: %s\n", temp->text);
            temp = temp -> next;
        }
    }
}

void fileUpload(char* url)
{
    CURL *curl;
    CURLcode res;
    char *urlCp = url;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, urlCp);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        FILE *file = fopen( "layers.txt", "w");

        curl_easy_setopt( curl, CURLOPT_WRITEDATA, file) ;

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_cleanup(curl);

        fclose(file);
    }
}

/*void parseNode (xmlDocPtr doc, xmlNodePtr cur, char *subchild)
{
	xmlChar *link;
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)subchild)))
	    {
		    link = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    printf("Link: %s\n", link);
		    xmlFree(link);
 	    }
        cur = cur->next;
	}
    return;
}*/

void getReference (xmlDocPtr doc, xmlNodePtr cur, int pos) {

	xmlChar *link;
	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
        if (!(xmlStrcmp(cur->name, "link"))){
            link = xmlGetProp(cur, "href");
            if(pos == 0)
                fillLayerReferenceList((char *)link, &firstLR);
            if(pos == 1)
                fillLayerReferenceList((char *)link, &firstDR);
            //xmlFree(link);
        }
        cur = cur->next;
	}
	return;
}


void parseDoc(char *docname, int pos) {

	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(docname);

	if (doc == NULL ) {
		printf("Document not parsed successfully. \n");
		return;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		printf("empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "layers")) {
		printf("document of the wrong type, root node different");
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		if (!(xmlStrcmp(cur->name, (const xmlChar *)child)))
		{
			getReference (doc, cur, pos);
		}

        cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}


int main() {
    //printf("Link: %s", url);

    //Call to the method that charge the url content to a file with all the layers.
    //fileUpload(url);

    //parseDoc(docName, 1);
    return 0;
}

