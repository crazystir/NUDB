#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "status.h"
#include "common.h"
#include "service.h"

static struct status_t nu_status;

static void Clear();

static void InitStatus();
static void Login();
static void Menu(); 
static void Transcript();
static void Enroll();
static void Withdraw();
static void Detail();
static void Show();
static void Help();

static void CheckCourse(char *uoSCode);
static void EnrollNewCourse(char *uoSCode);
static void WithdrawLearningCourse(char *uoSCode);
static void UpdatePasswd(char *passwd);
static void UpdateAddress(char *address);

static int CheckStatus(enum status newStatus);

struct command_t {
    char cmd_[LEN];
    void (*func_)();
};


struct command_sub_t {
    char cmd_[LEN];
    void (*func_)(char *);
};

#define NUM_OF_COMMANDS 6
#define NUM_OF_SUB_COMMANDS 5 
static const struct command_t commands[NUM_OF_COMMANDS] = {
    {"transcript", Transcript},
    {"enroll", Enroll},
    {"withdraw", Withdraw},
    {"detail", Detail},
    {"show", Show},
    {"help", Help},
};

static const struct command_sub_t commands_sub[NUM_OF_SUB_COMMANDS] = {
    {"check", CheckCourse},
    {"enrollcourse", EnrollNewCourse},
    {"withdrawcourse",  WithdrawLearningCourse},
    {"password", UpdatePasswd},
    {"address", UpdateAddress},
};

static const int sub_status[NUM_OF_STATUS] = {-1, -1, COURSE_DETAIL, 
    -1, ENROLL_COURSE, -1, WITHDRAW_COURSE, -1, UPDATE_PERSONAL_DETAIL, -1, -1};
static const char *path[NUM_OF_STATUS] = 
    {"", "menu", "transcript", "", "enroll", "", "withdraw", "", "detail", "", ""};
static const char bounder[] = "*************************************************************************\n";
 

int main() {
 DBInit();
 InitStatus();
 Clear();

 while (nu_status.status_ != EXIT) {
     switch (nu_status.status_) {
        case LOGIN:
            Login();
            break;

        case MENU:
            Menu();
            break;

        default:
            break;
     }
 }
 
 DBClose();
 printf("Exit the program.\n");
 return 0;
}

void Clear(){
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
}

void InitStatus() {
    nu_status.status_ = LOGIN;
    nu_status.studentId_ = NULL;
}


void Login() {
    char name[LEN];
    char passwd[LEN];

    while (1) {
        printf("username(type exit to end the program): ");
        fgets(name, LEN, stdin);
        name[strlen(name) - 1] = 0;
        if (strcmp(name, "exit") == 0) {
            nu_status.status_ = EXIT;
            return;
        }
        printf("password: ");
        fgets(passwd, LEN, stdin);
        passwd[strlen(passwd) - 1] = 0;
        if (ValidateStudent(&nu_status.info_, name, passwd) == 0 && nu_status.info_.num_of_values_ == 1) {
            nu_status.status_ = MENU;
            nu_status.studentId_ = nu_status.info_.values_[0][0];
            nu_status.username_ = nu_status.info_.values_[1][0]; 
            break;
        }
        printf("Invalid username or password, please try again.\n");
    }
    printf("Welcome, %s!\n", name);
}

void Menu() {
    int i;
    char command[2 * LEN];
    char cmd[LEN] = "";
    char val[LEN] = "";

    struct result_t *courses = &nu_status.currentCourses_;

    CourseMenu(courses, nu_status.studentId_);

    Show();
    Help();

    while (1) {
        int pass = 0;
        int pos = 0;
        cmd[0] = '\0';
        val[0] = '\0';

        printf("%s> ", path[nu_status.status_]);
        fgets(command, 2 * LEN, stdin);
        command[strlen(command) - 1] = '\0';
        pos = NextString(cmd, command, pos);
        pos = NextString(val, command, pos);

        
        if (strcmp(cmd, "back") == 0 || strcmp(cmd, "menu") == 0) {
            printf("Back to main menu\n");
            nu_status.status_ = MENU;
            CourseMenu(courses, nu_status.studentId_);
            continue;
        }

        if (strcmp(cmd, "logout") == 0) {
            printf("Bye, %s.\n", nu_status.info_.values_[1][0]);
            nu_status.status_ = LOGIN;
            break;
        }

        if (strcmp(cmd, "exit") == 0) {
            nu_status.status_ = EXIT;
            break;
        }


        for (i = 0; i < NUM_OF_COMMANDS && !pass; i++) {
            if (strcmp(cmd, commands[i].cmd_) == 0) {
               pass = 1;
               commands[i].func_(); 
            }
        }


        for (i = 0; i < NUM_OF_SUB_COMMANDS && !pass; i++) {
            if (strcmp(cmd, commands_sub[i].cmd_) == 0) { 
                pass = 1;
                commands_sub[i].func_(val);
            } 
        }

        if (!pass) {
            printf("Invalid command [%s], please input again.\n", cmd);
        }
    }
}


