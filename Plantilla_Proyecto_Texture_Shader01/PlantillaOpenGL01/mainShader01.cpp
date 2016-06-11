// Cubica

#include <stdlib.h>
#include <conio.h>

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include "glsl.h"
#include "glm.h"

// assimp include files. These three are usually needed.
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// the global Assimp scene object
const aiScene* scene = NULL;
GLuint scene_list = 0;
aiVector3D scene_min, scene_max, scene_center;

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)


using namespace std;

cwc::glShaderManager SM;
cwc::glShader *shader;


//Textures
static GLuint texflat;
unsigned char* imageflat = NULL;

//Central
static GLuint texcentral;
unsigned char* imagecentral = NULL;

//Relleno 01
static GLuint texrelleno01;
unsigned char* imagerelleno01 = NULL;

//Relleno 02
static GLuint texrelleno02;
unsigned char* imagerelleno02 = NULL;

//Patron del piso
static GLuint texpiso;
unsigned char* imagepiso = NULL;

//Variables para la intensidad de las luces
//El texture.frag lanza error si coloco GLfloat
float ambiental = 1.0;	// Valores en los que se pide iniciar el programa
float central   = 1.0;	
float relleno01 = 0.0;	// Al iniciar no se pueden reducir las luces de relleno (estan apagadas)
float relleno02 = 0.0;
float piso      = 0.0;

//Variables para el color de las luces
float colorA[4] = {1.0,1.0,1.0,1.0}; // Para RGB solo necesito 3 pero para el texture.frag 4
float colorC[4] = {1.0,1.0,1.0,1.0};
float colorP[4] = {0.0,0.0,0.0,0.0};
float colorR01[4] = {1.0,1.0,1.0,1.0};
float colorR02[4] = {1.0,1.0,1.0,1.0};

int iheight, iwidth;

//Filtro
float filtro;

void ejesCoordenada() {
	
	glDisable(GL_LIGHTING);	
	glLineWidth(2.5);
	glBegin(GL_LINES);
		glColor3f(1.0,0.0,0.0);
		glVertex2f(0,10);
		glVertex2f(0,-10);
		glColor3f(0.0,0.0,1.0);
		glVertex2f(10,0);
		glVertex2f(-10,0);
	glEnd();

	glLineWidth(1.5);
	int i;
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);
		for(i = -10; i <=10; i++){
			if (i!=0) {		
				if ((i%2)==0){	
					glVertex2f(i,0.4);
					glVertex2f(i,-0.4);

					glVertex2f(0.4,i);
					glVertex2f(-0.4,i);
				}else{
					glVertex2f(i,0.2);
					glVertex2f(i,-0.2);

					glVertex2f(0.2,i);
					glVertex2f(-0.2,i);
				}
			}
		}
		
	glEnd();

	glEnable(GL_LIGHTING);

	glLineWidth(1.0);
}

void changeViewport(int w, int h) {
	
	float aspectratio;

	if (h==0) h=1;

   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(55, (GLfloat) w/(GLfloat) h, 1.0, 300.0);
   glMatrixMode (GL_MODELVIEW);

}

void init(){
	
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   
   // Cargando Textura
   glGenTextures(1, &texflat);
   glBindTexture(GL_TEXTURE_2D, texflat);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   imageflat = glmReadPPM("baked_flat.ppm", &iwidth, &iheight);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageflat);

   // Luz central
   glGenTextures(1, &texcentral);
   glBindTexture(GL_TEXTURE_2D, texcentral);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   imagecentral = glmReadPPM("baked_keyrabbit.ppm", &iwidth, &iheight);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imagecentral);

   // Luz relleno 01
   glGenTextures(1, &texrelleno01);
   glBindTexture(GL_TEXTURE_2D, texrelleno01);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   imagerelleno01 = glmReadPPM("baked_fill01.ppm", &iwidth, &iheight);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imagerelleno01);

   // Luz relleno 02
   glGenTextures(1, &texrelleno02);
   glBindTexture(GL_TEXTURE_2D, texrelleno02);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   imagerelleno02 = glmReadPPM("baked_fill02.ppm", &iwidth, &iheight);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imagerelleno02);

	// Piso
   glGenTextures(1, &texpiso);
   glBindTexture(GL_TEXTURE_2D, texpiso);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   imagepiso = glmReadPPM("baked_checker.ppm", &iwidth, &iheight);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imagepiso);


   shader = SM.loadfromFile("texture.vert","texture.frag"); // load (and compile, link) from file
  		  if (shader==0) 
			  std::cout << "Error Loading, compiling or linking shader\n";

}

/*
char easytolower(char key){
	if(in<='Z' && in>='A') return in-('Z'-'z');
	return in;
}
*/

