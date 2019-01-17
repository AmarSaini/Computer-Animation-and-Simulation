# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_check_button.h>
# include <sigogl/ws_viewer.h>

# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_scene.h>
# include "move_graph.h"

// Custom
# include <sigogl/ui_slider.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvBlend, EvFollow, EvExit, EvAngle };
	KnScene* _kns;
	UiCheckButton* _blendbut;
	UiCheckButton* _followbut;
	bool _animating;
	int _lastkey;

	// Custom
	float angle;
	MoveGraph* mg;
	bool sprint;
	UiSlider* _driftslider;
	float driftIncrement;

   public:
	MyViewer ( int x, int y, int w, int h, const char* l, MoveGraph* mgraph );
	void build_scene ( KnSkeleton* sk, bool addfloor );
	KnScene* sk_scene() { return _kns; }
	int lastkey () { return _lastkey; }
	void invalidate_lastkey () { _lastkey=0; }
	bool blend_but () const { return _blendbut->value(); }
	bool follow_but () const { return _followbut->value(); }

	// Custom
	float getAngle();
	void setAngle(float a);
	void drawSphere();
	void initObstacle();

   protected:
	void build_ui ();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

