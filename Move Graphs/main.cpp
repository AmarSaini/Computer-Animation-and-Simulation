
# include "my_viewer.h"
# include "move_graph.h"

# include <sig/gs_euler.h>
# include <sigogl/ws_run.h>

# include <iostream>

using namespace std;

int main ( int argc, char** argv )
{

	GsOutput o;
	o << "Starting\n";
 
	// 1. Load skeleton definition:
	KnSkeleton* sk = new KnSkeleton();
	if ( !sk->load ( "../data/hdman.s" ) ) gsout.fatal("Error loading skeleton!");
	KnJoint* rootj = sk->joint("Hips");
	if ( !rootj ) gsout.fatal("Could not find root joint!");

	// 2. Create move graph and load all needed motions:
	MoveGraph* mg = new MoveGraph;
	mg->init ( sk ); // this call will load motions, which need to be compatible with skeleton sk

	// 3. Create viewer and build scene:
	MyViewer* v = new MyViewer ( -1, -1, 800, 600, "Example Move Graph App", mg );
	bool addfloor = true;
	//v->cmd ( WsViewer::VCmdAxis );
	v->build_scene ( sk, addfloor );
	v->view_all ();
	v->show ();

	// 4. Enter main loop
	// The integer below triggers examples of how to do different things, study one at a time:
	// 1:just display character, 2:play one motion, 3:test move graph, 4:move graph with blending
	int mainloop=4;

	//=================================================================
	if ( mainloop==1 ) // just display the character:
	{	ws_run ();
	}

	//=================================================================
	if ( mainloop==2 ) // just play one motion continously:
	{	KnMotion* curm = mg->nodes[0]->motion;
		float mduration = curm->duration();
		double mt0 = gs_time(); // start time for 1st motion
		// gsout<<mt0<<gsnl; // use gsout to check/debug SIG variables

		while ( true )
		{	// Compute local time, which is from 0 to the duration of the motion:
			float localt = float(gs_time()-mt0);

			// When local time goes beyond the end we start again:
			if ( localt>mduration ) // past end of current motion
			{	mt0 += mduration;
				localt -= mduration;
			}

			// Now apply motion at the current local time, update scene, and draw:
			curm->apply ( localt );
			v->sk_scene()->update();
			v->message().setf ( "loct=%4.2f  dur=%4.2f", localt, curm->duration() );
			v->render();
			ws_check(-20); // this updates window and interface (otherwise application will be unresponsive)
		}
	}

	//=================================================================
	if ( mainloop==3 ) // Concatenate motions
	{
		GsVec pg, pl; // global and local root joint position
		GsQuat qg, ql; // global and local root joint orientation

		double mt0 = gs_time(); // start time
		int state = 0; // state (or current node) in the move graph
		MoveGraph::Node* curn = mg->nodes[state];
		KnMotion* curm = curn->motion;
		float curmdur = curm->duration();
		int futurestate = -1; // new state to switch to after current one ends
		int newstatenow = -1; // new state to switch to right now

		while ( true )
		{	float localt = float(gs_time()-mt0);

			// if past end of current motion, go to next motion:
			if ( localt>curmdur )
			{	mt0 += curmdur;
				localt -= curmdur;
				if ( futurestate<0 ) futurestate = mg->nodes[state]->nextnode;
				newstatenow = futurestate;
			}

			// check if there was a keyboard request to change state:
			if ( v->lastkey() )
			{	for ( int i=0; i<curn->trans.size(); i++ )
				{	if ( v->lastkey()==curn->trans[i].key )
					{	futurestate = curn->trans[i].nextnode;
						v->invalidate_lastkey();
						if ( curn->anytimetransitions ) newstatenow=futurestate; // change now
						mt0 += localt;
						localt = 0;
					}
				}
			}

			if ( newstatenow>=0 ) // there is a new state requested
			{	state = newstatenow;
				curn = mg->nodes[state];
				curm = curn->motion;
				curmdur = curm->duration();
				newstatenow = futurestate = -1;

				// update global pos:
				pg = rootj->pos()->value();
				pg.y=0; // do not accumulate vertical offset
				float rx, ry, rz;
				gs_angles ( gsYXZ, rootj->rot()->value(), rx, ry, rz );
				qg.set ( GsVec::j, ry ); // only accumulate rotations around Y
				// to check results with full rotation use: qg=rootj->rot()->value();
			}

			// For drifts
			qg.set(GsVec::j, v->getAngle());

			// Now apply motion data, update to global, and draw:
			curm->apply ( localt );
			pl = rootj->pos()->value ();
			ql = rootj->rot()->value ();
			rootj->pos()->value ( pg + qg.apply(pl) );
			rootj->rot()->value ( ql * qg );

			v->sk_scene()->update();
			v->message().setf ( "Keys: w,s,t  state=%d  loct=%4.2f  dur=%4.2f", state, localt, curm->duration() );
			v->render();

			ws_check(-20); // this updates window and interface (otherwise application will be unresponsive)
		}
	}

	//=================================================================
	if ( mainloop==4 ) // Concatenate motions with a simplistic blending
	{
		GsVec pg, pl; // global and local root joint position
		GsQuat qg, ql; // global and local root joint orientation
		GsQuat drift;

		double mt0 = gs_time(); // start time
		int state = 0; // state (or current node) in the move graph
		MoveGraph::Node* curn = mg->nodes[state];
		KnMotion* curm = curn->motion;
		float curmdur = curm->duration();
		int futurestate = -1; // new state to switch to after current one ends
		int newstatenow = -1; // new state to switch to right now

		KnPosture post1(sk), post2(sk), post3(sk); // postures to be used for interpolation blending
		post1.make_quat_channels(); // let's do all interpolations with quaternions
		post2.make_quat_channels();
		post3.make_quat_channels();
		float blwin = 0.2f; // blending window

		// Custom
		/*KnPosture postX(sk); // postures to be used for interpolation blending
		float postXTime = 0.1f;
		postX.make_quat_channels();
		mg->nodes[14]->motion->apply(postXTime);
		postX.get();*/

		while ( true )
		{	
			
			float localt = float(gs_time()-mt0);

			// if past end of current motion, go to next motion:
			if ( localt>curmdur )
			{	mt0 += curmdur;
				localt -= curmdur;
				if ( futurestate<0 ) futurestate = mg->nodes[state]->nextnode;
				newstatenow = futurestate;
			}

			// check if there was a keyboard request to change state:
			if ( v->lastkey() )
			{	for ( int i=0; i<curn->trans.size(); i++ )
				{	if ( v->lastkey()==curn->trans[i].key )
					{	futurestate = curn->trans[i].nextnode;
						v->invalidate_lastkey();
						if ( curn->anytimetransitions ) newstatenow=futurestate; // change now
						mt0 += localt;
						localt = 0;
					}
				}
			}

			// Custom rotation blending
			/*if (localt < blwin)
			{
				post2.get();
				float t = 1.0f - ((blwin - localt) / blwin); // t here varies from 0 to 1 in blending window
				//gsout<<t<<gsnl;
				interp(postX, post2, t, post3);
				post3.apply();
			}*/

			// Check if there is a new state requested:
			if ( newstatenow>=0 )
			{	state = newstatenow;
				curn = mg->nodes[state];
				curm = curn->motion;
				curmdur = curm->duration();
				newstatenow = futurestate = -1;
				post1.get(); // last posture of current motion will be used for concatenation blending
				// update global pos:
				pg = rootj->pos()->value();
				pg.y=0; // do not accumulate vertical offset
				float rx, ry, rz;
				gs_angles_yxz ( rootj->lmat(), rx, ry, rz, 'L' );
				qg.set ( GsVec::j, ry ); // only accumulate rotations around Y
			}


			// ---------- For drifts ----------
			drift.set(GsVec::j, v->getAngle());
			qg = qg * drift;
			v->setAngle(0.0f);

			// ---------- Update postX ----------
			/*mg->nodes[14]->motion->apply(postXTime);
			pl = rootj->pos()->value();
			ql = rootj->rot()->value();
			rootj->pos()->value(pg + qg.apply(pl));
			rootj->rot()->value(qg * ql);
			postX.get();*/

			// Now apply motion data, update to global, and draw:
			curm->apply ( localt );
			pl = rootj->pos()->value ();
			ql = rootj->rot()->value ();
			rootj->pos()->value ( pg + qg.apply(pl) );
			rootj->rot()->value ( qg * ql );

			// Do a very simple blending out from post 1 to improve concatenation:
			if ( v->blend_but() && localt<blwin )
			{	post2.get();
				float t = 1.0f-((blwin-localt)/blwin); // t here varies from 0 to 1 in blending window
				//gsout<<t<<gsnl;
				interp(post1, post2, t, post3);
				//interp(postX, post2, t, post3);
				post3.apply ();
			}

			// Set camera:
			if ( v->follow_but() )
			{	v->camera().center = rootj->pos()->value();
				v->camera().eye = v->camera().center + (GsVec(0,0,150));
				v->camera().center.y = 50.0f;
				v->camera().eye.y = 80.0f;
			}

			v->sk_scene()->update();
			v->message().setf ( "Keys: w,s,t  state=%d  loct=%4.2f  dur=%4.2f", state, localt, curm->duration() );
			v->render();

			ws_check(-20); // this updates window and interface (otherwise application will be unresponsive)
		}
	}

	return 1;
}