void Transcript() {
    if (CheckStatus(TRANSCRIPT) < 0) {
        return;
    }

    if (nu_status.status_ != TRANSCRIPT) {
        nu_status.status_ = TRANSCRIPT;
        struct result_t *transcript = &nu_status.transcripts_;
        CourseTranscript(transcript, nu_status.studentId_);
        Show();
    }
}

void Enroll() {
    if (CheckStatus(ENROLL) < 0) {
        return;
    }
    
    if (nu_status.status_ != ENROLL) {
        nu_status.status_ = ENROLL;
        struct result_t *avaliableCourses= &nu_status.enroll_;
        struct result_t *constraintCourses = &nu_status.unenroll_;
        AvaliableCourses(avaliableCourses, nu_status.studentId_);
        ConstraintCourses(constraintCourses, nu_status.studentId_);
        Show();
    }
}


void Withdraw() {
    if (CheckStatus(WITHDRAW) < 0) {
        return;
    }


    if (nu_status.status_ != WITHDRAW) {
        nu_status.status_ = WITHDRAW;
        struct result_t *withdraw = &nu_status.withdraw_;
        WithdrawableCourses(withdraw, nu_status.studentId_);
        Show();
    }
}

void Detail() {
    if (CheckStatus(PERSONAL_DETAIL) < 0) {
        return;
    }

    if (nu_status.status_ != PERSONAL_DETAIL) {
        nu_status.status_ = PERSONAL_DETAIL;
        struct result_t *info = &nu_status.info_;
        GetStudentById(info, nu_status.studentId_);
        Show();
    }
}

