
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

#include <iostream>

using namespace std;

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l, MoveGraph* mgraph ) : WsViewer(x,y,w,h,l)
{
	_blendbut=_followbut=0;
	_animating=false;
	_lastkey=0;
	build_ui ();

	// Custom
	angle = 0.0f;
	mg = mgraph;
	sprint = false;
	driftIncrement = 0.1f;

}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Options", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _blendbut=new UiCheckButton ( "Blend", EvBlend ) ); 
		p->add ( _followbut=new UiCheckButton ( "Follow", EvFollow ) ); 
	}
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();

	// Custom
	p = uim->add_panel("Parameters", UiPanel::Vertical, UiPanel::Left, 0, 30);
	p->add(new UiElement("Drift Increment per Key Press:")); p->top()->separate();
	p->add(_driftslider = new UiSlider("", EvAngle));
	_driftslider->range(0.05f, 0.2f); _driftslider->value(0.1f);
}

void MyViewer::build_scene ( KnSkeleton* sk, bool addfloor )
{
	if ( addfloor )
	{	// Let's add a ground floor:
		GsPrimitive floor;
		floor.box(400.0f,1.0f,400.0f); floor.center.y=-1.0f;
		floor.material.diffuse=GsColor::darkgray;
		SnModel* ground = new SnModel;
		ground->model()->make_primitive ( floor );
		rootg()->add ( ground );
	}

	// Create a skeleton scene node:
	_kns = new KnScene;
	_kns->connect(sk);
	_kns->set_visibility(0, 0, 1, 0, 0);
	rootg()->add (_kns);

	// Custom
	initObstacle();

}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	_lastkey = e.key;
	//gsout<<"Key pressed: "<<e.key<<gsnl;

	switch (e.key)
	{

	case GsEvent::KeyEsc:
		gs_exit();
		return 1;

	case 'a':
		angle += driftIncrement;
		drawSphere();
		return 1;

	case 'd':
		angle -= driftIncrement;
		drawSphere();
		return 1;

	case 'q':
		cout << "Toggling Sprint..." << endl;
		sprint = !sprint;
		if (sprint) {
			mg->sprintOn();
		}
		else {
			mg->sprintOff();
		}
		return 1;

	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvBlend: break;
		case EvFollow: break;
		case EvExit: gs_exit();
		case EvAngle:
		{	driftIncrement = _driftslider->value();
		} break;
	}
	return WsViewer::uievent(e);
}

// Custom

float MyViewer::getAngle() {
	return angle;
}

void MyViewer::setAngle(float a) {
	angle = a;
}

void MyViewer::drawSphere() {

	float x = _kns->skeleton()->joint("Hips")->pos()->value().x;
	float z = _kns->skeleton()->joint("Hips")->pos()->value().z;

	cout << x << " " << z << endl;

	GsPrimitive myPrim;
	myPrim.sphere(5);
	myPrim.material.diffuse = GsColor::red;

	SnModel* myModel = new SnModel;
	myModel->model()->make_primitive(myPrim);
	myModel->model()->translate(GsVec(x, 5, z));

	rootg()->add(myModel);

}

void MyViewer::initObstacle() {
	GsPrimitive floor;
	floor.box(20.0f, 20.0f, 20.0f); floor.center.y = 20.0f;
	floor.material.diffuse = GsColor::blue;
	SnModel* ground = new SnModel;
	ground->model()->make_primitive(floor);
	ground->model()->translate(GsVec(50, 0, 100));
	rootg()->add(ground);
}