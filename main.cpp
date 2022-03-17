// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We use an orthographic projection
//   as the default projetion.

#include <fstream>
#include <stdlib.h>     /* srand, rand */

#define GL_SILENCE_DEPRECATION // Silence macOs deprecation errors
#include "Angel.h"

// RGBA colors
vec4 vertex_colors[9] = {
    vec4(0.0, 0.0, 0.0, 1.0),  // black
    vec4(1.0, 0.0, 0.0, 1.0),  // red
    vec4(1.0, 1.0, 0.0, 1.0),  // yellow
    vec4(0.0, 1.0, 0.0, 1.0),  // green
    vec4(0.0, 0.0, 1.0, 1.0),  // blue
    vec4(1.0, 0.0, 1.0, 1.0),  // magenta
    vec4(1.0, 1.0, 1.0, 1.0),  // white
    vec4(0.0, 1.0, 1.0, 1.0),   // cyan
    vec4(0.5, 0.5, 0.5, 1.0)   // gray
};

/**
 * Holds position of an object, used for transformation matrix
 */
struct Position3D {
    vec3 position;
    vec3 angle;
    vec3 scale;
    Position3D(vec3 p = {0}, vec3 a = {0}, vec3 s = {1})
    :position(p), angle(a), scale(s) {

    }
    Position3D operator + (const Position3D & second) const {
        Position3D x(position, angle, scale);
        return x += second;
    }
    Position3D operator += (const Position3D & second) {
        position += second.position;
        angle += second.angle;
        scale += second.scale;
        return *this;
    }
    Position3D operator *= (const float second) {
        position *= second;
        angle *= second;
        scale *= second;
        return *this;
    }
    Position3D operator * (const float second) {
        Position3D t(position, angle, scale);
        return t *= second;
    }
};

/**
 * A struct holding a list of vertices and associated colors
 */
struct VertexModel {
    VertexModel(int vertexCount = 0)
    :vertexCount(vertexCount) {
        if (vertexCount > 0) {
            this->vertex = new vec4[vertexCount];
        }
    }
    int vertexCount;
    vec4 *vertex = 0;
    ~VertexModel() {
        // if (vertex)
        //     delete[] vertex;
    }
};


/**
 * The app object
 * Holds all global state used in the app
 */
struct {
    vec3 worldSize = vec3(20, 10, 10);
    float frameCount = 0; // Total frames rendered
    float fps = 0; // Average FPS
    float playSpeed = 1; // Speeds up everything by this factor
    Position3D ball; // Position of the ball
    Position3D ballSpeed; // Speed of the ball

    int rotateAxis; // Which access to rotate on idle

    bool wireframe = true; // Whether to use wireframes or not
    int menu, submenu[3]; // OpenGL menu handles
    vec4 color = vertex_colors[0];

    GLuint vColor, vPosition; // Shader variables
    GLuint uTime, uColor; // Uniform variable
    GLuint ModelView, Projection; // OpenGL handles

    VertexModel model;
    int modelIndex = 0; // Index of the model among models
    VertexModel defaultModels[3]; // Cube, Sphere, Bunny
} app; // The global app object



//----------------------------------------------------------------------------

/**
 * Create a sphere model and return
 * @return VertexModel
 */
VertexModel sphereModel(int count = 1000) {

    int stackCount = sqrt(count) - 1;
    int sectorCount = sqrt(count) - 1;
    // std::cout << stackCount << " " << sectorCount << std::endl;

    float radius = 1; // Radius of circle

    int vertexCount = (stackCount+1) * (sectorCount+1);
    vec4 *vertices = new vec4[vertexCount];

    int index = 0;
    float x, y, z, xy;

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    // Generate vertices of the sphere
    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

            vertices[index++] = vec4(x, y, z, 1.0);
        }
    }
    // std::cout << index << std::endl;


    // Use vertices to generate triangles
    VertexModel m(index * 6); // Each face (quad) has 6 vertices
    index = 0;
    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;
        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0) {
                m.vertex[index++] = vertices[k1];
                m.vertex[index++] = vertices[k2];
                m.vertex[index++] = vertices[k1 + 1];
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1)) {
                m.vertex[index++] = vertices[k1 + 1];
                m.vertex[index++] = vertices[k2];
                m.vertex[index++] = vertices[k2 + 1];
            }
        }
    }
    delete[] vertices;

    return m;
}

