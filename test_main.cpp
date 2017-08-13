/*****************************************************************************
 * SoundViewer 1.0 2016
 * Copyright (c) 2016 Pier Paolo Ciarravano - http://www.larmor.com
 * All rights reserved.
 *
 * This file is part of Larmor-Physx.
 *
 * Larmor-Physx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Larmor-Physx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Larmor-Physx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * software.
 *
 * Author: Pier Paolo Ciarravano
 *
 ****************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <math.h>
#include <string.h>
#include <vector>


#if defined(WIN32) || defined(WIN64)
    #include "windows.h"
    #include "gl/gl.h"
    #include "gl/glu.h"
#elif defined(__APPLE__) || defined(LINUX) || defined(linux)
    #include "GL/gl.h"
    #include "GL/glu.h"
#endif
//#define FREEGLUT_STATIC
#include "GL/freeglut.h"

using namespace std;

#define DEGREES_PER_PIXEL_ROT 0.3f
#define DEGREES_PER_PIXEL_TLT 0.3f
#define UNITS_PER_PIXEL_ZOM 0.5f
#define CONV_PI  3.14159265358979323846f
#define STEP_PER_SEC 25

#define LReal double

#include "LarmorSoundAPI/LarmorSoundAPI_Client.h"

namespace LarmorSoundViewer {

    //Simple vector
    class LVector3 {

        public:
            LReal x;
            LReal y;
            LReal z;

            LVector3() {}

            LVector3(const LVector3 &a) : x(a.x), y(a.y), z(a.z) {}

            LVector3(LReal nx, LReal ny, LReal nz) : x(nx), y(ny), z(nz) {}

            LVector3 &operator =(const LVector3 &a) {
                x = a.x; y = a.y; z = a.z;
                return *this;
            }

            bool operator ==(const LVector3 &a) const {
                return x==a.x && y==a.y && z==a.z;
            }

            bool operator !=(const LVector3 &a) const {
                return x!=a.x || y!=a.y || z!=a.z;
            }

    };

    class Camera {

        public:
            LVector3 eyePosition;
            LVector3 lookAt;
            Camera() {
            }

    };

    unsigned int idFrame = 0;
    unsigned int idMaxFrame = 0;

    // Camera and lookat position
    Camera eyeCamera;

    // Timing Variables
    int g_nFPS = 0;
    int g_nFrames = 0;
    clock_t g_lastFPS = 0;
    clock_t time_now;
    clock_t movement_timer = 0;

    struct g_mouseState
    {
        bool leftButton;
        bool rightButton;
        bool middleButton;
        int x;
        int y;
    } MouseState;

    bool colorViewActive = false;
    bool specularActive = false;
    bool doAnim = false;

    //static float g_lightPos[4] = { 10, 10, -100, 1 };  // Position of light
    static float lmodel_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    static float material1[4] = { 0.9f, 0.0f, 0.9f, 1.0f };
    static float material2[4] = { 0.9f, 0.0f, 0.9f, 1.0f };
    static float material3[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
    static float material4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    float colorWhite[4] = { 1.0, 1.0, 1.0, 1.0 };
    float colorWhiteRed[4] = { 1.0, 1.0, 0.0, 1.0 };
    float colorGray[4] = { 0.5, 0.5, 0.5, 1.0 };
    float colorRed[4] = { 1.0, 0.0, 0.0, 1.0 };
    float colorBlue[4] = { 0.0, 0.0, 1.0, 1.0 };
    float colorGreen[4] = { 0.0, 1.0, 0.0, 1.0 };
    float colorNone[4] = { 0.0, 0.0, 0.0, 0.0 };

    Camera animCamera;
    bool isAnimCameraValid = false;


    //TODO: use Aubio compiled with double
    Larmor::LarmorSound *testObj = NULL;
    bool sendHeartbeat = true;
    void prepareAnimation()
    {
        //const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/testgiorgio_2.wav";
        //const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/test_wave_2.wav";
        //const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/silence.wav";
        //const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/voice2.wav";
        //const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/giorgio.mp3";
        //const char * file_test = "/home/larmor/Downloads/00010.MTS";
        //const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/liz.mp3";
        const char * file_test = "/home/larmor/DEVELOP/FabricSound_proj/samples/Anahata-Rising2.wav";

        testObj = new Larmor::LarmorSound(file_test);
        testObj->initPlay();
        testObj->setHeartbeatActive(true);

        std::cout << "TEST getNumSamples:" << testObj->getNumSamples() << std::endl;
        std::cout << "TEST getSamplerate:" << testObj->getSamplerate() << std::endl;
        std::cout << "TEST getNumChannels:" << (int)testObj->getNumChannels() << std::endl;


        std::cout << "TEST isInitedPlay:" << testObj->isInitedPlay() << std::endl;
        std::cout << "TEST isPlaying:" << testObj->isPlaying() << std::endl;
        std::cout << "TEST getPlayPosition:" << (int)testObj->getPlayPosition() << std::endl;

    }


    void normalise(LVector3& vec)
    {
        float length;

        length = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));

        if (length == 0.0f)
        {
            length = 1.0f;
        }

        vec.x /= length;
        vec.y /= length;
        vec.z /= length;
    }

    void calcNormal(LVector3 ta, LVector3 tb, LVector3 tc, LVector3& normal)
    {
        LVector3 v1, v2;

        v1.x = ta.x - tb.x;
        v1.y = ta.y - tb.y;
        v1.z = ta.z - tb.z;

        v2.x = tb.x - tc.x;
        v2.y = tb.y - tc.y;
        v2.z = tb.z - tc.z;

        normal.x = v1.y * v2.z - v1.z * v2.y;
        normal.y = v1.z * v2.x - v1.x * v2.z;
        normal.z = v1.x * v2.y - v1.y * v2.x;

        normalise(normal);
    }

    void print_bitmap_string(void* font, const char* s)
    {
        if (s && strlen(s))
        {
            while (*s)
            {
                glutBitmapCharacter(font, *s);
                s++;
            }
        }
    }


    void drawBar(float c_x, float c_z, float width, float height)
    {
        glPushMatrix();
        glTranslatef(c_x, 0.0, c_z);
        glScalef(width, 1.0, width);

        glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
        glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
        glColor4fv(colorWhite);
        glBegin(GL_QUADS);
            glNormal3d(0.0, 1.0, 0.0);
            glVertex3f( 1.0f,  height, -1.0f);
            glVertex3f(-1.0f,  height, -1.0f);
            glVertex3f(-1.0f,  height,  1.0f);
            glVertex3f( 1.0f,  height,  1.0f);

            glNormal3d(0.0, -1.0, 0.0);
            glVertex3f( 1.0f,  0.0f,  1.0f);
            glVertex3f(-1.0f,  0.0f,  1.0f);
            glVertex3f(-1.0f,  0.0f, -1.0f);
            glVertex3f( 1.0f,  0.0f, -1.0f);

            glNormal3d(0.0, 0.0, 1.0);
            glVertex3f( 1.0f,  height, 1.0f);
            glVertex3f(-1.0f,  height, 1.0f);
            glVertex3f(-1.0f,  0.0f, 1.0f);
            glVertex3f( 1.0f,  0.0f, 1.0f);

            glNormal3d(0.0, 0.0, -1.0);
            glVertex3f( 1.0f,  0.0f, -1.0f);
            glVertex3f(-1.0f,  0.0f, -1.0f);
            glVertex3f(-1.0f,  height, -1.0f);
            glVertex3f( 1.0f,  height, -1.0f);

            glNormal3d(-1.0, 0.0, 0.0);
            glVertex3f(-1.0f,  height,  1.0f);
            glVertex3f(-1.0f,  height, -1.0f);
            glVertex3f(-1.0f,  0.0f, -1.0f);
            glVertex3f(-1.0f,  0.0f,  1.0f);

            glNormal3d(1.0, 0.0, 0.0);
            glVertex3f(1.0f,  height, -1.0f);
            glVertex3f(1.0f,  height,  1.0f);
            glVertex3f(1.0f,  0.0f,  1.0f);
            glVertex3f(1.0f,  0.0f, -1.0f);
        glEnd();

        glPopMatrix();
    }

    void draw()
    {

        //FPS counter
        clock_t g_nowFPS = clock();
        if ( g_nowFPS - g_lastFPS >= CLOCKS_PER_SEC )
        {
            g_lastFPS = g_nowFPS;
            g_nFPS = g_nFrames;
            g_nFrames = 0;
            //std::cout << "FPS: " << g_nFPS <<std::endl;
        }
        g_nFrames++;


        // Clear frame buffer and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up viewing transformation, looking down -Z axis
        glLoadIdentity();

        // set camera and lookat position
        gluLookAt(eyeCamera.eyePosition.x, eyeCamera.eyePosition.y, eyeCamera.eyePosition.z,
                  eyeCamera.lookAt.x, eyeCamera.lookAt.y, eyeCamera.lookAt.z,
                  0, 1, 0);

        // Set up the stationary light
        //glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);

        if (specularActive)
        {
            colorNone[0] = 1.0;
            colorNone[1] = 1.0;
            colorNone[2] = 1.0;
            colorNone[3] = 1.0;
        }

        //text Labels
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glColor3f(1.0, 1.0, 1.0);

        //Number Frame
        glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
        std::stringstream stringText1;
        stringText1 << "Frame: " << idFrame;
        print_bitmap_string(GLUT_BITMAP_HELVETICA_12, stringText1.str().c_str());

        //FPS
        glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 40);
        std::stringstream stringText2;
        stringText2 << "FPS: " << g_nFPS;
        print_bitmap_string(GLUT_BITMAP_HELVETICA_12, stringText2.str().c_str());

        //Dump camera coordinate
        glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 60);
        std::stringstream stringTextCamCoo;
        stringTextCamCoo << "Camera at: " << eyeCamera.eyePosition.x;
        stringTextCamCoo << ", " << eyeCamera.eyePosition.y;
        stringTextCamCoo << ", " << eyeCamera.eyePosition.z;
        print_bitmap_string(GLUT_BITMAP_HELVETICA_12, stringTextCamCoo.str().c_str());

        //www.larmor.com
        glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) - 150, 20);
        print_bitmap_string(GLUT_BITMAP_HELVETICA_18, "www.larmor.com");
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glEnable(GL_LIGHTING);


        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        //Ground
        for (int i = -10; i <= 10; ++i)
        {
            for (int j = -10; j <= 10; ++j)
            {
                float ax = i * 10.0;
                float ay = j * 10.0;
                float bx = (i + 1) * 10.0;
                float by = j * 10.0;
                float cx = (i + 1) * 10.0;
                float cy = (j + 1) * 10.0;
                float dx = i * 10.0;
                float dy = (j + 1) * 10.0;

                if ( ((i + j) % 2) == 0)
                {
                    glMaterialfv(GL_FRONT, GL_DIFFUSE, colorRed);
                    glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
                    glColor4fv(colorRed);
                }
                else
                {
                    glMaterialfv(GL_FRONT, GL_DIFFUSE, colorBlue);
                    glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
                    glColor4fv(colorBlue);
                }

                glBegin(GL_TRIANGLES);
                glNormal3d(0.0, -10.0, 0.0);
                glVertex3f( ax, 0.0, ay );
                glVertex3f( bx, 0.0, by );
                glVertex3f( dx, 0.0, dy );
                glEnd();
                glBegin(GL_TRIANGLES);
                glNormal3d(0.0, 10.0, 0.0);
                glVertex3f( dx, 0.0, dy );
                glVertex3f( bx, 0.0, by );
                glVertex3f( ax, 0.0, ay );
                glEnd();

                glBegin(GL_TRIANGLES);
                glNormal3d(0.0, -10.0, 0.0);
                glVertex3f( bx, 0.0, by );
                glVertex3f( cx, 0.0, cy );
                glVertex3f( dx, 0.0, dy );
                glEnd();
                glBegin(GL_TRIANGLES);
                glNormal3d(0.0, 10.0, 0.0);
                glVertex3f( dx, 0.0, dy );
                glVertex3f( cx, 0.0, cy );
                glVertex3f( bx, 0.0, by );
                glEnd();

            }
        }


        //Draw bars
        /*
        uint_t block = challel_pos / 1024;
        for (uint_t i = 0; i < 50; i++) { //channel
            if (block >= i)
            for (uint_t j = 0; j < fft_sample[0][block-i].size(); j++) { //block
                //smpl_t val = fft_sample[0][block-i][j];

                smpl_t val = 0.0;
                for (uint_t d = 0; d < 5; d++) { //averange
                    if ((block-i) >= 4) {
                        val += fft_sample[0][block-i-d][j];
                    }
                }
                val + val / 5.0;

                drawBar(i * 1.0-80, j*1.0-100, 0.5, val);
            }
        }
        */


    
        for (uint8_t c = 0; c < testObj->getNumChannels(); c++) { //channel

            Larmor::vect_smpl *spectrum = testObj->getChannelSpectrum(c, testObj->getPlayPosition());
            if (spectrum != NULL) {
                for (uint32_t j = 0; j < (*spectrum).size(); j++) { //block
                    float val = (*spectrum)[j];
                    //if (val > 5.0)
                        drawBar(c * 3.0, j*1.0-80, 0.4, val);
                    //else
                    //    drawBar(c * 3.0, j*1.0-80, 0.4, 0.5);
                }

                float all_energy = testObj->getChannelEnergy(c, testObj->getPlayPosition());
                drawBar(c * 3.0, -90, 1.0, all_energy/100.0 );
            }
        }


