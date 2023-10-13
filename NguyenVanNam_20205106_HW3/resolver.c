#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define file_name "htmlSaver.txt"
#define link_file "links.csv"
#define text_file "text.csv"
#define video_file "video.csv"
#define MAX_URL_LENGTH 2048

// write to file from url for reading
void write2File(const char *url)
{
    CURL *curlHandle = curl_easy_init();
    curl_easy_setopt(curlHandle, CURLOPT_URL, url);
    FILE *file = fopen(file_name, "w+");
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, file);
    curl_easy_perform(curlHandle);
    fclose(file);
    curl_easy_cleanup(curlHandle);
}

// get full url from input
char *getFullUrl(const char *url)
{
    char *fullUrl = malloc(strlen(url) + strlen("https://") + 1);
    strcpy(fullUrl, "https://");
    strcat(fullUrl, url);
    return fullUrl;
}

// extract hyperlinks from text file return list of links
void extract_hyperlinks(const char *html_file_name, char ***links, int *num_links, regex_t regrex)
{
    char *html = NULL;
    size_t html_size = 0;
    FILE *file = fopen(html_file_name, "r");

    if (file == NULL)
    {
        perror("Error opening HTML file");
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        size_t len = strlen(buffer);
        char *new_html = realloc(html, html_size + len + 1);
        if (new_html == NULL)
        {
            perror("Memory allocation error");
            free(html);
            fclose(file);
            return;
        }
        html = new_html;
        strcpy(html + html_size, buffer);
        html_size += len;
    }
    fclose(file);

    regmatch_t pmatch[2];
    *num_links = 0;
    char **link_list = NULL;
    const char *cursor = html;

    while (regexec(&regrex, cursor, 2, pmatch, 0) == 0)
    {
        int start = pmatch[1].rm_so;
        int end = pmatch[1].rm_eo;

        char *link = malloc(end - start + 1);
        strncpy(link, cursor + start, end - start);
        link[end - start] = '\0';

        char **new_link_list = realloc(link_list, (*num_links + 1) * sizeof(char *));
        if (new_link_list == NULL)
        {
            perror("Memory allocation error");
            for (int i = 0; i < *num_links; i++)
            {
                free(link_list[i]);
            }
            free(link_list);
            free(html);
            regfree(&regrex);
            return;
        }
        link_list = new_link_list;
        link_list[(*num_links)++] = link;

        cursor += end;
    }
    *links = link_list;
    free(html);
    regfree(&regrex);
}

// check if link is relative or not
int isRelativeLink(const char *link)
{
    return link[0] == '/';
}

// add host name to relative link
void addHostName2RelativeLink(char **links, const char *fullUrl)
{
    for (int i = 0; i < sizeof(links); i++)
    {
        char *link = links[i];
        char *newLink = malloc(strlen(link) + strlen(fullUrl) + 1);
        strcpy(newLink, fullUrl);
        strcat(newLink, link);
        links[i] = newLink;
    }
}

// compare two string, return 1 if a > b, 0 if a == b, -1 if a < b
int cmp(char *a, char *b)
{
    int len_a = strlen(a);
    int len_b = strlen(b);
    int min_len = len_a < len_b ? len_a : len_b;
    for (int i = 0; i < min_len; i++)
    {
        if (a[i] > b[i])
        {
            return 1;
        }
        else if (a[i] < b[i])
        {
            return -1;
        }
    }
    if (len_a > len_b)
    {
        return 1;
    }
    else if (len_a < len_b)
    {
        return -1;
    }
    return 0;
}

// sort list of links using merge sort
void sort(char **links, int n)
{
    if (n <= 1)
    {
        return;
    }
    int mid = n / 2;
    char **left = malloc(mid * sizeof(char *));
    char **right = malloc((n - mid) * sizeof(char *));
    for (int i = 0; i < mid; i++)
    {
        left[i] = links[i];
    }
    for (int i = mid; i < n; i++)
    {
        right[i - mid] = links[i];
    }
    sort(left, mid);
    sort(right, n - mid);
    int i = 0, j = 0, k = 0;
    while (i < mid && j < n - mid)
    {
        if (cmp(left[i], right[j]) == -1)
        {
            links[k++] = left[i++];
        }
        else
        {
            links[k++] = right[j++];
        }
    }
    while (i < mid)
    {
        links[k++] = left[i++];
    }
    while (j < n - mid)
    {
        links[k++] = right[j++];
    }
    free(left);
    free(right);
}