void Keyboard(unsigned char key, int x, int y){

	//key = easytolower(key);

	switch (key){		
	// ---------- INTENSIDAD LUCES ----------
	// incrementa intensidad luz ambiental
		case '1':
			ambiental += 0.05;
		break;
	// reduce intensidad luz ambiental
		case '2':
			if (ambiental - 0.05 > 1.0) ambiental -= 0.05;
		break;
	// incrementa intensidad luz relleno 01
		case 'q':
			relleno01 += 0.05;
		break;
	// reduce intensidad luz relleno 01
		case 'w':
			if (relleno01 - 0.05 > 0.0) relleno01 -= 0.05;
		break;
	// incrementa intensidad luz relleno 02
		case 'a':
			relleno02 += 0.05;
		break;
	// reduce intensidad luz relleno 02
		case 's':
			if (relleno02 - 0.05 > 0.0) relleno02 -= 0.05;
		break;
	// incrementa intensidad luz central
		case 'z':
			central += 0.05;
		break;
	// reduce intensidad luz central
		case 'x':
			if (central - 0.05 > 1.0) central -= 0.05;
		break;
	// ---------- COLOR LUCES ----------
	// incrementos color luz relleno 01
		case 'e': // componente R
			colorR01[0] += 0.05;
		break;
		case 'r': // componente G
			colorR01[1] += 0.05;
		break;
		case 't': // componente B
			colorR01[2] += 0.05;
		break;
	// reducciones color luz relleno 01
	// componente R
		case 'y':
			if (colorR01[0] - 0.05 > 0.0) colorR01[0] -= 0.05;
		break;
	// componente G
		case 'u':
			if (colorR01[1] - 0.05 > 0.0) colorR01[1] -= 0.05;
		break;
	// componente B
		case 'i':
			if (colorR01[2] - 0.05 > 0.0) colorR01[2] -= 0.05;
		break;
	// incrementos color luz relleno 02
	// componente R
		case 'd':
			colorR02[0] += 0.05;
		break;
	// componente G
		case 'f':
			colorR02[1] += 0.05;
		break;
	// componente B
		case 'g':
			colorR02[2] += 0.05;
		break;
	// reducciones color luz relleno 02
	// componente R
		case 'h':
			if (colorR02[0] - 0.05 > 0.0) colorR02[0] -= 0.05;
		break;
	// componente G
		case 'j':
			if (colorR02[1] - 0.05 > 0.0) colorR02[1] -= 0.05;
		break;
	// componente B
		case 'k':
			if (colorR02[2] - 0.05 > 0.0) colorR02[2] -= 0.05;
		break;
	// incrementos color luz central
	// componente R
		case 'c':
			colorC[0] += 0.05;
		break;
	// componente G
		case 'v':
			colorC[1] += 0.05;
		break;
	// componente B
		case 'b':
			colorC[2] += 0.05;
		break;
	// reducciones color luz central
	// componente R
		case 'n':
			if (colorC[0] - 0.05 > 0.0) colorC[0] -= 0.05;
		break;
	// componente G
		case 'm':
			if (colorC[1] - 0.05 > 0.0) colorC[1] -= 0.05;
		break;
	// componente B
		case ',':
			if (colorC[2] - 0.05 > 0.0) colorC[2] -= 0.05;
		break;

	
	// ---------- FILTROS BILINEALES ----------

	// activacion
		case 'o':
			filtro = 1.0;
		break;
	// desactivacion
		case 'p':
			filtro = 0.0;
		break;
	// ---------- COLORES PATRON PISO ----------
		case '3':
			piso = 1.0;
			colorP[0] = 0.5;
			colorP[1] = 0.5;
			colorP[2] = 1;
		break;
		case '4':
			piso = 1.0;
			colorP[0] = 1;
			colorP[1] = 0.0;
			colorP[2] = 1;
		break;
		case '5':
			piso = 1.0;
			colorP[0] = 0.5;
			colorP[1] = 1;
			colorP[2] = 0.0;
		break;
		case '6':
			piso = 1.0;
			colorP[0] = 0.0;
			colorP[1] = 1;
			colorP[2] = 1;
		break;
		case '7':
			piso = 0.0;
		break;
	
		case ' ':
			ambiental = 1.0;	
			central   = 1.0;	
			relleno01 = 0.0;	
			relleno02 = 0.0;
			piso      = 0.0;

		break;

		default:
		break;
	}

	glutPostRedisplay();
}

