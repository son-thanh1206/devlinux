#include <fcntl.h>     
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#define FILE_PATH "products.dat"
#define NAME_SIZE 64
#define INPUT_SIZE 128

typedef struct {
    int   id;
    char  name[NAME_SIZE];
    int   quantity;
    double price;
} Product;

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
        if (len < buffer_size - 1)
            buffer[len++] = c;
    }
    buffer[len] = '\0';

    if (n <= 0 && len == 0)
        return -1;

    return (ssize_t)len;
}

int read_int(void) {
    char buf[32];

    while (1) {
        if (read_line(buf, sizeof(buf)) < 0)
            return 0;

        int i = 0;
        int sign = 1;
        int value = 0;

        if (buf[i] == '-' || buf[i] == '+') {
            if (buf[i] == '-')
                sign = -1;
            i++;
        }

        int start = i;
        while (buf[i] >= '0' && buf[i] <= '9' && i - start < 9) {
            value = value * 10 + (buf[i] - '0');
            i++;
        }

        if (i > start && buf[i] == '\0')
            return sign * value;

        out_console("Invalid number, please try again: ");
    }
}

double read_double(void) {
    char buf[32];

    while (1) {
        if (read_line(buf, sizeof(buf)) < 0)
            return 0;

        int i = 0;
        double sign = 1;
        double value = 0;
        int digits = 0;

        if (buf[i] == '-' || buf[i] == '+') {
            if (buf[i] == '-')
                sign = -1;
            i++;
        }

        while (buf[i] >= '0' && buf[i] <= '9') {
            value = value * 10 + (buf[i] - '0');
            digits++;
            i++;
        }

        if (buf[i] == '.') {
            double scale = 0.1;
            i++;
            while (buf[i] >= '0' && buf[i] <= '9') {
                value = value + (buf[i] - '0') * scale;
                scale = scale / 10;
                digits++;
                i++;
            }
        }

        if (digits > 0 && buf[i] == '\0')
            return sign * value;

        out_console("Invalid number, please try again: ");
    }
}

void out_int(int value) {
    char buf[12];
    int i = sizeof(buf) - 1;
    unsigned int u = value;

    if (value < 0) {
        out_console("-");
        u = 0u - u;
    }

    buf[i] = '\0';
    do {
        i--;
        buf[i] = '0' + u % 10;
        u = u / 10;
    } while (u > 0);

    out_console(&buf[i]);
}

void out_double(double value) {
    if (value < 0) {
        out_console("-");
        value = -value;
    }

    int cents = (int)(value * 100 + 0.5);
    out_int(cents / 100);
    out_console(".");
    if (cents % 100 < 10)
        out_console("0");
    out_int(cents % 100);
}

void show_product(const Product *p) {
    out_console("ID       : ");
    out_int(p->id);
    out_console("\nName     : ");
    out_console(p->name);
    out_console("\nQuantity : ");
    out_int(p->quantity);
    out_console("\nPrice    : ");
    out_double(p->price);
    out_console("\n");
}

// Methods
void add_product(int fd) {
    Product p;
    memset(&p, 0, sizeof(p));

    out_console("Enter ID: ");
    p.id = read_int();

    out_console("Enter name: ");
    read_line(p.name, sizeof(p.name));

    out_console("Enter quantity: ");
    p.quantity = read_int();

    out_console("Enter price: ");
    p.price = read_double();

    if (lseek(fd, 0, SEEK_END) < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    ssize_t n = write(fd, &p, sizeof(p));
    if (n != (ssize_t)sizeof(p)) {
        out_console("Error: write failed.\n");
        return;
    }

    out_console("Product added:\n");
    show_product(&p);
}

void show_by_index(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }
    if (size < (off_t)sizeof(Product)) {
        out_console("No product\n");
        return;
    }

    out_console("Enter index: ");
    int index = read_int();

    while (index < 0) {
        out_console("Index must be >= 0, please try again: ");
        index = read_int();
    }

    off_t offset = (off_t)index * sizeof(Product);
    if (lseek(fd, offset, SEEK_SET) < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    Product p;
    ssize_t n = read(fd, &p, sizeof(p));
    if (n == 0) {
        out_console("Error: index out of range.\n");
        return;
    }
    if (n != (ssize_t)sizeof(p)) {
        out_console("Error: read failed.\n");
        return;
    }

    show_product(&p);
}

void update_quantity(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }
    if (size < (off_t)sizeof(Product)) {
        out_console("No product\n");
        return;
    }

    out_console("Enter index: ");
    int index = read_int();

    while (index < 0) {
        out_console("Index must be >= 0, please try again: ");
        index = read_int();
    }

    off_t offset = (off_t)index * sizeof(Product);
    if (offset + (off_t)sizeof(Product) > size) {
        out_console("Error: index out of range.\n");
        return;
    }

    out_console("Enter new quantity: ");
    int quantity = read_int();

    off_t field_offset = offset + offsetof(Product, quantity);
    if (lseek(fd, field_offset, SEEK_SET) < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    ssize_t n = write(fd, &quantity, sizeof(quantity));
    if (n != (ssize_t)sizeof(quantity)) {
        out_console("Error: write failed.\n");
        return;
    }

    out_console("Quantity updated.\n");
}

void list_all(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    int count = size / sizeof(Product);
    if (count == 0) {
        out_console("No product\n");
        return;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        out_console("Error: lseek failed.\n");
        return;
    }

    for (int index = 0; index < count; index++) {
        Product p;
        ssize_t n = read(fd, &p, sizeof(p));
        if (n != (ssize_t)sizeof(p)) {
            out_console("Error: read failed.\n");
            return;
        }

        out_console("\n[Index ");
        out_int(index);
        out_console("]\n");
        show_product(&p);
    }
}

void show_menu(void) {
    out_console("\n==============================\n");
    out_console("   PRODUCT MANAGEMENT SYSTEM  \n");
    out_console("==============================\n");
    out_console("1. Add product\n");
    out_console("2. Show product by index\n");
    out_console("3. Update quantity by index\n");
    out_console("4. List all products\n");
    out_console("5. Exit\n");
}


int main(void) {
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        out_console("Error: cannot open " FILE_PATH "\n");
        return 1;
    }

    while (1) {
        show_menu();
        out_console("Enter your choice: ");
        int choice = read_int();

        if (choice == 5)
            break;

        switch (choice) {
            case 1:
                add_product(fd);
                break;
            case 2:
                show_by_index(fd);
                break;
            case 3:
                update_quantity(fd);
                break;
            case 4:
                list_all(fd);
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