/**
 * Create a cube model and return
 * @return VertexModel
 */
VertexModel cubeModel() {
    VertexModel m(36);
    vec4 vertices[8] = {
        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4(-0.5,  0.5,  0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0),
        vec4( 0.5, -0.5,  0.5, 1.0),
        vec4(-0.5, -0.5, -0.5, 1.0),
        vec4(-0.5,  0.5, -0.5, 1.0),
        vec4( 0.5,  0.5, -0.5, 1.0),
        vec4( 0.5, -0.5, -0.5, 1.0)
    };
    int i=0;
    // Lambda quad() with access to local vars
    auto quad = [&](int &i, int a, int b, int c, int d) {
        m.vertex[i++] = vertices[a];
        m.vertex[i++] = vertices[b];
        m.vertex[i++] = vertices[c];
        m.vertex[i++] = vertices[a];
        m.vertex[i++] = vertices[c];
        m.vertex[i++] = vertices[d];
    };
    quad(i, 1, 0, 3, 2);
    quad(i, 2, 3, 7, 6);
    quad(i, 3, 0, 4, 7);
    quad(i, 6, 5, 1, 2);
    quad(i, 4, 5, 6, 7);
    quad(i, 5, 4, 0, 1);
    return m;
}

//----------------------------------------------------------------------------

VertexModel loadOff(const char * filename) {
    std::ifstream infile(filename);
    int vertexCount;
    int triangleCount;
    std::string temp;
    infile >>temp; // OFF header
    // Read number of vertices and triangles
    infile >>vertexCount >>triangleCount >>temp;
    // Create a model with triangleCount * 3 vertices
    VertexModel m(triangleCount*3);

    // Read vertices from OFF file into array
    vec4 *vertices = new vec4[vertexCount];
    for (int i=0; i<vertexCount; ++i) {
        infile >>vertices[i][0] >>vertices[i][1] >>vertices[i][2];
        vertices[i][3] = 1.0;
    }

    // Now each line has 3 x y z which are indices of the vertices
    // from above, each line forming 1 triangle
    int vindex[3];
    int index = 0;
    for (int i=0; i<triangleCount; ++i)
    {
        infile >>temp >>vindex[0] >>vindex[1] >>vindex[2];
        for (int j=0; j<3; ++j)
            m.vertex[index++] = vertices[vindex[j]];
    }

    delete[] vertices;
    return m;
}

/**
 * Scale a model up or down
 * @param model VertexModel
 * @param scale vec4
 */
void scaleModel(VertexModel &model, vec4 scale) {
    for (int i=0; i<model.vertexCount; ++i)
        model.vertex[i] *=scale;
}

/**
 * Fill the vertex buffer used in the shaders
 * @param model VertexModel
 */
