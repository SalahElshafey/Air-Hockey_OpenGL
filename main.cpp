#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdlib.h>
#include <time.h>

GLuint backgroundTexture;

float r = 0.5f * ((float)rand() / (float)RAND_MAX) + 0.5f;
float g = 0.5f * ((float)rand() / (float)RAND_MAX) + 0.5f;
float b = 0.5f * ((float)rand() / (float)RAND_MAX) + 0.5f;

typedef struct {
    float x, y;
} Paddle;

typedef struct {
    float x, y, dx, dy;
} Puck;

Paddle player1 = {-0.9f, 0.0f};
Paddle player2 = {0.9f, 0.0f};
Puck pucks[3];
int puckCount = 1;
int score1 = 0, score2 = 0;
int remainingTime = 1200;
bool inMenu = true;
bool inHowToPlay = false;
int currentLevel = 0;

float distance(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void displayText(float x, float y, const char* text) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawCircle(float x, float y, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++) {
        float degInRad = i * M_PI / 180;
        glVertex2f(x + cos(degInRad) * radius, y + sin(degInRad) * radius);
    }
    glEnd();
}

void drawOval(float x, float y, float width, float height) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++) {
        float degInRad = i * M_PI / 180;
        glVertex2f(x + cos(degInRad) * width, y + sin(degInRad) * height);
    }
    glEnd();
}

void drawPaddle(Paddle paddle, float innerRadius, float outerRadius, float ovalWidth, float ovalHeight) {
    // Outer circle
    glColor3f(0.0f, 0.0f, 1.0f);
    drawCircle(paddle.x, paddle.y, outerRadius);

    // Inner circle
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(paddle.x, paddle.y, innerRadius);

    // Oval in the middle
    glColor3f(1.0f, 0.0f, 0.0f);
    drawOval(paddle.x, paddle.y, ovalWidth, ovalHeight);
}

void drawBackground() {
    glColor3f(1.0f, 0.0f, 0.0f);
    for (float x = -0.9f; x <= 0.9f; x += 0.1f) {
        for (float y = -0.4f; y <= 0.4f; y += 0.1f) {
            glPushMatrix();
            glTranslatef(x, y, 0.0f);
            glutSolidSphere(0.01f, 10, 10);
            glPopMatrix();
        }
    }

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float degInRad = i * M_PI / 180;
        glVertex2f(cos(degInRad) * 0.1f, sin(degInRad) * 0.1f);
    }
    glEnd();
}

void drawMenuBackground() {
    glColor3f(0.2f, 0.2f, 0.2f); // Dark grey color for the menu background
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

void displayMenu() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawMenuBackground();

    // Draw "Welcome!" text
    displayText(-0.25f, 0.35f, "Welcome To Our Game!");

    displayText(0.8f, 0.4f, "How to play?");

    displayText(-0.2f, 0.2f, "1. Start - Level 1");
    drawRectangle(-0.3f, 0.15f, 0.3f, 0.25f);

    displayText(-0.2f, 0.0f, "2. Start - Level 2");
    drawRectangle(-0.3f, -0.05f, 0.3f, 0.05f);

    displayText(-0.2f, -0.2f, "3. Start - Level 3");
    drawRectangle(-0.3f, -0.25f, 0.3f, -0.15f);

    displayText(-0.2f, -0.4f, "4. Exit");
    drawRectangle(-0.3f, -0.45f, 0.3f, -0.35f);

    // Draw a paddle in the top corner
    Paddle menuPaddle = {-0.8f, 0.8f};
    drawPaddle(menuPaddle, 0.02f, 0.04f, 0.015f, 0.035f);

    glFlush();
}

void displayHowToPlay() {
    glClear(GL_COLOR_BUFFER_BIT);

    displayText(-0.9f, 0.4f, "Ready to crank up the air hockey challenge? This twist adds three levels of increasing mayhem, with each level introducing an extra puck on the table!");
    displayText(-0.9f, 0.3f, "Level 1: Classic Clash");
    displayText(-0.9f, 0.25f, "Start with the standard setup – one puck and two players battling for control. Brush up on your basic shots and defensive maneuvers.");
    displayText(-0.9f, 0.15f, "Level 2: Double Trouble");
    displayText(-0.9f, 0.1f, "Now things get interesting! Introduce a second puck. The game remains the same – score goals in your opponent's net – but the strategy changes completely.");
    displayText(-0.9f, 0.05f, "You can trap one puck, use it to deflect your opponent's shots, or set up trickier offensive plays. Be prepared for the action to double!");
    displayText(-0.9f, -0.05f, "Level 3: Puckpocalypse!");
    displayText(-0.9f, -0.1f, "With three pucks zipping around the table, this is air hockey on steroids. Anticipation and split-second decisions are crucial.");
    displayText(-0.9f, -0.15f, "Mastering bank shots and rebounds becomes essential to navigate the puck chaos. Get ready for an intense and unpredictable battle!");
    displayText(-0.9f, -0.25f, "Remember, with more pucks, communication and teamwork (even against each other!) can become an unexpected advantage.");
    displayText(-0.9f, -0.3f, "So, grab a friend, choose your level of puck-fueled frenzy, and get ready for an air hockey experience that will take your skills to the next level!");
    displayText(-0.9f, -0.45f, "Press ESC to return to the menu");

    glFlush();
}