void Show() {
    int i;
    char data[MAX_NUM_OF_FIELD][RES_VAL_LEN][VALUE_LEN];
    char str[80];
    char field_name[FIELD_LEN];
    struct result_t *cache = NULL;
    enum status status = nu_status.status_;

    switch(status) {
        case MENU:
            cache = &nu_status.currentCourses_;
            GetValuesByField(cache, "UoSCode", data[0]);
            GetValuesByField(cache, "UoSName", data[1]);
            GetValuesByField(cache, "Grade", data[2]);
            printf(bounder);
            printf("Current course menu\n");
            printf("%-12s %-42s %-6s\n", "Course Id", "Course name", "Grade");

            for (i = 0; i < cache -> num_of_values_; i++) {
                printf("%-12s %-42s %-6s\n", data[0][i], data[1][i], data[2][i]);
            } 
            printf(bounder);
            break;

        case TRANSCRIPT:
            cache = &nu_status.transcripts_;
            GetValuesByField(cache, "UoSCode", data[0]);
            GetValuesByField(cache, "UoSName", data[1]);
            GetValuesByField(cache, "Grade", data[2]);
            printf(bounder);
            printf("Transcript menu\n");
            printf("%-12s %-42s %-6s\n", "Course Id", "Course name", "Grade");

            for (i = 0; i < cache -> num_of_values_; i++) {
                printf("%-12s %-42s %-6s\n", data[0][i], data[1][i], data[2][i]);
            } 
            printf(bounder);
            break;

        case COURSE_DETAIL:
            printf(bounder);
            PrintResultRowVertical(&nu_status.transcripts_, nu_status.row);
            printf(bounder);
            break;

        case ENROLL:
            cache = &nu_status.enroll_;
            GetValuesByField(cache, "UoSCode", data[0]);
            GetValuesByField(cache, "UoSName", data[1]);
            GetValuesByField(cache, "Semester", data[2]);
            GetValuesByField(cache, "Year", data[3]);
            printf(bounder);
            printf("Enroll menu\n");
            printf("%-12s %-42s %-10s %-6s %-50s\n", "Course Id", "Course name", "Semester", "Year", "Prerequisites");
            for (i = 0; i < cache -> num_of_values_; i++) {
                printf("%-12s %-42s %-10s %-6s %-50s\n", data[0][i], data[1][i], data[2][i], data[3][i],"Avaliable");
            }
            i = 0;
            cache = &nu_status.unenroll_;
            GetValuesByField(cache, "UoSCode", data[0]);
            GetValuesByField(cache, "UoSName", data[1]);
            GetValuesByField(cache, "Semester", data[2]);
            GetValuesByField(cache, "Year", data[3]);
            GetValuesByField(cache, "PrereqUoSCode", data[4]);
            while (i < cache -> num_of_values_) {
                strncpy(field_name, data[0][i], FIELD_LEN); 
                printf("%-12s %-42s %-10s %-6s ", data[0][i], data[1][i], data[2][i], data[3][i]);
                str[0] = '\0';
                do {
                    Concatenate(str, data[4][i], " ", 0);
                    i++;
                } while (i < cache -> num_of_values_ && strcmp(field_name, data[0][i]) == 0); 
                printf("%-50s\n", str);
            }
            printf(bounder);
            break;
        case WITHDRAW:
            cache = &nu_status.withdraw_;
            GetValuesByField(cache, "UoSCode", data[0]);
            GetValuesByField(cache, "UoSName", data[1]);
            GetValuesByField(cache, "Semester", data[2]);
            GetValuesByField(cache, "Year", data[3]);
            printf(bounder);
            printf("Withdraw menu.\n");
            printf("%-12s %-42s %-10s %-6s\n", "Course Id", "Course name", "Semester", "Year");
            for (i = 0; i < cache -> num_of_values_; i++) {
                printf("%-12s %-42s %-10s %-6s\n", data[0][i], data[1][i], data[2][i], data[3][i]);
            }
            printf(bounder);
            break;
        case PERSONAL_DETAIL:
            cache = &nu_status.info_;
            printf(bounder);
            PrintResultVertical(cache);
            printf(bounder);
            break;
        default:
            break;
    }
}


void CheckCourse(char *uoSCode) {
    if (CheckStatus(COURSE_DETAIL) < 0 || !uoSCode) {
        return;
    }

    int i;

    nu_status.status_ = COURSE_DETAIL;
    nu_status.row = -1;


    for (i = 0; i < nu_status.transcripts_.num_of_values_; i++) {
        if (strcmp(uoSCode, nu_status.transcripts_.values_[0][i]) == 0) {
            nu_status.row = i;
        } 
    }

    if (nu_status.row < 0) {
        printf("Course %s doesn't exist.\n", uoSCode); 
        nu_status.status_ = TRANSCRIPT;
        return;
    }

    Show();

    nu_status.status_ = TRANSCRIPT;
}

void EnrollNewCourse(char *uoSCode) {
    if (CheckStatus(ENROLL_COURSE) < 0 || !uoSCode) {
        return;
    }

    char semester[VALUE_LEN];
    char year[VALUE_LEN];

    nu_status.status_ = ENROLL_COURSE;
    printf("Semester: ");
    scanf("%s", semester);
    printf("Year: ");
    scanf("%s", year);
    getchar(); // Get rid of '\n'
    
    if (EnrollCourse(nu_status.studentId_, uoSCode, semester, year, &nu_status.enroll_) < 0) {
        printf("Course (%s %s %s) dose not exist or is not avaliable.\n", uoSCode, semester, year);
        nu_status.status_ = ENROLL;    
        return;
    }

    printf("Enroll course %s successfully.\n", uoSCode);
    Enroll();
}