void fillBuffer(VertexModel &model) {
    uint vertexSize = model.vertexCount * sizeof(model.vertex[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexSize,
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, model.vertex);
}

/**
 * Reset ball to the initial position and speed
 */
void resetBall() {
    app.ball.position = app.worldSize
        * vec3(-1, 1, 0) // Top-left of screen (-X, Y, 0)
        + vec3(1, -1, 0); // Size of ball
    // app.ball.angle = vec3(0, 0, 0); // Continue from last angle
    app.ball.scale = vec3(1, 1, 1);
    float initialSpeed = rand() % 3 + 2;
    app.ballSpeed.position = vec3(initialSpeed, 0, 0);
    app.ballSpeed.angle = vec3(1, 1, 1) * 10 * initialSpeed; // Rotate ever slightly
    app.ballSpeed.scale = vec3(0, 0, 0); // Do not change scale
}

/**
 * Do the physics calculation of the app
 */
void doPhysics(float timeDiff) {
    const float g = 9.8; // Earth gravity constant
    const vec3 vg = vec3(0, -g, 0);
    const float co = timeDiff * app.playSpeed;


    app.ballSpeed.position += vg * co; // Speed modified by gravity
    app.ball += app.ballSpeed * co; // Position modified by speed
    // std::cout << app.ballSpeed.position << std::endl;

    // Collision with bottom of world
    if (app.ball.position.y <= -(app.worldSize.y - 1)) { // Size is 1
        app.ballSpeed.position.y = -app.ballSpeed.position.y;
        app.ballSpeed *= .9; // Lose some momentum on the entire speed
        app.ball.position.y = -(app.worldSize.y - 1); // Can't go any lower

        if (fabs(app.ballSpeed.position.y) < 0.5) { // Speed too little, set to zero
            app.ballSpeed.position.y = 0;
        }
    }

    // Collision with walls
    if ((app.ball.position.x >= (app.worldSize.x - 1))
        || (app.ball.position.x <= -(app.worldSize.x - 1))) {
        app.ballSpeed.position.x = -app.ballSpeed.position.x;
        if (fabs(app.ballSpeed.position.x) < 0.5) // Speed too little, set to zero
            app.ballSpeed.position.x = 0;
    }

    // // Rotate at a fixed speed controlled by the mouse
    // app.ball.angle[app.rotateAxis] += 5 * co;
    // if (app.ball.angle[app.rotateAxis] > 360.0) {
    //     app.ball.angle[app.rotateAxis] -= 360.0;
    // }

}

// OpenGL initialization
void init() {

    app.defaultModels[0] = cubeModel();
    app.defaultModels[1] = sphereModel(100);
    app.defaultModels[2] = loadOff("bunny.off"); // Load bunny.off file
    scaleModel(app.defaultModels[2], vec4(.1,.1,.1,1)); // Scale to fit between -1 and 1

    app.modelIndex = 1; // Default to sphere
    app.model = app.defaultModels[app.modelIndex];

    resetBall();

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    fillBuffer(app.model);

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0));

    // Uniform variables
    app.uTime = glGetUniformLocation(program, "uniformTime");
    app.uColor = glGetUniformLocation(program, "uniformColor");

    // Retrieve transformation uniform variable locations
    app.ModelView = glGetUniformLocation(program, "ModelView");
    app.Projection = glGetUniformLocation(program, "Projection");

    // Set projection matrix
    mat4  projection;
    projection = Ortho(-app.worldSize.x, app.worldSize.x,
        -app.worldSize.y, app.worldSize.y,
        -app.worldSize.z, app.worldSize.z); // Ortho(): user-defined function in mat.h
    //projection = Perspective(45.0, 1.0, 0.5, 3.0);
    glUniformMatrix4fv(app.Projection, 1, GL_TRUE, projection);

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}


void display() {
    if (app.wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4  model_view;
    // Transformation based on app.ball
    model_view = Scale(app.ball.scale) * Translate(app.ball.position)
        * RotateX(app.ball.angle.x) * RotateY(app.ball.angle.y)
        * RotateZ(app.ball.angle.z);
    glUniformMatrix4fv(app.ModelView, 1, GL_TRUE, model_view);

    // Send time
    float etime;
    etime = 0.001 * glutGet(GLUT_ELAPSED_TIME);
    glUniform1f(app.uTime, etime);

    // Send active color
    glUniform4fv(app.uColor, 1, app.color);

    glDrawArrays(GL_TRIANGLES, 0, app.model.vertexCount);


    glutSwapBuffers();

}

void printHelp() {
    std::cout << "=================== BOUNCY! ====================\n";
    std::cout << "w\t\t wireframe on/off\n";
    std::cout << "+/-\t\t increase/decrease speed\n";
    std::cout << "0-8\t\t change color\n";
    std::cout << "space\t\t toggle object\n";
    std::cout << "enter/i\t\t reset simulation\n";
    std::cout << "h\t\t display help\n";
    std::cout << "q\t\t quit\n";
    std::cout << "================= FPS: " << app.fps << " =================\n";

}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '=': case '+':
        app.playSpeed*=1.5;
        break;
    case '-': case '_':
        app.playSpeed/=1.5;
        break;
    case 033:  // Escape key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case '0' ... '8':
        app.color = vertex_colors[key - '0'];
        break;
    case 'w':
        app.wireframe = !app.wireframe;
        break;
    case ' ':
        app.modelIndex = (app.modelIndex + 1)%3;
        app.model = app.defaultModels[app.modelIndex];
        fillBuffer(app.model);
        break;
    case 13: // Enter
    case 'i': case 'I':
        resetBall();
        break;
    case 'h': case 'H':
        printHelp();
        break;
    }
}

