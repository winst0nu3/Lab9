#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>

// Структура для узла списка смежности
typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

// Структура для графа (матрица смежности)
typedef struct {
    int** matrix;
    int vertices;
} GraphMatrix;

// Структура для графа (списки смежности)
typedef struct {
    Node** lists;
    int vertices;
} GraphList;

// Структура для очереди (для BFS)
typedef struct Queue {
    int front, rear, size;
    int capacity;
    int* array;
} Queue;

// Функции для работы с очередью
Queue* createQueue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(capacity * sizeof(int));
    return queue;
}

int isQueueEmpty(Queue* queue) {
    return (queue->size == 0);
}

void enqueue(Queue* queue, int item) {
    if (queue->size == queue->capacity) return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size++;
}

int dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return item;
}

void freeQueue(Queue* queue) {
    free(queue->array);
    free(queue);
}

// 1. Генерация матрицы смежности для неориентированного графа с петлями
GraphMatrix* generateGraphMatrix(int vertices) {
    GraphMatrix* graph = (GraphMatrix*)malloc(sizeof(GraphMatrix));
    graph->vertices = vertices;
    graph->matrix = (int**)malloc(vertices * sizeof(int*));

    for (int i = 0; i < vertices; i++) {
        graph->matrix[i] = (int*)calloc(vertices, sizeof(int));
    }

    // Заполняем граф с большей плотностью и добавляем петли
    for (int i = 0; i < vertices; i++) {
        // Добавляем петли с вероятностью 20%
        if (rand() % 100 < 20) {
            graph->matrix[i][i] = 1;
        }

        for (int j = i + 1; j < vertices; j++) {
            // 40% вероятность ребра между разными вершинами
            if (rand() % 100 < 40) {
                graph->matrix[i][j] = 1;
                graph->matrix[j][i] = 1;
            }
        }
    }
    return graph;
}

// Вывод матрицы смежности
void printMatrix(GraphMatrix* graph) {
    if (graph == NULL || graph->matrix == NULL) {
        printf("Ошибка: граф не инициализирован!\n");
        return;
    }

    printf("Матрица смежности (%d вершин):\n   ", graph->vertices);
    for (int j = 0; j < graph->vertices; j++) {
        printf("%2d ", j);
    }
    printf("\n");

    for (int i = 0; i < graph->vertices; i++) {
        printf("%2d ", i);
        for (int j = 0; j < graph->vertices; j++) {
            printf("%2d ", graph->matrix[i][j]);
        }
        printf("\n");
    }
}

// 2. Поиск расстояний BFS (матрица смежности) - 1.2
int* bfsMatrix(GraphMatrix* graph, int start) {
    if (graph == NULL || start < 0 || start >= graph->vertices) {
        return NULL;
    }

    int* distances = (int*)malloc(graph->vertices * sizeof(int));
    int* visited = (int*)calloc(graph->vertices, sizeof(int));

    for (int i = 0; i < graph->vertices; i++) {
        distances[i] = -1;
    }

    Queue* queue = createQueue(graph->vertices);
    distances[start] = 0;
    visited[start] = 1;
    enqueue(queue, start);

    while (!isQueueEmpty(queue)) {
        int current = dequeue(queue);

        for (int i = 0; i < graph->vertices; i++) {
            if (graph->matrix[current][i] == 1 && !visited[i]) {
                distances[i] = distances[current] + 1;
                visited[i] = 1;
                enqueue(queue, i);
            }
        }
    }

    free(visited);
    freeQueue(queue);
    return distances;
}

