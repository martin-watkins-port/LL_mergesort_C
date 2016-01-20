/* ----------------------------------------------------- */
/* Homework 2*/
/* Author: Martin Watkins  */
/* Inputs: One text file presumably filled with numbers 0-9 and chars , . - ' ' \n  OR
            an encrypted file to be decoded */
/* Outputs: Encrypted file OR a character distribution table */
/* output */
/* Lab Section: 1 */
/* Revision History: */
/* 10/7/2015 – Initial Specification */
/* ----------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>/

//Two global constants defining the number of command line arguments
#define INFILE_ARG 2
#define OUTFILE_ARG 3


//Container for the implementation of the character
//distribution table.  Each entry holds the actual character
//, an integer describing the number of occurrences, and a
//pointer to the next entry (implemented as a LL)
struct entry
{
    int character;
    unsigned int char_count;
    struct entry *next;
};

//Function prototypes for functions concerning the
//implementation of the character distribution table
struct entry *count_that(char* input_nm);
struct entry *mergesort(struct entry *head);
struct entry* mergehelper(struct entry *alist, struct entry *blist);
struct entry *splitlist(struct entry *head);
void push(int a, struct entry **head);
void print_count(struct entry *head);
void freedom(struct entry *head);

//Prototypes for functions concerning the compression and
//decompression algorithm
void compress(char* input_nm, char *output_nm);
void decompress(char* input_nm, char *output_nm);
int isAcceptableChar(int a);
void encrypt_vals(int *a, int *b);
void decrypt_vals(char *a, char *b);

//Prototype for determining file size
unsigned long filesize(char* file);

int main(int argc, char *argv[])
{
        //if -h is detected, simply prints the help screen
		if(strcmp(argv[1], "-h") == 0)
		{
			printf("usage: hw2.out -acdh input_file [output_file]\n");
			printf("	-a Analyze/Check file (will output a data distribution table)\n");
			printf("	-c Compress input_file and write compressed data to output_file\n");
			printf("	-d Decompress the input_file and write the results to output_file\n");
			printf("	-h Display this message\n");
		}else
		{
		    //if analysis is detected...
		    if(strcmp(argv[1], "-a") == 0)
            {
                assert(argc == 3);//making sure the number of arguments is correct
                struct entry *head = count_that(argv[INFILE_ARG]);//initialization of the structure holding the read/analyzed data
                                                                  //from input
                print_count(head);//Prints
                freedom(head);//To clean up malloc's

            }else
            {
                //if compression is detected...
                if(strcmp(argv[1], "-c") == 0)
                {
                    assert(argc == 4);//again asserting the number of arguments
                    compress(argv[INFILE_ARG],argv[OUTFILE_ARG]);//compression algorithm initiated
                    struct entry *head = count_that(argv[INFILE_ARG]);
                    print_count(head);
                    printf("Output file: %s \n",argv[OUTFILE_ARG]);//prints
                    printf ("New size of output: %ld bytes.\n",filesize( argv[OUTFILE_ARG] ));//prints

                //if decompression is detected...
                }else if(strcmp(argv[1], "-d") == 0)
                {
                    assert(argc == 4);
                    decompress(argv[INFILE_ARG],argv[OUTFILE_ARG]);//decompression algorithm
                    struct entry *head = count_that(argv[OUTFILE_ARG]);
                    print_count(head);
                    printf("Output file: %s \n",argv[OUTFILE_ARG]);//prints
                    printf ("Size of original input: %ld bytes.\n",filesize( argv[OUTFILE_ARG] ));//prints
                }else
                {
                    //error checking
                    fprintf(stderr,"Error: Command not recognized! Usage: hw2.out -acdh input_file [output_file]\n");
                    exit(1);
                }
            }

		}

	return 0;
}

//Function: count_that
//Input: name of input file
//Output: pointer to the first entry of the LL
//        and consequently the LL itself
struct entry *count_that(char* input_nm)
{
    int c;//used for reading each character from input
    struct entry *head = NULL;//initialize the head entry to null
    FILE *input;//input file

    //open her up
    if ((input = fopen(input_nm, "r")) == NULL)
    {
        // Open failed
        printf("Could not open file %s for reading\n",input_nm);
        exit(1);
    }
    //start the read
    while((c = fgetc(input)) != EOF)
    {
        push(c,&head);//make lots of entries
    }

    //shut her down
    if (fclose(input) == EOF)
    {
        printf("Error in closing input file\n");
        exit(1);
    }

    //Sorted for easy printing
    head = mergesort(head);
    return head;
}

//Function: push
//Input: int containing a read value and a pointer to the head
//       of the LL
//Output: (none)
//What's the point then?  It either inserts a fresh entry, or finds an existing entry
//and increments its counter.
void push(int a, struct entry **head)
{
    int found = 0;//bool-esque variable for determining whether or not a char is found

    if(isAcceptableChar(a) == 0){a = 'o'; }//for dealing with any characters not 0-9 or ,.- \n
    if(a == 13) return;//for dealing with Windows' carriage return

    //if empty list...
    if((*head) == NULL)
    {
        (*head) = (struct entry *) malloc(sizeof(struct entry));//allocate proper memory
        (*head)->character = a;//storage
        (*head)->char_count = 1;//storage
        (*head)->next = NULL;//storage
    }else
    {
        struct entry *temp = (*head);//temporary entry pointer for traversal
        while(temp!=NULL)
        {
            if(temp->character == a )//entry exists
            {
                found = 1;
                ++temp->char_count;
                break;//increment and get out of there
            }
            if(temp->next == NULL) break;//reached the end
            else temp = temp->next;//else proceed
        }

        if(found == 0)//if not found...
        {
            struct entry *new_entry = (struct entry *) malloc(sizeof(struct entry));//new entry allocation
            new_entry->character = a ;
            new_entry->char_count = 1;
            assert(temp->next == NULL);//make sure temp is the last possible entry
            temp->next = new_entry;//set pointers
            new_entry->next = NULL;
        }
    }
}

//Simple print function for character distribution table
void print_count(struct entry *head)
{
    printf("Character   Count\n");
    struct entry *temp = head;//temp for traversal
    while(temp != NULL)
    {
        if(temp->character == '\n') printf("\\n          %i\n",temp->char_count);//in case of newline
        else printf("%c           %i\n",temp->character,temp->char_count);

        temp = temp->next;
    }
}

//"destructor" for the entry LL
void freedom(struct entry *head)
{
    struct entry *current;//all pretty self explanatory, just free up each entry
    while( (current = head) != NULL )
    {
        head = head->next;
        free(current);
    }
}

//Merge sort driver function
//Merge sort was used because of its speed and practicality
//with respects to the use of a LL
//Implemented using my own knowledge and help from
//stackoverflow and cppreference
struct entry *mergesort(struct entry *head)
{
    struct entry *alist = head;//initialize the first temporary list
    if (alist == NULL || alist->next == NULL)//if empty list...
    {
        return alist;
    }
    struct entry *blist = splitlist(alist);//initialize the second temporary list to the "second half" of the original list
    return mergehelper(mergesort(alist), mergesort(blist));//return the merged and sorted list
}

//Merge sort helper function
//Basically takes in pointers to the head of the first and second lists
//and returns a pointer to the head of the merged lists
struct entry *mergehelper(struct entry *alist, struct entry *blist)
{
    struct entry header, *tracker = &header;//header that only contains a pointer to the real head
                                         //and a tracking pointer

    while (alist != NULL && blist != NULL)
    {
        struct entry **smaller = (alist->character < blist->character) ? &alist : &blist;//store the minimum element in smaller in each instance
        struct entry *next_elem = (*smaller)->next;//pointer for the next element
        tracker = tracker->next = *smaller;
        *smaller = next_elem;
    }
    tracker->next = alist ? alist : blist;
    return header.next; //return the head of the merged list
}

//Function that splits a list headed by *head
//and returns a pointer to the middle of the list
struct entry *splitlist(struct entry *head)
{
    //the fast pointer progresses two times faster than the slow
    //previous points to the entry before the slow pointer
    struct entry *fast = head, *slow = head, *prev = NULL;
    while (fast != NULL && fast->next != NULL)
    {
        //just trail and split the list from here
        fast = fast->next->next;
        prev = slow;
        slow = slow->next;
    }
    if (prev != NULL)
    {
        prev->next = NULL;
    }
    return slow;
}

//Function: compress
//Input: name of input and output file
//Output: compressed, encrypted file to output
void compress(char* input_nm, char *output_nm)
{
    int c;//reading variable
    char x;//storing variable

    FILE *input;
    if ((input = fopen(input_nm, "r")) == NULL)
    {
        // Open failed
        printf("Could not open file %s for reading\n",input_nm);
        exit(1);
    }

    FILE *output;
    if ((output = fopen(output_nm, "w")) == NULL)
    {
        // Open failed
        printf("Could not open file %s for writing\n",output_nm);
        exit(1);
    }

    while ((c = fgetc(input)) != EOF)//while not end of file
    {
        int d;//jump-ahead variable
        if(isAcceptableChar(c) == 1)//if not bad character
        {
            if(((d = getc(input)) != EOF) && (fseek(input,1,SEEK_CUR)) == 0 )//seek ahead once to store two numbers in one byte
            {
                int n1 = c, n2=d;//temporary storage variables
                int *a = &n1, *b = &n2;//pointers to "pass by reference"
                encrypt_vals(a,b);
                x = ((*b) << 4) | (*a);//actual bit manipulation
                fputc(x,output);//write to output

            }else//special case: odd number of characters
            {
                int n1 = c;
                int *a = &n1;
                encrypt_vals(a,&n1);
                x = (*a) | 15 << 4;
                fputc(x,output);
                break;
            }
            fseek(input,-1,SEEK_CUR);//can't forget to reset the file iterator
        }
    }

    //CLOSING TIME
    if (fclose(output) == EOF)
    {
        printf("Error in closing output file\n");
        exit(1);
    }
    if (fclose(input) == EOF)
    {
        printf("Error in closing input file\n");
        exit(1);
    }
}

//straightforward function to encrypt values
void encrypt_vals(int *a, int *b)
{
    if((*a) == '0') (*a) = 0;
    else if((*a) == '1') (*a) = 1;
    else if((*a) == '2') (*a) = 2;
    else if((*a) == '3') (*a) = 3;
    else if((*a) == '4') (*a) = 4;
    else if((*a) == '5') (*a) = 5;
    else if((*a) == '6') (*a) = 6;
    else if((*a) == '7') (*a) = 7;
    else if((*a) == '8') (*a) = 8;
    else if((*a) == '9') (*a) = 9;
    else if((*a) == ',') (*a) = 10;
    else if((*a) == '-') (*a) = 11;
    else if((*a) == '.') (*a) = 12;
    else if((*a) == ' ') (*a) = 13;
    else if((*a) == '\n') (*a) = 14;

    if((*b) == '0') (*b) = 0;
    else if((*b) == '1') (*b) = 1;
    else if((*b) == '2') (*b) = 2;
    else if((*b) == '3') (*b) = 3;
    else if((*b) == '4') (*b) = 4;
    else if((*b) == '5') (*b) = 5;
    else if((*b) == '6') (*b) = 6;
    else if((*b) == '7') (*b) = 7;
    else if((*b) == '8') (*b) = 8;
    else if((*b) == '9') (*b) = 9;
    else if((*b) == ',') (*b) = 10;
    else if((*b) == '-') (*b) = 11;
    else if((*b) == '.') (*b) = 12;
    else if((*b) == ' ') (*b) = 13;
    else if((*b) == '\n') (*b) = 14;
    else return;
}

//Function: decompress
//Input: name of input and output file
//Output: decompressed, original file to output
void decompress(char* input_nm, char *output_nm)
{
    int c;//reading variable

    FILE *input;
    if ((input = fopen(input_nm, "r")) == NULL)
    {
        // Open failed
        printf("Could not open file %s for reading\n",input_nm);
        exit(1);
    }

    FILE *output;
    if ((output = fopen(output_nm, "w")) == NULL)
    {
        // Open failed
        printf("Could not open file %s for writing\n",output_nm);
        exit(1);
    }

    while ((c = fgetc(input)) != EOF)
    {
        char num1,num2, *a, *b;//num1 and num2 for splitting up the byte, a and b for pass by reference
        num2 = (c & 0b11110000) >> 4;//bit manipulation
        num1 = (c & 0b00001111);
        a = &num1, b = &num2;
        decrypt_vals(a,b);
        fputc((*a),output);
        if(*b != 15) fputc((*b),output);//As to not print that annoying extraneous ascii character
    }

    //I KNOW WHO I WANT TO TAKE ME HOME
    if (fclose(output) == EOF)
    {
        printf("Error in closing output file\n");
        exit(1);
    }
    if (fclose(input) == EOF)
    {
        printf("Error in closing input file\n");
        exit(1);
    }
}

//Again pretty straightforward...
void decrypt_vals(char *a, char *b)
{
    if((*a) == 0) (*a) = '0';
    else if((*a) == 1) (*a) = '1';
    else if((*a) == 2) (*a) = '2';
    else if((*a) == 3) (*a) = '3';
    else if((*a) == 4) (*a) = '4';
    else if((*a) == 5) (*a) = '5';
    else if((*a) == 6) (*a) = '6';
    else if((*a) == 7) (*a) = '7';
    else if((*a) == 8) (*a) = '8';
    else if((*a) == 9) (*a) = '9';
    else if((*a) == 10) (*a) = ',';
    else if((*a) == 11) (*a) = '-';
    else if((*a) == 12) (*a) = '.';
    else if((*a) == 13) (*a) = ' ';
    else if((*a) == 14) (*a) = '\n';

    if((*b) == 0) (*b) = '0';
    else if((*b) == 1) (*b) = '1';
    else if((*b) == 2) (*b) = '2';
    else if((*b) == 3) (*b) = '3';
    else if((*b) == 4) (*b) = '4';
    else if((*b) == 5) (*b) = '5';
    else if((*b) == 6) (*b) = '6';
    else if((*b) == 7) (*b) = '7';
    else if((*b) == 8) (*b) = '8';
    else if((*b) == 9) (*b) = '9';
    else if((*b) == 10) (*b) = ',';
    else if((*b) == 11) (*b) = '-';
    else if((*b) == 12) (*b) = '.';
    else if((*b) == 13) (*b) = ' ';
    else if((*b) == 14) (*b) = '\n';
    else return;
}

//determines whether or not the character is encrypt-able
int isAcceptableChar(int a)
{
    if(a == '0' || a == '1' || a == '2' || a == '3' ||
       a == '4' || a == '5' || a == '6' || a == '7' ||
       a == '8' || a == '9' || a == ',' || a == '-' ||
       a == '.' || a == ' ' || a == '\n')
        {
            return 1;
        }
            return 0;
}

//simply determines the file size
unsigned long filesize(char* file)
{
    FILE *f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    unsigned long fsize = (unsigned long)ftell(f);
    fclose(f);
    return fsize;
}