// Check if a string is a valid IP address or not
int isIpAddress(char *hostnameOrIp)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, hostnameOrIp, &(sa.sin_addr));
    return result != 0;
}

void getAllAliases(struct hostent *host)
{
    struct in_addr **addr_list;
    int i;
    if (host == NULL)
    {
        printf("Not found information\n");
        exit(1);
    }
    addr_list = (struct in_addr **)host->h_addr_list;
    for (i = 0; addr_list[i] != NULL; i++)
    {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
}

/*
Print host information
*/
void printHostInfo(struct hostent *host)
{
    int i;
    if (host == NULL)
    {
        printf("Not found information\n");
        exit(1);
    }
    printf("Official name: %s\n", host->h_name);
    printf("Alias name:\n");
    for (i = 0; host->h_aliases[i] != NULL; i++)
    {
        printf("%s\n", host->h_aliases[i]);
    }
}

/*
Print IP addresses of host
*/
void printIPAddresses(struct hostent *host)
{
    struct in_addr **addr_list;
    int i;
    if (host == NULL)
    {
        printf("Not found information\n");
        exit(1);
    }
    addr_list = (struct in_addr **)host->h_addr_list;
    printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
    printf("Alias IP:\n");
    for (i = 0; addr_list[i] != NULL; i++)
    {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <URL|IP address>\n", argv[0]);
        return 1;
    }
    char *hostnameOrIp = argv[1];
    struct hostent *host;
    char *fullUrl;
    // check if input is IP address or hostname and get host information
    if (isIpAddress(hostnameOrIp))
    {
        host = gethostbyaddr(hostnameOrIp, strlen(hostnameOrIp), AF_INET);
        printHostInfo(host);
        fullUrl = getFullUrl(host->h_name);
    }
    else
    {
        host = gethostbyname(hostnameOrIp);
        printIPAddresses(host);
        fullUrl = getFullUrl(hostnameOrIp);
    }
    // printf("Full url: %s\n", fullUrl);
    write2File(fullUrl);
    char **final_links = NULL;
    int final_count = 0;
    char **final_texts = NULL;
    int final_count_text = 0;
    char **final_videos = NULL;
    int final_count_video = 0;
    regex_t hl_regex_links;
    regex_t hl_regex_text;
    regex_t hl_regex_video;
    // compile regex
    regcomp(&hl_regex_links, "<a\\s+href=\"([^\"]+)\"[^>]*>", REG_ICASE | REG_EXTENDED);
    regcomp(&hl_regex_text, "<[hH][23][^>]*><a[^>]*>([^<]*)</a></[hH][23]>", REG_ICASE | REG_EXTENDED);
    regcomp(&hl_regex_video, "<video\\s+src=\"([^\"]+)\"[^>]*>", REG_ICASE | REG_EXTENDED);
    extract_hyperlinks(file_name, &final_links, &final_count, hl_regex_links);
    extract_hyperlinks(file_name, &final_texts, &final_count_text, hl_regex_text);
    extract_hyperlinks(file_name, &final_videos, &final_count_video, hl_regex_video);
    // sort list of links
    sort(final_links, final_count);
    sort(final_texts, final_count_text);
    sort(final_videos, final_count_video);
    // write to file
    FILE *file = fopen(link_file, "w+");
    fprintf(file, "Links get from %s\n", fullUrl);
    for (int i = 0; i < final_count; i++)
    {
        if (isRelativeLink(final_links[i]))
        {
            addHostName2RelativeLink(&final_links[i], fullUrl);
        }
        fprintf(file, "%s\n", final_links[i]);
    }
    fclose(file);
    file = fopen(text_file, "w+");
    fprintf(file, "Texts get from %s\n", fullUrl);
    for (int i = 0; i < final_count_text; i++)
    {
        fprintf(file, "%s\n", final_texts[i]);
    }
    fclose(file);
    file = fopen(video_file, "w+");
    fprintf(file, "Videos get from %s\n", fullUrl);
    for (int i = 0; i < final_count_video; i++)
    {
        fprintf(file, "%s\n", final_videos[i]);
    }
    fclose(file);
    return 0;
}