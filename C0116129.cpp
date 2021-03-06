#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <stdbool.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define KEY_ESC 27

#define	imageWidth 128
#define	imageHeight 128

#define LAUNCH_MAX 30        // 花火の最大同時打ち上げ可能数

#define CIR_COUNT 12         // 1周あたりの火薬の数（完全な球verで使用）
#define FIRE_COUNT 100       // 火球の個数（ランダム爆発verで使用）
#define FIRE_SIZE 0.015      // 火球１粒あたりの大きさ
#define FIRE_TALE 1.35        // 火球が持つ尾の長さ
#define EXPLOSION_MIN 0.5    // 爆発する瞬間の大きさ（r_scaleの初期化値） 
#define EXPLOSION_MAX 22     // 爆発が広がる限界サイズ
#define EXPLOSION_SPEED 0.01 // 火球の広がる速度

float controllerXYZ[LAUNCH_MAX][3];      // 打ち上げの軌道
float r_scale[LAUNCH_MAX];               // 爆発の半径rの拡大率
float angle[LAUNCH_MAX][FIRE_COUNT][2];  // 広がる火球の角度（θ1，θ2）
float fireColor[LAUNCH_MAX][3];          // 色情報
float pos[LAUNCH_MAX][3];                // 生成される地点
bool launched[LAUNCH_MAX];               // 打ち上げられたかどうか
bool transed[LAUNCH_MAX];                // 爆発するモードかどうか
bool modePerfectSphere = false;          // 爆発の種類

float cameraAngle = 0.0;

unsigned char texImage[imageHeight][imageWidth][3];

unsigned char	mouseFlag = GL_FALSE;
int xStart, yStart;
double	xAngle = 0.0, yAngle = 0.0, theta = 0.0;

GLfloat diffuse[] = { 0.5, 0.3, 0.05, 1.0 };
GLfloat specular[] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat ambient[] = { 0.1, 0.1, 0.1, 1.0 };
GLfloat shininess = 2.0;

GLfloat lightPos[] = { 0.0, 2.0, 0.0, 1.0 };
GLfloat lightDif[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat lightAmb[] = { 0.5, 0.5, 0.5, 1.0 };

void readRAWImage(const char* filename)
{
	FILE *fp;

	if (fopen_s(&fp, filename, "r")) {
		fprintf(stderr, "Cannot open raw file %s\n", filename);
		exit(1);
	}

	fread(texImage, 1, imageWidth*imageHeight * 3, fp);	// read RGB data
	fclose(fp);
}

void setupTextures(void)
{
	readRAWImage("C0116129-1.raw");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texImage);

}

void resetParam(int b) {
	controllerXYZ[b][0] = 0;
	controllerXYZ[b][1] = -2.0;
	controllerXYZ[b][2] = 0;
	r_scale[b] = EXPLOSION_MIN;
	transed[b] = false;
	launched[b] = false;
}

void setupParam() {
	for (int i = 0; i < LAUNCH_MAX; i++) {
		resetParam(i);
	}
}

float toRad(float degree) {
	return degree * M_PI / 180.0;
}

float randomTo(float min, float max) {
	float range = max - min;
	return (float)rand() / (float)RAND_MAX * range + min;
}

float currentAngle(int count, int n) {
	return (360 / (float)count) * n;
}

void decideFireColor(int b) {
	for (int i = 0; i < 3; i++) {
		fireColor[b][i] = randomTo(0.3, 1);
	}
}

void decideFireAngle(int b) {
	for (int i = 0; i < FIRE_COUNT; i++) {
		angle[b][i][0] = randomTo(0, 360);
		angle[b][i][1] = randomTo(0, 360);
	}
}

void decideNextPos(int b) {
	for (int i = 0; i < 3; i++) {
		pos[b][i] = randomTo(-1, 1);
	}
}

void checkCanLaunch() {
	for (int i = 0; i < LAUNCH_MAX; i++) {
		if (!launched[i]) {
			launched[i] = true;
			decideNextPos(i);
			break;
		}
	}
}

//キーボードイベント
void myKeyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case KEY_ESC:
		exit(0);
		break;
	case 'f':
		//発射可能であれば発射を行う
		checkCanLaunch();
		break;
	case 's':
		//爆発モードの変更
		modePerfectSphere = !modePerfectSphere;
		break;
	}
}

