#include "Scenario.h"
#ifdef __linux__ 
#define ZeroMemory(x,y) memset(x,0,y)
#define wcscpy_s(x,y,z) wcscpy(x,z)
#define wcscat_s(x,y,z) wcscat(x,z)
#endif

Scenario::Scenario(Camera *cam) {
    glm::vec3 translate;
	glm::vec3 scale;
    Model* model = new Model("models/Cube/Cube.obj", cam);
	translate = glm::vec3(0.0f, 0.0f, 3.0f);
	scale = glm::vec3(0.25f, 0.25f, 0.25f);	// it's a bit too big for our scene, so scale it down
	model->setScale(&scale);
	model->setTranslate(&translate);
	model->setNextTranslate(&translate);
	InitGraph(model);

}
Scenario::Scenario(Model *camIni) {
    InitGraph(camIni);
}
void Scenario::InitGraph(Model* main) {
	float matAmbient[] = { 1,1,1,1 };
	float matDiff[] = { 1,1,1,1 };
	angulo = 0;
	camara = main;
	//creamos el objeto skydome
	sky = new SkyDome(32, 32, 50, (WCHAR*)L"skydome/earth.jpg", main->cameraDetails);
	//creamos el terreno
	terreno = new Terreno((WCHAR*)L"skydome/mars.png", (WCHAR*)L"skydome/texterr2.jpg", 400, 400, main->cameraDetails);
	water = new Water((WCHAR*)L"textures/terreno.bmp", (WCHAR*)L"textures/water.bmp", 20, 20, camara->cameraDetails);
	glm::vec3 translate;
	glm::vec3 scale;
	glm::vec3 rotation;
	rotation = glm::vec3(1.0f, 0.0f, 0.0f); //rotation X
	water->setRotX(180);
	water->setNextRotX(180);
	translate = glm::vec3(-7.0f, terreno->Superficie(-7.0f, 4.0f)+0.5, 4.0f);
	water->setTranslate(&translate);
	// load models
	// -----------
	ourModel.emplace_back(main);

	//NAVE ALIEN
	Model* alien;
	alien = new Model("models/nave_alien/nave_alien.obj", main->cameraDetails);					//1
	translate = glm::vec3(2.0f, 20.0f, 10.0f);
	alien->setTranslate(&translate);
	alien->setNextTranslate(&translate);
	rotation = glm::vec3(1.0f, 0.0f, 0.0f); //rotation X
	alien->setNextRotX(-30); // 45� rotation
	ourModel.emplace_back(alien);

	// ESTRELLA
	Model* star = new Model("models/star/star.fbx", main->cameraDetails);
	translate = glm::vec3(0.0f, terreno->Superficie(0.0f, 50.0f), 50.0f);
	star->setNextTranslate(&translate);
	star->setTranslate(&translate);
	ourModel.emplace_back(star);
	ModelAttributes m;
	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x = 5;
	star = CollitionBox::GenerateAABB(m.translate, star->AABBsize, main->cameraDetails);		//2
	star->setTranslate(&m.translate);
	star->setNextTranslate(&m.translate);
	m.hitbox = star;
	star->getModelAttributes()->push_back(m);
	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x = 10;
	star = CollitionBox::GenerateAABB(m.translate, star->AABBsize, main->cameraDetails);
	star->setTranslate(&m.translate);
	star->setNextTranslate(&m.translate);
	m.hitbox = star; // Le decimos al ultimo ModelAttribute que tiene un hitbox asignado
	star->getModelAttributes()->push_back(m);

	// ALIEN
	//Model* alien;
	//alien = new Model("models/alien/hiphop(1).fbx", main->cameraDetails);
	//translate = glm::vec3(-10.0f, terreno->Superficie(15.0f, 2.0f) + 2, 60.0f);
	//scale = glm::vec3(0.04f, 0.04f, 0.04f);	// it's a bit too big for our scene, so scale it down
	//alien->setTranslate(&translate);
	//alien->setNextTranslate(&translate);
	//rotation = glm::vec3(0.0f, 1.0f, 0.0f); //rotation Y
	//alien->setNextRotY(180);
	//alien->setScale(&scale);
	//ourModel.emplace_back(alien);
	//model = new CollitionBox(60.0f, 15.0f, 10.0f, 10, 10, 10, main->cameraDetails);
	//scale = glm::vec3(1.0f, 1.0f, 1.0f);	// it's a bit too big for our scene, so scale it down
	//model->setNextTranslate(model->getTranslate());
	//model->setScale(&scale);
	//ourModel.emplace_back(model);

	// CARRITO
	Model* carrito;
	carrito = new Model("models/carrito/carro.obj", main->cameraDetails);
	translate = glm::vec3(30.0f, terreno->Superficie(30.0f, 15.0f) + 1, 30.0f);					//3
	scale = glm::vec3(0.3f, 0.3f, 0.3f);	// it's a bit too big for our scene, so scale it down
	carrito->setTranslate(&translate);
	carrito->setNextTranslate(&translate);
	rotation = glm::vec3(0.0f, 1.0f, 1.0f); //rotation Y
	carrito->setNextRotY(200);
	carrito->setNextRotX(-10);
	carrito->setScale(&scale);
	ourModel.emplace_back(carrito);



	Model* alienhip;
	alienhip = new Model("models/alien/hiphop(1).fbx", main->cameraDetails);
	translate = glm::vec3(0.0f, terreno->Superficie(0.0f, 60.0f), 60.0f);						//4
	scale = glm::vec3(0.0005f, 0.0005f, 0.0005f);	// it's a bit too big for our scene, so scale it down
	alienhip->setTranslate(&translate);
	alienhip->setNextTranslate(&translate);
	alienhip->setScale(&scale);
	alienhip->setNextRotY(90);
	ourModel.emplace_back(alienhip);
	try {
		std::vector<Animation> animations = Animation::loadAllAnimations("models/alien/hiphop(1).fbx", alienhip->GetBoneInfoMap(), alienhip->getBonesInfo(), alienhip->GetBoneCount());
		std::vector<Animation> animation = Animation::loadAllAnimations("models/alien/hiphop(1).fbx", alienhip->GetBoneInfoMap(), alienhip->getBonesInfo(), alienhip->GetBoneCount());
		std::move(animation.begin(), animation.end(), std::back_inserter(animations));
		for (Animation animation : animations)
			alienhip->setAnimator(Animator(animation));
		alienhip->setAnimation(1);
	}
	catch (...) {
		ERRORL("Could not load animation!", "ANIMACION");
	}


	Model* alienb;
	alienb = new Model("models/alien/breakdance.fbx", main->cameraDetails);						//5
	translate = glm::vec3(10.0f, terreno->Superficie(10.0f, 60.0f), 60.0f);
	scale = glm::vec3(0.0005f, 0.0005f, 0.0005f);	// it's a bit too big for our scene, so scale it down
	alienb->setTranslate(&translate);
	alienb->setNextTranslate(&translate);
	alienb->setScale(&scale);
	alienb->setNextRotY(90);
	ourModel.emplace_back(alienb);
	try {
		std::vector<Animation> animations = Animation::loadAllAnimations("models/alien/breakdance.fbx", alienb->GetBoneInfoMap(), alienb->getBonesInfo(), alienb->GetBoneCount());
		std::vector<Animation> animation = Animation::loadAllAnimations("models/alien/breakdance.fbx", alienb->GetBoneInfoMap(), alienb->getBonesInfo(), alienb->GetBoneCount());
		std::move(animation.begin(), animation.end(), std::back_inserter(animations));
		for (Animation animation : animations)
			alienb->setAnimator(Animator(animation));
		alienb->setAnimation(1);
	}
	catch (...) {
		ERRORL("Could not load animation!", "ANIMACION");
	}

	Model* alienb2;
	alienb2 = new Model("models/alien/Breakdance2.fbx", main->cameraDetails);
	translate = glm::vec3(20.0f, terreno->Superficie(20.0f, 60.0f) + 2, 60.0f);						//6
	scale = glm::vec3(0.0005f, 0.0005f, 0.0005f);	// it's a bit too big for our scene, so scale it down
	alienb2->setTranslate(&translate);
	alienb2->setNextTranslate(&translate);
	alienb2->setScale(&scale);
	alienb2->setNextRotY(90);
	ourModel.emplace_back(alienb2);
	try {
		std::vector<Animation> animations = Animation::loadAllAnimations("models/alien/breakdance2.fbx", alienb2->GetBoneInfoMap(), alienb2->getBonesInfo(), alienb2->GetBoneCount());
		std::vector<Animation> animation = Animation::loadAllAnimations("models/alien/breakdance2.fbx", alienb2->GetBoneInfoMap(), alienb2->getBonesInfo(), alienb2->GetBoneCount());
		std::move(animation.begin(), animation.end(), std::back_inserter(animations));
		for (Animation animation : animations)
			alienb2->setAnimator(Animator(animation));
		alienb2->setAnimation(1);
	}
	catch (...) {
		ERRORL("Could not load animation!", "ANIMACION");
	}
	/*
	Model* silly = new Model("models/Silly_Dancing/Silly_Dancing.fbx", main->cameraDetails);
	translate = glm::vec3(10.0f, terreno->Superficie(10.0f, 60.0f) , 60.0f);
	scale = glm::vec3(0.04f, 0.04f, 0.04f);	// it's a bit too big for our scene, so scale it down
	silly->setTranslate(&translate);
	silly->setNextTranslate(&translate);
	silly->setScale(&scale);
	silly->setNextRotY(180);
	ourModel.emplace_back(silly);
	try{
		std::vector<Animation> animations = Animation::loadAllAnimations("models/Silly_Dancing/Silly_Dancing.fbx", silly->GetBoneInfoMap(), silly->getBonesInfo(), silly->GetBoneCount());
		for (Animation animation : animations)
			silly->setAnimator(Animator(animation));
		silly->setAnimation(0);
	}catch(...){
		ERRORL("Could not load animation!", "ANIMACION");
	}

	m.setTranslate(&translate);
	m.setNextTranslate(&translate);
	m.translate.x += 10;
	m.setScale(&scale);
	m.setNextRotY(180);
	m.setRotY(180);
	model = CollitionBox::GenerateAABB(m.translate, silly->AABBsize, main->cameraDetails);
	model->setTranslate(&m.translate);
	model->setNextTranslate(&m.translate);
	model->setScale(&scale);
	model->setNextRotY(180);
	model->setRotY(180);
	m.hitbox = model; // Le decimos al ultimo ModelAttribute que tiene un hitbox asignado
	silly->getModelAttributes()->push_back(m);
	// Import model and clone with bones and animations
	model = new Model("models/Silly_Dancing/Silly_Dancing.fbx", main->cameraDetails);
	translate = glm::vec3(30.0f, terreno->Superficie(30.0f, 60.0f) , 60.0f);
	scale = glm::vec3(0.05, 0.05, 0.05);	// it's a bit too big for our scene, so scale it down
	model->name = "Silly_Dancing1";
	model->setTranslate(&translate);
	model->setNextTranslate(&translate);
	model->setScale(&scale);
	model->setNextRotY(180);
	ourModel.emplace_back(model);
	// Para clonar la animacion se eliminan los huesos del modelo actual y se copian los modelos y animators
	model->GetBoneInfoMap()->clear();
	model->getBonesInfo()->clear();
	*model->GetBoneInfoMap() = *silly->GetBoneInfoMap();
	*model->getBonesInfo() = *silly->getBonesInfo();
	model->setAnimator(silly->getAnimator());
	*/
	//	model = new Model("models/IronMan.obj", main->cameraDetails);
//	translate = glm::vec3(0.0f, 20.0f, 30.0f);
//	scale = glm::vec3(0.025f, 0.025f, 0.025f);	// it's a bit too big for our scene, so scale it down
//	model->setScale(&scale);
//	model->setTranslate(&translate);
//	ourModel.emplace_back(model);


	// NAVE ASTRONAUTA
	Model* nave;
	nave = new Model("models/nave_astronauta/nave_astro.obj", main->cameraDetails, false, false);
	translate = glm::vec3(60.0f, terreno->Superficie(60.0f, 20.0f) + 6, 20.0f);					//7
	scale = glm::vec3(3.0f, 3.0f, 3.0f);	// it's a bit too big for our scene, so scale it down
	nave->setTranslate(&translate);
	nave->setNextTranslate(&translate);
	nave->setScale(&scale);
	ourModel.emplace_back(nave);
	/*nave = new CollitionBox(60.0f, 15.0f, 10.0f, 10, 10, 10, main->cameraDetails);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);	// it's a bit too big for our scene, so scale it down
	nave->setNextTranslate(nave->getTranslate());
	nave->setScale(&scale);
	ourModel.emplace_back(nave);*/
	delete nave->getModelAttributes()->at(0).hitbox;
	nave->getModelAttributes()->at(0).hitbox = 0;

	// HITBOXES PARA LA NAVE (ESTRUCTURA NAVEGABLE)
	Model* suelo;
	suelo = new Model("models/hitbox/hitboxsuelo.obj", main->cameraDetails, false, false);
	translate = glm::vec3(60.0f, terreno->Superficie(60.0f, 20.0f) - 1, 20.0f);					//7
	scale = glm::vec3(3.0f, 3.0f, 3.0f);	// it's a bit too big for our scene, so scale it down
	suelo->setTranslate(&translate);
	suelo->setNextTranslate(&translate);
	suelo->setScale(&scale);
	ourModel.emplace_back(suelo);

	Model* pared1;
	pared1 = new Model("models/hitbox/hitboxpared1.obj", main->cameraDetails, false, false);
	translate = glm::vec3(60.0f, terreno->Superficie(60.0f, 20.0f) + 0, 20.0f);					//7
	scale = glm::vec3(3.0f, 3.0f, 3.0f);	// it's a bit too big for our scene, so scale it down
	pared1->setTranslate(&translate);
	pared1->setNextTranslate(&translate);
	pared1->setScale(&scale);
	ourModel.emplace_back(pared1);
	
	Model* pared2;
	pared2 = new Model("models/hitbox/hitboxpared2.obj", main->cameraDetails, false, false);
	translate = glm::vec3(60.0f, terreno->Superficie(60.0f, 20.0f) + 0, 20.0f);					//7
	scale = glm::vec3(3.0f, 3.0f, 3.0f);	// it's a bit too big for our scene, so scale it down
	pared2->setTranslate(&translate);
	pared2->setNextTranslate(&translate);
	pared2->setScale(&scale);
	ourModel.emplace_back(pared2);
	

	// METEOROOOOOOO(rip)
	Model* meteoro;
	meteoro = new Model("models/meteoro/meteoro.obj", main->cameraDetails, false, false);
	translate = glm::vec3(50.0f, terreno->Superficie(40.0f, 10.0f) + 2, 20.0f);						//8
	scale = glm::vec3(3.0f, 3.0f, 3.0f);	// it's a bit too big for our scene, so scale it down
	meteoro->setTranslate(&translate);
	meteoro->setNextTranslate(&translate);
	meteoro->setScale(&scale);
	ourModel.emplace_back(meteoro);


	// telescopio
	Model* telescopio;
	telescopio = new Model("models/telescopio/telescopio.obj", main->cameraDetails, false, false);
	translate = glm::vec3(-10.0f, terreno->Superficie(0.0f, 0.0f) + 0, 20.0f);						//9
	scale = glm::vec3(4.0f, 4.0f, 4.0f);	// it's a bit too big for our scene, so scale it down
	telescopio->setTranslate(&translate);
	telescopio->setNextTranslate(&translate);
	telescopio->setScale(&scale);
	ourModel.emplace_back(telescopio);


	// ROBOT
	Model* robot;
	robot = new Model("models/robot/robot.fbx", main->cameraDetails, false, false);
	translate = glm::vec3(50.0f, terreno->Superficie(50.0f, -17.0f) + 0, -17.0f);						//10
	scale = glm::vec3(10.0f, 10.0f, 10.0f);	// it's a bit too big for our scene, so scale it down
	rotation = glm::vec3(0.0f, 1.0f, 0.0f); //rotation Y
	robot->setNextRotY(180);
	robot->setTranslate(&translate);
	robot->setNextTranslate(&translate);
	robot->setScale(&scale);
	ourModel.emplace_back(robot);


	//BANDERA
	Model* bandera;
	bandera = new Model("models/bandera/flag2.obj", main->cameraDetails, false, false);
	translate = glm::vec3(20.0f, terreno->Superficie(5.0f, 10.0f) + 2, 10.0f);						//11
	scale = glm::vec3(0.5f, 0.5f, 0.5f);	// it's a bit too big for our scene, so scale it down
	bandera->setTranslate(&translate);
	bandera->setNextTranslate(&translate);
	rotation = glm::vec3(0.0f, 1.0f, 0.0f); //rotation Y
	bandera->getNextRotY(); // 45� rotation
	bandera->setScale(&scale);
	ourModel.emplace_back(bandera);


	// PLANTA ALIEN
	Model* planta;
	planta = new Model("models/planta/planta.obj", main->cameraDetails, false, false);
	translate = glm::vec3(25.0f, terreno->Superficie(18.0f, 11.0f) + 0, 0.0f);						//12
	scale = glm::vec3(5.0f, 5.0f, 5.0f);	// it's a bit too big for our scene, so scale it down
	planta->setTranslate(&translate);
	planta->setNextTranslate(&translate);
	planta->setScale(&scale);
	ourModel.emplace_back(planta);


	// LUNA
	Model* luna;
	luna = new Model("models/luna/luna.obj", main->cameraDetails, false, false);
	translate = glm::vec3(50.0f, terreno->Superficie(50.0f, 0.0f) + 60, 0.0f);						//13
	scale = glm::vec3(0.7f, 0.7f, 0.7f);	// it's a bit too big for our scene, so scale it down
	luna->setTranslate(&translate);
	luna->setNextTranslate(&translate);
	luna->setScale(&scale);
	ourModel.emplace_back(luna);


	// MONSTRUO
	Model* monstruo;
	monstruo = new Model("models/monstruo/monstruo.fbx", main->cameraDetails, false, false);
	translate = glm::vec3(35.0f, terreno->Superficie(18.0f, 11.0f) + 0, 0.0f);						//14
	scale = glm::vec3(0.5f, 0.5f, 0.5f);	// it's a bit too big for our scene, so scale it down
	monstruo->setTranslate(&translate);
	monstruo->setNextTranslate(&translate);
	monstruo->setScale(&scale);
	ourModel.emplace_back(monstruo);


	// SATURNO
	Model* saturno;
	saturno = new Model("models/saturno/saturno.obj", main->cameraDetails, false, false);
	translate = glm::vec3(-40.0f, terreno->Superficie(40.0f, 0.0f) + 50, 0.0f);						//15
	scale = glm::vec3(5.0f, 5.0f, 5.0f);	// it's a bit too big for our scene, so scale it down
	saturno->setTranslate(&translate);
	saturno->setNextTranslate(&translate);
	saturno->setScale(&scale);
	ourModel.emplace_back(saturno);


	// JUPITER
	Model* jupiter;
	jupiter = new Model("models/jupiter/jupiter.obj", main->cameraDetails, false, false);
	translate = glm::vec3(45.0f, terreno->Superficie(45.0f, 0.0f) + 50, 0.0f);						//16
	scale = glm::vec3(7.0f, 7.0f, 7.0f);	// it's a bit too big for our scene, so scale it down
	jupiter->setTranslate(&translate);
	jupiter->setNextTranslate(&translate);
	jupiter->setScale(&scale);
	ourModel.emplace_back(jupiter);

	// TIERRA
	Model* tierra;
	tierra = new Model("models/tierra/tierra.obj", main->cameraDetails, false, false);
	translate = glm::vec3(50.0f, terreno->Superficie(50.0f, 0.0f) + 50, 0.0f);						//17 + prota = 18 modelos
	scale = glm::vec3(4.0f, 4.0f, 4.0f);	// it's a bit too big for our scene, so scale it down
	tierra->setTranslate(&translate);
	tierra->setNextTranslate(&translate);
	tierra->setScale(&scale);
	ourModel.emplace_back(tierra);


	inicializaBillboards();
	std::wstring prueba(L"Coloca todas las banderas!");
	ourText.emplace_back(new Texto(prueba, 20, 0, 0, 50, 0, camara));
	std::wstring score(L"Banderas restantes:");
	ourText.emplace_back(new Texto(score, 20, 0, 0, 80, 0, camara));
	/*billBoard2D.emplace_back(new Billboard2D((WCHAR*)L"billboards/awesomeface.png", 6, 6, 100, 200, 0, camara->cameraDetails));
	scale = glm::vec3(100.0f, 100.0f, 0.0f);	// it's a bit too big for our scene, so scale it down
	billBoard2D.back()->setScale(&scale);*/
}

