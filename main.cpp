#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtx/string_cast.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_m.h"
#include "camera.h"
#include "include/stb_image.h"
#include "include/Shader.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
int contarLinhas(char arquivoLeitura[]);
void carregarVetor (float *vertices, char arquivoLeitura[]);
void geraVetorNormal (char nomeArquivoLeitura[], char nomeArquivoEscrita[]);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 6.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Reflexo especular
float specularStrength = 0.5;

int main()
{
    const float radius = 4.0f;

    //Gera os arquivos com os vetores normais.
//    geraVetorNormal("res/arquivos/Chamine8.csv"     ,"res/arquivos/Chamine.csv");
//    geraVetorNormal("res/arquivos/FolhasArvore8.csv","res/arquivos/FolhasArvore.csv");
//    geraVetorNormal("res/arquivos/Paredes8.csv"     ,"res/arquivos/Paredes.csv");
//    geraVetorNormal("res/arquivos/PortaJanela8.csv" ,"res/arquivos/PortaJanela.csv");
//    geraVetorNormal("res/arquivos/Telhado8.csv"     ,"res/arquivos/Telhado.csv");
//    geraVetorNormal("res/arquivos/TroncoArvore8.csv","res/arquivos/TroncoArvore.csv");

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modelo de Iluminação Phong", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glew: load all OpenGL function pointers
    // ---------------------------------------
    if(glewInit()!=GLEW_OK)
    {
        std::cout << "Ocorreu um erro iniciando GLEW!" << std::endl;
    }
    else
    {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("phong_lighting.vs", "phong_lighting.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");

    //Carregar as coordernadas do desenho no vetor
    unsigned int texture[7];
    glGenTextures(7, &texture[0]);

    //Arquivo Chamine
    char arqChamine[] = "res/arquivos/Chamine.csv";
    int linhasChamine = contarLinhas(arqChamine);
    float *verticesChamine = (float *)malloc((linhasChamine * 11)*sizeof(float));
    carregarVetor(verticesChamine,arqChamine);

    // first, configure the cube's VAO (and VBO)
    unsigned int ChamineVBO, ChamineVAO;
    glGenVertexArrays(1, &ChamineVAO);
    glGenBuffers(1, &ChamineVBO);

    glBindBuffer(GL_ARRAY_BUFFER, ChamineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasChamine*11), &verticesChamine[0], GL_STATIC_DRAW);

    glBindVertexArray(ChamineVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture;
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;

    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    unsigned char *data = stbi_load("res/images/Paredes.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture1" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture1" << std::endl;
    }
    stbi_image_free(data);

//    //Arquivo FolhasArvore
    char arqFolhasArvore[] = "res/arquivos/FolhasArvore.csv";
    int linhasFolhasArvore = contarLinhas(arqFolhasArvore);
    float *verticesFolhasArvore = (float *)malloc((linhasFolhasArvore * 11)*sizeof(float));
    carregarVetor(verticesFolhasArvore,arqFolhasArvore);

    // first, configure the cube's VAO (and VBO)
    unsigned int FolhasArvoreVBO, FolhasArvoreVAO;
    glGenVertexArrays(1, &FolhasArvoreVAO);
    glGenBuffers(1, &FolhasArvoreVBO);

    glBindBuffer(GL_ARRAY_BUFFER, FolhasArvoreVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasFolhasArvore*11), &verticesFolhasArvore[0], GL_STATIC_DRAW);

    glBindVertexArray(FolhasArvoreVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture
    glBindTexture(GL_TEXTURE_2D, texture[1]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    // Podem ser GL_REPEAT. GL_MIRRORED_REPEAT, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    // Podem ser GL_LINEAR, GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    // Corrige o alinhamento da imagem em imagens cujas dimensões não são potências de dois
    // NPOT (Not Power-of-Two)
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    data = stbi_load("res/images/FolhasArvore.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture2" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture2" << std::endl;
    }
    stbi_image_free(data);

//    //Arquivo Paredes
    char arqParedes[] = "res/arquivos/Paredes.csv";
    int linhasParedes = contarLinhas(arqParedes);
    float *verticesParedes = (float *)malloc((linhasParedes * 11)*sizeof(float));
    carregarVetor(verticesParedes,arqParedes);

    // first, configure the cube's VAO (and VBO)
    unsigned int ParedesVBO, ParedesVAO;
    glGenVertexArrays(1, &ParedesVAO);
    glGenBuffers(1, &ParedesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, ParedesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasParedes*11), &verticesParedes[0], GL_STATIC_DRAW);

    glBindVertexArray(ParedesVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture
    glBindTexture(GL_TEXTURE_2D, texture[2]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    data = stbi_load("res/images/Paredes.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture3" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture3" << std::endl;
    }
    stbi_image_free(data);

    //Arquivo PortaJanela
    char arqPortaJanela[] = "res/arquivos/PortaJanela.csv";
    int linhasPortaJanela = contarLinhas(arqPortaJanela);
    float *verticesPortaJanela = (float *)malloc((linhasPortaJanela * 11)*sizeof(float));
    carregarVetor(verticesPortaJanela,arqPortaJanela);

    // first, configure the cube's VAO (and VBO)
    unsigned int PortaJanelaVBO, PortaJanelaVAO;
    glGenVertexArrays(1, &PortaJanelaVAO);
    glGenBuffers(1, &PortaJanelaVBO);

    glBindBuffer(GL_ARRAY_BUFFER, PortaJanelaVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasPortaJanela*11), &verticesPortaJanela[0], GL_STATIC_DRAW);

    glBindVertexArray(PortaJanelaVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture
    glBindTexture(GL_TEXTURE_2D, texture[3]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    data = stbi_load("res/images/PortaJanela.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture4" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture4" << std::endl;
    }
    stbi_image_free(data);

//    //Arquivo Telhado
    char arqTelhado[] = "res/arquivos/Telhado.csv";
    int linhasTelhado = contarLinhas(arqTelhado);
    float *verticesTelhado = (float *)malloc((linhasTelhado * 11)*sizeof(float));
    carregarVetor(verticesTelhado,arqTelhado);

    // first, configure the cube's VAO (and VBO)
    unsigned int TelhadoVBO, TelhadoVAO;
    glGenVertexArrays(1, &TelhadoVAO);
    glGenBuffers(1, &TelhadoVBO);

    glBindBuffer(GL_ARRAY_BUFFER, TelhadoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasTelhado*11), &verticesTelhado[0], GL_STATIC_DRAW);

    glBindVertexArray(TelhadoVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture
    glBindTexture(GL_TEXTURE_2D, texture[4]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    data = stbi_load("res/images/Telhado.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture5" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture5" << std::endl;
    }
    stbi_image_free(data);

    //Arquivo TroncoArvore
    char arqTroncoArvore[] = "res/arquivos/TroncoArvore.csv";
    int linhasTroncoArvore = contarLinhas(arqTroncoArvore);
    float *verticesTroncoArvore = (float *)malloc((linhasTroncoArvore * 11)*sizeof(float));
    carregarVetor(verticesTroncoArvore,arqTroncoArvore);

    // first, configure the cube's VAO (and VBO)
    unsigned int TroncoArvoreVBO, TroncoArvoreVAO;
    glGenVertexArrays(1, &TroncoArvoreVAO);
    glGenBuffers(1, &TroncoArvoreVBO);

    glBindBuffer(GL_ARRAY_BUFFER, TroncoArvoreVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasTroncoArvore*11), &verticesTroncoArvore[0], GL_STATIC_DRAW);

    glBindVertexArray(TroncoArvoreVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture
    glBindTexture(GL_TEXTURE_2D, texture[5]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    data = stbi_load("res/images/TroncoArvore.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture6" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture6" << std::endl;
    }
    stbi_image_free(data);

        //Arquivo Sol
    char arqSol[] = "res/arquivos/Sol.csv";
    int linhasSol = contarLinhas(arqSol);
    float *verticesSol = (float *)malloc((linhasSol * 11)*sizeof(float));
    carregarVetor(verticesSol,arqSol);

    // first, configure the cube's VAO (and VBO)
    unsigned int SolVBO, SolVAO;
    glGenVertexArrays(1, &SolVAO);
    glGenBuffers(1, &SolVBO);

    glBindBuffer(GL_ARRAY_BUFFER, SolVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasSol*11), &verticesSol[0], GL_STATIC_DRAW);

    glBindVertexArray(SolVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // load and create a texture
    glBindTexture(GL_TEXTURE_2D, texture[6]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    data = stbi_load("res/images/Sol.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "success to load texture Sol" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture Sol" << std::endl;
    }
    stbi_image_free(data);



    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Luz dando volta no Objeto!
        lightPos.x = sin(glfwGetTime()*0.5) * radius;
        lightPos.y = cos(glfwGetTime()*0.5) * radius;

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("specularStrength",specularStrength);


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        // Demonstra o problema da distorção do vetor normal
        //model = glm::scale(model, glm::vec3(0.5f, 0.2f, 3.0f)); // transformação de escala não linear
        lightingShader.setMat4("model", model);

        // render the cube
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glBindVertexArray(ChamineVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasChamine * 11);

        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glBindVertexArray(FolhasArvoreVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasFolhasArvore * 11);

        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glBindVertexArray(ParedesVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasParedes * 11);

        glBindTexture(GL_TEXTURE_2D, texture[3]);
        glBindVertexArray(PortaJanelaVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasPortaJanela * 11);

        glBindTexture(GL_TEXTURE_2D, texture[4]);
        glBindVertexArray(TelhadoVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasTelhado * 11);

        glBindTexture(GL_TEXTURE_2D, texture[5]);
        glBindVertexArray(TroncoArvoreVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasTroncoArvore * 11);



         //also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(10.0f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glBindTexture(GL_TEXTURE_2D, texture[6]);
        glBindVertexArray(SolVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasSol*11);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &ChamineVAO);
    glDeleteBuffers(1, &ChamineVBO);

    glDeleteVertexArrays(1, &FolhasArvoreVAO);
    glDeleteBuffers(1, &FolhasArvoreVBO);

    glDeleteVertexArrays(1, &ParedesVAO);
    glDeleteBuffers(1, &ParedesVBO);

    glDeleteVertexArrays(1, &PortaJanelaVAO);
    glDeleteBuffers(1, &PortaJanelaVBO);

    glDeleteVertexArrays(1, &TelhadoVAO);
    glDeleteBuffers(1, &TelhadoVBO);

    glDeleteVertexArrays(1, &TroncoArvoreVAO);
    glDeleteBuffers(1, &TroncoArvoreVBO);

    glDeleteVertexArrays(1, &SolVAO);
    glDeleteBuffers(1, &SolVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        specularStrength += 0.01f;
        if(specularStrength > 5.0f)
            specularStrength = 5.0f;
        std::cout << "Especular = " << specularStrength << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        specularStrength -= 0.01f;
        if(specularStrength < 0.0f)
            specularStrength = 0.0f;
        std::cout << "Especular = " << specularStrength << std::endl;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

int contarLinhas(char arquivoLeitura[])
{
    int linhas = 0;
    char c;

    FILE *arqin = fopen(arquivoLeitura, "rt");
    if (!arqin)
    {
        printf("Erro na abertura de %s\n",arquivoLeitura);
        exit(0);
    }

    while(fread (&c, sizeof(char), 1, arqin))
    {
        if(c == '\n') linhas++;
    }

    printf("Linhas: %d \n",linhas);

    fclose(arqin);
    return linhas;
}

void carregarVetor (float vertices[], char arquivoLeitura[])
{
    int i = 0;
    char linha[100];
    char *pch;
    char *linhaComentario;
    float *ponteiro = vertices;

    FILE *arqin = fopen(arquivoLeitura, "rt");
    while (!feof(arqin))
    {
        fgets(linha, 100, arqin);

        linhaComentario = strstr(linha, "//");

        if (linhaComentario == NULL)
        {
            pch = strtok(linha, ";");
            while (pch != NULL) //Enquanto houver token
            {
                int validarNumerico = strcmp(pch,"\n");
                if (validarNumerico)
                {
                    *(ponteiro+i) =  atof(pch);

                    printf("vertices[%d]: %f\n ",i,ponteiro[i]);
                    i++;
                }

                pch = strtok(NULL, ";"); //Procura próximo token
            }
        }
    }

    fclose(arqin);
}

void geraVetorNormal (char nomeArquivoLeitura[], char nomeArquivoEscrita[])
{
    int i = 0,z = 0;
    int contLinhas = 0;
    int controle = 0;
    int ultimaPosicao = 0;
    char linha[100];
    char *pch;
    char *linhaComentario;
    int contQuebraLinha = 0;

    int linhasVetorNormal = contarLinhas(nomeArquivoLeitura);
    float *verticesNormal = (float *)malloc((linhasVetorNormal * 8)*sizeof(float));

    //Abre arquivo de escrita
    FILE *arquivoEscrita = fopen(nomeArquivoEscrita, "a+");
    if (arquivoEscrita == NULL)
    {
        printf("Problemas na CRIACAO do arquivo\n");
        return;
    }

    //Abre arquivo de leitura
    FILE *ArquivoLeitura = fopen(nomeArquivoLeitura, "rt");
    controle = 1;
    while (!feof(ArquivoLeitura) && controle == 1)
    {
        if(contLinhas == 3)
        {
            glm::vec3 va(verticesNormal[ultimaPosicao - 24 ], verticesNormal[ultimaPosicao - 23], verticesNormal[ultimaPosicao - 22]);
            //std::cout<<glm::to_string(va)<<std::endl;
            glm::vec3 vb(verticesNormal[ultimaPosicao - 16], verticesNormal[ultimaPosicao - 15], verticesNormal[ultimaPosicao - 14]);
            //std::cout<<glm::to_string(vb)<<std::endl;
            glm::vec3 vc(verticesNormal[ultimaPosicao - 8], verticesNormal[ultimaPosicao - 7], verticesNormal[ultimaPosicao - 6]);
            //std::cout<<glm::to_string(vc)<<std::endl;
            glm::vec3 normal = normalize(cross(vc - va,vb - va));
            //std::cout<<glm::to_string(normal)<<std::endl;

            for (z = (ultimaPosicao - 24); z < ultimaPosicao ; z++)
            {
                fprintf(arquivoEscrita,"%-5.2f;",verticesNormal[z]);

                contQuebraLinha++;
                if (contQuebraLinha == 8)
                {
                    fprintf(arquivoEscrita,"%-5.2f;",normal[0]);
                    fprintf(arquivoEscrita,"%-5.2f;",normal[1]);
                    fprintf(arquivoEscrita,"%-5.2f;",normal[2]);
                    fprintf(arquivoEscrita,"\n");
                    contQuebraLinha = 0;
                }
            }

            contLinhas = 0;
            contQuebraLinha = 0;

            if (feof(ArquivoLeitura)){
                controle = 0;
            }
        }
        else
        {
            fgets(linha, 100, ArquivoLeitura);
            linhaComentario = strstr(linha, "//");

            if (linhaComentario == NULL)
            {
                contLinhas++;
                pch = strtok(linha, ";");

                while (pch != NULL) //Enquanto houver token
                {
                    int validarNumerico = strcmp(pch,"\n");
                    if (validarNumerico)
                    {
                        *(verticesNormal + i) =  atof(pch);
                        //printf("verticesNormal[%d]: %f\n ",i,verticesNormal[i]);
                        i++;
                        ultimaPosicao = i;
                    }
                    pch = strtok(NULL, ";"); //Procura próximo token
                }
            }
        }
    }

    fclose(arquivoEscrita);
    fclose(ArquivoLeitura);
}




