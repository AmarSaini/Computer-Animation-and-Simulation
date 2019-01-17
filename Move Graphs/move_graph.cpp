
# include "move_graph.h"

MoveGraph::MoveGraph ()
{
}

static KnMotion* load_motion ( KnSkeleton* sk, const char* filename )
{
	KnMotion* m = new KnMotion;
	if ( !m->load ( filename ) ) gsout.fatal("Error loading [%s]!",filename);
	m->connect ( sk );
	m->ref ();
	return m;
}

void MoveGraph::init ( KnSkeleton* sk )
{
	nodes.push()->motion = load_motion(sk, "../data/idle_stand.bvh"); // 0
	nodes.push()->motion = load_motion(sk, "../data/stand2left_step.bvh"); // 1
	nodes.push()->motion = load_motion(sk, "../data/stand2right_step.bvh"); // 2
	nodes.push()->motion = load_motion(sk, "../data/left_step.bvh"); // 3
	nodes.push()->motion = load_motion(sk, "../data/right_step.bvh"); // 4
	nodes.push()->motion = load_motion(sk, "../data/turn180.bvh"); // 5
	nodes.push()->motion = load_motion(sk, "../data/left_step2stand.bvh"); // 6
	nodes.push()->motion = load_motion(sk, "../data/right_step2stand.bvh"); // 7

	nodes.push()->motion = load_motion(sk, "../data/stand2run_leftstep.bvh"); // 8
	nodes.push()->motion = load_motion(sk, "../data/stand2run_rightstep.bvh"); // 9
	nodes.push()->motion = load_motion(sk, "../data/run_leftstep.bvh"); // 10
	nodes.push()->motion = load_motion(sk, "../data/run_rightstep.bvh"); // 11
	nodes.push()->motion = load_motion(sk, "../data/run2stand_leftstep.bvh"); // 12
	nodes.push()->motion = load_motion(sk, "../data/run2stand_rightstep.bvh"); // 13

	// Testing new bvh
	nodes.push()->motion = load_motion(sk, "../data/test.bvh"); // 14

	Node* n;
	Transition* t;

	// motion node 0 - idle_stand:
	n = nodes[0]; n->nextnode = 0;
	n->anytimetransitions = true;
	t = &n->trans.push(); t->key = 'w'; t->nextnode = 1;

	// motion node 1 - stand2left_step:
	n = nodes[1]; n->nextnode = 4;
	n->anytimetransitions = false;

	// motion node 3 - left_step:
	n = nodes[3]; n->nextnode = 4;
	n->anytimetransitions = false;

	// motion node 4 - right_step:
	n = nodes[4]; n->nextnode = 3;
	n->anytimetransitions = false;

}

void MoveGraph::sprintOn() {

	Node* n;
	Transition* t;

	// motion node 0 - idle_stand:
	n = nodes[0]; n->nextnode = 0;
	n->anytimetransitions = true;
	t = &n->trans.push(); t->key = 'w'; t->nextnode = 8;
	// ---
	t = &n->trans.push(); t->key = 's'; t->nextnode = 14;

	// motion node 1 - stand2left_step:
	n = nodes[1]; n->nextnode = 11;
	n->anytimetransitions = false;

	// motion node 3 - left_step:
	n = nodes[3]; n->nextnode = 11;
	n->anytimetransitions = false;

	// motion node 4 - right_step:
	n = nodes[4]; n->nextnode = 10;
	n->anytimetransitions = false;

	// motion node 8 - stand2run_leftstep:
	n = nodes[8]; n->nextnode = 11;
	n->anytimetransitions = false;

	// motion node 10 - run_leftstep:
	n = nodes[10]; n->nextnode = 11;
	n->anytimetransitions = false;

	// motion node 11 - run_rightstep:
	n = nodes[11]; n->nextnode = 10;
	n->anytimetransitions = false;

	// --- motion node 14 - test: ---
	/*n = nodes[14]; n->nextnode = 0;
	n->anytimetransitions = false;*/

}

void MoveGraph::sprintOff() {

	Node* n;
	Transition* t;

	// motion node 0 - idle_stand:
	n = nodes[0]; n->nextnode = 0;
	n->anytimetransitions = true;
	t = &n->trans.push(); t->key = 'w'; t->nextnode = 1;

	// motion node 1 - stand2left_step:
	n = nodes[1]; n->nextnode = 4;
	n->anytimetransitions = false;

	// motion node 3 - left_step:
	n = nodes[3]; n->nextnode = 4;
	n->anytimetransitions = false;

	// motion node 4 - right_step:
	n = nodes[4]; n->nextnode = 3;
	n->anytimetransitions = false;

	// motion node 8 - stand2run_leftstep:
	n = nodes[8]; n->nextnode = 3;
	n->anytimetransitions = false;

	// motion node 10 - run_leftstep:
	n = nodes[10]; n->nextnode = 3;
	n->anytimetransitions = false;

	// motion node 11 - run_rightstep:
	n = nodes[11]; n->nextnode = 4;
	n->anytimetransitions = false;

}