void Scenario::inicializaBillboards() {
	float ye = terreno->Superficie(0, 0) + 3;
	Billboard *arbol = new Billboard((WCHAR*)L"billboards/Arbol.png", 6, 6, 0, ye, 0, camara->cameraDetails);
	billBoard.emplace_back(arbol);
	ModelAttributes mAttr;
	glm::vec3 pos(5, ye, 0);
	mAttr.setTranslate(&pos);
	arbol->getModelAttributes()->push_back(mAttr);
	pos = glm::vec3(10, ye, 0);
	mAttr.setTranslate(&pos);
	arbol->getModelAttributes()->push_back(mAttr);
	pos = glm::vec3(-10, ye, 0);
	mAttr.setTranslate(&pos);
	arbol->getModelAttributes()->push_back(mAttr);

	ye = terreno->Superficie(-9, -15) + 4;
	billBoard.emplace_back(new Billboard((WCHAR*)L"billboards/Arbol3.png", 8, 8, -9, ye, -15, camara->cameraDetails));

	BillboardAnimation *billBoardAnimated = new BillboardAnimation();
	ye = terreno->Superficie(5, -5) + 3;
	for (int frameArbol = 1; frameArbol < 4; frameArbol++){
		wchar_t textura[50] = {L"billboards/Arbol"};
		if (frameArbol != 1){
			wchar_t convert[25];
			swprintf(convert, 25, L"%d", frameArbol);
			wcscat_s(textura, 50, convert);
		}
		wcscat_s(textura, 50, L".png");
		billBoardAnimated->pushFrame(new Billboard((WCHAR*)textura, 6, 6, 5, ye, -5, camara->cameraDetails));		
	}
	billBoardAnim.emplace_back(billBoardAnimated);
}

	//el metodo render toma el dispositivo sobre el cual va a dibujar
	//y hace su tarea ya conocida
