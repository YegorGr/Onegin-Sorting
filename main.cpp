#define NDEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#define HINT    "\x1b[93m"            //!< Sand color (yellow but light yellow)
#define SUCCESS "\x1b[38;5;46m"       //!< Bright green color (specially for success)
#define BOLD    "\x1b[1m"             //!< Very fat (like me)
#define RESET   "\x1b[0m"             //!< To reset style
#define PART_C  "\x1b[38;5;203m"      //!< Bright red color

typedef struct {
    char*  ptr_str;
    size_t len;
} String;

typedef struct {
    int     descriptor;
    size_t  size_array;
    char*   buffer;
    size_t  strings;
    String* index;      
    String* copyindex;  
    FILE*   file;
} FileInfo;

int      OpenAndGetName  (int argc, const char** argv);
FileInfo GetFileInfo     (int argc, const char** argv);
void     FreeMem         (FileInfo* ptr_data);

char*    GetFileName     (void);
int      OpenFile        (const char* filename);
size_t   FileSize        (int descriptor);
char*    CopyFromFile    (int descriptor, size_t size_array);
size_t   CountStrings    (char* buffer);

String*  StringsArray    (char* buffer, size_t strings);
String*  CopyIndex       (String* index, size_t strings);
int      WriteToFile     (String* index, size_t strings, FILE* file, const char* header);

void     Swap            (void* val1, void* val2, size_t size_elem);
void     myqsort         (void* data, const size_t size_array, 
                          size_t size_elem, int (*CompareFunc)(const void* a, const void* b));
int      CompareStr      (const void* a, const void* b);
int      CompareStrInvers(const void* a, const void* b);
int      HasLetters      (const char* str);

int main (int argc, const char* argv[])
{
    FileInfo data = GetFileInfo(argc, argv);

    const char SORTED_filename[] = "Sorted Onegin.txt";
    
    data.file = fopen(SORTED_filename, "w");
    if (data.file == NULL) 
    {
        fprintf(stderr, PART_C "Failed to open output file\n" RESET);
        return 1;
    }

    qsort(data.index, data.strings, sizeof(String), &CompareStr);
    if (WriteToFile(data.index, data.strings, data.file, 
        "\n\n- - - - - THE FIRST SORTING (TO A TO Z) - - - - - \n\n") == 0)
        printf(SUCCESS "\nSuccess to put sorted text to file!" RESET); 
    
    myqsort(data.index, data.strings, sizeof(String), &CompareStrInvers);
    if (WriteToFile(data.index, data.strings, data.file, 
        "\n\n- - - - - END SORTING (Z TO A) - - - - - \n\n") == 0)
        printf(SUCCESS "\nSuccess to put reverse sorted text to file!" RESET); 

    if (WriteToFile(data.copyindex, data.strings, data.file, 
        "\n\n- - - - - NOT SORTED - - - - - \n\n") == 0)
        printf(SUCCESS "\nSuccess to put NOT sorted text to file!\n" RESET); 

    fclose(data.file);
    FreeMem(&data);
 
    getchar();

    return 0;
}

int OpenAndGetName (int argc, const char** argv)
{
    int descriptor = -1;

    if (argc >= 2)
        descriptor = OpenFile(argv[1]);
    else 
    {
        char* filename = GetFileName();
        descriptor = OpenFile(filename);
        free(filename);
    }

    return descriptor;
}

FileInfo GetFileInfo (int argc, const char** argv)
{
    FileInfo data = {};

    data.descriptor = OpenAndGetName (argc, argv);
    data.size_array = FileSize       (data.descriptor);
    data.buffer     = CopyFromFile   (data.descriptor, data.size_array);
    data.strings    = CountStrings   (data.buffer);
    data.index      = StringsArray   (data.buffer, data.strings);
    data.copyindex  = CopyIndex      (data.index, data.strings);

    close(data.descriptor);

    return data;
}

void FreeMem (FileInfo* ptr_data)
{
    assert(ptr_data != NULL);

    free(ptr_data->copyindex);
    ptr_data->copyindex = NULL;

    free(ptr_data->index);
    ptr_data->index = NULL;

    free(ptr_data->buffer);
    ptr_data->buffer = NULL;
}

char* GetFileName(void)
{
    const int MAXNAME = 256;
    const int MAXTRIES = 3;
    int tries = 0;

    char* filename = (char*) calloc(MAXNAME, sizeof(char));

    printf(HINT "\nWrite the file name for sorting the lines.");

    while(tries < MAXTRIES)
    {
        printf(HINT "\nTries left: %d" HINT
              "\nFilename (" SUCCESS "name.txt" HINT "): " RESET, MAXTRIES - tries);

        if (fgets(filename, MAXNAME, stdin) != NULL)
        {
            filename[strcspn(filename, "\r\n")] = '\0';
            return filename;
        }

        tries++;
    }

    return NULL;
}

int OpenFile (const char* filename) 
{
    int descriptor = open(filename, _O_RDONLY);

    if (descriptor == -1)
    {
        fprintf(stderr, PART_C "\n%s" RESET, strerror(errno));
        abort();
    }

    return descriptor;
}

size_t FileSize (int descriptor)
{
    assert(descriptor != -1);

    struct stat mystat = {};
    fstat(descriptor, &mystat);

    size_t size_array = mystat.st_size + 2;

    return size_array;
}

