#include <memory>
#include "brigade.h"

//using namespace Craze;
//using namespace Craze::Graphics2;

PathTracer* pathtracer;
Scene* scene;

void initialize()
{
	pathtracer = new PathTracer(int2(1024, 640), true); //set bc res

	scene = new Scene();
	pathtracer->SetScene(scene);

	//load scene
	pathtracer->SetRaysPerPixel(8);

	scene->SetSkyColor(float3(.1f, .1f, .8f));

	//load model
	ImportResult result;
	scene->GetRoot()->Add("sponza\sponza.obj");

	//scene->GetCamera()->SetFOV(//get battlecraze FOV);
}

void update()
{
	//set camera
	//scene->GetCamera()->SetView(//get battlecraze FOV);
}

void render()
{
	pathtracer->RenderBegin();
	pathtracer->RenderEnd();

	Bitmap finalimage = pathtracer->GetFinalImage();
	finalimage.Save("screenshot.png"); //don't do this every frame crazypants!
}