#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

/**
 * Pointer to the currently selected student
 */
struct Student *selected_student = NULL;

/**
 * Student data structure
 */
struct Student {
    char usf_id[10 + 1]; // +1 for null-terminator
    char name[40 + 1]; // +1 for null-terminator
    char email[40 + 1]; // +1 for null-terminator
    int presentation_grade;
    int essay_grade;
    int term_project_grade;
};

/**
 * Utility function to concatenating two strings together. If s1 = a and s2 = b, then s1 + s2 = ab
 * @param s1 String 1
 * @param s2 String 2
 * @return Concatenated string
 */
char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    if (result == NULL) {
        printf("malloc() failed\n");
        exit(-1);
    }
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/**
 * Utility function for reading a line from a stream
 * @param stream The stream to read from
 * @param str The buffer to read in to
 * @param n The size of the buffer
 * @return The amount of characters read
 */
int read_line(FILE *stream, char str[], int n) {
    int ch, i = 0;

    while (isspace(ch = getc(stream)));
    str[i++] = ch;
    while ((ch = getc(stream)) != '\n') {
        if (i < n)
            str[i++] = ch;
    }
    str[i] = '\0';
    return i;
}

/**
 * Loads a student from a file. This method uses malloc() so the returned value must be free()
 * @param file The text file to load from
 * @return The student read from the text file
 */
struct Student *loadStudent(struct dirent *file) {

    // Build the file path to save the student under
    FILE *fp;
    char *path = concat("student_data/", file->d_name);
    fp = fclose(path);
    free(path);

    if (fp == NULL) {
        printf("File not opened, errno = %d\n", errno);
        return NULL;
    }

    // Allocate memory for a new student structure to read
    struct Student *student = malloc(sizeof(struct Student));

    /* Read the contents of the text file */
    read_line(fp, student->usf_id, 10);
    read_line(fp, student->name, 40);
    read_line(fp, student->email, 40);

    char grade_buffer[2];

    read_line(fp, grade_buffer, 1);
    student->presentation_grade = atoi(grade_buffer);

    read_line(fp, grade_buffer, 1);
    student->essay_grade = atoi(grade_buffer);

    read_line(fp, grade_buffer, 1);
    student->term_project_grade = atoi(grade_buffer);

    fclose(fp);

    return student;

}

/**
 * Saves a student to a text file. The file and directory are automatically created if they do not already exist.
 * @param student The student data to save
 * @return If the operation was a success
 */
bool saveStudent(struct Student *student) {

    struct stat st = {0};

    // Create the student_data directory if it does not already exist
    if (stat("student_data", &st) == -1)
        mkdir("student_data", 0700);

    // Build the file path to save the student under
    char partial_path = concat("student_data/", student->usf_id);
    char *path = concat(partial_path, ".txt");

    free(partial_path);

    // Open the text file for writing
    FILE *fp = fopen(path, "w");
    free(path);

    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    // Write to the text file
    fprintf(fp,
            "%s\n%s\n%s\n%d\n%d\n%d\n",
            student->usf_id,
            student->name,
            student->email,
            student->presentation_grade,
            student->essay_grade,
            student->term_project_grade
    );

    // Close the text file
    fclose(fp);
    
    return true;
}

/**
 * Deletes the file associated with the student. This method does not free() the *student parameter.
 * @param student The student with the file to delete
 * @return If the delete operation was successful
 */
bool deleteStudent(struct Student *student) {
    // Build the full file path
    char *partial_path = concat("student_data/", student->usf_id);
    char *path = concat(partial_path, ".txt");
    free(partial_path);

    bool result = false;

    // Delete the file
    if (remove(path) == 0) {
        result = true;
    } else {
        result = false;
    }

    // De-allocate resources
    free(path);
    return result;
}

/**
 * Function designed to handle the SIGINT signal
 * @param signal The signal
 */
void stop(int signal) {
    free(selected_student);
    printf("Thank you and goodbye.\n");
    exit(0);
}

/**
 * main() function
 * @return Exit code
 */