char* CopyFromFile (int descriptor, size_t size_array)
{
    char* buffer = (char*) (calloc(size_array, sizeof(char)));
    size_t real_size = read(descriptor, buffer, size_array);

    buffer[0] = '\0';
    memset(&buffer[real_size], 0, size_array - real_size);

    return buffer + 1;
}

size_t CountStrings (char* buffer)
{
    assert(buffer != NULL);

    size_t strings = 1;
    char* ptr = buffer;

    while ((ptr = strchr(ptr, '\n')) != NULL)
    {
        *ptr = '\0';

        ptr++;
        strings++;
    }

    return strings;
}

String* StringsArray (char* buffer, size_t strings)  
{
    String* index = (String*) calloc(strings, sizeof(String));

    char* ptrcopy = buffer;

    for (size_t i = 0; i < strings; i++)
    {
        index[i].ptr_str = ptrcopy;
        char* next_null = strchr(ptrcopy, '\0');

        if (next_null != NULL)
        {
            index[i].len = (size_t)(next_null - ptrcopy);
            ptrcopy = next_null + 1;
        }
        else 
            index[i].len = 0;
    }

    return index;
}

String* CopyIndex (String* index, size_t strings)
{
    assert(index != NULL);

    String* copyindex = (String*) calloc(strings, sizeof(String));

    for (size_t i = 0; i < strings; i++)
        copyindex[i] = index[i];

    return copyindex;
}

int WriteToFile (String* index, size_t strings, FILE* file, const char* header)
{
    if (file == NULL)
    {
        fprintf(stderr, "%s", strerror(errno));
        return -1;
    }

    assert(header != NULL);
    fputs(header, file);

    for (size_t i = 0; i < strings; i++)
    {
        if (HasLetters(index[i].ptr_str)) 
        {
            fputs(index[i].ptr_str, file);
            fputc('\n', file);
        }
    }

    return 0;
}

int HasLetters (const char* str)
{
    assert(str != NULL);

    while (*str != '\0')
    {
        if (isalpha((unsigned char)*str))
            return 1;

        str++;
    }

    return 0;
}

void myqsort(void* data, size_t size_array, const size_t size_elem, int (*CompareFunc)(const void* a, const void* b))
{
    if (size_array <= 1)
        return;

    char* partition = (char*) calloc(1, size_elem); 

    for (size_t i = 0; i < size_elem; i++)
        partition[i] = *((char*)(data) + (size_array / 2) * size_elem + i);

    char* left  = (char*) data;
    char* right = (char*) data + (size_array - 1) * size_elem;

    while (left <= right)
    {
        while (CompareFunc(left, partition) < 0)
            left += size_elem;

        while(CompareFunc(right, partition) > 0)
            right -= size_elem;

        if (left <= right)
        {
            Swap(left, right, size_elem);

            left += size_elem;
            right -= size_elem;
        }
    }

    size_t new_size_l = (size_t) ((left - (char*)data) / size_elem);
    size_t new_size_r = size_array - new_size_l;

    free(partition);

    if (new_size_l > 1 && new_size_l < size_array)
        myqsort(data, new_size_l, size_elem, CompareFunc);

    if (new_size_r > 1 && new_size_r < size_array)
        myqsort(left, new_size_r, size_elem, CompareFunc);
}

int CompareStrInvers(const void* a, const void* b)
{
    const String* str1 = (const String*)a;
    const String* str2 = (const String*)b;

    int len1 = (int) (str1->len - 1); 
    int len2 = (int) (str2->len - 1);

    while (len1 >= 0 || len2 >= 0)
    {
        while (len1 >= 0 && !isalpha((unsigned char)str1->ptr_str[len1])) 
            len1--;
        while (len2 >= 0 && !isalpha((unsigned char)str2->ptr_str[len2])) 
            len2--;

        if (len1 < 0 && len2 < 0) 
            return 0;

        if (len1 < 0) 
            return -1;

        if (len2 < 0) 
            return 1;

        int c1 = tolower((unsigned char)str1->ptr_str[len1]);
        int c2 = tolower((unsigned char)str2->ptr_str[len2]);

        if (c1 != c2)
            return c1 - c2;

        len1--;
        len2--;
    }

    return 0;
}

int CompareStr(const void* a, const void* b)
{
    const String* str1 = (const String*)a;
    const String* str2 = (const String*)b;

    const char* s1 = str1->ptr_str;
    const char* s2 = str2->ptr_str;

    while (*s1 != '\0' || *s2 != '\0')
    {
        while (*s1 != '\0' && !isalpha((unsigned char)*s1))
            s1++;

        while (*s2 != '\0' && !isalpha((unsigned char)*s2))
            s2++;

        if (*s1 == '\0' && *s2 == '\0')
            return 0;

        if (*s1 == '\0' || *s2 == '\0')
            return (unsigned char)*s1 - (unsigned char)*s2;

        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);

        if (c1 != c2)
            return c1 - c2;

        s1++;
        s2++;
    }

    return 0;
}

void Swap (void* val1, void* val2, size_t size_elem)
{
    assert(val1 != NULL);
    assert(val2 != NULL);

    char* p1 = (char*) val1;
    char* p2 = (char*) val2;

    for (size_t i = 0; i < size_elem; i++) 
    {
        char p_temp = *p1;

        *p1 = *p2;
        *p2 = p_temp;

        p1++;
        p2++;
    }
}
