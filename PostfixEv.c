#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h> // For pow function

#define MAX 100

int stack[MAX];
int top = -1;

// Stack operations with error handling
void push(int val, HWND hwndOutput) {
    if (top >= MAX - 1) {
        MessageBox(hwndOutput, "Stack overflow!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    stack[++top] = val;
}

int pop(int *success, HWND hwndOutput) {
    if (top < 0) {
        *success = 0;
        MessageBox(hwndOutput, "Stack underflow!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }
    *success = 1;
    return stack[top--];
}

// Print stack state to file
void print_stack(FILE *fp) {
    fprintf(fp, "Stack: ");
    if (top < 0) {
        fprintf(fp, "(empty)\n");
    } else {
        for (int i = 0; i <= top; i++) {
            fprintf(fp, "%d", stack[i]);
            if (i < top) fprintf(fp, " ");
        }
        fprintf(fp, "\n");
    }
}

// Operator precedence (updated for ^)
int precedence(char op) {
    switch (op) {
        case '^': return 3; // Exponentiation has highest precedence, right-associative
        case '*':
        case '/': return 2;
        case '+':
        case '-': return 1;
        default: return 0;
    }
}

// Check if operator is right-associative (only ^ for now)
int is_right_associative(char op) {
    return op == '^';
}

// Validate infix expression (updated for ^)
int is_valid_infix(const char *infix) {
    int paren_count = 0;
    for (int i = 0; infix[i]; i++) {
        if (isspace(infix[i])) continue;
        if (!isdigit(infix[i]) && !strchr("+-*/^()", infix[i])) {
            return 0; // Invalid character (variables like a,b,c will fail here)
        }
        if (infix[i] == '(') paren_count++;
        if (infix[i] == ')') paren_count--;
        if (paren_count < 0) return 0; // Unmatched closing parenthesis
    }
    return paren_count == 0; // Ensure balanced parentheses
}

// Convert infix to postfix (updated for ^)
int infix_to_postfix(const char *infix, char *postfix, size_t postfix_size, HWND hwndOutput) {
    char opStack[MAX];
    int topOp = -1, j = 0;
    postfix[0] = '\0';

    if (!is_valid_infix(infix)) {
        MessageBox(hwndOutput, "Invalid infix expression! (Variables not supported yet)", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    for (int i = 0; infix[i]; i++) {
        if (isspace(infix[i])) continue;
        if (isdigit(infix[i])) {
            while (isdigit(infix[i]) && j < postfix_size - 2) {
                postfix[j++] = infix[i++];
            }
            postfix[j++] = ' ';
            i--;
        } else if (infix[i] == '(') {
            if (topOp >= MAX - 1) {
                MessageBox(hwndOutput, "Operator stack overflow!", "Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            opStack[++topOp] = infix[i];
        } else if (infix[i] == ')') {
            while (topOp >= 0 && opStack[topOp] != '(' && j < postfix_size - 2) {
                postfix[j++] = opStack[topOp--];
                postfix[j++] = ' ';
            }
            if (topOp < 0 || opStack[topOp] != '(') {
                MessageBox(hwndOutput, "Unmatched parenthesis!", "Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            topOp--; // Pop '('
        } else if (strchr("+-*/^", infix[i])) {
            while (topOp >= 0 && opStack[topOp] != '(' &&
                   (precedence(opStack[topOp]) > precedence(infix[i]) ||
                    (precedence(opStack[topOp]) == precedence(infix[i]) && !is_right_associative(infix[i]))) &&
                   j < postfix_size - 2) {
                postfix[j++] = opStack[topOp--];
                postfix[j++] = ' ';
            }
            if (topOp >= MAX - 1) {
                MessageBox(hwndOutput, "Operator stack overflow!", "Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            opStack[++topOp] = infix[i];
        }
        if (j >= postfix_size - 2) {
            MessageBox(hwndOutput, "Postfix buffer too small!", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
    }
    while (topOp >= 0 && j < postfix_size - 2) {
        if (opStack[topOp] == '(') {
            MessageBox(hwndOutput, "Unmatched parenthesis!", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
        postfix[j++] = opStack[topOp--];
        postfix[j++] = ' ';
    }
    if (j > 0 && j < postfix_size) {
        postfix[j-1] = '\0'; // Remove trailing space
    } else {
        postfix[0] = '\0';
    }
    return 1;
}

// Evaluate postfix expression (updated for ^)
int evaluate_postfix(const char *postfix, FILE *fp, HWND hwndOutput) {
    top = -1;
    int i = 0;

    while (postfix[i]) {
        if (isspace(postfix[i])) {
            i++;
            continue;
        }
        if (isdigit(postfix[i])) {
            int num = 0;
            while (isdigit(postfix[i])) {
                num = num * 10 + (postfix[i++] - '0');
            }
            push(num, hwndOutput);
            fprintf(fp, "After processing number '%d':\n", num);
            print_stack(fp);
        } else if (strchr("+-*/^", postfix[i])) {
            int success;
            int b = pop(&success, hwndOutput);
            if (!success) return 0;
            int a = pop(&success, hwndOutput);
            if (!success) return 0;
            double res = 0; // Use double for exponentiation
            switch (postfix[i]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/':
                    if (b == 0) {
                        MessageBox(hwndOutput, "Division by zero!", "Error", MB_OK | MB_ICONERROR);
                        return 0;
                    }
                    res = (double)a / b;
                    break;
                case '^':
                    res = pow((double)a, (double)b); // Exponentiation
                    break;
            }
            push((int)res, hwndOutput); // Cast back to int (may lose precision)
            fprintf(fp, "After processing '%c':\n", postfix[i]);
            print_stack(fp);
        } else {
            MessageBox(hwndOutput, "Invalid character in postfix!", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
        i++;
    }
    int success;
    int result = pop(&success, hwndOutput);
    if (!success || top != -1) {
        MessageBox(hwndOutput, "Invalid postfix expression!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }
    fprintf(fp, "Final Result: %d\n", result);
    return 1;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndInput, hwndOutput;

    switch (uMsg) {
        case WM_CREATE: {
            hwndInput = CreateWindow("EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 20, 250, 25,
                hwnd, (HMENU)1, NULL, NULL);

            CreateWindow("BUTTON", "Evaluate",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                280, 20, 100, 25,
                hwnd, (HMENU)2, NULL, NULL);

            hwndOutput = CreateWindow("EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
                20, 60, 360, 200,
                hwnd, (HMENU)3, NULL, NULL);
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == 2) {
                char input[256], postfix[512];
                GetWindowText(hwndInput, input, sizeof(input));
                if (!infix_to_postfix(input, postfix, sizeof(postfix), hwndOutput)) {
                    SetWindowText(hwndOutput, "Error in conversion. See message box for details.");
                    return 0;
                }

                FILE *fp = fopen("temp_output.txt", "w");
                if (!fp) {
                    MessageBox(hwndOutput, "Failed to create temporary file!", "Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hwndOutput, "Error: Could not create output file.");
                    return 0;
                }
                fprintf(fp, "Infix: %s\r\n", input);
                fprintf(fp, "Postfix: %s\r\n\r\n", postfix);
                if (!evaluate_postfix(postfix, fp, hwndOutput)) {
                    fclose(fp);
                    remove("temp_output.txt");
                    SetWindowText(hwndOutput, "Error in evaluation. See message box for details.");
                    return 0;
                }
                fclose(fp);

                fp = fopen("temp_output.txt", "r");
                if (!fp) {
                    MessageBox(hwndOutput, "Failed to read temporary file!", "Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hwndOutput, "Error: Could not read output file.");
                    return 0;
                }
                char output[2048] = {0}; // Initialize to empty string
                char line[256];
                int offset = 0;
                while (fgets(line, sizeof(line), fp)) {
                    // Replace \n with \r\n if needed
                    char *pos = line;
                    while ((pos = strchr(pos, '\n')) != NULL) {
                        *pos = '\r';
                        memmove(pos + 1, pos, strlen(pos) + 1);
                        *++pos = '\n';
                        pos += 2;
                    }
                    strncat_s(output + offset, sizeof(output) - offset, line, _TRUNCATE);
                    offset += strlen(line);
                }
                fclose(fp);
                remove("temp_output.txt");

                // Clear the edit control before setting new text
                SetWindowText(hwndOutput, "");
                SetWindowText(hwndOutput, output);

                // Debug: Show the output string to verify newlines
                // MessageBox(hwndOutput, output, "Debug Output", MB_OK);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "PostfixEvaluator";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Postfix Evaluator GUI",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 320,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Window creation failed!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}