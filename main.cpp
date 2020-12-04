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
void geraVetorNormal ();
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

//Vertices
//float *vertices;
float *verticesNormal;
//Linhas do arquivo
int linhas = 1;

int main()
{
    //geraVetorNormal();

    const float radius = 4.0f;

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
    char arqCasa[] = "arqCasa.csv";
    int linhasCasa = contarLinhas(arqCasa);
    float *verticesCasa = (float *)malloc((linhasCasa * 11)*sizeof(float));
    carregarVetor(verticesCasa,arqCasa);

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, casaVAO;
    glGenVertexArrays(1, &casaVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(linhasCasa*11), &verticesCasa[0], GL_STATIC_DRAW);

    glBindVertexArray(casaVAO);

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
    // -------------------------
    unsigned int texture[2];
    glGenTextures(2, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    // Podem ser GL_REPEAT. GL_MIRRORED_REPEAT, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    // Podem ser GL_LINEAR, GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;

    // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    stbi_set_flip_vertically_on_load(1);

    // Corrige o alinhamento da imagem em imagens cujas dimensões não são potências de dois
    // NPOT (Not Power-of-Two)
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //unsigned char *data = stbi_load("res/images/gremio.jpg", &width, &height, &nrChannels, 0);
    unsigned char *data = stbi_load("res/images/gremio2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // Se a imagem for PNG com transparência
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        // Se a imagem for JPG, e portanto sem transparência
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightcasaVAO;
    glGenVertexArrays(1, &lightcasaVAO);
    glBindVertexArray(lightcasaVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


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

        glBindTexture(GL_TEXTURE_2D, texture[0]);

        //Luz dando volta no Objeto!
        lightPos.x = sin(glfwGetTime()) * radius;
        lightPos.y = cos(glfwGetTime()) * radius;

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
        glBindVertexArray(casaVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasCasa*11);


        // also draw the lamp object
        //lightCubeShader.use();
        //lightCubeShader.setMat4("projection", projection);
        //lightCubeShader.setMat4("view", view);
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, lightPos);
        //model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        //lightCubeShader.setMat4("model", model);

//        glBindVertexArray(lightcasaVAO);
        glDrawArrays(GL_TRIANGLES, 0, linhasCasa);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &casaVAO);
    //glDeleteVertexArrays(1, &lightcasaVAO);
    glDeleteBuffers(1, &VBO);

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
    // 1ª abertura do arquivo para Verificar tamanho!
    FILE *arqin = fopen(arquivoLeitura, "rt"); // é um char criar define
    if (!arqin)
    {
        printf("Erro na abertura de %s %d\n",arquivoLeitura,strlen(arquivoLeitura));
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
    char c;
    char linha[100];
    char *pch;
    char *linhaComentario;
    float *ponteiro = vertices;

    //contarLinhas();
    //vertices = (float *)malloc((linhas * 11)*sizeof(float));

    //2ª abertura do arquivo para popular Vetor de Vertices
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
                    //vertices[i] =  atof(pch);

                    printf("vertices[%d]: %f\n ",i,ponteiro[i]);
                    i++;
                }

                pch = strtok(NULL, ";"); //Procura próximo token
            }
        }

    }
    fclose(arqin);
}

//void geraVetorNormal ()
//{
//    int i = 0,z = 0;
//    int contLinhas = 0;
//    int controle = 0;
//    int ultimaPosicao = 0;
//    char c;
//    char linha[100];
//    char *pch;
//    char *linhaComentario;
//    int contQuebraLinha = 0;
//
//
//    //contarLinhas();
//
//    verticesNormal = (float *)malloc((linhas * 8)*sizeof(float));
//
//    //2ª abertura do arquivo para popular Vetor de Vertices
//    FILE *arqin2 = fopen(arquivo, "rt");
//    controle = 1;
//    while (!feof(arqin2) && controle == 1)
//    {
//
//        if(contLinhas == 3)
//        {
//            glm::vec3 va(verticesNormal[ultimaPosicao - 24], verticesNormal[ultimaPosicao - 23], verticesNormal[ultimaPosicao - 22]);
//            //std::cout<<glm::to_string(va)<<std::endl;
//            glm::vec3 vb(verticesNormal[ultimaPosicao - 16], verticesNormal[ultimaPosicao - 15], verticesNormal[ultimaPosicao - 14]);
//            //std::cout<<glm::to_string(vb)<<std::endl;
//            glm::vec3 vc(verticesNormal[ultimaPosicao - 8], verticesNormal[ultimaPosicao - 7], verticesNormal[ultimaPosicao - 6]);
//            //std::cout<<glm::to_string(vc)<<std::endl;
//            glm::vec3 normal = normalize(cross(vc - va,vb - va));
//            //std::cout<<glm::to_string(normal)<<std::endl;
//
//
//            char Str[100];
//            FILE *arq;
//
//            arq = fopen("ArqGrav.csv", "a+");
//            if (arq == NULL) // Se não conseguiu criar
//            {
//                printf("Problemas na CRIACAO do arquivo\n");
//                return;
//            }
//
//            for (z = (ultimaPosicao - 24); z < ultimaPosicao ; z++)
//            {
//                fprintf(arq,"%-5.2f;",verticesNormal[z]);
//
//                contQuebraLinha++;
//                if (contQuebraLinha == 8)
//                {
//                    fprintf(arq,"%-5.2f;",normal[0]);
//                    fprintf(arq,"%-5.2f;",normal[1]);
//                    fprintf(arq,"%-5.2f;",normal[2]);
//                    fprintf(arq,"\n");
//                    contQuebraLinha = 0;
//                }
//
//
//                //fputs(verticesNormal[z],arq);
//                //fputs(';',arq);
//
//            }
//            fclose(arq);
//            contLinhas = 0;
//            contQuebraLinha = 0;
//            if (feof(arqin2)){
//                controle = 0;
//            }
//        }
//        else
//        {
//
//            fgets(linha, 100, arqin2);
//            linhaComentario = strstr(linha, "//");
//
//            if (linhaComentario == NULL)
//            {
//                contLinhas++;
//                pch = strtok(linha, ";");
//                while (pch != NULL) //Enquanto houver token
//                {
//                    int validarNumerico = strcmp(pch,"\n");
//                    if (validarNumerico)
//                    {
//                        *(verticesNormal+i) =  atof(pch);
//                        //printf("verticesNormal[%d]: %f\n ",i,verticesNormal[i]);
//                        i++;
//                        ultimaPosicao = i;
//                    }
//                    pch = strtok(NULL, ";"); //Procura próximo token
//                }
//            }
//        }
//    }
//    fclose(arqin2);
//}




