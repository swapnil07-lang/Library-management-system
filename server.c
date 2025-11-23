#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096

// Data structures
typedef struct Book {
    int id;
    char title[100];
    char author[100];
    int available;
    struct Book *next;
} Book;

typedef struct Student {
    int bookId;
    char name[100];
    char bookTitle[100];
    char dateIssued[50];
    char dueDate[50];
    struct Student *next;
} Student;

// Global data
Book *library = NULL;
Student *students = NULL;
char adminUser[50] = "admin";
char adminPass[50] = "admin";

// Function prototypes
void send_response(SOCKET client, const char *status, const char *content_type, const char *body);
void send_cors_headers(SOCKET client);
void handle_request(SOCKET client, char *request);
char* get_books_json();
char* get_students_json();
void add_book(int id, const char *title, const char *author);
void issue_book(int id, const char *studentName, int days);
int return_book(int id);
int delete_book(int id);
Book* find_book(int id);

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Prepare sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Listen
    listen(server_socket, 3);
    printf("Library Management System Server running on http://localhost:%d\n", PORT);
    printf("Admin credentials: %s / %s\n\n", adminUser, adminPass);

    // Accept incoming connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        // Receive request
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        // Handle request
        handle_request(client_socket, buffer);

        // Close client socket
        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void send_cors_headers(SOCKET client) {
    char headers[512];
    sprintf(headers, 
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n");
    send(client, headers, strlen(headers), 0);
}

void send_response(SOCKET client, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE * 2];
    sprintf(response,
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\n"
        "\r\n%s",
        status, content_type, (int)strlen(body), body);
    send(client, response, strlen(response), 0);
}

void handle_request(SOCKET client, char *request) {
    char method[10], path[256];
    sscanf(request, "%s %s", method, path);

    printf("Request: %s %s\n", method, path);

    // Handle OPTIONS (CORS preflight)
    if (strcmp(method, "OPTIONS") == 0) {
        char response[512];
        sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n\r\n");
        send(client, response, strlen(response), 0);
        return;
    }

    // GET /api/books
    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/books") == 0) {
        char *json = get_books_json();
        send_response(client, "200 OK", "application/json", json);
        free(json);
    }
    // GET /api/students
    else if (strcmp(method, "GET") == 0 && strcmp(path, "/api/students") == 0) {
        char *json = get_students_json();
        send_response(client, "200 OK", "application/json", json);
        free(json);
    }
    // POST /api/books
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/books") == 0) {
        // Parse JSON body (simple parsing)
        char *body = strstr(request, "\r\n\r\n");
        if (body) {
            body += 4;
            int id = 0;
            char title[100] = {0}, author[100] = {0};
            
            // Improved JSON parsing
            char *idPtr = strstr(body, "\"id\":");
            char *titlePtr = strstr(body, "\"title\":\"");
            char *authorPtr = strstr(body, "\"author\":\"");
            
            if (idPtr && titlePtr && authorPtr) {
                sscanf(idPtr + 5, "%d", &id);
                sscanf(titlePtr + 9, "%[^\"]", title);
                sscanf(authorPtr + 10, "%[^\"]", author);
                
                if (id > 0 && strlen(title) > 0 && strlen(author) > 0) {
                    add_book(id, title, author);
                    send_response(client, "201 Created", "application/json", "{\"success\":true}");
                } else {
                    send_response(client, "400 Bad Request", "application/json", "{\"success\":false,\"error\":\"Invalid book data\"}");
                }
            } else {
                send_response(client, "400 Bad Request", "application/json", "{\"success\":false,\"error\":\"Invalid JSON format\"}");
            }
        }
    }
    // POST /api/issue
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/issue") == 0) {
        char *body = strstr(request, "\r\n\r\n");
        if (body) {
            body += 4;
            int id, days;
            char studentName[100];
            
            sscanf(body, "{\"bookId\":%d,\"studentName\":\"%[^\"]\",\"days\":%d}", &id, studentName, &days);
            issue_book(id, studentName, days);
            send_response(client, "200 OK", "application/json", "{\"success\":true}");
        }
    }
    // POST /api/return
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/return") == 0) {
        char *body = strstr(request, "\r\n\r\n");
        if (body) {
            body += 4;
            int id;
            sscanf(body, "{\"bookId\":%d}", &id);
            
            if (return_book(id)) {
                send_response(client, "200 OK", "application/json", "{\"success\":true}");
            } else {
                send_response(client, "404 Not Found", "application/json", "{\"success\":false,\"error\":\"Book not found\"}");
            }
        }
    }
    // DELETE /api/books/:id
    else if (strcmp(method, "DELETE") == 0 && strncmp(path, "/api/books/", 11) == 0) {
        int id = atoi(path + 11);
        if (delete_book(id)) {
            send_response(client, "200 OK", "application/json", "{\"success\":true}");
        } else {
            send_response(client, "404 Not Found", "application/json", "{\"success\":false}");
        }
    }
    // POST /api/login
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/login") == 0) {
        char *body = strstr(request, "\r\n\r\n");
        if (body) {
            body += 4;
            char username[50], password[50];
            
            // Parse JSON: {"username":"xxx","password":"xxx"}
            char *userPtr = strstr(body, "\"username\":\"");
            char *passPtr = strstr(body, "\"password\":\"");
            
            if (userPtr && passPtr) {
                sscanf(userPtr + 12, "%[^\"]", username);
                sscanf(passPtr + 12, "%[^\"]", password);
                
                // Verify credentials
                if (strcmp(username, adminUser) == 0 && strcmp(password, adminPass) == 0) {
                    printf("Login successful: %s\n", username);
                    send_response(client, "200 OK", "application/json", "{\"success\":true}");
                } else {
                    send_response(client, "401 Unauthorized", "application/json", "{\"success\":false,\"error\":\"Invalid credentials\"}");
                }
            } else {
                send_response(client, "400 Bad Request", "application/json", "{\"success\":false,\"error\":\"Invalid request\"}");
            }
        }
    }
    // POST /api/reset-password
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/reset-password") == 0) {
        char *body = strstr(request, "\r\n\r\n");
        if (body) {
            body += 4;
            char oldPass[50], newUser[50], newPass[50];
            
            // Parse JSON: {"oldPassword":"xxx","newUsername":"xxx","newPassword":"xxx"}
            char *oldPtr = strstr(body, "\"oldPassword\":\"");
            char *userPtr = strstr(body, "\"newUsername\":\"");
            char *passPtr = strstr(body, "\"newPassword\":\"");
            
            if (oldPtr && userPtr && passPtr) {
                sscanf(oldPtr + 15, "%[^\"]", oldPass);
                sscanf(userPtr + 15, "%[^\"]", newUser);
                sscanf(passPtr + 15, "%[^\"]", newPass);
                
                // Verify old password
                if (strcmp(oldPass, adminPass) == 0) {
                    strcpy(adminUser, newUser);
                    strcpy(adminPass, newPass);
                    printf("Admin credentials updated: %s / %s\n", adminUser, adminPass);
                    send_response(client, "200 OK", "application/json", "{\"success\":true}");
                } else {
                    send_response(client, "401 Unauthorized", "application/json", "{\"success\":false,\"error\":\"Invalid old password\"}");
                }
            } else {
                send_response(client, "400 Bad Request", "application/json", "{\"success\":false,\"error\":\"Invalid request\"}");
            }
        }
    }
    else {
        send_response(client, "404 Not Found", "text/plain", "Not Found");
    }
}

