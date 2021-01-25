//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando. 
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Kálmán Bendegúz Bence
// Neptun : PTW6BD
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;

const int nv = 50;

vec2 a, b, c;
int clickCount = 0;

class Circle {
	float r;
	vec2 center;
	vec2 vertices[4 * nv];
	unsigned int vao, vbo;

public:

	Circle(float _r, vec2 _center) {
		r = _r;
		center = vec2(_center);
		for (int i = 0; i < 4 * nv; ++i) {
			float fi = i * 2 * M_PI / (4 * nv);
			vertices[i] = center + r * vec2(cosf(fi), sinf(fi));
		}
	}

	void create() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * 4 * nv, vertices, GL_STATIC_DRAW);
	}

	void Draw() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 0.1f, 0.2f, 0.3f);
		float MVPtransf[4][4] = { 1, 0, 0, 0,
								  0, 1, 0, 0,
								  0, 0, 1, 0,
								  0, 0, 0, 1 };
		location = glGetUniformLocation(gpuProgram.getId(), "MVP");
		glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4 * nv);
	}

};

class SiriusLine {
	
	vec2 p1;
	vec2 p2;
	float angleOfView;
	unsigned int vao, vbo;

public:

	float r;
	vec2 center;
	vec2 vertices[nv];

	SiriusLine(vec2 _p1, vec2 _p2) {
		p1 = _p1;
		p2 = _p2;
		float f = p1.x;
		float g = p1.y;
		float a = p2.x;
		float b = p2.y;
		float c = a / (a * a + b * b);
		float d = b / (a * a + b * b);
		float i = ((((f - c) * (f * f - a * a)) / ((2.0) * (f - a))) + (((f - c) * (g * g - b * b)) / (2.0 * (f - a))) - ((f * f - c * c) / (2.0)) - ((g * g - d * d) / (2.0))) / ((d - g) - (((f - c) * (g - b)) / (a - f)));
		float h = ((g - b) * i - ((f * f - a * a) / (2.0)) - ((g * g - b * b) / (2.0))) / (a - f);

		center = vec2(h, i);
		vec2 radius1(p1 - center);
		vec2 radius2(p2 - center);
		r = length(radius1);
		angleOfView = acosf((dot((radius1), (radius2))) / ((length(radius1) * length(radius2))));
		if (angleOfView > M_PI) angleOfView = 2 * M_PI - angleOfView;
		float initialAngle = atan2f((radius1).y , (radius1).x);
		
		vec2 rotated(cosf(angleOfView) * radius1.x - sinf(angleOfView) * radius1.y, sinf(angleOfView) * radius1.x + cosf(angleOfView) * radius1.y);
		float drawDirection = 1.0;
		if (length(rotated - radius2) > 0.1) {
			drawDirection = -1.0;
		}

		for (int i = 0; i < nv ; ++i) {
			float fi = initialAngle + (drawDirection) * ((i * angleOfView) / nv);
			vertices[i] =  center + r * vec2(cosf(fi), sinf(fi));
		}
		
	}
	
	void reverse() {
		for (int i = 0; i < nv / 2; ++i) {
			vec2 temp(vertices[i]);
			vertices[i] = vec2(vertices[nv - i - 1]);
			vertices[nv - i - 1] = vec2(temp);
		}
	}

};


class SiriusTriangle {

	SiriusLine ab = SiriusLine(vec2(0.0, 0.0), vec2(0.0, 0.0));
	SiriusLine bc = SiriusLine(vec2(0.0, 0.0), vec2(0.0, 0.0));;
	SiriusLine ac = SiriusLine(vec2(0.0, 0.0), vec2(0.0, 0.0));;
	vec2 vertices[3 * nv];
	vec2 triangles[3 * (3 * nv - 2)];
	unsigned int vao[2];
	unsigned int vbo[2];

public:
	SiriusTriangle(vec2 p1, vec2 p2, vec2 p3) {

		ab = SiriusLine(p1, p2);
		bc = SiriusLine(p2, p3);
		ac = SiriusLine(p3, p1);
		
		if (length(ab.vertices[nv - 1] - bc.vertices[0]) > length(ab.vertices[nv - 1] - bc.vertices[nv - 1])) bc.reverse(); 
		if (length(bc.vertices[nv - 1] - ac.vertices[0]) > length(bc.vertices[nv - 1] - ac.vertices[nv - 1])) ac.reverse();
			
		for (int i = 0; i < nv; ++i) {
			vertices[i].x = ab.vertices[i].x;
			vertices[i].y = ab.vertices[i].y;
			vertices[nv + i].x = bc.vertices[i].x;
			vertices[nv + i].y = bc.vertices[i].y;
			vertices[2 * nv + i].x = ac.vertices[i].x;
			vertices[2 * nv + i].y = ac.vertices[i].y;
		}

		int t = 0;
		for (int i = 0; i < nv / 2; ++i) {
			triangles[3*t] = vertices[i];
			triangles[3 * t + 1] = vertices[i + 1];
			triangles[3 * t + 2] = vertices[3 * nv - 1 - i];
			++t;
		}
		for (int i = nv; i < nv + nv / 2 ; ++i) {
			triangles[3 * t] = vertices[i];
			triangles[3 * t + 1] = vertices[i + 1];
			triangles[3 * t + 2] = vertices[2 * nv - 1 - i];
			++t;
		}
		for (int i = 2 * nv; i < 2 * nv + nv / 2; ++i) {
			triangles[3 * t] = vertices[i];
			triangles[3 * t + 1] = vertices[i + 1];
			triangles[3 * t + 2] = vertices[4 * nv - 1 - i];
			++t;
		}
		for (int i = 3* nv -1 ; i > 2 * nv + nv / 2; --i) {
			triangles[3 * t] = vertices[i];
			triangles[3 * t + 1] = vertices[i - 1];
			triangles[3 * t + 2] = vertices[3 * nv - i];
			++t;
		}
		for (int i = nv - 1; i > nv / 2; --i) {
			triangles[3 * t] = vertices[i];
			triangles[3 * t + 1] = vertices[i - 1];
			triangles[3 * t + 2] = vertices[2 * nv - i];
			++t;
		}
		for (int i = 2 * nv - 1; i > nv + nv / 2; --i) {
			triangles[3 * t] = vertices[i];
			triangles[3 * t + 1] = vertices[i - 1];
			triangles[3 * t + 2] = vertices[4 * nv - i];
			++t;
		}
		triangles[3 * (3 * nv - 2) - 3] = vertices[nv / 2];
		triangles[3 * (3 * nv - 2) - 2] = vertices[nv + nv / 2];
		triangles[3 * (3 * nv - 2) - 1] = vertices[2 * nv + nv / 2];
	}