// Создание списков смежности из матрицы (включая петли)
GraphList* matrixToList(GraphMatrix* graph) {
    if (graph == NULL) return NULL;

    GraphList* listGraph = (GraphList*)malloc(sizeof(GraphList));
    listGraph->vertices = graph->vertices;
    listGraph->lists = (Node**)malloc(graph->vertices * sizeof(Node*));

    for (int i = 0; i < graph->vertices; i++) {
        listGraph->lists[i] = NULL;

        // Добавляем петли
        if (graph->matrix[i][i] == 1) {
            Node* newNode = (Node*)malloc(sizeof(Node));
            newNode->vertex = i;
            newNode->next = listGraph->lists[i];
            listGraph->lists[i] = newNode;
        }

        // Добавляем остальные ребра
        for (int j = 0; j < graph->vertices; j++) {
            if (i != j && graph->matrix[i][j] == 1) {
                Node* newNode = (Node*)malloc(sizeof(Node));
                newNode->vertex = j;
                newNode->next = listGraph->lists[i];
                listGraph->lists[i] = newNode;
            }
        }
    }
    return listGraph;
}

// 3. Поиск расстояний BFS (списки смежности) - 1.3
int* bfsList(GraphList* graph, int start) {
    if (graph == NULL || start < 0 || start >= graph->vertices) {
        return NULL;
    }

    int* distances = (int*)malloc(graph->vertices * sizeof(int));
    int* visited = (int*)calloc(graph->vertices, sizeof(int));

    for (int i = 0; i < graph->vertices; i++) {
        distances[i] = -1;
    }

    Queue* queue = createQueue(graph->vertices);
    distances[start] = 0;
    visited[start] = 1;
    enqueue(queue, start);

    while (!isQueueEmpty(queue)) {
        int current = dequeue(queue);
        Node* temp = graph->lists[current];

        while (temp != NULL) {
            int neighbor = temp->vertex;
            if (!visited[neighbor]) {
                distances[neighbor] = distances[current] + 1;
                visited[neighbor] = 1;
                enqueue(queue, neighbor);
            }
            temp = temp->next;
        }
    }

    free(visited);
    freeQueue(queue);
    return distances;
}

// 4. Поиск расстояний DFS (матрица смежности) -  2.1
void dfsMatrixUtil(GraphMatrix* graph, int current, int* distances, int* visited, int distance) {
    visited[current] = 1;
    if (distances[current] == -1 || distance < distances[current]) {
        distances[current] = distance;
    }

    for (int i = 0; i < graph->vertices; i++) {
        if (graph->matrix[current][i] == 1 && !visited[i]) {
            dfsMatrixUtil(graph, i, distances, visited, distance + 1);
        }
    }
}

int* dfsMatrix(GraphMatrix* graph, int start) {
    if (graph == NULL || start < 0 || start >= graph->vertices) {
        return NULL;
    }

    int* distances = (int*)malloc(graph->vertices * sizeof(int));
    int* visited = (int*)calloc(graph->vertices, sizeof(int));

    for (int i = 0; i < graph->vertices; i++) {
        distances[i] = -1;
    }

    dfsMatrixUtil(graph, start, distances, visited, 0);
    free(visited);
    return distances;
}

// 5. Поиск расстояний DFS (списки смежности) - 2.2
void dfsListUtil(GraphList* graph, int current, int* distances, int* visited, int distance) {
    visited[current] = 1;
    if (distances[current] == -1 || distance < distances[current]) {
        distances[current] = distance;
    }

    Node* temp = graph->lists[current];
    while (temp != NULL) {
        if (!visited[temp->vertex]) {
            dfsListUtil(graph, temp->vertex, distances, visited, distance + 1);
        }
        temp = temp->next;
    }
}

int* dfsList(GraphList* graph, int start) {
    if (graph == NULL || start < 0 || start >= graph->vertices) {
        return NULL;
    }

    int* distances = (int*)malloc(graph->vertices * sizeof(int));
    int* visited = (int*)calloc(graph->vertices, sizeof(int));

    for (int i = 0; i < graph->vertices; i++) {
        distances[i] = -1;
    }

    dfsListUtil(graph, start, distances, visited, 0);
    free(visited);
    return distances;
}

