// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <zip.h>
#include <zipconf.h>
#include "glew.h"
#include "glfw3.h"
using namespace std;

struct Vertex {
    float x, y, z;
};
// Shader source code
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    void main()
    {
        gl_Position = vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    }
)";

// Read PLY file and extract vertex positions
bool readPLY(const std::string& filename, std::vector<Vertex>& vertices) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open PLY file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int numVertices = 0;
    bool readingVertices = false;

    while (std::getline(file, line)) {
        std::istringstream iss(line);

        if (!readingVertices) {
            if (line.find("element vertex") != std::string::npos) {
                iss >> line >> line >> numVertices;
            } else if (line.find("end_header") != std::string::npos) {
                readingVertices = true;
            }
        } else {
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);

            if (vertices.size() == numVertices) {
                break;
            }
        }
    }

    file.close();
    return true;
}

int readZip(char* filePathParam) {
    zip* archive = zip_open(filePathParam, 0, nullptr);

    if (archive)
    {
        int numEntries = zip_get_num_entries(archive, 0);

        for (int i = 0; i < numEntries; ++i)
        {
            struct zip_stat fileStat;
            zip_stat_init(&fileStat);
            zip_stat_index(archive, i, 0, &fileStat);

            // Assuming PLY files have ".ply" extension
            if (strstr(fileStat.name, ".ply"))
            {
                std::vector<Vertex> vertices;
                std::string filePath = "extracted/" + std::string(fileStat.name); // Adjust the path as needed

                // Extract the file
                zip_file* file = zip_fopen_index(archive, i, 0);
                if (file)
                {
                    std::ofstream outFile(filePath, std::ios::binary);
                    char buffer[1024];
                    zip_int64_t bytesRead;

                    while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0)
                    {
                        outFile.write(buffer, bytesRead);
                    }

                    outFile.close();
                    zip_fclose(file);

                    // Read and render the PLY file
                    if (readPLY(filePath, vertices))
                    {
                        // Render the vertices (same as before)
                        // ...
                    }
                }
            }
        }

        zip_close(archive);
    }
    return 0;
}

int RenderVer(char* filePathPLY) {
    // Initialize GLFW and create a window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 600, "PLY Renderer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glewInit();

    // Create and compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Read PLY file and extract vertex positions
    std::vector<Vertex> vertices;
    if (readPLY(filePathPLY, vertices))
    {
        // Create and bind vertex buffer
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Set up vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        RenderVer(argv[i]);
        int num;
        std::cin >> num;
    }
    return 0;
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
