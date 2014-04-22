/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2010 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

/*
    TODOS
        - [ ] Scan left to right
        - [ ] Output stuff
        - [ ] Top Bar and Lower Bar
*/
static const double pi = 3.14159265358979323846;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libfreenect.h"
#include "old.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <vector>

#include "lo/lo.h"

#define ADDRESS "127.0.0.1"
#define PORT 57120




#define WINDOW "mainWin"

using namespace cv;
using namespace std;


// for OSC

#define SDL 1


#ifdef SDL
#define REDOFF 0
#define BLUEOFF 1
#define GREENOFF 2

#include <SDL/SDL.h>

#endif

#define WIDTH 640
#define HEIGHT 480


#include <math.h>



pthread_t freenect_thread;
volatile int die = 0;

int g_argc;
char **g_argv;

int window;

pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

// back: owned by libfreenect (implicit for depth)
// mid: owned by callbacks, "latest frame ready"
// front: owned by GL, "currently being drawn"
uint8_t *depth_mid, *depth_front, *color_diff_depth_map;
uint8_t *rgb_back, *rgb_mid, *rgb_front;


freenect_context *f_ctx;
freenect_device *f_dev;
int freenect_angle = 0;
int freenect_led;

freenect_video_format requested_format = FREENECT_VIDEO_RGB;
freenect_video_format current_format = FREENECT_VIDEO_RGB;

pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;
int got_rgb = 0;
int got_depth = 0;

uint16_t gthreshold = 1024;

int * depth_map;/*[ FREENECT_FRAME_PIX ]; */
int * old_depth_map;/*[ FREENECT_FRAME_PIX ];*/
int * diff_depth_map;/*[ FREENECT_FRAME_PIX ];*/
int * tmp_diff_depth_map;/*[ FREENECT_FRAME_PIX ];*/

int depth_frame = 1024;
#define SHADOW 2047
#define WIDTH 640
#define HEIGHT 480

#ifdef SDL
SDL_Surface * sdlSurface = NULL;
#endif

int object_depth = 500;
int ghit = 0;
int win = 0;
int wotime = 0;

int gdegree = 0;
int gtempo = 1;
int gyline = HEIGHT/2;

bool gkeydown = false;
char gkeysym = ' ';

#define MYCVTYPE CV_32S
//define MYCVTYPE CV_32F

cv::Mat lastDepthFrame(HEIGHT, WIDTH, MYCVTYPE);
cv::Mat depthFrame(HEIGHT, WIDTH, MYCVTYPE);

lo_address lo_t = lo_address_new("127.0.0.1", "57120");


void clearBorders( int * map ) {
    for (int y = 0 ; y < HEIGHT; y++) {
        map[WIDTH * y + 0] = 0;
        map[WIDTH * y + WIDTH - 1] = 0;
    }
    for (int x = 0 ; x < WIDTH; x++) {
        map[x] = 0;
        map[FREENECT_FRAME_PIX - WIDTH + x] = 0;
    }

}

void despeckleInPlace( int * map ) {
    clearBorders( tmp_diff_depth_map );
    for (int y = 1 ; y < HEIGHT-1; y++) {
        for (int x = 1 ; x < WIDTH-1; x++) {
            int cnt = (map[WIDTH*(y+1) + x] > 0)
                      + (map[WIDTH*(y-1) + x] > 0)
                      + (map[WIDTH*(y) + x + 1] > 0)
                      + (map[WIDTH*(y) + x - 1] > 0);
            tmp_diff_depth_map[ WIDTH * y + x ] = (cnt > 2)?map[ WIDTH * y + x]:0;
        }
    }
    memcpy( map, tmp_diff_depth_map, sizeof(int) * FREENECT_FRAME_PIX );
}


void smoothInPlace( int * map ) {
    clearBorders( map );
    for (int y = 1 ; y < HEIGHT-1; y++) {
        for (int x = 1 ; x < WIDTH-1; x++) {
            int V = WIDTH * y + x;
            int cnt = map[V - WIDTH - 1] + map[V - WIDTH] + map[V - WIDTH + 1] +
                      map[V] + map[V] + map[V + 1] +
                      map[V + WIDTH - 1] + map[V + WIDTH] + map[V + WIDTH + 1];
            tmp_diff_depth_map[ V ] = cnt / 9;
        }
    }
    memcpy( map, tmp_diff_depth_map, sizeof(int) * FREENECT_FRAME_PIX );
}

void maxThresholdInPlace( int * map ) {
    for (int y = 1 ; y < HEIGHT-1; y++) {
        for (int x = 1 ; x < WIDTH-1; x++) {
            int V = WIDTH * y + x;
            int value = map[V];
            map[V] = (value > gthreshold)?gthreshold:value;
        }
    }
}