// Вывод расстояний с временем выполнения
void printDistancesWithTime(int* distances, int vertices, const char* algorithm, double time_ms, int start_vertex) {
    if (distances == NULL) {
        printf("Ошибка: расстояния не вычислены!\n");
        return;
    }

    printf("\nРасстояния (%s) от вершины %d:\n", algorithm, start_vertex);
    printf("Время выполнения: %.3f мс\n", time_ms);
    printf("Вершина | Расстояние\n");
    printf("--------|-----------\n");
    for (int i = 0; i < vertices; i++) {
        printf("%7d | ", i);
        if (distances[i] == -1) {
            printf("недостижима\n");
        }
        else {
            printf("%9d\n", distances[i]);
        }
    }
}

// Освобождение памяти матрицы
void freeGraphMatrix(GraphMatrix* graph) {
    if (graph == NULL) return;

    if (graph->matrix != NULL) {
        for (int i = 0; i < graph->vertices; i++) {
            free(graph->matrix[i]);
        }
        free(graph->matrix);
    }
    free(graph);
}

// Освобождение памяти списков
void freeGraphList(GraphList* graph) {
    if (graph == NULL) return;

    if (graph->lists != NULL) {
        for (int i = 0; i < graph->vertices; i++) {
            Node* current = graph->lists[i];
            while (current != NULL) {
                Node* temp = current;
                current = current->next;
                free(temp);
            }
        }
        free(graph->lists);
    }
    free(graph);
}

// Функция для безопасного ввода целого числа
int safeInputInt(const char* prompt, int min_value, int max_value) {
    int value;
    char buffer[100];

    while (1) {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Ошибка ввода. Попробуйте снова.\n");
            continue;
        }

        // Проверяем, что введено число
        char* endptr;
        value = (int)strtol(buffer, &endptr, 10);

        // Если есть символы после числа или ввод пустой
        if (endptr == buffer) {
            printf("Ошибка: введите целое число! Попробуйте снова.\n");
            continue;
        }

        // Проверяем, есть ли лишние символы (кроме пробелов и перевода строки)
        while (*endptr != '\0') {
            if (!isspace((unsigned char)*endptr)) {
                printf("Ошибка: введите только целое число! Попробуйте снова.\n");
                break;
            }
            endptr++;
        }
        if (*endptr != '\0') continue;

        // Проверяем диапазон
        if (value < min_value || value > max_value) {
            printf("Ошибка: число должно быть от %d до %d! Попробуйте снова.\n", min_value, max_value);
            continue;
        }

        break;
    }

    return value;
}

// Функция для измерения времени выполнения
double measureAlgorithmTime(void* graph, int* (*algorithm)(void*, int), int start, int** result) {
    clock_t start_time = clock();
    *result = algorithm(graph, start);
    clock_t end_time = clock();

    return ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;
}

// 6. Тестирование производительности - 2.3
void performanceTest() {
    printf("\n=== ТЕСТИРОВАНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===\n");
    printf("Оценка времени работы алгоритмов поиска расстояний\n\n");

    int test_sizes[] = { 100, 500, 1000 };
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    printf("Размер графа | BFS (матрица) | BFS (списки) | DFS (матрица) | DFS (списки)\n");
    printf("-------------|---------------|--------------|---------------|--------------\n");

    for (int i = 0; i < num_tests; i++) {
        int size = test_sizes[i];
        printf("%12d |", size);
        fflush(stdout);

        GraphMatrix* test_graph = generateGraphMatrix(size);
        if (test_graph == NULL) {
            printf(" ошибка графа      | ошибка списков | ошибка DFS мтр | ошибка DFS сп\n");
            continue;
        }

        GraphList* test_list = matrixToList(test_graph);
        if (test_list == NULL) {
            printf(" ошибка списков    | ошибка DFS мтр | ошибка DFS сп\n");
            freeGraphMatrix(test_graph);
            continue;
        }

        // Измеряем время для каждого алгоритма
        int* result;

        double time_bfs_matrix = measureAlgorithmTime(test_graph, (int* (*)(void*, int))bfsMatrix, 0, &result);
        printf(" %13.3f |", time_bfs_matrix);
        free(result);
        fflush(stdout);

        double time_bfs_list = measureAlgorithmTime(test_list, (int* (*)(void*, int))bfsList, 0, &result);
        printf(" %12.3f |", time_bfs_list);
        free(result);
        fflush(stdout);

        double time_dfs_matrix = measureAlgorithmTime(test_graph, (int* (*)(void*, int))dfsMatrix, 0, &result);
        printf(" %13.3f |", time_dfs_matrix);
        free(result);
        fflush(stdout);

        double time_dfs_list = measureAlgorithmTime(test_list, (int* (*)(void*, int))dfsList, 0, &result);
        printf(" %12.3f\n", time_dfs_list);
        free(result);

        freeGraphMatrix(test_graph);
        freeGraphList(test_list);
    }

    printf("\nВремя указано в миллисекундах\n");
}

