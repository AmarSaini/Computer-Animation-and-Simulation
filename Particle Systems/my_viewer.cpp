
# include "my_viewer.h"

# include <sig/sn_model.h>
# include <sig/sn_material.h>
# include <sig/sn_transform.h>

# include <sigogl/ws_run.h>

# define CHKVAR(s) if(_vars->search(s)<0) { fltk::alert("Missing parameter [%s]!",s); exit(0); }

MyViewer::MyViewer ( int x, int y, int w, int h, const char *l ) : WsViewer ( x, y, w, h, l )
{
	_root = WsViewer::rootg();

	_lines = new SnLines;
	_world = new SnLines;
	_points = new SnPoints;
	_spheres = new SnGroup;

	view ( false, true, true ); // lines, world, spheres

	_root->add(_lines);
	_root->add(_points);
	_root->add(_world);
	_root->add(_spheres);
  
	_sphereradius = 1.0f;
}

MyViewer::~MyViewer ()
{
}

enum Events {	EvInit,
				EvViewSpheres, EvViewVelVecs,
				EvRun, EvWind, EvImpact,
				EvSCollision, EvVerlet, EvTScale, EvStiffness,
				EvView, EvExit
			};

void MyViewer::build_ui ()
{
	UiPanel *p, *sp, *paramsp;
	UiManager* uim = WsWindow::uim();
	bool detachparms = true;

	// Top Menu Bar:
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Init", EvInit ) );
	if (!detachparms) p->add ( new UiButton ( "Parameters", sp=new UiPanel() ) ); paramsp=sp;
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _vvel = new UiCheckButton ( "Velocities", EvView, false ) ); p->top()->separate();
		p->add ( _vworld = new UiCheckButton ( "World", EvView, true ) ); 
		p->add ( _vsphere = new UiCheckButton ( "Spheres", EvView, true ) ); 
	}
	p->add ( new UiButton ( "Exit", EvExit ) );

	if (!detachparms) p = paramsp; // build parameters panel now
	else p = uim->add_panel ( "Parameters", UiPanel::Vertical, UiPanel::Left, 0, 30 );

	p->add ( _run = new UiCheckButton ( "Run", EvRun ) );
	p->add ( _wind = new UiCheckButton ( "Vertical Wind", EvWind ) ); 
	p->add ( _scol = new UiCheckButton ( "Spring-Based Collision", EvSCollision ) );
	p->add ( _vcol = new UiCheckButton ( "Verlet Integration + Geometric Enforcements", EvVerlet ) );

	p->add ( new UiElement("Wind Force:") ); p->top()->separate();
	p->add ( _windslider = new UiSlider("", EvWind) );
	_windslider->range(-5.0f,5.0f);

	p->add(new UiElement("Impact Strength:"));
	p->add(_impactslider = new UiSlider("", EvImpact));
	_impactslider->range(1.0f, 3.0f); _impactslider->value(1.0f);

	p->add ( new UiElement("Geometric Enforcement Stiffness:") ); 
	p->add ( _crestslider = new UiSlider("", EvStiffness) );
	_crestslider->range(1.0f, 2.0f); _crestslider->value(1.0f);

	p->add(new UiElement("Time Scale:"));
	p->add(_tscaleslider = new UiSlider("", EvTScale));
	_tscaleslider->range(0.01f, 4.0f); _tscaleslider->value(1.0f);
}

void MyViewer::view ( bool vel, bool world, bool spheres )
{
	_lines->visible ( vel );
	_world->visible ( world );
	_spheres->visible ( spheres );
	_points->visible ( !spheres );
}