void calcStats( int * map, double * avg, int * sum ) {
    int summation = 0;
    for(int i = 0 ; i < FREENECT_FRAME_PIX; i++) {
        summation += map[ i ];
    }
    *avg = (double)summation / (double)FREENECT_FRAME_PIX;
    *sum = summation;
}

void calcStatsForRegion( int * map, double * avg, int * sum, int startx, int starty, int width, int height ) {
    int summation = 0;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            summation += map[ WIDTH * (starty+y) + (startx+x) ];
        }
    }
    *avg = (double)summation / (double)(width * height);
    *sum = summation;
}

double semivariogram(Mat & m, int n) {
    // https://en.wikipedia.org/wiki/Variogram
    int x[n];
    int y[n];
    for (int i = 0 ; i < n; i++) {
        x[i] = rand() % WIDTH;
        y[i] = rand() % HEIGHT;
    }
    float dist[n][n];
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            float v = m.at<int>(y[i],x[i]) - m.at<int>(y[j],x[j]);
            float v2 = v * v;
            dist[i][j] = v2;
            dist[j][i] = v2;
        }
    }
    Mat distm( n, n, CV_32F, dist, n * sizeof( float ) );
    Scalar mean;
    Scalar std;
    meanStdDev( distm, mean, std ); // 1/N * sum( dists squared )
    //fprintf(stderr, "MEAN: %e STD: %e \n", mean[0], std[0]);
    return sqrt(mean[0]);
}

// this variogram is to deal with the fact that I don't like the 0s they are special values
double semivariogram(Mat & m, int n, int minthreshold) {
    // https://en.wikipedia.org/wiki/Variogram
    int x[n];
    int y[n];
    for (int i = 0 ; i < n; i++) {
        x[i] = rand() % WIDTH;
        y[i] = rand() % HEIGHT;
    }
    float dist[n][n];
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            int d1 = m.at<int>(y[i],x[i]);
            int d2 = m.at<int>(y[j],x[j]);
            float v =  (d1 < minthreshold || d2 < minthreshold)?0.0:(d1 - d2);

            float v2 = v * v;
            dist[i][j] = v2;
            dist[j][i] = v2;
        }
    }
    Mat distm( n, n, CV_32F, dist, n * sizeof( float ) );
    Scalar mean;
    Scalar std;
    meanStdDev( distm, mean, std ); // 1/N * sum( dists squared )
    //fprintf(stderr, "MEAN: %e STD: %e \n", mean[0], std[0]);
    return sqrt(mean[0]);
}

void mirrorDiff(Mat & m, double * meanout, double * stddevout) {
    Mat mflip;
    flip(m, mflip, 1); //flip around Y axis = 1, X = 0, -1 both
    Scalar mean;
    Scalar std;
    //fprintf(stderr, "MEAN: %e STD: %e \n", mean[0], std[0]);
    Mat diff;
    absdiff( m, mflip, diff);
    meanStdDev( diff, mean, std ); // 1/N * sum( dists squared )
    //fprintf(stderr, "MEAN: %e STD: %e \n", mean[0], std[0]);
    *meanout = mean[0];
    *stddevout = std[0];
}
// From http://stackoverflow.com/questions/3486172/angle-between-3-points#3487062
int getAngleABC( Point a, Point b, Point c )
{
    Point ab( b.x - a.x, b.y - a.y );
    Point cb( b.x - c.x, b.y - c.y );

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);

    return (int) floor(alpha * 180. / pi + 0.5);
}

