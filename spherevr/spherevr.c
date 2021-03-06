#include <stdio.h>
#include <SDL.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>
#include "vuzix.h"

#define RES_WIDTH  640
#define RES_HEIGHT 480

GLUquadricObj *sphere;
GLfloat yaw = 0.0f, raw_yaw = 0.0f, zero_yaw = 0.0f;
GLfloat roll = 0.0f, raw_roll = 0.0f, zero_roll = 0.0f;
GLfloat pitch = 0.0f, raw_pitch = 0.0f, zero_pitch = 0.0f;
GLfloat zoom = 1.0f;
GLuint tex;
CvCapture *capture;
IplImage *frame;

void initGL(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    gluPerspective(32.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);

    sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void deinitGL()
{
    gluDeleteQuadric(sphere);
    glDeleteTextures(1, &tex);
}

void drawGLScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

    if (capture) {
        frame = cvQueryFrame(capture);
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0, GL_BGR, GL_UNSIGNED_BYTE, frame->imageData);

    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(roll, 0.0f, 1.0f, 0.0f);
    glRotatef(yaw, 0.0f, 0.0f, 1.0f);
    gluSphere(sphere, 10.0f, 32, 16);

    SDL_GL_SwapBuffers();
}

int handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return 1;
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) return 1;
            if (event.key.keysym.sym == SDLK_r) {
                zero_yaw = raw_yaw;
                zero_roll = raw_roll;
                zero_pitch = raw_pitch;
                return 0;
            }
        }
        if (event.type == SDL_MOUSEMOTION) {
            yaw -= event.motion.xrel;
            pitch += event.motion.yrel;
        }
    }
    return 0;
}

void readVuzix()
{
    vuzix_read(&raw_pitch, &raw_roll, &raw_yaw);
    // printf("%f %f %f\n", yaw, pitch, roll);
    yaw   = raw_yaw - zero_yaw;
    pitch = raw_pitch - zero_pitch;
    roll  = raw_roll - zero_roll;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: spherevr texture\n");
        return 3;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_SetVideoMode(RES_WIDTH, RES_HEIGHT, 0, SDL_OPENGL) == NULL) {
        fprintf(stderr, "Could not create OpenGL window: %s\n", SDL_GetError());
        SDL_Quit();
        return 2;
    }

    vuzix_open("/dev/hidraw0");

    SDL_WM_SetCaption("SphereVR", NULL);
    SDL_WM_GrabInput(SDL_GRAB_ON);

    initGL(RES_WIDTH, RES_HEIGHT);

    const char *filename = argv[1];
    if (filename[strlen(filename)-1] == '4') { // stupid heuristics (mp4 - video, other - image)
        capture = cvCaptureFromAVI(filename);
    } else {
        capture = 0;
        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    }

    for (;;) {
        // readVuzix();
        drawGLScene();
        if (handleEvents()) break;
        SDL_Delay(1);
    }
    deinitGL();

    vuzix_close();
    if (capture) {
        cvReleaseCapture(&capture);
    }
    SDL_Quit();

    return 0;
}
