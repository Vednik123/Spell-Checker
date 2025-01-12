#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 45
#define DICTIONARY_FILE "dictionary.txt"
#define SUGGESTION_LIMIT 5

// Trie node definition
typedef struct TrieNode
{
    struct TrieNode *children[26];
    int is_end_of_word;
} TrieNode;

// Stack node definition for undo history
typedef struct StackNode
{
    char word[MAX_WORD_LENGTH];
    struct StackNode *next;
} StackNode;

// Queue node definition
typedef struct QueueNode
{
    char word[MAX_WORD_LENGTH];
    struct QueueNode *next;
} QueueNode;

// Queue structure
typedef struct Queue
{
    QueueNode *front;
    QueueNode *rear;
} Queue;

// Function declarations
TrieNode *create_trienode();
void insert_word_in_trie(TrieNode *root, const char *word);
int search_in_trie(TrieNode *root, const char *word);
void load_dictionary(TrieNode *root);
void check_spelling(TrieNode *root, Queue *queue);
void suggest_corrections(TrieNode *root, const char *misspelled_word);
void push_stack(StackNode **top, const char *word);
void pop_stack(StackNode **top);
void enqueue(Queue *queue, const char *word);
char *dequeue(Queue *queue);
void free_trie(TrieNode *root);
void free_stack(StackNode *top);
void free_queue(Queue *queue);
void display_menu();
void display_spell_check_menu();
// Function declarations for the new features
void check_sentence(TrieNode *root);
void correct_sentence(TrieNode *root);

// Create a new TrieNode
TrieNode *create_trienode()
{
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (!node)
    {
        printf("Memory allocation error!\n");
        exit(1);
    }
    for (int i = 0; i < 26; i++)
    {
        node->children[i] = NULL;
    }
    node->is_end_of_word = 0;
    return node;
}

// Insert a word into the Trie
void insert_word_in_trie(TrieNode *root, const char *word)
{
    TrieNode *node = root;
    while (*word)
    {
        int index = tolower(*word) - 'a';
        if (index < 0 || index >= 26)
        {
            word++;
            continue;
        }
        if (node->children[index] == NULL)
        {
            node->children[index] = create_trienode();
        }
        node = node->children[index];
        word++;
    }
    node->is_end_of_word = 1;
}

// Search for a word in the Trie
int search_in_trie(TrieNode *root, const char *word)
{
    TrieNode *node = root;
    while (*word)
    {
        int index = tolower(*word) - 'a';
        if (index < 0 || index >= 26)
        {
            word++;
            continue;
        }
        if (node->children[index] == NULL)
        {
            return 0; // Word not found
        }
        node = node->children[index];
        word++;
    }
    return node != NULL && node->is_end_of_word;
}

// Load dictionary file into the Trie
void load_dictionary(TrieNode *root)
{
    FILE *file = fopen(DICTIONARY_FILE, "r");
    if (!file)
    {
        printf("Error: Dictionary file not found.\n");
        return;
    }
    char word[MAX_WORD_LENGTH];
    while (fscanf(file, "%s", word) != EOF)
    {
        insert_word_in_trie(root, word);
    }
    fclose(file);
}

// Push onto the stack (store undo history)
void push_stack(StackNode **top, const char *word)
{
    StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
    if (!new_node)
    {
        printf("Memory allocation error!\n");
        exit(1);
    }
    strcpy(new_node->word, word);
    new_node->next = *top;
    *top = new_node;
}

// Pop from the stack
void pop_stack(StackNode **top)
{
    if (*top == NULL)
        return;
    StackNode *temp = *top;
    *top = (*top)->next;
    free(temp);
}

// Enqueue into the queue
void enqueue(Queue *queue, const char *word)
{
    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!new_node)
    {
        printf("Memory allocation error!\n");
        exit(1);
    }
    strcpy(new_node->word, word);
    new_node->next = NULL;
    if (queue->rear == NULL)
    {
        queue->front = queue->rear = new_node;
    }
    else
    {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}

// Dequeue from the queue
char *dequeue(Queue *queue)
{
    if (queue->front == NULL)
        return NULL;
    QueueNode *temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL)
        queue->rear = NULL;
    char *word = (char *)malloc(MAX_WORD_LENGTH);
    if (!word)
    {
        printf("Memory allocation error!\n");
        exit(1);
    }
    strcpy(word, temp->word);
    free(temp);
    return word;
}