	void create() {
		glGenVertexArrays(1, &vao[0]);
		glBindVertexArray(vao[0]);
		glGenBuffers(1, &vbo[0]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vec2) * 3 * (3 * nv - 2),
			triangles,
			GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,
			2, GL_FLOAT, GL_FALSE,
			0, NULL);

		createEdges();
	}

	void Draw() {
		glBindVertexArray(vao[0]);
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 0.1f, 0.55f, 0.05f);
		glDrawArrays(GL_TRIANGLES, 0, 3 * (3 * nv - 2));
		drawEdges();
	}

	void createEdges() {
		glGenVertexArrays(1, &vao[1]);
		glBindVertexArray(vao[1]);
		glGenBuffers(1, &vbo[1]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vec2) * 3 * nv,
			vertices,
			GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,
			2, GL_FLOAT, GL_FALSE,
			0, NULL);
	}

	void drawEdges() {
		glBindVertexArray(vao[1]);
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 1.0f, 1.0f);
		glDrawArrays(GL_LINE_LOOP, 0, 3 * nv);
	}

	void printAngles() {
		float d_ab_bc = length(ab.center - bc.center);
		float d_bc_ac = length(bc.center - ac.center);
		float d_ab_ac = length(ab.center - ac.center);

		float angle_A = acosf((ab.r * ab.r + bc.r * bc.r - d_ab_bc * d_ab_bc) / (2 * ab.r * bc.r));
		float angle_B = acosf((bc.r * bc.r + ac.r * ac.r - d_bc_ac * d_bc_ac) / (2 * bc.r * ac.r));
		float angle_C = acosf((ab.r * ab.r + ac.r * ac.r - d_ab_ac * d_ab_ac) / (2 * ab.r * ac.r));

		printf("\nAngle A = %3.3f radians", angle_A);
		printf("\nAngle B = %3.3f radians", angle_B);
		printf("\nAngle C = %3.3f radians", angle_C);
	}

	void printLengths() {
		float sideLength[3];
		for (int j = 0; j < 3; ++j) {
			sideLength[j] = 0.0;
			for (int i = 0; i < nv; ++i) {
				vec2 part(vertices[j * nv + i] - vertices[(j * nv + i + 1) % (3 * nv)]);
				float partLength = (sqrtf(part.x * part.x + part.y * part.y)) / (1.0 - vertices[j * nv + i].x * vertices[j * nv + i].x - vertices[j * nv + i].y * vertices[j * nv + i].y);
				sideLength[j] += partLength;
			}
		}

		printf("\nLength of AB: %3.3f", sideLength[0]);
		printf("\nLength of BC: %3.3f", sideLength[1]);
		printf("\nLength of AC: %3.3f", sideLength[2]);
	}
};

SiriusTriangle st(vec2(0.0,0.0), vec2(0.0, 0.0), vec2(0.0, 0.0));
Circle circle(1.0, vec2(0.0, 0.0));

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	circle.create();
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	circle.Draw();
	st.Draw();
	glutSwapBuffers(); 
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { 
	float cX = 2.0f * pX / windowWidth - 1;	
	float cY = 1.0f - 2.0f * pY / windowHeight;

	if (state == GLUT_DOWN) {
		if (clickCount % 3 == 0) a = vec2(cX, cY);
		else if (clickCount % 3 == 1) b = vec2(cX, cY);
		else if (clickCount % 3 == 2) {
			c = vec2(cX, cY);
			st = SiriusTriangle(a, b, c);
			st.create();
			st.Draw();
			st.printAngles();
			st.printLengths();
		}
		++clickCount;
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {}

