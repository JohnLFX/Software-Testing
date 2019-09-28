#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

struct Student *selected_student = NULL;

struct Student {
    char usf_id[10 + 1];
    char name[40 + 1];
    char email[40 + 1];
    int presentation_grade;
    int essay_grade;
    int term_project_grade;
};

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // TODO check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

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


struct Student *loadStudent(struct dirent *file) {
    FILE *fp;
    char *path = concat("student_data/", file->d_name);
    fp = fopen(path, "r");
    free(path);

    if (fp == NULL) {
        printf("File not opened, errno = %d\n", errno);
        return NULL;
    }

    struct Student *student = malloc(sizeof(struct Student));

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

bool saveStudent(struct Student *student) {

    struct stat st = {0};

    if (stat("student_data", &st) == -1)
        mkdir("student_data", 0700);

    char *partial_path = concat("student_data/", student->usf_id);
    char *path = concat(partial_path, ".txt");

    free(partial_path);

    FILE *fp = fopen(path, "w");
    free(path);

    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(fp,
            "%s\n%s\n%s\n%d\n%d\n%d\n",
            student->usf_id,
            student->name,
            student->email,
            student->presentation_grade,
            student->essay_grade,
            student->term_project_grade
    );

    fclose(fp);

}

void stop(int signal) {
    free(selected_student);
    printf("Thank you and goodbye.\n");
    exit(0);
}

int main() {

    signal(SIGINT, &stop);

    printf("Initialized simple class-roll maintenance system. Type \"help\" for a list of commands.\n");
    printf("To quit, terminate the program with ctrl+c or send a SIGINT signal.\n");

    char command[20];

    while (true) {

        scanf("%19s", command);

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

            if (stat("student_data", &st) == -1)
                mkdir("student_data", 0700);

            DIR *dir;
            struct dirent *ent;
            struct Student *student;

            printf("----\n");

            if ((dir = opendir("student_data")) != NULL) {

                printf("ID\t\t\tName\tEmail\t\t\t\t\tPresentation Grade\tEssay Grade\tProject Grade\n");

                while ((ent = readdir(dir)) != NULL) {

                    if (ent->d_type == DT_REG) { // Files only

                        student = loadStudent(ent);

                        printf("%s\t%s\t%s\t\t\t%d\t\t\t\t\t%d\t\t\t%d\n", student->usf_id, student->name,
                               student->email,
                               student->presentation_grade, student->essay_grade, student->term_project_grade);

                        free(student);

                    }

                }

                closedir(dir);

            }

            printf("----\n");

        } else if (strcasecmp(command, "select") == 0) {

            char needle[128 + 1];
            read_line(stdin, needle, 128);

            struct Student *student = NULL;

            struct stat st = {0};

            if (stat("student_data", &st) == -1)
                mkdir("student_data", 0700);

            DIR *dir;
            struct dirent *ent;

            if ((dir = opendir("student_data")) != NULL) {

                while ((ent = readdir(dir)) != NULL) {

                    if (ent->d_type == DT_REG) { // Files only

                        student = loadStudent(ent);

                        if (strcasecmp(student->usf_id, needle) == 0 || strcasecmp(student->name, needle) == 0 ||
                            strcasecmp(student->email, needle) == 0) {
                            break; // Student found
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
            printf("Enter edit operation [0-4]: ");

            int operation;
            scanf("%d", &operation);

            if (operation >= 0 && operation <= 4) {

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
                        break;
                    case 3:
                        selected_student->essay_grade = atoi(new_value);
                        break;
                    case 4:
                        selected_student->term_project_grade = atoi(new_value);
                        break;
                    default:
                        break;
                }

                saveStudent(selected_student);

                printf("Edit operation complete\n");

            } else {
                printf("Unknown operation entered: %d\n", operation);
            }

        } else if (strcasecmp(command, "create") == 0 || strcasecmp(command, "add") == 0) {

            struct Student *student = malloc(sizeof(struct Student));

            printf("Enter name: ");
            read_line(stdin, student->name, 40);

            printf("Enter USF ID: ");
            read_line(stdin, student->usf_id, 10);

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

            saveStudent(student);
            free(student);

            printf("New student has been created successfully.\n");

        } else if (strcasecmp(command, "delete") == 0) {

            char *partial_path = concat("student_data/", selected_student->usf_id);
            char *path = concat(partial_path, ".txt");
            free(partial_path);

            if (remove(path) == 0)
                printf("Student deleted successfully\n");
            else
                printf("Unable to delete student\n");

            free(path);
            free(selected_student);

        } else if (strcasecmp(command, "quit") == 0 || strcasecmp(command, "stop") == 0) {
            break;
        } else {
            printf("Unknown command entered. Type \"help\" for help.\n");
        }

    }

    free(selected_student);
    printf("Thank you and goodbye.\n");
    return 0;
}
