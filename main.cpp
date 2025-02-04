#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <sstream>
#include<algorithm>

// Shaders --------------------------------------------------------------------------------------->
// Vertex Shader (updated for lighting)
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 mvp;
uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = mvp * vec4(aPos, 1.0);
}
)glsl";

// Vertex shader (for gouraud shading)
const char* vertexShaderGouraudSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 mvp;
uniform mat4 model;
uniform vec3 lightDir;    // Directional light direction
uniform vec3 lightColor;
uniform vec3 viewPos;     // Camera position (world space)
uniform float shininess;  // Controls sharpness of specular highlight
uniform vec3 objectColor;

out vec3 Color; // Color to be interpolated across the face

void main() {
    // Transform vertex position to world space
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));

    // Transform normal to world space (correct for non-uniform scaling)
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    vec3 norm = normalize(Normal);

    // Light direction (normalized)
    vec3 lightDirNorm = normalize(-lightDir);

    // View direction (from fragment to camera)
    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    // Combine all components
    Color = (ambient + diffuse + specular) * objectColor; 

    gl_Position = mvp * vec4(aPos, 1.0);
}
)glsl";

// Fragment Shader (updated for phong shading lighting)
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos; // Camera Position
uniform float shininess; // Shininess exponent

void main() {
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm); // reflect light direction
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    // Combine
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)glsl";

// Fragment Shader (updated for blinn-phong shading and lighting)
const char* fragmentShaderBlinnPhongSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos; // Camera Position
uniform float shininess; // Shininess exponent

void main() {
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(-lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDirNorm + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    // Combine
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)glsl";

// Fragment shader for gouraud shading
const char* fragmentShaderGouraudSource = R"glsl(
#version 330 core
in vec3 Color;
out vec4 FragColor;

void main() {
    FragColor = vec4(Color, 1.0);
}
)glsl";


// ----------------------------------------------------------------------------------------------->

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;

    // Add equality operator.
    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal;
    }

    // Add inequality operator as well.
    bool operator !=(const Vertex& other) const {
        return!(*this == other);
    }

};

struct Mesh {
    std::vector<Vertex> vertices; // interleaved positions + normals
    std::vector<unsigned int> indices;
    std::vector<unsigned int> edgeIndices;
};

/*
* Load the mesh specifications from an OBJ file.
*/ 
Mesh loadOBJ(const char* path) {
    Mesh mesh;
    std::ifstream file(path);
    std::string line;

    // Temporary Data
    std::vector<glm::vec3> tempPositions;
    std::vector<glm::vec3> tempNormals;
    std::vector<std::string> faceTokens;


    // Parse the OBJ file to get the required data.
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") { // Positions Extraction
            std::istringstream ss(line.substr(2));
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            tempPositions.push_back(vertex);
        }
        else if (line.substr(0, 3) == "vn ") { // Normals extraction
            std::istringstream ss(line.substr(3));
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (line.substr(0, 2) == "f ") { // face extraction
            faceTokens.clear();
            std::istringstream ss(line.substr(2));
            std::string token;

            while (ss >> token) {
                faceTokens.push_back(token);
            }

            // Triangulate face (assuming convex polygon)
            for (size_t i = 1; i < faceTokens.size() - 1; i++) {
                

                // Parse the three vertices of a triangle.
                for (int j = 0; j < 3; j++) {
                    
                    // Get the position/normals indices for this vertex.
                    size_t idx = (j == 0) ? 0 : (j == 1 ? i : i + 1);
                    std::string& face = faceTokens[idx];

                    // split v/vt/vn into parts
                    size_t pos1 = face.find("/");
                    size_t pos2 = face.find("/", pos1 + 1);

                    // Extract the indices (OBJ uses 1-based indexing)
                    int posIdx = std::stoi(face.substr(0, pos1)) - 1;
                    int normIdx = std::stoi(face.substr(pos2 + 1)) - 1;

                    // Create a vertex with a position and normal
                    Vertex vertex;
                    vertex.position = tempPositions[posIdx];
                    vertex.normal = tempNormals[normIdx];

                    // Check if this vertex already exists.
                    auto it = std::find(mesh.vertices.begin(), mesh.vertices.end(), vertex);
                    if (it == mesh.vertices.end()) {
                        // New vertex: add to the list and reference by Index.
                        mesh.vertices.push_back(vertex);
                        mesh.indices.push_back(mesh.vertices.size() - 1);
                    }
                    else {
                        // Existing vertex: reuse
                        mesh.indices.push_back(it - mesh.vertices.begin());
                    }
                }
            }
        }
    }

    return mesh;
}