void drawFireworks(int b) {
	if (transed[b]) {
		glColor3fv(fireColor[b]);
		if (!modePerfectSphere) {
			// ランダム爆発ver ---------------------------------------------------------
			for (int i = 0; i < FIRE_COUNT; i++) {
				//超カラフル------------------------
				/*decideFireColor(b);
				glColor3fv(fireColor[b]);*/
				//--------------------------------

				float theta1 = toRad(angle[b][i][0]);
				float theta2 = toRad(angle[b][i][1]);
				float ax = cos(theta1) * cos(theta2) * (r_scale[b] / 20);
				float ay = sin(theta1) * (r_scale[b] / 20);
				float az = cos(theta1) * sin(theta2) * (r_scale[b] / 20);
				glPushMatrix();
				//glEnable(GL_NORMALIZE);
				//glNormal3d(cos(toRad(cameraAngle)), 0, sin(toRad(cameraAngle)));
				glBegin(GL_LINES);
				glVertex3f(pos[b][0] + ax, ay, pos[b][2] + az);
				glVertex3f(pos[b][0] + ax / FIRE_TALE, ay / FIRE_TALE, pos[b][2] + az / FIRE_TALE);
				glEnd();
				//glDisable(GL_NORMALIZE);
				glTranslatef(pos[b][0] + ax, ay, pos[b][2] + az);
				glutSolidCube(FIRE_SIZE);
				//glutSolidSphere(FIRE_SIZE, 5, 5);
				glPopMatrix();
			}
			// ------------------------------------------------------------------------
		}
		else {
			// 完全な球ver -------------------------------------------------------------
			for (int i = 0; i < CIR_COUNT; i++) {
				float theta_i = toRad(currentAngle(CIR_COUNT, i));
				for (int j = 0; j < CIR_COUNT; j++) {
					float theta_j = toRad(currentAngle(CIR_COUNT, j));
					float ax = cos(theta_i) * cos(theta_j) * (r_scale[b] / 20);
					float ay = sin(theta_i) * (r_scale[b] / 20);
					float az = cos(theta_i) * sin(theta_j) * (r_scale[b] / 20);

					glPushMatrix();
					glBegin(GL_LINES);
					glVertex3f(pos[b][0] + ax, ay, pos[b][2] + az);
					glVertex3f(pos[b][0] + ax / FIRE_TALE, ay / FIRE_TALE, pos[b][2] + az / FIRE_TALE);
					glEnd();
					glTranslatef(pos[b][0] + ax, ay, pos[b][2] + az);
					//glutSolidSphere(FIRE_SIZE, 5, 5);
					glutSolidCube(FIRE_SIZE);
					glPopMatrix();
				}
			}
			// ------------------------------------------------------------------------
		}
	}
	else {
		decideFireColor(b);
		glPushMatrix();
		glColor3fv(fireColor[b]);
		glTranslatef(pos[b][0] + controllerXYZ[b][0], controllerXYZ[b][1], pos[b][2] + controllerXYZ[b][2]);
		glutSolidSphere(0.03, 16, 16);
		glPopMatrix();
	}
}

//描画関数
void display() {
	double	s, t;
	double	p0[] = { -1.0, -1.945, -1.0 }, p1[] = { 1.0, -1.945, -1.0 },
		p2[] = { 1.0, -1.945, 1.0 }, p3[] = { -1.0, -1.945, 1.0 };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//視点移動
	gluLookAt(cos(toRad(cameraAngle + yAngle)) * 3.0,
		tan(toRad(xAngle)),
		sin(toRad(cameraAngle + yAngle)) * 3.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);

	glPushMatrix();
	glTranslatef(0.0, 1.0, 0.0);

	glEnable(GL_TEXTURE_2D);

	s = 4.0;
	t = 4.0;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0); glVertex3dv(p0);
	glTexCoord2d(s, 0.0); glVertex3dv(p1);
	glTexCoord2d(s, t); glVertex3dv(p2);
	glTexCoord2d(0.0, t); glVertex3dv(p3);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

	glPushMatrix();
	glTranslatef(0, -2.0, 0.0);
	glScalef(1.0, 0.05, 1.0);
	//glColor3f(0.2, 0.2, 0.2);
	glutSolidCube(2);
	glPopMatrix();

	glDisable(GL_LIGHTING);

	// 花火の描画
	for (int b = 0; b < LAUNCH_MAX; b++) {
		if (launched[b]) {
			drawFireworks(b);
		}
	}

	glColor3f(0.8, 0.8, 0.8);
	glPopMatrix();

	glFinish();
	//glFlush();
}

//idle関数
void idle() {
	cameraAngle += 0.001;
	if (cameraAngle >= 360) cameraAngle = 0.0;

	for (int i = 0; i < LAUNCH_MAX; i++) {
		if (launched[i]) {
			//打ち上げ高度が0に達したとき爆発
			if (controllerXYZ[i][1] > 0 && !transed[i]) {
				transed[i] = true;
				decideFireAngle(i);
				decideFireColor(i);
			}
			//爆発の広がりが指定値に達したら終了
			if (r_scale[i] > EXPLOSION_MAX) {
				resetParam(i);
			}
			//爆発 or 上昇
			if (transed[i]) {
				r_scale[i] += EXPLOSION_SPEED;
			}
			else {
				controllerXYZ[i][0] = cos(toRad(controllerXYZ[i][1] * 1500)) / 40;
				controllerXYZ[i][2] = sin(toRad(controllerXYZ[i][1] * 1500)) / 40;
				controllerXYZ[i][1] += 0.0002;
			}
		}
	}
	glutPostRedisplay();
}

//マウスの動きから角度を計算
void myMouseMotion(int x, int y)
{
	int xdis, ydis;
	double a = 0.5;

	if (mouseFlag == GL_FALSE)
		return;
	xdis = x - xStart;
	ydis = y - yStart;

	xAngle += (double)ydis*a;
	yAngle += (double)xdis*a;

	if (abs((int)xAngle) > 60) {
		int s;
		if (xAngle < 0) {
			s = -1;
		}
		else {
			s = 1;
		}
		xAngle = 60 * s;
	}

	xStart = x;
	yStart = y;
	glutPostRedisplay();
}

//マウスの左ボタンが押された位置を特定
void myMouseFunc(int button, int state, int x, int y)
{
	//追記
	//button：マウスの対象ボタン
	//state:押したか離したか
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		xStart = x;
		yStart = y;
		mouseFlag = GL_TRUE;
	}
	else {
		mouseFlag = GL_FALSE;
	}
}

//初期化
void myInit(char *progname) {
	int width = 700, height = 700;
	double aspect = (double)width / (double)height;

	setupParam();

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(progname);
	glClearColor(0.0, 0.0, 0.15, 1.0);
	glShadeModel(GL_SMOOTH);

	glutKeyboardFunc(myKeyboard);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(myMouseMotion);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.0, aspect, 0.1, 20.0);

	//視点変更
	//gluLookAt(0.0, -1.5, 4.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	/*-- 0番ライトの設定を記述--*/
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);

	setupTextures();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);

}

void myReshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.0, (double)width / (double)height, 0.1, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//メイン
int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	myInit(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(myReshape);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}