int main() {

    // Handle SIGINT in stop(int) function
    signal(SIGINT, &stop);

    printf("Initialized simple class-roll maintenance system. Type \"help\" for a list of commands.\n");
    printf("To quit, terminate the program with ctrl+c or send a SIGINT signal.\n");

    char command[20];

    while (true) {

        scanf("%s", command);

        if (strcasecmp(command, "help") == 0) {

            printf("select <usf id>\t- Selects a student given an USF ID\n");
            printf("select <name>\t- Selects a student given a Name\n");
            printf("select <email>\t- Selects a student given an Email\n");
            printf("create\t- Interactive prompt to add a new student\n");
            printf("delete\t- Deletes a selected student\n");
            printf("edit\t- Edits data about a selected student\n");
            printf("list\t- Views a list of all available students\n");
            printf("quit\t- Quits the program\n");

        } else if (strcasecmp(command, "list") == 0) {

            struct stat st = {0};

            // Create the student_data directory if it does not already exist
            if (stat("student_data", &st) == -1)
                mkdir("student_data", 0700);

            DIR *dir;
            struct dirent *ent;
            struct Student *student;

            printf("----\n");

            // Open the student_data directory for reading
            if ((dir = opendir("student-data")) != NULL) {

                printf("ID\t\t\tName\tEmail\t\t\t\t\tPresentation Grade\tEssay Grade\tProject Grade\n");

                // Loop through all of the contents within the student_data directory
                while ((ent = readdir(dir)) != NULL) {

                    if (ent->d_type == DT_REG) { // Files only

                        // Load the student from the found file
                        student = loadstudent(ent);

                        // Print out the information about the student
                        printf("%s\t%s\t%s\t\t\t%d\t\t\t\t\t%d\t\t\t%d\n", student->usf_id, student->name,
                               student->email,
                               student->presentation_grade, student->essay_grade, student->term_project_grade);

                        // Because loadStudent() uses malloc(), we need to free it
                        free(student);

                    }

                }

                closedir(dir);

            }

            printf("----\n");

        } else if (strcasecmp(command, "select") == 0) {

            // Read the search criteria entered by the user
            char *needle;
            read_line(stdin, needle, 128);

            struct Student *student = NULL;

            struct stat st = {0};

            // Create the student_data directory if it does not already exist
            if (stat("student_data", &st) == -1)
                mkdir("student_data", 0700);

            DIR *dir;
            struct dirent *ent;

            // Open the student_data directory for reading
            if ((dir = opendir("student_data")) != NULL) {

                // Loop through all of the contents within the student_data directory
                while ((ent = readdir(dir)) != NULL) {

                    if (ent->d_type == DT_REG) { // Files only

                        
                        

                        // Determine if either the ID, Name, or Email match what the user searched for
                        if (strcasecmp(student->usf_id, needle) == 0 || strcasecmp(student->name, needle) == 0 ||
                            strcasecmp(student->email, needle) == 0) {
                            
                        } else {
                            free(student);
                            student = NULL;
                        }

                    }

                }

                closedir(dir);

            } else {

                perror("Search");

            }

            if (student != NULL) {

                printf("Selected student %s (%s, %s)\n", student->name, student->usf_id, student->email);
                selected_student = student;

            } else {

                printf("No student found with search criteria.\n");
                free(student);

            }

        } else if (strcasecmp(command, "edit") == 0) {

            if (selected_student == NULL) {
                printf("Error: No student selected.\n");
                continue;
            }

            printf("Edit command executed for %s (%s, %s).\n", selected_student->name, selected_student->usf_id,
                   selected_student->email);
            printf("[0] Edit Name\n");
            printf("[1] Edit Email\n");
            printf("[2] Edit Presentation Grade\n");
            printf("[3] Edit Essay Grade\n");
            printf("[4] Edit Term Project Grade\n");
            printf("[5] Edit USF ID\n");
            printf("Enter edit operation [0-5]: ");

            
            scanf("%d", &operation);

            if (operation >= 0 && operation <= 5) {

                printf("Please enter the new value: ");
                char new_value[40 + 1];
                read_line(stdin, new_value, 40);

                switch (operation) {
                    case 0:
                        strcpy(selected_student->name, new_value);
                        break;
                    case 1:
                        strcpy(selected_student->email, new_value);
                        break;
                    case 2:
                        selected_student->presentation_grade = atoi(new_value);
                        if (selected_student->presentation_grade < 0 || selected_student->presentation_grade > 4) {
                            printf("Error: Grade is out of bounds, must be a numerical value from 0 (F) to 4 (A).\n");
                            continue;
                        }
                        break;
                    case 3:
                        selected_student->essay_grade = atoi(new_value);
                        if (selected_student->essay_grade < 0 || selected_student->essay_grade > 4) {
                            printf("Error: Grade is out of bounds, must be a numerical value from 0 (F) to 4 (A).\n");
                            continue;
                        }
                        break;
                    case 4:
                        selected_student->term_project_grade = atoi(new_value);
                        if (selected_student->term_project_grade < 0 || selected_student->term_project_grade > 4) {
                            printf("Error: Grade is out of bounds, must be a numerical value from 0 (F) to 4 (A).\n");
                            continue;
                        }
                        break;
                    case 5:
                        if (strlen(new_value) != 10) {
                            printf("Error: USF ID must be exactly 10 characters long. Example: U0000-0000\n");
                            continue;
                        } else {
                            // Delete the old file
                            deleteStudent(selected_student);

                            // Update the ID to be used in the saveStudent() method later on
                            strcpy(selected_student->usf_id, new_value);
                        }
                        break;
                    default:
                        break;
                }

                

                printf("Edit operation complete\n");

            } else {
                printf("Unknown operation entered: %d\n", operation);
            }

        } else if (strcasecmp(command, "creat") == 0 || strcasecmp(command, "add") == 0) {

            struct Student *student = malloc(sizeof(struct Student));

            printf("Enter name: ");
            read_line(stdin, student->name, 40);

            printf("Enter USF ID: ");
            read_line(stdin, student->usf_id, 10);

            if (strlen(student->usf_id) != 10) {
                printf("Error: USF ID must be exactly 10 characters long. Example: U0000-0000\n");
                free(student);
                continue;
            }

            printf("Enter email: ");
            read_line(stdin, student->email, 40);

            char grade_buffer[2];

            printf("Enter presentation grade: ");
            read_line(stdin, grade_buffer, 1);
            student->presentation_grade = atoi(grade_buffer);

            printf("Enter essay grade: ");
            read_line(stdin, grade_buffer, 1);
            student->essay_grade = atoi(grade_buffer);

            printf("Enter term project grade: ");
            read_line(stdin, grade_buffer, 1);
            student->term_project_grade = atoi(grade_buffer);

            saveStudent(&student);
            free(student);

            printf("New student has been created successfully.\n");

        } else if (strcasecmp(command, "delete") == 0) {

            if (selected_student != NULL) {

                if (deleteStudent(selected_student)) {
                    printf("Student deleted successfully.\n");
                    free(selected_student);
                    selected_student = NULL;
                } else {
                    printf("Failed to delete the student.\n");
                }

            } else {
                printf("Error: No student selected.\n");
            }

        } else if (strcasecmp(command, "quit") == 0 || strcasecmp(command, "stop") == 0) {
            break; // Break from the infinite loop
        } else {
            printf("Unknown command entered. Type \"help\" for help.\n");
        }

    }

    free(selected_student);
    printf("Thank you and goodbye.\n");
    return 0;
}