// Suggest corrections for a misspelled word
void suggest_corrections(TrieNode *root, const char *misspelled_word)
{
    char buffer[MAX_WORD_LENGTH];
    int len = strlen(misspelled_word);

    printf("Suggestions for '%s':\n", misspelled_word);
    // Generate suggestions based on common misspelling patterns
    for (int i = 0; i < len; i++)
    {
        for (char c = 'a'; c <= 'z'; c++)
        {
            strncpy(buffer, misspelled_word, len);
            buffer[i] = c;
            buffer[len] = '\0';
            if (search_in_trie(root, buffer) && strcmp(buffer, misspelled_word) != 0)
            {
                printf("  %s\n", buffer);
            }
        }
    }
}

// Spell check function (compare words from the input with the Trie)
void check_spelling(TrieNode *root, Queue *queue)
{
    char word[MAX_WORD_LENGTH];
    int choice;

    while (1)
    {
        printf("Enter a word to check its spelling (or press 4 to end): ");
        fgets(word, MAX_WORD_LENGTH, stdin);
        word[strcspn(word, "\n")] = '\0'; // Remove newline

        if (strcmp(word, "4") == 0)
        {
            break;
        }

        if (search_in_trie(root, word))
        {
            printf("%s is spelled correctly.\n", word);
        }
        else
        {
            printf("%s is misspelled.\n", word);
            enqueue(queue, word); // Store misspelled words in the queue

            while (1)
            {
                display_spell_check_menu();
                scanf("%d", &choice);
                getchar(); // Consume newline

                if (choice == 1)
                {
                    // Continue checking spelling
                    break;
                }
                else if (choice == 2)
                {
                    // Suggest corrections
                    suggest_corrections(root, word);
                }
                else if (choice == 3)
                {
                    // End the current spell check session
                    return;
                }
                else
                {
                    printf("Invalid choice. Please try again.\n");
                }
            }
        }
    }
}

// Basic grammar checks: articles, capitalization, subject-verb agreement
int check_basic_grammar(const char *sentence)
{
    char buffer[256];
    strcpy(buffer, sentence);

    char *words[20];
    int word_count = 0;
    char *word = strtok(buffer, " ");

    // Split sentence into words
    while (word != NULL && word_count < 20)
    {
        words[word_count++] = word;
        word = strtok(NULL, " ");
    }

    if (word_count == 0)
    {
        printf("Empty sentence.\n");
        return 0;
    }

    // Check capitalization of the first word
    if (islower(words[0][0]))
    {
        printf("The sentence should start with a capital letter.\n");
    }

    // Article check ("a", "an")
    for (int i = 0; i < word_count - 1; i++)
    {
        if (strcmp(words[i], "a") == 0 && strchr("aeiou", tolower(words[i + 1][0])) != NULL)
        {
            printf("Use 'an' before a vowel: '%s' -> 'an'.\n", words[i + 1]);
        }
        else if (strcmp(words[i], "an") == 0 && strchr("aeiou", tolower(words[i + 1][0])) == NULL)
        {
            printf("Use 'a' before a consonant: '%s' -> 'a'.\n", words[i + 1]);
        }
    }

    // Basic subject-verb agreement (only for simple present tense)
    for (int i = 0; i < word_count - 1; i++)
    {
        if (strcmp(words[i], "he") == 0 || strcmp(words[i], "she") == 0 || strcmp(words[i], "it") == 0)
        {
            if (strcmp(words[i + 1], "are") == 0)
            {
                printf("Use 'is' with singular subjects: '%s' -> 'is'.\n", words[i + 1]);
            }
        }
    }

    // Check punctuation at the end
    int len = strlen(words[word_count - 1]);
    if (words[word_count - 1][len - 1] != '.')
    {
        printf("The sentence should end with a period.\n");
    }

    return 1; // Grammar checked
}

// Check if the entire sentence is correct
void check_sentence(TrieNode *root)
{
    char sentence[256];
    char *word;
    int correct = 1;

    printf("Enter a sentence (max 20 words): ");
    fgets(sentence, sizeof(sentence), stdin);
    sentence[strcspn(sentence, "\n")] = '\0'; // Remove newline

    word = strtok(sentence, " ");
    int word_count = 0;

    while (word != NULL && word_count < 20)
    {
        if (!search_in_trie(root, word))
        {
            printf("'%s' is misspelled.\n", word);
            correct = 0;
        }
        word = strtok(NULL, " ");
        word_count++;
    }

    if (word_count > 20)
    {
        printf("Sentence exceeds the word limit of 20 words.\n");
    }
    else if (correct)
    {
        if (check_basic_grammar(sentence))
        {
            printf("The sentence is grammatically and spelling-wise correct.\n");
        }
    }
}