Scene* Scenario::Render() {
	//borramos el biffer de color y el z para el control de profundidad a la 
	//hora del render a nivel pixel.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
//	glClearColor(255.0f, 255.0f, 255.0f, 255.0f);

	if (this->animacion > 10) { // Timer se ejecuta cada 1000/30 = 33.333 ms
		for (BillboardAnimation *b : billBoardAnim){
			b->nextAnimation();
		}
		this->animacion = 0;
	} else {
		animacion = animacion + (1 * gameTime.deltaTime/100);
	}
	// Decimos que dibuje la media esfera
	sky->Draw();
	// Ahora el terreno
	terreno->Draw();
	water->Draw();
	// Dibujamos cada billboard que este cargado en el arreglo de billboards.
	for (int i = 0; i < billBoard.size(); i++)
		billBoard[i]->Draw();
	for (int i = 0; i < billBoardAnim.size(); i++)
		billBoardAnim[i]->Draw();
	for (int i = 0; i < billBoard2D.size(); i++)
		billBoard2D[i]->Draw();
	// Dibujamos cada modelo que este cargado en nuestro arreglo de modelos
	for (int i = 0; i < ourModel.size(); i++) {
			ourModel[i]->Draw();
	}
	for (int i = 0; i < ourText.size(); i++) {
		ourText[i]->Draw();
	}
		// Le decimos a winapi que haga el update en la ventana
	return this;
}
	