/*
        // TODO: media di bar attuale e N bar precedenti
        uint_t block = (tracks_sample[0].size() - audio_len) / 1024;
        for (uint_t channel = 0; channel < fft_sample.size(); channel++) { //channel
            smpl_t all_energy = 0.0;
            for (uint_t j = 0; j < fft_sample[channel][block].size(); j++) { //block

                all_energy += fft_sample[channel][block][j];

                smpl_t val = 0.0;
                for (uint_t i = 0; i < 5; i++) { //averange
                    if (block >= 4) {
                        val += fft_sample[channel][block-i][j];
                    }
                }
                if (val > 10.0)
                    drawBar(channel * 3.0, j*1.0-80.0, 0.4, val / 5.0 );
                else
                    drawBar(channel * 3.0, j*1.0-80.0, 0.4, 0.5 );
            }

            all_energy = all_energy / 100.0;
            drawBar(channel * 3.0, -90, 1.0, all_energy );
        }
*/

        glPopMatrix();

        // Make sure changes appear onscreen
        glutSwapBuffers();

    }

    void animate()
    {
        //framerate limit
        time_now = clock();
        if (time_now - movement_timer > (CLOCKS_PER_SEC / (STEP_PER_SEC * 1.0)))
        {
            //std::cout << "s: " << (time_now - movement_timer) << std::endl;
            movement_timer = time_now;
            if (doAnim)
            {
                idFrame +=  1;
                if (idFrame > idMaxFrame)
                {
                    idFrame = 0;
                }
                glutPostRedisplay();
            }
        }

        //Force animation without press F11; Remove this line for old behaviour
        glutPostRedisplay();

        if (sendHeartbeat) {
            testObj->heartbeat();
        }

    }

    void initRenderer()
    {
        /*
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        */

        //initialize the mouse state
        MouseState.leftButton = MouseState.rightButton = MouseState.middleButton = false;
        MouseState.x = MouseState.y = 0;

        //init eye camera
        eyeCamera.eyePosition = LVector3(100.0, 100.0, 0.0);
        eyeCamera.lookAt = LVector3(0.0, 0.0, 0.0);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // set up our viewing frustum
        int m_width = glutGet(GLUT_WINDOW_WIDTH);
        int m_height = glutGet(GLUT_WINDOW_HEIGHT);
        float aspect = (float)m_width / (float)m_height;
        gluPerspective(40.0f, aspect, 0.0f, 10000.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity(); //Aggiunto

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
        glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_ambient);
        glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f );
        glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION,  0.0f );
        glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f );

        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, material1 );
        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, material2 );
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, material3 );
        glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, material4 );
        glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 10.0f );

        // Solid render
        glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
        //glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );

        // Shade mode
        //glShadeModel(GL_FLAT);
        glShadeModel(GL_SMOOTH);

        // Turn on the zbuffer
        glEnable( GL_DEPTH_TEST );
        //glDisable( GL_DEPTH_TEST );

        // Turn on zbuffer writing
        glDepthMask(1);

        // Turn on zbuffer function
        glDepthFunc(GL_LESS);

        // Turn on culling (CCW --> ogl default mode - like d3d)
        glEnable( GL_CULL_FACE );
        glFrontFace( GL_CCW );

    }

    void handleMouseState(int button, int state, int x, int y)
    {
        //printf("Mouse button: %d\n", button);

        // update the button state
        if (button == GLUT_LEFT_BUTTON)
        {
            if (state == GLUT_DOWN)
            {
                MouseState.leftButton = true;
                //printf("leftButton true\n");
            }
            else
            {
                MouseState.leftButton = false;
                //printf("leftButton false\n");
            }
        }
        if (button == GLUT_RIGHT_BUTTON)
        {
            if (state == GLUT_DOWN)
            {
                MouseState.rightButton = true;
                //printf("rightButton true\n");
            }
            else
            {
                MouseState.rightButton = false;
                //printf("rightButton false\n");
            }
        }
        if (button == GLUT_MIDDLE_BUTTON)
        {
            if (state == GLUT_DOWN)
            {
                MouseState.middleButton = true;
                //printf("middleButton true\n");
            }
            else
            {
                MouseState.middleButton = false;
                //printf("middleButton false\n");
            }
        }

        // update our position so we know a delta when the mouse is moved
        MouseState.x = x;
        MouseState.y = y;
    }

    void handleMouseMove(int x, int y)
    {
        // calculate a delta in movement
        int yDelta = MouseState.y - y;
        int xDelta = MouseState.x - x;

        // commit the mouse position
        MouseState.x = x;
        MouseState.y = y;

        // when we need to rotate (only the left button is down)
        if (MouseState.leftButton && !MouseState.rightButton && !MouseState.middleButton)
        {
            /*
            // rotate around lookat horizontal axes
            float rotZ = -yDelta * DEGREES_PER_PIXEL_LPV;
            float xc = eyeCamera.eyePosition.x;
            float yc = eyeCamera.eyePosition.y;
            float zc = eyeCamera.eyePosition.z;
            float xct = xc * cos(rotZ*CONV_PI/180.0) - yc * sin(rotZ*CONV_PI/180.0);
            float yct = xc * sin(rotZ*CONV_PI/180.0) + yc * cos(rotZ*CONV_PI/180.0);
            float zct = zc;
            eyeCamera.eyePosition.x = xct;
            eyeCamera.eyePosition.y = yct;
            eyeCamera.eyePosition.z = zct;
            */
            /*
            // rotate around lookat vertical axes
            float rotY = xDelta * DEGREES_PER_PIXEL_LPV;
            xc = eyeCamera.eyePosition.x;
            yc = eyeCamera.eyePosition.y;
            zc = eyeCamera.eyePosition.z;
            xct = zc * sin(rotY*CONV_PI/180.0) + xc * cos(rotY*CONV_PI/180.0);
            yct = yc;
            zct = zc * cos(rotY*CONV_PI/180.0) - xc * sin(rotY*CONV_PI/180.0);
            eyeCamera.eyePosition.x = xct;
            eyeCamera.eyePosition.y = yct;
            eyeCamera.eyePosition.z = zct;
            */

            float xac = eyeCamera.eyePosition.x - eyeCamera.lookAt.x; //X
            float yac = eyeCamera.eyePosition.y - eyeCamera.lookAt.y; //Z
            float zac = eyeCamera.eyePosition.z - eyeCamera.lookAt.z; //Y
            float rac = sqrt(xac * xac + yac * yac + zac * zac);
            float tac = acos(yac / rac); //Tilt
            float pac = atan2(zac, xac); //Rotation
            float rotY = -xDelta * DEGREES_PER_PIXEL_ROT * CONV_PI / 180.0;
            float rotZ = yDelta * DEGREES_PER_PIXEL_TLT * CONV_PI / 180.0;
            rotY += pac;
            rotZ += tac;
            if (rotZ < 0 )
            {
                rotZ = 0.001;
            }
            if (rotZ > CONV_PI )
            {
                rotZ = CONV_PI - 0.001;
            }
            eyeCamera.eyePosition.x = rac * sin(rotZ) * cos(rotY) + eyeCamera.lookAt.x;
            eyeCamera.eyePosition.y = rac * cos(rotZ) + eyeCamera.lookAt.y;
            eyeCamera.eyePosition.z = rac * sin(rotZ) * sin(rotY) + eyeCamera.lookAt.z;

        }

        // zoom
        else if (!MouseState.leftButton && MouseState.rightButton && !MouseState.middleButton)
        {
            // zoom
            float tranR = yDelta * 0.01;
            // save the eye position coordinates
            float xt = eyeCamera.eyePosition.x;
            float yt = eyeCamera.eyePosition.y;
            float zt = eyeCamera.eyePosition.z;
            // move the eyePosition
            eyeCamera.eyePosition.x += (eyeCamera.eyePosition.x - eyeCamera.lookAt.x) * tranR;
            eyeCamera.eyePosition.y += (eyeCamera.eyePosition.y - eyeCamera.lookAt.y) * tranR;
            eyeCamera.eyePosition.z += (eyeCamera.eyePosition.z - eyeCamera.lookAt.z) * tranR;
            // move the lookAt
            eyeCamera.lookAt.x += (xt - eyeCamera.lookAt.x) * tranR;
            eyeCamera.lookAt.y += (yt - eyeCamera.lookAt.y) * tranR;
            eyeCamera.lookAt.z += (zt - eyeCamera.lookAt.z) * tranR;
        }

        // if we need to move the lookat position (left and right buttons are down)
        //if(!MouseState.leftButton && !MouseState.rightButton && MouseState.middleButton)
        if (MouseState.leftButton && MouseState.rightButton && !MouseState.middleButton)
        {
            float xac = eyeCamera.lookAt.x - eyeCamera.eyePosition.x; //X
            float yac = eyeCamera.lookAt.y - eyeCamera.eyePosition.y; //Z
            float zac = eyeCamera.lookAt.z - eyeCamera.eyePosition.z; //Y
            float rac = sqrt(xac * xac + yac * yac + zac * zac);
            float tac = acos(yac / rac); //Tilt
            float pac = atan2(zac, xac); //Rotation
            float rotY = -xDelta * DEGREES_PER_PIXEL_ROT * CONV_PI / 180.0;
            float rotZ = yDelta * DEGREES_PER_PIXEL_TLT * CONV_PI / 180.0;
            rotY += pac;
            rotZ += tac;
            if (rotZ < 0 )
            {
                rotZ = 0.001;
            }
            if (rotZ > CONV_PI )
            {
                rotZ = CONV_PI - 0.001;
            }
            eyeCamera.lookAt.x = rac * sin(rotZ) * cos(rotY) + eyeCamera.eyePosition.x;
            eyeCamera.lookAt.y = rac * cos(rotZ) + eyeCamera.eyePosition.y;
            eyeCamera.lookAt.z = rac * sin(rotZ) * sin(rotY) + eyeCamera.eyePosition.z;
        }

        glutPostRedisplay();

    }

    void handleKeyboard(unsigned char c, int x, int y)
    {
        //printf("Keyboard: %d %d %d\n", c, x, y);
        if (c == 113)
        {
            printf("Quit!!\n");
            delete testObj;
            exit(0);
        }

        //if (c == char('c'))

        if (c == char('s'))
        {
            eyeCamera.lookAt.x += 0.3;
            glutPostRedisplay();
        }

        if (c == char('a'))
        {
            eyeCamera.lookAt.x -= 0.3;
            glutPostRedisplay();
        }

        if (c == char('w'))
        {
            eyeCamera.lookAt.y += 0.3;
            glutPostRedisplay();
        }

        if (c == char('z'))
        {
            eyeCamera.lookAt.y -= 0.3;
            glutPostRedisplay();
        }

        if (c == char('x'))
        {
            eyeCamera.lookAt.z += 0.3;
            glutPostRedisplay();
        }

        if (c == char('c'))
        {
            eyeCamera.lookAt.z -= 0.3;
            glutPostRedisplay();
        }
    }

    void processSpecialKeys(int key, int x, int y)
    {
        switch (key)
        {
            case GLUT_KEY_F1 :
                glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
                glutPostRedisplay();
                break;
            case GLUT_KEY_F2 :
                glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
                glutPostRedisplay();
                break;
            case GLUT_KEY_F3 :
                glPolygonMode( GL_FRONT_AND_BACK , GL_POINT );
                glutPostRedisplay();
                break;

            case GLUT_KEY_F4 :
                testObj->play();
                break;
            case GLUT_KEY_F5 :
                testObj->play(100);
                break;
            case GLUT_KEY_F6 :
                testObj->stop();
                break;
            case GLUT_KEY_F7 :
                std::cout << "TEST isPlaying:" << testObj->isPlaying() << std::endl;
                break;
            case GLUT_KEY_F8 :
                testObj->play(0);
                break;
            case GLUT_KEY_F9 :
                //std::cout << "TEST getPlayPosition:" << (int)testObj->getPlayPosition() << std::endl;
                sendHeartbeat = true;
                break;
            case GLUT_KEY_F10 :
                //std::cout << "TEST closePlay:" << (int)testObj->closePlay() << std::endl;
                sendHeartbeat = false;
                break;

            case GLUT_KEY_F11 :
                doAnim = !doAnim;
                break;
            case GLUT_KEY_LEFT :
                if (idFrame == 0)
                {
                    idFrame = idMaxFrame;
                }
                else
                {
                    idFrame--;
                }
                glutPostRedisplay();
                break;
            case GLUT_KEY_RIGHT :
                idFrame++;
                if (idFrame > idMaxFrame)
                {
                    idFrame = 0;
                }
                glutPostRedisplay();
                break;
        }

    }

    void reshapeWindow(int width, int height)
    {
        //std::cout << "reshapeWindow" << std::endl;
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)width / (float)height;
        //gluPerspective(40.0f, aspect, 0.0f, 10000.0f);
        gluPerspective(65.0f, aspect, 0.001f, 1000.0f);
        glMatrixMode(GL_MODELVIEW);
    }


    void run_animation_viewer(int argc, char** argv)
    {
        glutInit(&argc, argv);

        prepareAnimation();

        glutInitWindowSize(1024, 860); //720x480
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
        glutCreateWindow("SoundViewer :: Larmor");

        initRenderer();

        glutDisplayFunc(draw);
        glutReshapeFunc(reshapeWindow);
        glutMouseFunc(handleMouseState);
        glutMotionFunc(handleMouseMove);
        glutSpecialFunc(processSpecialKeys);
        glutKeyboardFunc(handleKeyboard);
        glutIdleFunc(animate);
        glutMainLoop();

    }

}

int main(int argc, char** argv)
{

    std::cout << "LarmorSoundAPI OpenGL Test Oct. 2016\nAuthor: Pier Paolo \"Larmor\" Ciarravano\nhttp://www.larmor.com\n\n" << std::endl;

    LarmorSoundViewer::run_animation_viewer(argc, argv);

}