static int dfirst = 1;
// Does everything
void DrawScene()
{

    pthread_mutex_lock(&gl_backbuf_mutex);

    // When using YUV_RGB mode, RGB frames only arrive at 15Hz, so we shouldn't force them to draw in lock-step.
    // However, this is CPU/GPU intensive when we are receiving frames in lockstep.
    if (current_format == FREENECT_VIDEO_YUV_RGB) {
        while (!got_depth && !got_rgb) {
            pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
        }
    } else {
        while ((!got_depth || !got_rgb) && requested_format != current_format) {
            pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
        }
    }

    if (requested_format != current_format) {
        pthread_mutex_unlock(&gl_backbuf_mutex);
        return;
    }

    uint8_t *tmp;

    if (got_depth) {

        int i = 0;
        int j = 0;
        int jmin = 0;
        int mprn = 200;
        int d = 0;
        int dv = 0;
        tmp = depth_front;
        depth_front = depth_mid;
        depth_mid = tmp;
        memcpy(color_diff_depth_map, depth_mid, WIDTH*HEIGHT*3);
        got_depth = 0;
        jmin = depth_map[ 0 ];
        Mat depthFrame(HEIGHT,WIDTH, MYCVTYPE, depth_map, sizeof(int) * WIDTH);


        Scalar color( rand()&255, rand()&255, rand()&255 );
        Mat dst(HEIGHT, WIDTH, CV_8UC3, color_diff_depth_map);

        gdegree = (gdegree + gtempo) % 360;
        const int len = min(WIDTH,HEIGHT)/2;

        int xprime = WIDTH-1;
#define NSamples 100
        int samples[NSamples];
        int xs[NSamples];
        int ys[NSamples];
        for (int i = 0; i < NSamples; i++) {
            xs[i] = rand() % WIDTH;
            ys[i] = rand() % HEIGHT;
            int j = xs[i] + ys[i] * WIDTH;
            samples[i] = depth_map[j];
        }
        for (int i = 0; i < NSamples; i++) {
            fprintf(stdout,"%d,%d,%d,%d\n", xs[i],ys[i],samples[i], samples[i] + 1024*(xs[i] + ys[i] * WIDTH));
        }
        fflush(stdout);
        lo_blob btest = lo_blob_new(NSamples * sizeof(int), samples);
        lo_send(lo_t, "/samples", "b", btest);

        flip(dst,dst,1);


        depthFrame.copyTo(lastDepthFrame);

        depth_frame++;

    }
    if (got_rgb) {
        tmp = rgb_front;
        rgb_front = rgb_mid;
        rgb_mid = tmp;
        got_rgb = 0;
    }

    pthread_mutex_unlock(&gl_backbuf_mutex);
#ifdef SDL
    SDL_Flip(sdlSurface);
#endif



}


uint16_t t_gamma[2048];

void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
    int i;
    uint16_t *depth = (uint16_t*)v_depth;

    pthread_mutex_lock(&gl_backbuf_mutex);
    for (i=0; i<FREENECT_FRAME_PIX; i++) {
        // Thresholding is being done here
        depth[i] = (depth[ i ] > gthreshold)?0:depth[i];//gthreshold:depth[ i ] ;
        int pval = t_gamma[depth[i]];
        depth_map[ i ] = depth[ i ]; // (depth[ i ] > gthreshold)?gthreshold:depth[ i ] ; /* pval; */
        int lb = pval & 0xff;
        switch (pval>>8) {
        case 0:
            depth_mid[3*i+0] = 255;
            depth_mid[3*i+1] = 255-lb;
            depth_mid[3*i+2] = 255-lb;
            break;
        case 1:
            depth_mid[3*i+0] = 255;
            depth_mid[3*i+1] = lb;
            depth_mid[3*i+2] = 0;
            break;
        case 2:
            depth_mid[3*i+0] = 255-lb;
            depth_mid[3*i+1] = 255;
            depth_mid[3*i+2] = 0;
            break;
        case 3:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 255;
            depth_mid[3*i+2] = lb;
            break;
        case 4:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 255-lb;
            depth_mid[3*i+2] = 255;
            break;
        case 5:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 0;
            depth_mid[3*i+2] = 255-lb;
            break;
        default:
            depth_mid[3*i+0] = 0;
            depth_mid[3*i+1] = 0;
            depth_mid[3*i+2] = 0;
            break;
        }
    }
    got_depth++;
    pthread_cond_signal(&gl_frame_cond);
    pthread_mutex_unlock(&gl_backbuf_mutex);
}


void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
    pthread_mutex_lock(&gl_backbuf_mutex);

    // swap buffers
    assert (rgb_back == rgb);
    rgb_back = rgb_mid;
    freenect_set_video_buffer(dev, rgb_back);
    rgb_mid = (uint8_t*)rgb;

    got_rgb++;
    pthread_cond_signal(&gl_frame_cond);
    pthread_mutex_unlock(&gl_backbuf_mutex);
}

void *freenect_threadfunc(void *arg)
{
    int accelCount = 0;

    freenect_set_tilt_degs(f_dev,freenect_angle);
    freenect_set_led(f_dev,LED_RED);
    freenect_set_depth_callback(f_dev, depth_cb);
    freenect_set_video_callback(f_dev, rgb_cb);
    freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, current_format));
    freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));

    freenect_set_video_buffer(f_dev, rgb_back);

    freenect_start_depth(f_dev);
    freenect_start_video(f_dev);

    fprintf(stderr,"'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");

    while (!die && freenect_process_events(f_ctx) >= 0) {
        //Throttle the text output
        if (accelCount++ >= 2000)
        {
            accelCount = 0;
            freenect_raw_tilt_state* state;
            freenect_update_tilt_state(f_dev);
            state = freenect_get_tilt_state(f_dev);
            double dx,dy,dz;
            freenect_get_mks_accel(state, &dx, &dy, &dz);
            fprintf(stderr,"\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
            fflush(stdout);
        }

        if (requested_format != current_format) {
            freenect_stop_video(f_dev);
            //freenect_set_video_format(f_dev, requested_format);
            freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, requested_format));

            freenect_start_video(f_dev);
            current_format = requested_format;
        }
    }

    printf("\nshutting down streams...\n");

    freenect_stop_depth(f_dev);
    freenect_stop_video(f_dev);

    freenect_close_device(f_dev);
    freenect_shutdown(f_ctx);

    printf("-- done!\n");
    return NULL;
}