// Демонстрация работы всех алгоритмов на одном графе с временем выполнения
void demonstrateAlgorithms() {
    printf("\n=== ДЕМОНСТРАЦИЯ РАБОТЫ АЛГОРИТМОВ ===\n");

    // Безопасный ввод количества вершин
    int vertices = safeInputInt("Введите количество вершин для графа: ", 1, 15);

    printf("\n1. ГЕНЕРАЦИЯ МАТРИЦЫ СМЕЖНОСТИ:\n");
    GraphMatrix* graph = generateGraphMatrix(vertices);
    printMatrix(graph);

    // Безопасный ввод стартовой вершины
    int start_vertex = safeInputInt("Введите стартовую вершину: ", 0, vertices - 1);

    printf("\n2. ПОИСК РАССТОЯНИЙ BFS (МАТРИЦА СМЕЖНОСТИ):\n");
    clock_t start_time = clock();
    int* bfs_matrix_distances = bfsMatrix(graph, start_vertex);
    clock_t end_time = clock();
    double time_bfs_matrix = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    if (bfs_matrix_distances != NULL) {
        printDistancesWithTime(bfs_matrix_distances, graph->vertices, "BFS (матрица)", time_bfs_matrix, start_vertex);
        free(bfs_matrix_distances);
    }

    printf("\n3. ПОИСК РАССТОЯНИЙ BFS (СПИСКИ СМЕЖНОСТИ):\n");
    GraphList* listGraph = matrixToList(graph);
    if (listGraph != NULL) {
        start_time = clock();
        int* bfs_list_distances = bfsList(listGraph, start_vertex);
        end_time = clock();
        double time_bfs_list = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

        if (bfs_list_distances != NULL) {
            printDistancesWithTime(bfs_list_distances, graph->vertices, "BFS (списки)", time_bfs_list, start_vertex);
            free(bfs_list_distances);
        }

        printf("\n4. ПОИСК РАССТОЯНИЙ DFS (МАТРИЦА СМЕЖНОСТИ):\n");
        start_time = clock();
        int* dfs_matrix_distances = dfsMatrix(graph, start_vertex);
        end_time = clock();
        double time_dfs_matrix = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

        if (dfs_matrix_distances != NULL) {
            printDistancesWithTime(dfs_matrix_distances, graph->vertices, "DFS (матрица)", time_dfs_matrix, start_vertex);
            free(dfs_matrix_distances);
        }

        printf("\n5. ПОИСК РАССТОЯНИЙ DFS (СПИСКИ СМЕЖНОСТИ):\n");
        start_time = clock();
        int* dfs_list_distances = dfsList(listGraph, start_vertex);
        end_time = clock();
        double time_dfs_list = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

        if (dfs_list_distances != NULL) {
            printDistancesWithTime(dfs_list_distances, graph->vertices, "DFS (списки)", time_dfs_list, start_vertex);
            free(dfs_list_distances);
        }

      
    }

    freeGraphMatrix(graph);
}



int main() {
    setlocale(LC_ALL, "Russian");
    srand((unsigned int)time(NULL));

    printf("=== ПРОГРАММА ДЛЯ АНАЛИЗА АЛГОРИТМОВ ПОИСКА РАССТОЯНИЙ В ГРАФАХ ===\n");

    demonstrateAlgorithms();
    performanceTest();

    printf("\nНажмите Enter для выхода...");
    getchar();
    return 0;
}