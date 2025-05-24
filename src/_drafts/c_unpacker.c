#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the same structure as in Python
typedef struct {
    int id_number;
    float temperature;
    char name[5];  // 4 bytes + null terminator
} DataStruct;

int main() {
    FILE *file;
    DataStruct data;
    
    // Open the binary file
    file = fopen("data.bin", "rb");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    
    // Read the binary data into the struct
    size_t read_size = fread(&data, sizeof(int) + sizeof(float) + 4, 1, file);
    fclose(file);
    
    if (read_size != 1) {
        printf("Error reading file!\n");
        return 1;
    }
    
    // Ensure name is null-terminated
    data.name[4] = '\0';
    
    // Print the unpacked data
    printf("Data unpacked from data.bin:\n");
    printf("ID: %d\n", data.id_number);
    printf("Temperature: %.1f\n", data.temperature);
    printf("Name: %s\n", data.name);
    
    return 0;
}