// Helper function to check if a word starts with a vowel
int starts_with_vowel(const char *word)
{
    char first_letter = tolower(word[0]);
    return (first_letter == 'a' || first_letter == 'e' || first_letter == 'i' ||
            first_letter == 'o' || first_letter == 'u');
}

// Correct the entire sentence
void correct_sentence(TrieNode *root)
{
    char sentence[500];
    char words[20][MAX_WORD_LENGTH];
    char corrected_sentence[500] = "";
    int word_count = 0;

    printf("Enter a sentence to correct (max 20 words): ");
    fgets(sentence, 500, stdin);
    sentence[strcspn(sentence, "\n")] = '\0'; // Remove newline

    // Tokenize the sentence into words
    char *token = strtok(sentence, " ");
    while (token != NULL && word_count < 20)
    {
        strcpy(words[word_count], token);
        word_count++;
        token = strtok(NULL, " ");
    }

    printf("Correcting...\n");

    for (int i = 0; i < word_count; i++)
    {
        if (!search_in_trie(root, words[i]))
        {
            printf("'%s' is misspelled. Suggesting corrections...\n", words[i]);
            suggest_corrections(root, words[i]);
            // For simplicity, let's assume the first suggestion is chosen
            strcpy(words[i], "corrected_word"); // Replace with actual correct word logic
        }

        // Apply grammar rules
        if (i == 0 && islower(words[i][0]))
        {
            // Capitalize the first word
            words[i][0] = toupper(words[i][0]);
        }

        // Check for article usage
        if (strcmp(words[i], "a") == 0 && i + 1 < word_count && starts_with_vowel(words[i + 1]))
        {
            strcpy(words[i], "an");
        }
        else if (strcmp(words[i], "an") == 0 && i + 1 < word_count && !starts_with_vowel(words[i + 1]))
        {
            strcpy(words[i], "a");
        }

        strcat(corrected_sentence, words[i]);
        strcat(corrected_sentence, " ");
    }

    // Remove trailing space and add a period if needed
    corrected_sentence[strlen(corrected_sentence) - 1] = '\0'; // Remove the last space
    if (corrected_sentence[strlen(corrected_sentence) - 1] != '.')
    {
        strcat(corrected_sentence, ".");
    }

    printf("Corrected sentence: %s\n", corrected_sentence);
}

// Display menu
void display_menu()
{
    printf("\n **Menu\n");
    printf("1. Check word\n");
    printf("2. Suggest corrections for the misspelled word\n");
    printf("3. Check sentence\n");
    printf("4. Suggest corrections for the given sentence\n");
    printf("5. End spell check session\n");
    printf("Enter your choice: ");
}

// Display spell check menu
void display_spell_check_menu()
{
    printf("\n **Spell Check Menu\n");
    printf("1. Check another word\n");
    printf("2. Suggest corrections for the misspelled word\n");
    printf("3. Check another sentence\n");
    printf("4. Suggest corrections for the given sentence\n");
    printf("5. End spell check session\n");
    printf("Enter your choice: ");
}

// Free Trie memory
void free_trie(TrieNode *root)
{
    if (!root)
        return;
    for (int i = 0; i < 26; i++)
    {
        if (root->children[i])
            free_trie(root->children[i]);
    }
    free(root);
}

// Free stack memory
void free_stack(StackNode *top)
{
    while (top)
    {
        pop_stack(&top);
    }
}

// Free queue memory
void free_queue(Queue *queue)
{
    while (queue->front)
    {
        dequeue(queue);
    }
}

// Main function
int main()
{
    TrieNode *root = create_trienode();
    StackNode *undo_stack = NULL;
    Queue word_queue = {NULL, NULL};

    load_dictionary(root);

    int choice;
    char input_word[MAX_WORD_LENGTH];

    while (1)
    {
        display_menu();
        scanf("%d", &choice);
        getchar(); // Consume newline character left in the buffer

        switch (choice)
        {
        case 1:
            check_spelling(root, &word_queue);
            break;
        case 2:
            printf("Enter the misspelled word for suggestions: ");
            fgets(input_word, MAX_WORD_LENGTH, stdin);
            input_word[strcspn(input_word, "\n")] = '\0'; // Remove newline
            suggest_corrections(root, input_word);
            break;
        case 3:
            check_sentence(root);
            break;
        case 4:
            correct_sentence(root);
            break;
        case 5:
            printf("Exiting program.\n");
            // Free resources
            free_trie(root);
            free_stack(undo_stack);
            free_queue(&word_queue);
            return 0;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }
}
