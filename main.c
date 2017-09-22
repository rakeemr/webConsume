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

        /*curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));*/

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

        //printf("\n\t\t\tEND OF FILE\n\n");

        i+=1;
        temp = temp->next;
    }
}

void writeInFile(char *title, char * abstract)
{
    FILE * file = fopen("data.txt", "a");

    fprintf(file,"Title: %s\n", title);
    fprintf(file, "Description: %s", abstract);
    fprintf(file, "\n\t---------------------------------------\n");

    fclose(file);
}

void readInFile(char *name)
{
    FILE * file = fopen(name, "r");
    char line[256];

    if(!file)
        printf("\nThe document does not exist!\n");

    while (EOF != fscanf(file, "%[^\n]\n", line))
    {
         printf("%s\n", line);
    }

    fclose(file);
}

void getLayerData(xmlDocPtr doc, xmlNodePtr cur, char *child1, char *child2)
{
    char * title;
    char * abstract;
    char * finalAbs;

    if (!(xmlStrcmp(cur->name, (const xmlChar *)child1)))
        title = (char *) xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);

    if (!(xmlStrcmp(cur->name, (const xmlChar *)child2)))
    {
        abstract = (char *) xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
        char abs[strlen(abstract)];
        strcpy(abs, abstract);
        finalAbs = strtok(abs,"\n");
        writeInFile(title, finalAbs);
    }

}

void parseDoc2(char *docname, char *root, char *child1, char *child2)
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
        getLayerData(doc, cur, child1, child2);
        cur = cur->next;
	}

	xmlFreeDoc(doc);
	return;
}

void getLayerMetadata()
{
    struct layersRef * temp = firstDR;
    int i = 7;
    char *subURL;

    while(temp->next != NULL)
    {
        subURL = (char *) malloc(19+strlen(temp->text));
        char *subString = strndup(temp->text+i, strlen(temp->text)-i);
        strcpy(subURL, prefix);
        strcat(subURL, subString);

        fileUpload(subURL);
        parseDoc2(docName, "featureType", "title", "abstract");

        //printf("\n\t\t\tEND OF FILE\n\n");

        temp = temp->next;
    }
}

void printLayersList(struct layersRef ** list)
{
    struct layersRef * temp = * list;
    char * layers;
    int i = 95, j = 1;

    if(*list == NULL)
        printf("You need to run at least 1 time ./IProject -f before executing this option.");

    while(temp != NULL)
    {
        layers = (char *) malloc(strlen(temp->text));
        char * subLayers = strndup(temp->text+i, strlen(temp->text)-i);
        strcpy(layers, subLayers);

        printf("\n\t\t%i) %s\n", j, layers);

        j +=1;
        temp = temp -> next;
    }
}

void insertLayersByInput(char *layerToAdd)
{
    //printf("%s\n", layerToAdd);
    char * baseLayerDataUrl = (char *) malloc(1024);
    strcpy(baseLayerDataUrl, "http://test1:test1@www.idehn.tec.ac.cr/geoserver/rest/workspaces/geonode/datastores/datastore/featuretypes/");
    strcat(baseLayerDataUrl,layerToAdd);

    //printf("%s\n", baseLayerDataUrl);

    fileUpload(baseLayerDataUrl);
    parseDoc2(docName, "featureType", "title", "abstract");
}

void insertInput()
{

    printf("\nWrite the layers you want to save, Separate them with [, ]: \n>");

    char * layersToSave = (char *) malloc(1024);
    strcpy(layersToSave,"");
    scanf("%[^\n]", layersToSave);

    char str[strlen(layersToSave)];
    strcpy(str,layersToSave);
    const char s[2] = ", ";
    char *token;

    /* get the first token */
    token = strtok(str, s);

    /* walk through other tokens */
    while( token != NULL )
    {
        insertLayersByInput(token);
        //printf( " %s\n", token );

        token = strtok(NULL, s);
    }

    printf("\n\t\tPrinting file in Console...........\n");
    readInFile("data.txt");
}


void menuOptions(int option)
{
    if(option == 1)
    {
        printf("\n\t\tObtaining Data.......\n");
        fileUpload("http://test1:test1@www.idehn.tec.ac.cr/geoserver/rest/layers.xml");

        parseDoc(docName, "layers", "layer", 0);

        getLayerDataReference();
        getLayerMetadata();
        readInFile("data.txt");
    }

    if(option == 2)
    {
        fileUpload("http://test1:test1@www.idehn.tec.ac.cr/geoserver/rest/layers.xml");

        parseDoc(docName, "layers", "layer", 0);

        getLayerDataReference();

        printf("\n\nThe layers are: ");
        printLayersList(&firstDR);

        remove("data.txt");

        insertInput();

    }


}


int main(int argc, char * argv[]) {

    if(argc > 3)
    {
        printf("Usage [OPTIONS] of %s \n\t-f = ALL \n\t-p = Show results.txt content (Only if it exists)", argv[0]);
        return 0;
    }
    else if (argc == 3)
    {
        if ((!strcmp("-f", argv[1])) || (!strcmp("-f", argv[2])))
            menuOptions(1);
        if ((!strcmp("-p", argv[1])) || (!strcmp("-p", argv[2])))
        {
            printf("\n\t\tPrinting file in Console.........\n\n");
            readInFile("data.txt");
        }
    }
    else if (argc == 2)
    {
        if (!strcmp("-f", argv[1]))
            menuOptions(1);
        if (!strcmp("-p", argv[1]))
        {
            printf("\n\t\tPrinting file in Console.........\n\n");
            readInFile("data.txt");
        }
    }
    else
        menuOptions(2);

    //menuOptions(1);
    return 0;
}