void MyViewer::build_scene ( ParticleSystem& psys, int nfaces )
{
	const GsArray<Particle>& p = psys.particles();
	int i, psize=p.size();
	_psys = &psys;
	_nfaces = nfaces;

	SnModel* sphere = new SnModel;
	sphere->model()->make_sphere ( GsPnt::null, _sphereradius, nfaces, true );
	SnModel* bigSphere = new SnModel;
	bigSphere->model()->make_sphere(GsPnt::null, _sphereradius*3, nfaces, true);

	SnGroup* g;
	SnTransform* t;
	SnMaterial* m;

	_spheres->init();
	_positions.size(0); // fast access to later update positions
	int n = int(sqrt(psize));
	for ( i=0; i<psize; i++ )
	{	_spheres->add ( g=new SnGroup );
		g->separator(true); // transformations inside group do not affect outside group
		g->add ( t=new SnTransform );
		g->add ( m=new SnMaterial );
		if (i < n*n) {
			g->add(sphere);
		}
		else {
			g->add(bigSphere);
		}
		_positions.push() = &(t->get());
		t->get().translation ( p[i].x );
		m->material().diffuse = GsColor::random();
    }

	_points->init();
	_points->point_size ( 3.0f );
	_points->color ( GsColor::blue );
	for ( i=0; i<psize; i++ )
	{	_points->push ( p[i].x );
	}

	_lines->init();
	_lines->color ( GsColor::gray );
	for ( i=0; i<psize; i++ )
	{
		_lines->push ( p[i].x, p[i].x+p[i].v );
	}

	_world->init();
	_world->color ( GsColor::red );
      
	int r=3;
	float fr = (float) r;
	float fr2 = fr+fr;
	const GsBox& w = psys.world();
	GsVec dx = GsVec::i*w.dx();
	GsVec dz = GsVec::k*w.dz();
	GsPnt a ( w.a.x*fr2, w.a.y, w.a.z*fr2 );
	for ( i=-r; i<=r; i++ )
	{	_world->push ( a, a+(dx*fr2) );
		a+=dz;
	}
	a.set ( w.a.x*fr2, w.a.y, w.a.z*fr2 );
	for ( i=-r; i<=r; i++ )
	{	_world->push ( a, a+(dz*fr2) );
		a+=dx;
	}
}

void MyViewer::update_scene ()
{
	const GsArray<Particle>& p = _psys->particles();
	int i, psize=p.size();

	if ( _spheres->visible() )
	{	for ( i=0; i<psize; i++ )
		{	_positions[i]->setrans ( p[i].x );
		}
	}
	else
    {	for ( i=0; i<psize; i++ )
		{	_points->P[i] = p[i].x;
			_points->touch();
		}
	}

	if ( _lines->visible() )
	{	_lines->init();
		for ( i=0; i<psize; i++ )
		{	_lines->push ( p[i].x, p[i].x+p[i].v );
		}
	}
}

void MyViewer::run ()
{
	if ( !_run->value() ) return; // turning sim off

	double tfac = 1.0; //ui_timeslider->value();
	double t0, tprior, tnow;

	t0 = tprior = gs_time()*tfac;
	while ( _run->value() )
	{
		// draw current state:
		update_scene ();
		ws_check();

		// check time scale:
		if ( tfac!=_tscaleslider->value() )
		{	t0 /= tfac;
			tprior /= tfac;
			tfac=_tscaleslider->value();
			t0 *= tfac;
			tprior *= tfac;
		}

		// compute next step:
		tnow = gs_time () * tfac;
		_psys->integration_step( float(tnow-tprior) );

		// display some info:
		message().setf ( "run: %5.2f s | fps: %5.2f", float(tnow-t0), 1/float(tnow-tprior) );
		tprior = tnow;
	}
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvInit:
		{	_psys->init ( _psys->last_cfg_file() );
			build_scene ( *_psys, _nfaces );
		} break;

		case EvWind:
		{	float windmag = _windslider->value();
			if ( !_wind->value() ) windmag=0;
			_psys->external_force ( GsVec( 0, windmag, 0 ) );
		} break;

		case EvImpact:
		{	_psys->set_impactStrength(_impactslider->value());
		} break;

		case EvSCollision:
		{	_psys->spring_collision_prevention ( _scol->value() );
		} break;

		case EvVerlet:
		{	_psys->select_integration_method ( _vcol->value() );
		} break;

		case EvStiffness:
		{	_psys->set_stiffness ( _crestslider->value() );
		} break;

		case EvView:
		{	view ( _vvel->value(), _vworld->value(), _vsphere->value() );
		} break;

		case EvRun: run(); break;

		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}

int MyViewer::handle_scene_event ( const GsEvent &e )
{
	if ( e.button1 )
	{	// nothing here for now
	}

	return WsViewer::handle_scene_event ( e );
}
