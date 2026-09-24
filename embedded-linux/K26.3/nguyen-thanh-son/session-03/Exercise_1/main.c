#include <fcntl.h>     
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define FILE_PATH "students.dat"
#define NAME_SIZE 64
#define INPUT_SIZE 128

typedef struct {
    int   id;
    char  name[NAME_SIZE];
    int   age;
    float gpa;
} Student;

//I/O Helpers
void write_str(int fd, const char *str) {
    write(fd, str, strlen(str));
}

void out_console(const char *message) {
    write_str(STDOUT_FILENO, message);
}

ssize_t read_line(char *buffer, size_t buffer_size) {
    size_t len = 0;
    char c;
    ssize_t n;

    while ((n = read(STDIN_FILENO, &c, 1)) == 1 && c != '\n') {
        if (len < buffer_size - 1)  /* keep 1 byte for '\0', drop the rest */
            buffer[len++] = c;
    }
    buffer[len] = '\0';

    if (n <= 0 && len == 0)         /* EOF or error, nothing typed */
        return -1;

    return (ssize_t)len;
}

int read_int(void) {
    char buf[32];

    while (1) {
        /* Step 1: read one line */
        if (read_line(buf, sizeof(buf)) < 0)    /* EOF or error */
            return 0;

        /* Step 2: convert the string to a number */
        int i = 0;
        int sign = 1;
        int value = 0;

        if (buf[i] == '-' || buf[i] == '+') {
            if (buf[i] == '-')
                sign = -1;
            i++;
        }

        int start = i;
        while (buf[i] >= '0' && buf[i] <= '9' && i - start < 9) {  /* max 9 digits, no overflow */
            value = value * 10 + (buf[i] - '0');
            i++;
        }

        /* Valid only if we read at least one digit and reached the end */
        if (i > start && buf[i] == '\0')
            return sign * value;

        out_console("Invalid number, please try again: ");
    }
}

float read_float(void) {
    char buf[32];

    while (1) {
        /* Step 1: read one line */
        if (read_line(buf, sizeof(buf)) < 0)    /* EOF or error */
            return 0;

        /* Step 2: convert the string to a number */
        int i = 0;
        float sign = 1;
        float value = 0;
        int digits = 0;

        if (buf[i] == '-' || buf[i] == '+') {
            if (buf[i] == '-')
                sign = -1;
            i++;
        }

        /* Integer part: "12" in "12.34" */
        while (buf[i] >= '0' && buf[i] <= '9') {
            value = value * 10 + (buf[i] - '0');
            digits++;
            i++;
        }

        /* Fractional part: "34" in "12.34" -> 3 * 0.1 + 4 * 0.01 */
        if (buf[i] == '.') {
            float scale = 0.1f;
            i++;
            while (buf[i] >= '0' && buf[i] <= '9') {
                value = value + (buf[i] - '0') * scale;
                scale = scale / 10;
                digits++;
                i++;
            }
        }

        /* Valid only if we read at least one digit and reached the end */
        if (digits > 0 && buf[i] == '\0')
            return sign * value;

        out_console("Invalid number, please try again: ");
    }
}

void out_int(int value) {
    char buf[12];                   /* "-2147483648" + '\0' */
    int i = sizeof(buf) - 1;
    unsigned int u = value;

    if (value < 0) {
        out_console("-");
        u = 0u - u;
    }

    /* Fill digits from the end: 123 -> '3', '2', '1' */
    buf[i] = '\0';
    do {
        i--;
        buf[i] = '0' + u % 10;
        u = u / 10;
    } while (u > 0);

    out_console(&buf[i]);
}

void out_float(float value) {
    if (value < 0) {
        out_console("-");
        value = -value;
    }

    int cents = (int)(value * 100 + 0.5f);  /* round to 2 decimals */
    out_int(cents / 100);
    out_console(".");
    if (cents % 100 < 10)
        out_console("0");
    out_int(cents % 100);
}

void print_student(const Student *student) {
    out_console("+-------------------------------\n");
    out_console("| ID   : ");
    out_int(student->id);
    out_console("\n| Name : ");
    out_console(student->name);
    out_console("\n| Age  : ");
    out_int(student->age);
    out_console("\n| GPA  : ");
    out_float(student->gpa);
    out_console("\n+-------------------------------\n");
}

// Methods
void add(int fd) {
    Student s;
    memset(&s, 0, sizeof s);

    out_console("\n--- Add Student ---\n");
    out_console("ID   : ");
    s.id = read_int();

    out_console("Name : ");
    read_line(s.name, sizeof(s.name));

    out_console("Age  : ");
    s.age = read_int();

    out_console("GPA  : ");
    s.gpa = read_float();

    if (write(fd, &s, sizeof(s)) != sizeof(s)) {
        out_console("Error: failed to save student.\n");
        return;
    }
}

void list(int fd) {
    Student s;
    int count = 0;

    /* After add() the offset is at the end of the file, so go back to the start */
    if (lseek(fd, 0, SEEK_SET) < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    /* Read one record at a time until there is no full record left */
    while (read(fd, &s, sizeof(s)) == sizeof(s)) {
        print_student(&s);
        count++;
    }

    if (count == 0) {
        out_console("No student.\n");
        return;
    }

    out_console("Total: ");
    out_int(count);
    out_console(" student(s)\n");
}

void find(int fd) {
    Student s;

    out_console("Enter ID to find: ");
    int id = read_int();

    /* Start scanning from the first record */
    if (lseek(fd, 0, SEEK_SET) < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    /* Read records one by one, stop at the first matching id */
    while (read(fd, &s, sizeof(s)) == sizeof(s)) {
        if (s.id == id) {
            print_student(&s);
            return;
        }
    }

    out_console("No student with ID ");
    out_int(id);
    out_console(".\n");
}

void show_menu(void) {
    out_console("\n==============================\n");
    out_console("   STUDENT MANAGEMENT SYSTEM  \n");
    out_console("==============================\n");
    out_console("1. Add student\n");
    out_console("2. List all students\n");
    out_console("3. Find student by ID\n");
    out_console("4. Exit\n");
}


int main(void) {
    /* O_CREAT: create the file on first run
       O_APPEND: every write goes to the end
       no O_TRUNC, so data persists between runs */
    int fd = open(FILE_PATH, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        out_console("Error: cannot open " FILE_PATH "\n");
        return 1;
    }

    while (1) {
        show_menu();
        out_console("Enter your choice: ");
        int choice = read_int();

        if (choice == 4)        /* break here leaves the while loop */
            break;

        switch (choice) {
            case 1:
                add(fd);
                break;
            case 2:
                list(fd);
                break;
            case 3:
                find(fd);
                break;
            default:
                out_console("Invalid choice. Please try again.\n");
        }
    }

    if (close(fd) < 0) {
        out_console("Error: close failed.\n");
        return 1;
    }

    out_console("Exiting...!\n");
    return 0;
}