char* get_books_json() {
    char *json = malloc(BUFFER_SIZE * 4);
    strcpy(json, "[");
    
    Book *temp = library;
    int first = 1;
    while (temp != NULL) {
        if (!first) strcat(json, ",");
        char book_json[512];
        sprintf(book_json, "{\"id\":%d,\"title\":\"%s\",\"author\":\"%s\",\"available\":%s}",
                temp->id, temp->title, temp->author, temp->available ? "true" : "false");
        strcat(json, book_json);
        temp = temp->next;
        first = 0;
    }
    
    strcat(json, "]");
    return json;
}

char* get_students_json() {
    char *json = malloc(BUFFER_SIZE * 4);
    strcpy(json, "[");
    
    Student *temp = students;
    int first = 1;
    while (temp != NULL) {
        if (!first) strcat(json, ",");
        char student_json[512];
        sprintf(student_json, "{\"bookId\":%d,\"name\":\"%s\",\"bookTitle\":\"%s\",\"dateIssued\":\"%s\",\"dueDate\":\"%s\"}",
                temp->bookId, temp->name, temp->bookTitle, temp->dateIssued, temp->dueDate);
        strcat(json, student_json);
        temp = temp->next;
        first = 0;
    }
    
    strcat(json, "]");
    return json;
}

void add_book(int id, const char *title, const char *author) {
    Book *newBook = (Book*)malloc(sizeof(Book));
    newBook->id = id;
    strcpy(newBook->title, title);
    strcpy(newBook->author, author);
    newBook->available = 1;
    newBook->next = library;
    library = newBook;
    printf("Book added: %s by %s\n", title, author);
}

void issue_book(int id, const char *studentName, int days) {
    Book *book = find_book(id);
    if (book && book->available) {
        book->available = 0;
        
        Student *newStudent = (Student*)malloc(sizeof(Student));
        newStudent->bookId = id;
        strcpy(newStudent->name, studentName);
        strcpy(newStudent->bookTitle, book->title);
        strcpy(newStudent->dateIssued, "2025-11-22"); // Simplified
        strcpy(newStudent->dueDate, "2025-12-06");     // Simplified
        newStudent->next = students;
        students = newStudent;
        
        printf("Book %d issued to %s\n", id, studentName);
    }
}

int return_book(int id) {
    Student *temp = students, *prev = NULL;
    while (temp != NULL && temp->bookId != id) {
        prev = temp;
        temp = temp->next;
    }
    
    if (temp == NULL) return 0;
    
    // Mark book available
    Book *book = find_book(id);
    if (book) book->available = 1;
    
    // Remove student record
    if (prev == NULL) {
        students = temp->next;
    } else {
        prev->next = temp->next;
    }
    free(temp);
    
    printf("Book %d returned\n", id);
    return 1;
}

int delete_book(int id) {
    Book *temp = library, *prev = NULL;
    while (temp != NULL && temp->id != id) {
        prev = temp;
        temp = temp->next;
    }
    
    if (temp == NULL) return 0;
    
    if (prev == NULL) {
        library = temp->next;
    } else {
        prev->next = temp->next;
    }
    free(temp);
    
    printf("Book %d deleted\n", id);
    return 1;
}

Book* find_book(int id) {
    Book *temp = library;
    while (temp != NULL) {
        if (temp->id == id) return temp;
        temp = temp->next;
    }
    return NULL;
}