void recursive_render (const aiScene *sc, const aiNode* nd)
{
	unsigned int i;
	unsigned int n = 0, t;
	aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiTransposeMatrix4(&m);
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

		for (t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch(face->mNumIndices) {
				case 1: face_mode = GL_POINTS; break;
				case 2: face_mode = GL_LINES; break;
				case 3: face_mode = GL_TRIANGLES; break;
				default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);

			for(i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				
				if(mesh->mColors[0] != NULL)
					glColor4fv((GLfloat*)&mesh->mColors[0][index]);
				
				if(mesh->mNormals != NULL) 
					glNormal3fv(&mesh->mNormals[index].x);
				
				if (mesh->HasTextureCoords(0)) 
					glTexCoord2f(mesh->mTextureCoords[0][index].x, 1-mesh->mTextureCoords[0][index].y);
				
				glVertex3fv(&mesh->mVertices[index].x);
			}

			glEnd();
		}

	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {		
		recursive_render(sc, nd->mChildren[n]);
	}

	glPopMatrix();
}

void render(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat zExtent, xExtent, xLocal, zLocal;
    int loopX, loopZ;

	glLoadIdentity ();                       
	gluLookAt (0.0, 37.0, 98.0, 0.0, 30.0, 0.0, 0.0, 1.0, 0.0);
	
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_LINE_SMOOTH );	

			
	glPushMatrix();

	if (shader) shader->begin();

	// Texturas
	shader->setTexture("stexflat", texflat,0);
	shader->setTexture("stexcentral", texcentral,1);
	shader->setTexture("stexrelleno01", texrelleno01,2);
	shader->setTexture("stexrelleno02", texrelleno02,3);
	shader->setTexture("stexpiso", texpiso,4);

	// Intensidad Luces
	shader->setUniform1f("ambientalInt",ambiental);
	shader->setUniform1f("centralInt",central);
	shader->setUniform1f("relleno01Int",relleno01);
	shader->setUniform1f("relleno02Int",relleno02);
	shader->setUniform1f("pisoInt",piso);

	// Color Luces
	shader->setUniform4f("ambientalColor",colorA[0],colorA[1],colorA[2],colorA[3]);
	shader->setUniform4f("centralColor",colorC[0],colorC[1],colorC[2],colorC[3]);
	shader->setUniform4f("relleno01Color",colorR01[0],colorR01[1],colorR01[2],colorR01[3]);
	shader->setUniform4f("relleno02Color",colorR02[0],colorR02[1],colorR02[2],colorR02[3]);
	shader->setUniform4f("pisoColor",colorP[0],colorP[1],colorP[2],colorP[3]);

	// Filtro Bilineal
	shader->setUniform1f("filtroBilineal",filtro);


	// Codigo para el mesh	
	glEnable(GL_NORMALIZE);
	glTranslatef(0.0, -2.0, 0.0);
	glRotatef(0.0, 0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, 1.0);
	if(scene_list == 0) {
	    scene_list = glGenLists(1);
	    glNewList(scene_list, GL_COMPILE);
            // now begin at the root node of the imported data and traverse
            // the scenegraph by multiplying subsequent local transforms
            // together on GL's matrix stack.
	    recursive_render(scene, scene->mRootNode);
	    glEndList();
	}
	glCallList(scene_list);
	
	glPopMatrix();
	
	
	/*
	glPushMatrix();
	glLoadIdentity();	
	glTranslatef(1.1, 0.5, -3.0);
	glutSolidSphere(0.2f,30,30);
	glPopMatrix();*/
	 

	if (shader) shader->end();
	
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glutSwapBuffers();
}

void animacion(int value) {
	
	glutTimerFunc(10,animacion,1);
    glutPostRedisplay();
	
}

void get_bounding_box_for_node (const aiNode* nd, 	aiVector3D* min, 	aiVector3D* max, 	aiMatrix4x4* trafo){
	aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

void get_bounding_box (aiVector3D* min, aiVector3D* max){
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
	
	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

int loadasset (const char* path){
	// we are taking one of the postprocessing presets to avoid
	// spelling out 20+ single postprocessing flags here.
	scene = aiImportFile(path,aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene) {
		get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
		return 0;
	}
	return 1;
}

int main (int argc, char** argv) {

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(960,540);

	glutCreateWindow("Test Opengl");


	// Codigo para cargar la geometria usando ASSIMP

	aiLogStream stream;
	// get a handle to the predefined STDOUT log stream and attach
	// it to the logging system. It remains active for all further
	// calls to aiImportFile(Ex) and aiApplyPostProcessing.
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);

	// ... same procedure, but this stream now writes the
	// log messages to assimp_log.txt
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&stream);

	// the model name can be specified on the command line. If none
	// is specified, we try to locate one of the more expressive test 
	// models from the repository (/models-nonbsd may be missing in 
	// some distributions so we need a fallback from /models!).
	
	
	loadasset( "escenario_proyecto01.obj");



	init ();

	glutReshapeFunc(changeViewport);
	glutDisplayFunc(render);
	glutKeyboardFunc (Keyboard);
	
	glutMainLoop();
	return 0;

}