void display() {
    if (inMenu) {
        if (inHowToPlay) {
            displayHowToPlay();
        } else {
            displayMenu();
        }
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    drawBackground();

    drawPaddle(player1, 0.03f, 0.05f, 0.02f, 0.05f);
    drawPaddle(player2, 0.03f, 0.05f, 0.02f, 0.05f);

    for (int i = 0; i < puckCount; ++i) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(pucks[i].x, pucks[i].y, 0.0f);
        glutSolidSphere(0.03f, 20, 20);
        glPopMatrix();
    }

    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 0.1f);
    glVertex2f(-0.95f, 0.1f);
    glVertex2f(-0.95f, -0.1f);
    glVertex2f(-1.0f, -0.1f);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(1.0f, 0.1f);
    glVertex2f(0.95f, 0.1f);
    glVertex2f(0.95f, -0.1f);
    glVertex2f(1.0f, -0.1f);
    glEnd();

    char scoreDisplay[80];
    sprintf(scoreDisplay, "Player 1: %d Player 2: %d | Time Left: %d:%02d", score1, score2, remainingTime / 60, remainingTime % 60);
    displayText(-0.5f, 0.45f, scoreDisplay);

    glFlush();
}

void update(int value) {
    if (inMenu) {
        return;
    }

    static int elapsedTime = 0;
    elapsedTime += 16;

    float r = (float)rand() / (float)RAND_MAX;
    float g = (float)rand() / (float)RAND_MAX;
    float b = (float)rand() / (float)RAND_MAX;
    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < puckCount; ++i) {
        pucks[i].x += pucks[i].dx;
        pucks[i].y += pucks[i].dy;

        if (pucks[i].y >= 0.47 || pucks[i].y <= -0.47) {
            pucks[i].dy = -pucks[i].dy;
        }

        if ((pucks[i].x <= -0.95f && (pucks[i].y < -0.1f || pucks[i].y > 0.1f)) ||
            (pucks[i].x >= 0.95f && (pucks[i].y < -0.1f || pucks[i].y > 0.1f))) {
            pucks[i].dx = -pucks[i].dx;
        }

        if (distance(pucks[i].x, pucks[i].y, player1.x, player1.y) <= 0.08f) {
            pucks[i].dx = -pucks[i].dx;
        }
        if (distance(pucks[i].x, pucks[i].y, player2.x, player2.y) <= 0.08f) {
            pucks[i].dx = -pucks[i].dx;
        }

        if (pucks[i].x < -0.95f && pucks[i].y >= -0.1f && pucks[i].y <= 0.1f) {
            score2++;
            pucks[i].x = 0;
            pucks[i].y = 0;
            pucks[i].dx = 0.02f;
            pucks[i].dy = 0.01f;
        } else if (pucks[i].x > 0.95f && pucks[i].y >= -0.1f && pucks[i].y <= 0.1f) {
            score1++;
            pucks[i].x = 0;
            pucks[i].y = 0;
            pucks[i].dx = -0.02f;
            pucks[i].dy = 0.01f;
        }
    }

    if (remainingTime > 0 && elapsedTime >= 1000) {
        remainingTime--;
        elapsedTime = 0;
    }

    if (remainingTime <= 0) {
        printf("Final Score - Player 1: %d, Player 2: %d\n", score1, score2);
        exit(0);
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void startGame(int level) {
    inMenu = false;
    inHowToPlay = false;
    currentLevel = level;
    score1 = 0;
    score2 = 0;
    remainingTime = 600;

    if (level == 1) {
        puckCount = 1;
    } else if (level == 2) {
        puckCount = 2;
    } else if (level == 3) {
        puckCount = 3;
    }

    for (int i = 0; i < puckCount; ++i) {
        pucks[i].x = 0;
        pucks[i].y = 0;
        pucks[i].dx = 0.02f * (i + 1);
        pucks[i].dy = 0.01f * (i + 1);
    }

    glutTimerFunc(16, update, 0);
}

void keyInput(unsigned char key, int x, int y) {
    if (inMenu) {
        if (inHowToPlay) {
            if (key == 27) {  // ESC key
                inHowToPlay = false;
                glutPostRedisplay();
            }
        } else {
            switch (key) {
                case '1':
                    startGame(1);
                    break;
                case '2':
                    startGame(2);
                    break;
                case '3':
                    startGame(3);
                    break;
                case '4':
                    exit(0);
                    break;
                case '?':
                    inHowToPlay = true;
                    glutPostRedisplay();
                    break;
            }
        }
    } else {
        switch (key) {
            case 'w':
            case 'W':
                if (player1.y < 0.4) player1.y += 0.05f;
                break;
            case 's':
            case 'S':
                if (player1.y > -0.4) player1.y -= 0.05f;
                break;
            case 27:  // ESC key
                inMenu = true;
                glutPostRedisplay();
                break;
        }
    }
}

void keySpecialInput(int key, int x, int y) {
    if (!inMenu) {
        switch (key) {
            case GLUT_KEY_UP:
                if (player2.y < 0.4) player2.y += 0.05f;
                break;
            case GLUT_KEY_DOWN:
                if (player2.y > -0.4) player2.y -= 0.05f;
                break;
        }
    }
}

void init() {
    srand(time(NULL));
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(-1.0, 1.0, -0.5, 0.5);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Air Hockey Game");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyInput);
    glutSpecialFunc(keySpecialInput);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
