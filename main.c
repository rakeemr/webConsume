#include <stdio.h>
#include <stdlib.h>
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
char *prefix = "http://test1:test1@";
char *docName = "layers.txt";


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

void fillLayerDataList(char *title, char *abstract, struct layerData **list)
{
    struct layerData *newNode = (struct layerData*) malloc(sizeof(struct layerData));
    newNode->title = title;
    newNode->abstract = abstract;
    newNode->next = NULL;

    if(*list == NULL)
        *list = newNode;
    else
    {
        struct layerData *temp = *list;
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

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        FILE *file = fopen(docName, "w");

        curl_easy_setopt( curl, CURLOPT_WRITEDATA, file) ;

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_cleanup(curl);

        fclose(file);
    }
}


void getReference (xmlDocPtr doc, xmlNodePtr cur, int pos) {

	xmlChar * link;
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


void parseDoc(char *docname, char *root, char *subChild, int pos) {

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

	if (xmlStrcmp(cur->name, (const xmlChar *) root)) {
		printf("document of the wrong type, root node different");
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		if (!(xmlStrcmp(cur->name, (const xmlChar *)subChild)))
		{
			getReference (doc, cur, pos);
		}

        cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}


void getLayerDataReference()
{
    struct layersRef * temp = firstLR;
    int i = 0;
    char *subURL;
    while(temp->next != NULL)
    {
        subURL = (char *) malloc(19+strlen(temp->text));
        char *subString = strndup(temp->text+7, strlen(temp->text)-7);
        strcpy(subURL, prefix);
        strcat(subURL, subString);
        fileUpload(subURL);
        parseDoc(docName, "layer", "resource", 1);
        //printf("URL: %s\n", subURL);
        printf("\n\t\t\tEND OF FILE\n\n");
        i+=1;
        temp = temp->next;
    }
}

void parseDoc2(char *docname, char *root, char *subChild1, char *subChild2)
{
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

	if (xmlStrcmp(cur->name, (const xmlChar *) root)) {
		printf("document of the wrong type, root node different");
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
        if (!(xmlStrcmp(cur->name, (const xmlChar *)"title")))
        {
            xmlChar *title;
            title = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
            printf("\n\tTitle: %s\n", title);
        }
        if (!(xmlStrcmp(cur->name, (const xmlChar *)"abstract")))
        {
            xmlChar *abstract;
            abstract = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
            printf("\n\tAbstract: %s\n", abstract);
        }

        cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}

void getLayerConection()
{
    struct layersRef * temp = firstDR;
    int i = 0;
    char *subURL;
    while(temp->next != NULL)
    {
        subURL = (char *) malloc(19+strlen(temp->text));
        char *subString = strndup(temp->text+7, strlen(temp->text)-7);
        strcpy(subURL, prefix);
        strcat(subURL, subString);
        fileUpload(subURL);
        parseDoc2(docName, "featureType", "title", "abstract");
        printf("\n\t\t\tEND OF FILE\n\n");
        i+=1;
        temp = temp->next;
    }
}


int main() {

    //Call to the method that charge the url content to a file with all the layers.
    fileUpload("http://test1:test1@www.idehn.tec.ac.cr/geoserver/rest/layers.xml");

    parseDoc(docName, "layers", "layer", 0);
    //printList(&firstLR);
    printf("\n---------------------------------------------------------------\n");
    getLayerDataReference();
    //printf("\n---------------------------------------------------------------\n");
    //printList(&firstDR);

    printf("\n---------------------------------------------------------------\n");
    getLayerConection();

    return 0;
}