int main(int argc, char **argv)
{
    int res;

#ifdef SDL
    SDL_Event e;
    SDL_MouseMotionEvent m;

#endif
    int pixels = 640*480;
    srand(time(0));


    depth_mid = (uint8_t*)malloc(640*480*3);
    depth_map = (int *) malloc(sizeof(int)*pixels);
    memset(depth_map, 0, sizeof(int)*pixels);
    old_depth_map = (int *) malloc(sizeof(int)*pixels);
    memset(old_depth_map, 0, sizeof(int)*pixels);
    diff_depth_map = (int *) malloc(sizeof(int)*pixels);
    memset(diff_depth_map, 0, sizeof(int)*pixels);
    tmp_diff_depth_map = (int *) malloc(sizeof(int)*pixels);
    memset(tmp_diff_depth_map, 0, sizeof(int)*pixels);

    depth_front = (uint8_t*)malloc(640*480*3);
    rgb_back = (uint8_t*)malloc(640*480*3);
    rgb_mid = (uint8_t*)malloc(640*480*3);
    rgb_front = (uint8_t*)malloc(640*480*3);

    //printf("Kinect camera test\n");

    int i;
    for (i=0; i<2048; i++) {
        float v = i/2048.0;
        v = powf(v, 3)* 6;
        t_gamma[i] = v*6*256;
    }


    g_argc = argc;
    g_argv = argv;

    if (freenect_init(&f_ctx, NULL) < 0) {
        printf("freenect_init() failed\n");
        return 1;
    }

    freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);

    int nr_devices = freenect_num_devices (f_ctx);
    fprintf (stderr,"Number of devices found: %d\n", nr_devices);

    int user_device_number = 0;
    if (argc > 1)
        user_device_number = atoi(argv[1]);

    if (nr_devices < 1)
        return 1;

    if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
        printf("Could not open device\n");
        return 1;
    }





    res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
    if (res) {
        printf("pthread_create failed\n");
        return 1;
    }





#ifdef SDL

    sdlSurface = SDL_SetVideoMode(WIDTH , HEIGHT, 24, 0);
    SDL_WM_SetCaption("Shape Of Sound",0);
    atexit(SDL_Quit);
    color_diff_depth_map = (uint8_t*) sdlSurface->pixels;

    SDL_EnableKeyRepeat(30, 10);

    for(;;) {
        DrawScene();
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_KEYUP:
                gkeydown = false;
                break;
            case SDL_KEYDOWN:
                gkeydown = true;
                gkeysym = e.key.keysym.sym;
                if (e.key.keysym.sym == 'x') {
                    exit(0);
                } else if (e.key.keysym.sym == SDLK_ESCAPE) { //Escape
                    exit(0);
                } else if (e.key.keysym.sym == 'h') {
                    gthreshold=(gthreshold>1024-10)?1024:(gthreshold+10);
                } else if (e.key.keysym.sym == 'j') {
                    gthreshold=(gthreshold<256+10)?256:(gthreshold-10);
                } else if (e.key.keysym.sym == 'r') {
                    freenect_angle=0;
                    freenect_set_tilt_degs(f_dev,freenect_angle);
                } else if (e.key.keysym.sym == '=') {
                    freenect_angle++;
                    if (freenect_angle > 30) {
                        freenect_angle = 30;
                    }
                    freenect_set_tilt_degs(f_dev,freenect_angle);

                } else if ( e.key.keysym.sym == '-') {
                    freenect_angle--;
                    if (freenect_angle < -30) {
                        freenect_angle = -30;
                    }
                    freenect_set_tilt_degs(f_dev,freenect_angle);

                }
                break;
            case SDL_QUIT:
                exit(0);
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
                m = e.motion;
                /* paint in the cursor on click */
                if (m.state) {
                    gyline = m.y;
                } /* if state */
            } /* event type */
        } /* Poll */
        SDL_Delay(33);
    }




#endif


    return 0;
}
