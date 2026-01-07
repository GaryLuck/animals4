#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define DATA_FILE "animals.dat"

typedef struct Node {
    char *question;
    struct Node *yes;
    struct Node *no;
} Node;

// Create a new node
Node* create_node(const char *question) {
    Node *node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->question = (char*)malloc(strlen(question) + 1);
    if (node->question == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcpy(node->question, question);
    node->yes = NULL;
    node->no = NULL;
    return node;
}

// Free the tree
void free_tree(Node *node) {
    if (node == NULL) return;
    free_tree(node->yes);
    free_tree(node->no);
    free(node->question);
    free(node);
}

// Get yes/no answer from user
int get_yes_no(const char *prompt) {
    char answer[MAX_LINE];
    printf("%s (yes/no): ", prompt);
    fgets(answer, MAX_LINE, stdin);

    // Remove newline
    answer[strcspn(answer, "\n")] = 0;

    if (strcmp(answer, "yes") == 0 || strcmp(answer, "y") == 0) {
        return 1;
    }
    return 0;
}

// Get a line of text from user
void get_line(const char *prompt, char *buffer, int size) {
    printf("%s: ", prompt);
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
}

// Save tree to file
void save_tree(FILE *fp, Node *node) {
    if (node == NULL) {
        fprintf(fp, "NULL\n");
        return;
    }

    fprintf(fp, "%s\n", node->question);
    save_tree(fp, node->yes);
    save_tree(fp, node->no);
}

// Load tree from file
Node* load_tree(FILE *fp) {
    char line[MAX_LINE];

    if (fgets(line, MAX_LINE, fp) == NULL) {
        return NULL;
    }

    // Remove newline
    line[strcspn(line, "\n")] = 0;

    if (strcmp(line, "NULL") == 0) {
        return NULL;
    }

    Node *node = create_node(line);
    node->yes = load_tree(fp);
    node->no = load_tree(fp);
    return node;
}

// Play the game
void play_game(Node **current) {
    if (*current == NULL) {
        // This shouldn't happen in a well-formed tree, but handle it
        return;
    }

    // If this is a leaf node (both children are NULL), it's a guess
    if ((*current)->yes == NULL && (*current)->no == NULL) {
        if (get_yes_no((*current)->question)) {
            printf("I win! Great game.\n");
        } else {
            // Computer guessed wrong, need to learn
            char animal[MAX_LINE];
            char difference[MAX_LINE];

            // Extract the guessed animal name from the question (e.g., "Is it a human?" -> "human")
            char guessed_animal[MAX_LINE];
            strcpy(guessed_animal, (*current)->question);
            // Remove "Is it a " prefix and "?" suffix if present
            if (strncmp(guessed_animal, "Is it a ", 8) == 0) {
                memmove(guessed_animal, guessed_animal + 8, strlen(guessed_animal) - 7);
            }
            if (guessed_animal[strlen(guessed_animal) - 1] == '?') {
                guessed_animal[strlen(guessed_animal) - 1] = '\0';
            }

            get_line("What animal were you thinking of", animal, MAX_LINE);
            printf("What question would distinguish a %s from a %s", guessed_animal, animal);
            get_line("", difference, MAX_LINE);

            // Create new nodes
            char new_question[MAX_LINE];
            snprintf(new_question, MAX_LINE, "Is it a %s?", animal);

            Node *old_guess = *current;
            Node *new_animal = create_node(new_question);
            Node *old_animal = create_node((*current)->question);

            // The current node becomes the distinguishing question
            free(old_guess->question);
            old_guess->question = (char*)malloc(strlen(difference) + 1);
            strcpy(old_guess->question, difference);

            // Ask which answer leads to the new animal
            printf("For a %s, what is the answer to \"%s\" (yes/no): ", animal, difference);
            char answer[MAX_LINE];
            fgets(answer, MAX_LINE, stdin);
            answer[strcspn(answer, "\n")] = 0;

            if (strcasecmp(answer, "yes") == 0 || strcasecmp(answer, "y") == 0) {
                old_guess->yes = new_animal;
                old_guess->no = old_animal;
            } else {
                old_guess->yes = old_animal;
                old_guess->no = new_animal;
            }

            printf("Thanks! I'll remember that for next time.\n");
        }
        return;
    }

    // This is an internal node, ask the question
    if (get_yes_no((*current)->question)) {
        play_game(&((*current)->yes));
    } else {
        play_game(&((*current)->no));
    }
}

// Initialize the tree with the starting question
Node* init_tree() {
    return create_node("Is it a human?");
}

int main() {
    Node *root = NULL;
    FILE *fp;

    // Try to load existing tree
    fp = fopen(DATA_FILE, "r");
    if (fp != NULL) {
        root = load_tree(fp);
        fclose(fp);
    }

    // If no saved tree, initialize with starting question
    if (root == NULL) {
        root = init_tree();
    }

    printf("Welcome to the Animal Guessing Game!\n");
    printf("Think of an animal and I'll try to guess it.\n\n");

    // Game loop
    while (1) {
        play_game(&root);

        if (!get_yes_no("\nDo you want to play again")) {
            break;
        }
        printf("\n");
    }

    // Save the tree
    fp = fopen(DATA_FILE, "w");
    if (fp != NULL) {
        save_tree(fp, root);
        fclose(fp);
        printf("Game data saved. Thanks for playing!\n");
    } else {
        fprintf(stderr, "Warning: Could not save game data to %s\n", DATA_FILE);
    }

    free_tree(root);
    return 0;
}