/*
* Compile the shaders
*/
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "Shader error:\n" << infoLog << std::endl;
    }
    return id;
}

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Teapot with Lighting", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Load teapot data
    Mesh mesh = loadOBJ("teapot.obj");

    // Create and bind buffers
    unsigned int VAO, VBO, EBO, NBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &NBO);

    glBindVertexArray(VAO);


    // Interleave vertex data: [positionX, positionY, positionZ, normal.X, normal.Y, normal.Z]
    
    std::vector<float> interleavedData;

    for (const Vertex& v : mesh.vertices) {
        interleavedData.push_back(v.position.x);
        interleavedData.push_back(v.position.y);
        interleavedData.push_back(v.position.z);
        interleavedData.push_back(v.normal.x);
        interleavedData.push_back(v.normal.y);
        interleavedData.push_back(v.normal.z);
    }
    // Vertex buffer (interleaved)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleavedData.size() * sizeof(float), interleavedData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Position attribute (location=0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal Attribute(location=1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    // Create shaders
    unsigned int mainShader = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    unsigned int blinnPhongShaderAlt = createShaderProgram(vertexShaderSource, fragmentShaderBlinnPhongSource);
    unsigned int gouraudShadingAlt = createShaderProgram(vertexShaderGouraudSource, fragmentShaderGouraudSource);
    unsigned int currentShader = mainShader;

    glEnable(GL_DEPTH_TEST); // enable z-buffer during the rasterization stage. 

    // Lighting setup
    glm::vec3 lightDir(-0.2f, -1.0f, -0.3f); // Directional light
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);  // White light
    float redT = 1.0f; // red component
    float blueT = 0.0f; // blue component
    float greenT = 0.0f; // green component
    glm::vec3 objectColor(redT, greenT, blueT); // Red teapot
    char currentObjectColourToggle = 'R'; // 'R' -> red, 'G' -> green, 'B' -> blue
    float shininess = 32.0f;

    // Rotation and zoom variables
    float angleY = 0.0f;
    float angleZ = 0.0f;
    float rotationSpeed = 2.0f;
    float zoomSpeed = 10.0f;
    float cameraDistance = 41.56921938165306f; // Initial distance (sqrt(3²+3²+3²)) * 8
    glm::vec3 cameraDir = glm::normalize(glm::vec3(1.0f)); // Original direction (3,3,3) normalized
    float lastFrameTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;

        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Rotation controls
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            angleY += rotationSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            angleY -= rotationSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            angleZ += rotationSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            angleZ -= rotationSpeed * deltaTime;

        // Zoom controls
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            cameraDistance -= zoomSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraDistance += zoomSpeed * deltaTime;

        // Teapot colour controls
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            currentObjectColourToggle = 'R';
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            currentObjectColourToggle = 'G';
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            currentObjectColourToggle = 'B';
        }

        // Toggle shaders (gourad, phong, blinn-phong shading)
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            currentShader = mainShader;
            shininess = 32.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            currentShader = blinnPhongShaderAlt;
            shininess = 45.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            currentShader = gouraudShadingAlt;
            shininess = 20.0f;
        }

        // Adjust colours if keypress
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {

            switch (currentObjectColourToggle) {

            case 'R':
                redT += 0.01f;
                break;
            case 'G':
                greenT += 0.01f;
                break;
            case 'B':
                blueT += 0.01f;
                break;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {

            switch (currentObjectColourToggle) {

            case 'R':
                redT -= 0.01f;
                break;
            case 'G':
                greenT -= 0.01f;
                break;
            case 'B':
                blueT -= 0.01f;
                break;
            }
        }

        // Adjust the teapot colour every frame, in case it changed.
        objectColor[0] = redT;
        objectColor[1] = greenT;
        objectColor[2] = blueT;

        // Clamp camera distance
        cameraDistance = glm::clamp(cameraDistance, 1.5f, 40.0f);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate camera position
        glm::vec3 eye = cameraDir * cameraDistance;

        // Create transformation matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(
            eye,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, angleY, glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis
        model = glm::rotate(model, angleZ, glm::vec3(0.0f, 0.0f, 1.0f)); // Z-axis

        glm::mat4 mvp = projection * view * model;

        // Draw main teapot
        glUseProgram(currentShader);
        glUniformMatrix4fv(glGetUniformLocation(currentShader, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(glGetUniformLocation(currentShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(currentShader, "objectColor"), 1, &objectColor[0]);
        glUniform3fv(glGetUniformLocation(currentShader, "lightDir"), 1, &lightDir[0]);
        glUniform3fv(glGetUniformLocation(currentShader, "lightColor"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(currentShader, "viewPos"), 1, &eye[0]);
        glUniform1f(glGetUniformLocation(currentShader, "shininess"), shininess);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &NBO);
    glDeleteProgram(mainShader);

    glfwTerminate();
    return 0;
}