std::vector<Model*> *Scenario::getLoadedModels() {
	return &ourModel;
}
std::vector<Billboard*> *Scenario::getLoadedBillboards() {
	return &billBoard;
}
std::vector<Billboard2D*> *Scenario::getLoadedBillboards2D(){
	return &billBoard2D;
}
std::vector<Texto*> *Scenario::getLoadedText(){
	return &ourText;
}
std::vector<BillboardAnimation*> *Scenario::getLoadedBillboardsAnimation(){
	return &billBoardAnim;
}

Model* Scenario::getMainModel() {
	return this->camara;
}
void Scenario::setMainModel(Model* mainModel){
	this->camara = mainModel;
}
float Scenario::getAngulo() {
	return this->angulo;
}
void Scenario::setAngulo(float angulo) {
	this->angulo = angulo;
}
SkyDome* Scenario::getSky() {
	return sky;
}
Terreno* Scenario::getTerreno() {
	return terreno;
}

Scenario::~Scenario() {
	if (this->sky != NULL) {
		delete this->sky;
		this->sky = NULL;
	}
	if (this->terreno != NULL) {
		delete this->terreno;
		this->terreno = NULL;
	}
	if (billBoard.size() > 0)
		for (int i = 0; i < billBoard.size(); i++)
			delete billBoard[i];
	if (billBoardAnim.size() > 0)
		for (int i = 0; i < billBoardAnim.size(); i++)
			delete billBoardAnim[i];
	if (billBoard2D.size() > 0)
		for (int i = 0; i < billBoard2D.size(); i++)
			delete billBoard2D[i];
	this->billBoard.clear();
	if (ourText.size() > 0)
		for (int i = 0; i < ourText.size(); i++)
			if (!(ourText[i]->name.compare("FPSCounter") || ourText[i]->name.compare("Coordenadas")))
				delete ourText[i];
	this->ourText.clear();
	if (ourModel.size() > 0)
		for (int i = 0; i < ourModel.size(); i++)
			if (ourModel[i] != camara)
			delete ourModel[i];
	this->ourModel.clear();
}