void mouse(int button, int state, int x, int y) {
     // if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    if (state == GLUT_DOWN) {
        switch (button) {
        case GLUT_LEFT_BUTTON:    app.rotateAxis = 1;  break;
        case GLUT_MIDDLE_BUTTON:  app.rotateAxis = 2;  break;
        case GLUT_RIGHT_BUTTON:   app.rotateAxis = 0;  break;
        }
    }
}

void idle() {
    // Calculating elapsed time
    float elapsedSeconds = 0.001 * glutGet(GLUT_ELAPSED_TIME);
    static float lastTime = 0;
    if (lastTime == 0)
        lastTime = elapsedSeconds;
    float timeDiff = elapsedSeconds - lastTime;
    lastTime = elapsedSeconds;
    if (timeDiff <=0 || timeDiff >50) // Too slow or too fast, ignore
        return;

    // FPS calculation
    float currentFps = 1.0 / timeDiff;
    app.frameCount++;
    app.fps = (app.fps*app.frameCount + currentFps) / (app.frameCount + 1) ; // Weighted average

    doPhysics(timeDiff);

    glutPostRedisplay();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void menuHandler(int selection) {
    switch(selection) {
        case 202:
            app.wireframe = true;
            break;
        case 201:
            app.wireframe = false;
            break;
        case 300 ... 308:
            app.color = vertex_colors[selection % 300];
            break;
        case 100 ... 102:
            app.modelIndex = selection % 100;
            app.model = app.defaultModels[app.modelIndex];
            fillBuffer(app.model);
            break;
        case 0:
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("Unknown menu selection: %d\n", selection);
    }
}

void createMenu() {
    app.submenu[0] = glutCreateMenu(menuHandler);
    glutAddMenuEntry("Cube", 100);
    glutAddMenuEntry("Sphere", 101);
    glutAddMenuEntry("Bunny", 102);

    app.submenu[1] = glutCreateMenu(menuHandler);
    glutAddMenuEntry("Fill", 201);
    glutAddMenuEntry("Wireframe", 202);

    app.submenu[2] = glutCreateMenu(menuHandler);
    glutAddMenuEntry("Black", 300);
    glutAddMenuEntry("Color1", 301);
    glutAddMenuEntry("Color2", 302);
    glutAddMenuEntry("Color3", 303);
    glutAddMenuEntry("Color4", 304);
    glutAddMenuEntry("Color5", 305);
    glutAddMenuEntry("Color6", 306);
    glutAddMenuEntry("Color7", 307);
    glutAddMenuEntry("Color8", 308);

    app.menu = glutCreateMenu(menuHandler);
    glutAddSubMenu("Object Type", app.submenu[0]);
    glutAddSubMenu("Drawing Mode", app.submenu[1]);
    glutAddSubMenu("Color", app.submenu[2]);
    glutAddMenuEntry("Quit", 0);

    glutAttachMenu(GLUT_RIGHT_BUTTON);

}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(1024, 512);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Bouncy!");

    printf("Supported GLSL version is %s.\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("%s\n%s\n%s\n",
           glGetString(GL_RENDERER),  // e.g. Intel HD Graphics 3000 OpenGL Engine
           glGetString(GL_VERSION),    // e.g. 3.2 INTEL-8.0.61
           glGetString(GL_SHADING_LANGUAGE_VERSION));

    glewExperimental = GL_TRUE;
    glewInit();

    init();

    createMenu();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}