void WithdrawLearningCourse(char *uoSCode) {
    if (CheckStatus(WITHDRAW_COURSE) < 0 || !uoSCode) {
        return;
    }

    nu_status.status_ = WITHDRAW_COURSE;
    if (WithdrawCourse(nu_status.studentId_, uoSCode, &nu_status.withdraw_) < 0) {
        printf("Course %s does not exist or can not be dropped.\n", uoSCode);
        nu_status.status_ = WITHDRAW;
        return;
    }
    printf("Withdraw course %s successfully.\n", uoSCode);
    printf("Withdrawable courses: \n");
    Withdraw();
}


void UpdatePasswd(char *password) {
    if (CheckStatus(UPDATE_PERSONAL_DETAIL) < 0 || !password) {
        return;
    }
    char repeated_password[VALUE_LEN];

    do {
        printf("New password(type \"exit\" to give up updating password): ");
        scanf("%s", password);
        if (strcmp(password, "exit") == 0) {
            getchar(); //To remove '\n'
            nu_status.status_ = PERSONAL_DETAIL;
            return;
        }
        printf("Confirm password: ");
        scanf("%s", repeated_password);
        if (strcmp(password, repeated_password) == 0) {
            break;
        }
        printf("Two passwords are not the same.\n");
    } while (1);
    
    getchar(); // The same reason
    nu_status.status_ = UPDATE_PERSONAL_DETAIL;
    UpdatePersonalDetail(nu_status.studentId_, password, NULL);
    printf("Update password successfully.\n");
    Detail();
}

void UpdateAddress(char *address) {
    if (CheckStatus(UPDATE_PERSONAL_DETAIL) < 0 || !address) {
        return;
    }

    nu_status.status_ = UPDATE_PERSONAL_DETAIL;
    UpdatePersonalDetail(nu_status.studentId_, NULL, address);
    printf("Update address successfully.\n");
    Detail();
}

void Help() {
    printf(bounder);
    char content[2048];
    sprintf(content, "Help menu\n");
    switch(nu_status.status_) {
        case TRANSCRIPT:
            sprintf(content + strlen(content), "%-30s %-30s\n%-30s %-30s\n", "check [course id]",  "Check the detail of course with [course id]",
                    "back|menu", "Back to main menu");
            break;
        case ENROLL:
            sprintf(content + strlen(content), "%-30s %-30s\n%-30s %-30s\n", "enrollcourse [course id]", "Enroll a new course with [course id]", "back|menu", "Back to main menu");
            break;
        case WITHDRAW:
            sprintf(content + strlen(content),"%-30s %-30s\n%-30s %-30s\n", "withdrawcourse [course id]", "Withdraw a course with [course id]", "back|menu", "Back to main menu");
            break;
        case PERSONAL_DETAIL:
            sprintf(content + strlen(content),"%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n", "password", "Update the password", "Address [new address]", 
                    "Update the address", "back|menu", "Back to main menu");
            break;
        default:
            break;
        
    }
    sprintf(content + strlen(content),"%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n%-30s %-30s\n","transcript", "Transcript menu",
                    "enroll", "Enroll menu", "withdraw", "Withdraw menu", "detail", "Personal detail menu", "show", "Show the content of menu", "help", "Help menu",
                    "logout", "Sign out", "exit", "exit the program");

    printf("%s", content);
    printf(bounder);
}


int CheckStatus(enum status newStatus) {
    enum status curStatus = nu_status.status_;
    switch(newStatus) {
        case COURSE_DETAIL:
            if (curStatus != TRANSCRIPT) {
                printf("You can't check course detail since you are not in the transcript.\n");
                return -1;
            }
            break;
        case ENROLL_COURSE:
            if (curStatus != ENROLL) {
                printf("You can't enroll a new course since you are not in the enroll screen.\n");
                return -1;
            }
            break;
        case WITHDRAW_COURSE:
            if (curStatus != WITHDRAW) {
                printf("You can't withdraw a course since you are not ni the withdraw screen.\n");
                return -1;
            }
            break;
        case UPDATE_PERSONAL_DETAIL:
            if (curStatus != PERSONAL_DETAIL) {
                printf("You can't update your infomation since you are not in the personal page.\n");
                return -1;
            }
            break;
        default:
            break;
    }
    return 0;
}
    
    
    
